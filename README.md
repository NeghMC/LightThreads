<!-- ABOUT THE PROJECT -->
## About The Project

This repository contains a simple multi-threading system written in C. It is intended for embedded systems containing MCUs with limited resources. That library simplifies implementing multi-tasking by allowing to divide program into threads (functions) which are scheduled in time and executed one after another.

The library does not meet Real-Time requirements as a result for resource usage reduction.

Features:
* Light - insignificant size comparing to the entire program size
* Simple - bases on logical and easy to understand concept
* Flexible - almost no need for configuration, short implementation time

### Key concept

Each task is a set of written by a programmer functions (called `threads` in this library), which are executed in the designed by the programmer order. At the end of the thread the programmer needs to specify which thread from that task is supposed to run next by calling scheduling functions. After each thread a scheduler (build in the library) selects the next thread to run - it might be the thread specified by the programmer, or a thread from another task. Each task has its context (`lt_context_t`) containing information needed by the scheduler to manage the order of execution. Threads can be scheduled to run immediately (`lt_schedule(...)`) or with some delay (`lt_delay(...)`).

### Limitations

The application is memory usage optimized, which means some limited functionality compared to real OSes:
* Application runs on a single stack - blocking functions will block the entire application.
* No memory management and safety checks.

<!-- GETTING STARTED -->
## Getting Started

Developer needs to implement functions: 
```
void lt_criticalEnter(void); // this function shall disable interrupts
void lt_criticalExit(void); // this function shall restore interrupts state
void lt_idleCallback(void); // this function gets called when no other threads are scheduled
```

For more details please search through the source files in `source` folder.

<!-- USAGE EXAMPLES -->
## Usage

Example implementation can be seen in `tests` folder. The most practical example resides in `tests/Arduino_Mega_test` folder.

<!-- ROADMAP -->
## Roadmap

- [x] Create first public version
- [ ] Add proper unit tests
- [ ] Add advanced test (either simulated or emulated)
- [ ] Optimize delay functionality
- [ ] Add context status (scheduled, blocked, ...)
- [ ] Add priorities
- [ ] Add mutexes

<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.