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
<img width="1165" height="592" alt="Screenshot 2026-02-24 140443" src="https://github.com/user-attachments/assets/2e5f95b6-2a87-4ec7-8b93-c1df1550fc23" />
