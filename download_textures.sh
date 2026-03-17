#!/bin/bash
# ─── download_textures.sh ─────────────────────────────────────────────────────
# Downloads free NASA/public-domain planet textures.
# Run this ONCE before building the project.
#
# Usage:  chmod +x download_textures.sh && ./download_textures.sh

set -e
mkdir -p textures
cd textures

BASE="https://www.solarsystemscope.com/textures/download"

echo "Downloading planet textures (this may take a minute)..."

files=(
    "2k_sun.jpg|sun.jpg"
    "2k_mercury.jpg|mercury.jpg"
    "2k_venus_surface.jpg|venus.jpg"
    "2k_earth_daymap.jpg|earth.jpg"
    "2k_mars.jpg|mars.jpg"
    "2k_jupiter.jpg|jupiter.jpg"
    "2k_saturn.jpg|saturn.jpg"
    "2k_uranus.jpg|uranus.jpg"
    "2k_neptune.jpg|neptune.jpg"
    "2k_moon.jpg|moon.jpg"
)

for entry in "${files[@]}"; do
    src="${entry%%|*}"
    dst="${entry##*|}"
    url="$BASE/$src"
    if [ -f "$dst" ]; then
        echo "  [skip] $dst already exists"
    else
        echo "  Downloading $dst ..."
        curl -L -o "$dst" "$url" --silent --show-error || \
            echo "  [WARN] Failed to download $dst — you can add it manually"
    fi
done

# Saturn ring — using a public-domain image from NASA
RING_URL="https://upload.wikimedia.org/wikipedia/commons/thumb/b/b5/Solarsystemscope_texture_8k_saturn_ring_alpha.png/1024px-Solarsystemscope_texture_8k_saturn_ring_alpha.png"
if [ ! -f "saturn_ring.png" ]; then
    echo "  Downloading saturn_ring.png ..."
    curl -L -o saturn_ring.png "$RING_URL" --silent --show-error || \
        echo "  [WARN] Failed to download saturn_ring.png"
fi

echo ""
echo "✅ Textures ready! Now run:  make && ./solar_system"
