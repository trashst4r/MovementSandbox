# MovementSandbox Repo Workflow

## Build System Overview

This project uses CMake with a single out-of-source build directory:

- project root: `MovementSandbox/`
- build directory: `MovementSandbox/build/`

Executable output path is always:


MovementSandbox/build/Debug/MovementSandbox.exe


Source entrypoint:


src/main.cpp


---

## Canonical Workflow (ONLY WORKFLOW)

All build and run commands are executed **from inside the build directory**.

### Step 1 — Enter build directory

```powershell
cd C:\Users\dylan\Desktop\Content\1ABC\MovementSandbox\build
Step 2 — Build
cmake --build .
Step 3 — Run
.\Debug\MovementSandbox.exe
Absolute Rules

Always run build commands from:

MovementSandbox/build

Always use:

cmake --build .

Never run:

cmake --build build
Never assume execution path from project root.

The executable is always located at:

build/Debug/MovementSandbox.exe

The only valid run command (from build dir) is:

.\Debug\MovementSandbox.exe
Fresh Setup (Only if build folder is missing)
cd C:\Users\dylan\Desktop\Content\1ABC\MovementSandbox
Remove-Item build -Recurse -Force
mkdir build
cd build
cmake ..
cmake --build .
.\Debug\MovementSandbox.exe
PATH Requirement

CMake must be available on PATH.

Temporary fix:

$env:Path += ";C:\Program Files\CMake\bin"

Verify:

cmake --version
Codex Instructions

Follow this workflow exactly:

Always assume working directory is build/
Always build using cmake --build .
Always run using .\Debug\MovementSandbox.exe
Never construct paths using build/build
Never run build commands from project root
Never guess executable location
File Targeting Rules
Only edit: src/main.cpp
Do not edit or use root-level main.cpp if it exists
Do not create duplicate entrypoints
Summary

This repo has a single valid execution context:

cd build
cmake --build .
.\Debug\MovementSandbox.exe