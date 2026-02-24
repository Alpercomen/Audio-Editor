# Sona (C++ / Qt) — DAW-Style Multi-Track Timeline

A lightweight **DAW-inspired audio editor** built in **C++ + Qt** featuring a **multi-track timeline**, **waveform clip rendering**, and a **channel-aware audio pipeline** designed to scale beyond stereo.
---

## Highlights

- **DAW-style timeline** with horizontal time axis and vertical track lanes
- **Multi-track clip system**
  - Place audio as “clips” on tracks
  - **Drag clips between tracks** (overlap is allowed)
  - Independent clip positioning on the timeline
- **Waveform visualization**
  - Renders waveform previews inside each clip
  - Distinct colors per clip/track for readability
- **Track controls (mixer strip on the left)**
  - **Mute**
  - **Mono** (downmix behavior / mono monitoring)
  - Designed to support **future panning and per-track gain**
- **Channel-aware, future-proof audio architecture**
  - Not hard-coded to stereo; supports higher channel counts later
- **Performance-oriented design**
  - Intended for real-time playback and responsive UI
  - Separation between UI and audio processing responsibilities
---

## In Editor View
<img width="1194" height="727" alt="image" src="https://github.com/user-attachments/assets/13c0ef27-26e4-4f55-8942-0e7aa0928ba0" />

---

## Getting Started

1. **Clone the repo**
   ```bash
   git clone https://github.com/Alpercomen/Sona.git
   cd Sona

2. **Generate project files using CMake**
   ```bash
   cmake -S . -B Build
   cmake --build Build

3. **Run the Editor**
   - Navigate to the build directory and launch the Sona executable.
