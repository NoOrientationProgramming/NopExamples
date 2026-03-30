# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

FrapExamples demonstrates the **Fractal System Architecture (FSA)** — a C++11 design pattern where every system level shares the same self-similar `Processing` base class. Three examples of increasing complexity:

- `t01_tcp-echo-server` — supervised TCP echo server
- `t02_multithreading-mandelbrot` — multithreaded Mandelbrot renderer (optional Vulkan/AVX2)
- `t03_log-catching` — log aggregation across a process tree

**Dependencies** (git submodules in `deps/`):
- `SystemCore` — core framework (Processing, TCP, logging, debugging)
- `LibNaegCommon` — threading, file I/O, memory utilities
- `tclap_loc` — CLI argument parsing (optional; detected at build time)

## Build Commands

Each example is built independently. Both Meson and CMake produce identical outputs.

### Meson (preferred)
```bash
cd t01_tcp-echo-server          # or t02_multithreading-mandelbrot, t03_log-catching
meson setup build-native .
cd build-native
meson compile
./app
```

### CMake
```bash
cd t01_tcp-echo-server
cmake -B build .
cmake --build build
./build/app
```

### Prerequisites
- CMake ≥ 3.10 and/or Meson + Ninja
- C++11-capable compiler (GCC, Clang, or MSVC)
- On Windows: WinSock2 is linked automatically

### Optional features (auto-detected at configure time)
- **TCLAP**: present when `deps/tclap_loc/include/` exists → `APP_HAS_TCLAP=1`, enables `./app --help`
- **AVX2** (t02 only): detected via `-mavx2` probe → `APP_HAS_AVX2=1`
- **Vulkan** (t02 only): GPU compute path in `t02_multithreading-mandelbrot/vulkan/`

## Architecture

### The `Processing` base class

Every domain class inherits from `Processing` (defined in `deps/SystemCore/`). A process is a stateful, schedulable unit modelled as `y = p(x, t)` rather than a pure function:

```
Processing (abstract)
├── process()   — called repeatedly by the scheduler; drives logic
├── shutdown()  — cleanup when the process finishes
└── treeTick()  — recursively drives child processes
```

Processes form trees. A parent either drives children synchronously (`DrivenByParent`) or spawns them as threads (`DrivenByExternalDriver` / `DrivenByNewInternalDriver`). Memory lifetime is tied to process lifetime (RAII).

### Process tree per example

```
t01: Supervising → TcpListening → TcpTransfering (per connection)
                 → UserInteracting

t02: Supervising → MandelbrotCreating → MandelBlockFilling × N workers
                 → UserInteracting

t03: Supervising → LogCatching
                 → UserInteracting
```

`Supervising` is often the root; it owns and coordinates all children.

### Conventions across examples

- `env.h` — compile-time constants (ports, sizes, defaults) per example
- `Supervising.{cpp,h}` — top-level orchestrator
- `UserInteracting.{cpp,h}` — handles user-facing I/O
- Domain logic lives in additional `*Creating` / `*Filling` / `*Catching` classes

### Compiler discipline

All warnings are enabled and treated as errors (`-Werror -Wfatal-errors`). The full flag lists are in each example's `CMakeLists.txt` and `meson.build`. GCC and Clang have separate flag sets; MSVC uses `/W4 /WX`.

## Submodule Setup

If `deps/` is empty after cloning:
```bash
git submodule update --init --recursive
```
