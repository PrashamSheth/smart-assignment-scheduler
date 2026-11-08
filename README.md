# Smart Assignment Scheduler (C++17)

A robust, constraint-satisfaction study planner and assignment scheduling engine written in modern C++17. The scheduler models a student's calendar year (365 days, 6 daily study slots of 30 minutes each = 3 hours/day max default capacity) and dynamically allocates study time for multiple overlapping assignments while enforcing strict deadline and workload constraints.

---

## 1. Context, Authorship & Attribution

- **Academic Context:** Final Course Project for **Programming and Data Structures (PDS)**, Semester 2 (Spring 2024), Plaksha University.
- **Original Teammate Repository:** [`rishaand1706/PDSassignment`](https://github.com/rishaand1706/PDSassignment)
- **Original Coursework Team:** Rishaan, Tanvi Sanghai, Rishit, Avishkar Rawal, and Prasham Sheth.
- **Prasham's Core Contributions:**
  - Designed and authored the core scheduling algorithms: round-robin distribution (`slicefill`), deadline-driven constraint shifting (`moveinsert`), and alternative feasibility window search.
  - Implemented calendar day-to-index mapping and dynamic timetable slot representation.
- **Project Timeline:**
  - *Original Coursework & Development:* May 2024 (Semester 2).
  - *Reconstruction, Modularization & Testing:* November 2026.

---

## 2. Technical Improvements in This Repository

The original coursework implementation (`PDSassignment`) was an exploratory team repository with tightly coupled files, extensive `friend` declarations violating encapsulation, unchecked input pointers, and compiled binaries in git. This reconstructed version delivers:

1. **Clean Object-Oriented Architecture:** Refactored into a clear modular library (`include/scheduler.hpp`, `src/scheduler.cpp`, `src/main.cpp`) with encapsulated `Date`, `Assignment`, `DaySchedule`, and `Timetable` classes.
2. **Deterministic Constraint-Solving Engine:**
   - **`slicefill` ($O(D \cdot S)$):** Evenly distributes study workload across available days within the valid window $[start, deadline]$ using round-robin slot allocation.
   - **`moveinsert` ($O(A \cdot D)$):** If a window is congested, the engine identifies assignments with *later* deadlines, shifts their non-urgent slots into their extended future availability window, and accommodates the urgent task.
   - **`findAlternativeWindow`:** If capacity cannot be cleared, the engine searches forward for the earliest feasible window matching the assignment's duration.
   - **Overfill & Rollback Guarantee:** Atomic transaction semantics ensure that if a schedule strategy cannot be completed, the state rolls back to prevent schedule corruption.
3. **Automated Unit Test Suite:** Includes comprehensive test assertions (`tests/test_scheduler.cpp`) covering date bounds, round-robin fairness, constraint shifting, and task deletion.
4. **Standard Build Systems:** Full support for both `CMake` (`CMakeLists.txt`) and `Makefile`.
5. **Interactive & Scripted CLI:** Supports both interactive terminal navigation and automated headless replay via `--script` or `--file`.

---

## 3. Algorithmic Overview

```
New Assignment (Start, Deadline, Hours)
                 │
                 ▼
       Can window fit slots? ──[YES]──► slicefill() (Round-Robin allocation)
                 │
                [NO]
                 ▼
    Can later tasks be shifted? ──[YES]──► moveinsert() (Relocate later tasks)
                 │
                [NO]
                 ▼
  Search Alternative Window ──► Prompt User: (1) Accept Alt, (2) Skip, (3) Force Overfill
```

---

## 4. Building and Running

### Prerequisites
- C++17 compatible compiler (`g++`, `clang++`, or MSVC)
- CMake 3.12+ (optional, or use `make`)

### Using Make
```bash
# Build the CLI application and test suite
make all

# Run the automated unit tests
make test

# Launch the interactive CLI
./smart_scheduler
```

### Using CMake
```bash
mkdir build && cd build
cmake ..
cmake --build .
ctest --output-on-failure
./smart_scheduler
```

### Scripted / Demo Replay
You can test the scheduling engine with pre-recorded input scripts:
```bash
./smart_scheduler --script < examples/demo.in
```

---

## 5. Directory Structure

```text
smart-assignment-scheduler/
├── CMakeLists.txt          # Modern CMake configuration
├── Makefile                # Fast make-based build
├── LICENSE                 # MIT License
├── README.md               # Architecture and documentation
├── include/
│   └── scheduler.hpp       # Core class declarations and interfaces
├── src/
│   ├── scheduler.cpp       # Scheduling algorithms and calendar logic
│   └── main.cpp            # Interactive and scripted CLI runner
├── tests/
│   └── test_scheduler.cpp  # Automated unit test suite
└── examples/
    ├── demo.in             # Sample scenario input script
    └── demo.out            # Expected sample output
```
