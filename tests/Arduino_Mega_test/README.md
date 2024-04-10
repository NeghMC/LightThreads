### About

That is a simple Arduino project presenting example usage of the LightThreads library. Program can be run on any Arduino.
Program implements two tasks: one controls blinking build-in led, the other one echoes serial port.

### How to run

1. Add source files by selecting `Sketch->Add File...` and selecting all files in the `source` folder (root directory) separately.
2. Install `TimerOne` library by selecting `Tools -> Manage Libraries...`, searching for the library, and installing version 1.1.1.
3. Connect Arduino.
4. Flash program.
5. Start testing by typing text in `Tools->Serial Monitor`.

### Usage

Observe the build-in led, it should blink about once per second. Also try to type some text on the serial monitor, the text should be displayed. If thats the case the example is working correctly.
