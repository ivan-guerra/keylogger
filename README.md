# keylogger

https://github.com/user-attachments/assets/fe342cd6-9c76-4a30-b501-5c401fab5b79

`keylogger` is a cross platform keylogging application that can capture and
record global keypress events. Keypress events can be logged to a file on the
victim PC or transmitted over the network to a remote server using UDP.

### Program Usage

Below is the program usage message:

```text
A cross platform keylogger.

Usage: keylogger [OPTIONS] <COMMAND>

Commands:
  net   log keystrokes to a remote server
  file  log keystrokes to a file
  help  Print this message or the help of the given subcommand(s)

Options:
  -n, --keystroke-threshold <KEYSTROKE_THRESHOLD>
          max number of keystrokes buffered before recording to the target [default: 8]
  -h, --help
          Print help
  -V, --version
          Print version
```

The output of a `keylogger` run is a file or network data containing newline
seperated [rdev][1] plaintext key codes. The key codes must be postprocessed and
played back to decipher exactly what was typed. That said, a quick scan of the
keycodes often reveals information of interest.

[1]: https://docs.rs/rdev/latest/rdev/enum.Key.html
