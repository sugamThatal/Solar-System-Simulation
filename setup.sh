#!/bin/bash
mkdir -p textures
cd textures
BASE="https://www.solarsystemscope.com/textures/download"
download() { echo "Downloading $1..."; curl -L --silent "$BASE/$2" -o "$1" && echo "[OK] $1" || echo "[WARN] $1 failed"; }
download "sun.jpg"         "2k_sun.jpg"
download "mercury.jpg"     "2k_mercury.jpg"
download "venus.jpg"       "2k_venus_atmosphere.jpg"
download "earth.jpg"       "2k_earth_daymap.jpg"
download "moon.jpg"        "2k_moon.jpg"
download "mars.jpg"        "2k_mars.jpg"
download "jupiter.jpg"     "2k_jupiter.jpg"
download "saturn.jpg"      "2k_saturn.jpg"
download "saturn_ring.png" "2k_saturn_ring_alpha.png"
download "uranus.jpg"      "2k_uranus.jpg"
download "neptune.jpg"     "2k_neptune.jpg"
download "stars.jpg"       "2k_stars_milky_way.jpg"
cd ..
echo "Done! Now run: make && ./SolarSystem"
