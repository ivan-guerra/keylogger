# keylogger

`keylogger` is a Windows and Linux keylogging application that can capture and
record global keypress events. Keypress events can be logged to a file on the
victim PC or transmitted over the network to a remote server using UDP.

### Building

To build the project locally, you will need the following libraries and tools
installed:

* CMake3.27+
* C++ compiler supporting C++20 features
* [Doxygen][1]
* Visual Studio 2022 (Windows Only)
* X11 Developer Libs (Linux Only)

> **Note**
> Older versions of Visual Studio may work as well. I only tested with VS2022
> because that is what I had available. To try out an older version, update the
> generator arg in the first `cmake` command located in the
> [`scripts/windows/build.bat`](scripts/windows/build.bat) script.

To build the project on Windows/Linux, change directory to the
`scripts/[windows|linux]` directory and run `build.[bat|sh]`.

After a successful build, you will find the binary installed to `keylogger/bin/`.

### Program Usage

`keylogger` must be configured prior to compiling. Application configurations
can be found in [`src/keylogger/keylogger.cpp`](src/keylogger/keylogger.cpp).
From within the source, one can set the recording mode, one of either text or
network, as well as additional configurations associated with the chosen mode.
This form of configuration while cumbersome was intentionally chosen to make the
keylogger application easier to deploy under certain circumstances (e.g., when
conducting a buffer overflow attack).

To run the keylogger, run the executable: `./keylogger` on Linux and
`keylogger.exe` on Windows. 

To terminate the keylogger, type "klexit" on the victim PC or simply kill the
process.

### Running the Tests

`keylogger` has been unit tested using the GoogleTest framework in conjunction
with `ctest`. To run the tests, build the project and change directory to
`keylogger/build/`. Run `ctest` to execute all unit tests.

### Doxygen Docs

This project is documented using Doxygen. Doxygen docs are built automatically
by both the Windows and Linux build script. Docs can be found under
`keylogger/docs/keylogger/`.

[1]: https://www.doxygen.nl/
