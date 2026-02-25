# KPI-DSP-lab 1  

Digital Signal Processing laboratory assignments for KPI. This project implements Fast Fourier Transform (FFT), windowing functions, and spectral visualization for audio signals.

## System Requirements

The project is designed for **Linux** environments.

### Prerequisites
* **CMake** (3.10+)
* **Python 3** (with `venv` support)
* **C/C++ Compiler** (GCC or Clang)



## Installation and Setup

Run the setup script once to initialize the Python virtual environment and install visualization dependencies:

```bash
./scripts/setup.sh
```

## Usage
### Clean
To clean build files, run: 

```Bash
./scripts/clean.sh
```

### Build
To build the project, run:

```Bash
./scripts/build.sh
```

### Execution
To analyze an audio file, provide the path to a .wav file as an argument:

```Bash
./scripts/run.sh path/to/audio.wav
```