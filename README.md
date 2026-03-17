# 🌍 Solar System Simulation
### Computer Graphics & Visualization — BTech Project
**Language:** C++ | **API:** OpenGL 2.1 + GLFW + GLM | **Platform:** macOS

---

## Features
- All 8 planets orbiting the Sun with correct relative speeds
- Axial tilt for every planet (Saturn: 26.7°, Uranus: 97.8°, etc.)
- Self-rotation on each planet
- Earth's Moon with its own orbit
- Saturn's translucent ring
- Phong lighting from the Sun (point light at origin)
- Texture mapping (real NASA/Solar System Scope textures)
- Starfield skybox background
- Free-fly camera (first-person mouse + keyboard)
- Simulation speed control, pause/resume

---

## Quick Start

### Step 1 — Install dependencies
```bash
# Install Homebrew (if you don't have it)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install GLFW and GLM
brew install glfw glm
```

### Step 2 — Download textures + stb_image
```bash
chmod +x setup.sh
./setup.sh
```

### Step 3 — Build & Run
```bash
make
./SolarSystem
```

---

## Controls

| Key / Input | Action |
|---|---|
| `W / S` | Fly forward / backward |
| `A / D` | Strafe left / right |
| `Q / E` | Fly up / down |
| `Mouse` | Look around |
| `+` | Speed up simulation |
| `-` | Slow down simulation |
| `P` | Pause / Resume |
| `R` | Reset camera to default |
| `ESC` | Quit |

---

## Project Structure
```
SolarSystem/
├── main.cpp          ← All source code (single file)
├── stb_image.h       ← Image loading (auto-downloaded)
├── Makefile          ← Build system
├── setup.sh          ← One-time setup script
├── README.md
└── textures/         ← Planet texture images (auto-downloaded)
    ├── sun.jpg
    ├── earth.jpg
    ├── moon.jpg
    ├── mars.jpg
    ├── ...
```

---

## CG Concepts Demonstrated

| Concept | Implementation |
|---|---|
| 3D Transformations | `glRotate`, `glTranslate` for orbits & rotation |
| Phong Lighting | `GL_LIGHT0` at sun origin, ambient + diffuse + specular |
| Texture Mapping | UV sphere with `glTexCoord2f`, mipmapped textures |
| Camera | `glm::lookAt` with free-fly mouse + keyboard |
| Animation | Time-based simulation with speed multiplier |
| Transparency | `GL_BLEND` for Saturn's rings |
| Backface Culling | `glFrontFace(GL_CW)` for inside-out starfield sphere |

---

## If Textures Don't Download
The simulation works without textures — each planet will be rendered in a
realistic fallback colour. You can also manually place any `.jpg` images 
(named as above) in the `textures/` folder.

Free textures: https://www.solarsystemscope.com/textures/

---

## Troubleshooting

**`glfw/glfw3.h` not found**
```bash
brew install glfw
# For Intel Mac: change BREW_PREFIX in Makefile to /usr/local
```

**`glm/glm.hpp` not found**
```bash
brew install glm
```

**OpenGL deprecation warnings**
Already suppressed with `#define GL_SILENCE_DEPRECATION` — safe to ignore.
