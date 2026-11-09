# Smart Assignment Scheduler

A study planner and assignment scheduling engine written in C++. The scheduler models a calendar year (365 days, 6 daily study slots of 30 minutes each, representing 3 hours per day capacity) and allocates study time for overlapping assignments while enforcing deadline and workload constraints.

## Overview

- Academic Context: Final course project for Programming and Data Structures (PDS), Semester 2 (Spring 2024), Plaksha University.
- Original Teammate Repository: [rishaand1706/PDSassignment](https://github.com/rishaand1706/PDSassignment)
- Teammates: Rishaan, Tanvi Sanghai, Rishit, Avishkar Rawal, and Prasham Sheth.
- Key Contributions: Core scheduling algorithms (slicefill, moveinsert, alternative window search), date-to-day conversion, slot management, transaction rollback, and modular C++ library design.

## Scheduling Engine

The engine allocates 30-minute time slots using three strategies:

1. Direct Fill (slicefill): Distributes workload evenly across available days between the assignment start date and deadline using round-robin allocation.
2. Constraint Shifting (moveinsert): When an assignment cannot fit into its window, the scheduler checks whether existing tasks have later deadlines. Non-urgent slots are shifted into future available days, freeing capacity for the urgent task. If relocation cannot satisfy the constraint, state changes roll back atomically.
3. Alternative Window Suggestion: When required hours exceed window capacity, the engine searches forward for the earliest feasible interval matching the assignment duration.

## Project Structure

```text
smart-assignment-scheduler/
|-- CMakeLists.txt
|-- Makefile
|-- LICENSE
|-- README.md
|-- include/
|   `-- scheduler.hpp
|-- src/
|   |-- main.cpp
|   `-- scheduler.cpp
|-- tests/
|   `-- test_scheduler.cpp
`-- examples/
    |-- demo.in
    `-- demo.out
```

## Build and Run

### Prerequisites

- C++ compiler with C++17 support (g++, clang++, or MSVC)
- CMake 3.12+ or GNU Make

### Using Make

```bash
make all
./smart_scheduler
```

Run test suite:

```bash
make test
```

### Using CMake

```bash
mkdir build && cd build
cmake ..
cmake --build .
ctest --output-on-failure
./smart_scheduler
```

### Running Demo Input

```bash
./smart_scheduler --script < examples/demo.in
```

## License

This project is licensed under the [MIT License](LICENSE).
