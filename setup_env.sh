#!/bin/bash
set -e

echo "==============================================="
echo "   Virtual Biomedical Sensor-Node Setup"
echo "   (Zephyr RTOS + Renode + Robot Framework)"
echo "==============================================="

# ─── Step 1: System packages ─────────────────────────────────────
echo ""
echo "[1/6] Installing system build tools (needs sudo password)..."
sudo apt-get update -q
sudo apt-get install --no-install-recommends -y \
  git cmake ninja-build gperf ccache dfu-util device-tree-compiler wget \
  python3-dev python3-setuptools python3-tk python3-wheel python3-venv \
  python3-full xz-utils file make gcc gcc-multilib g++-multilib \
  libsdl2-dev libmagic1 curl \
  libusb-1.0-0-dev pkg-config libudev-dev

# ─── Step 2: Python virtual environment ──────────────────────────
# A venv is an isolated Python sandbox that avoids the PEP 668
# "externally managed environment" restriction on Ubuntu 23+/24+
echo ""
echo "[2/6] Creating Python virtual environment at ~/zephyr-venv..."
python3 -m venv ~/zephyr-venv
source ~/zephyr-venv/bin/activate

# Persist venv activation for all future WSL sessions
grep -qxF 'source ~/zephyr-venv/bin/activate' ~/.bashrc || \
  echo 'source ~/zephyr-venv/bin/activate' >> ~/.bashrc

# ─── Step 3: Python tools (west + robot) ─────────────────────────
echo ""
echo "[3/6] Installing west (Zephyr project manager) and Robot Framework..."
pip install -q -U pip
pip install -q west robotframework

# ─── Step 4: Zephyr RTOS workspace ───────────────────────────────
echo ""
echo "[4/6] Initialising Zephyr workspace (~1.5 GB, this takes a while)..."
cd ~
if [ ! -d "zephyrproject" ]; then
    west init zephyrproject
    cd ~/zephyrproject
    west update
    # IMPORTANT: Install Python deps BEFORE west zephyr-export
    # west zephyr-export needs 'jsonschema' and other packages to run
    pip install -q -r ~/zephyrproject/zephyr/scripts/requirements.txt
    west zephyr-export
else
    echo "   zephyrproject already exists — running west update..."
    cd ~/zephyrproject
    west update
    pip install -q -r ~/zephyrproject/zephyr/scripts/requirements.txt
    west zephyr-export
fi

# ─── Step 5: Zephyr SDK (ARM cross-compiler) ─────────────────────
echo ""
echo "[5/6] Installing Zephyr SDK 0.16.5-1 (ARM cross-compiler)..."
cd ~
SDK_VER="0.16.5-1"
SDK_DIR="zephyr-sdk-${SDK_VER}"
if [ ! -d "$SDK_DIR" ]; then
    wget --show-progress -q \
      "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VER}/zephyr-sdk-${SDK_VER}_linux-x86_64.tar.xz"
    tar xf "zephyr-sdk-${SDK_VER}_linux-x86_64.tar.xz"
    cd "$SDK_DIR"
    # -t arm-zephyr-eabi installs only the ARM toolchain (saves ~3 GB vs -t all)
    ./setup.sh -t arm-zephyr-eabi -h -c
    cd ~
else
    echo "   SDK already present — skipping download."
fi

# ─── Step 6: Renode ──────────────────────────────────────────────
echo ""
echo "[6/6] Installing Renode emulation framework..."
cd ~
if ! command -v renode &> /dev/null; then
    RENODE_VER="1.15.0"
    wget --show-progress -q \
      "https://github.com/renode/renode/releases/download/v${RENODE_VER}/renode_${RENODE_VER}_amd64.deb"
    sudo apt-get install -y "./renode_${RENODE_VER}_amd64.deb"
    rm "renode_${RENODE_VER}_amd64.deb"
else
    echo "   Renode already installed — skipping."
fi

# ─── Done ─────────────────────────────────────────────────────────
echo ""
echo "================================================"
echo " Setup Complete!"
echo ""
echo " NEXT STEPS:"
echo "   1. Close this terminal"
echo "   2. Open a fresh Ubuntu WSL window"
echo "   3. Run these to verify everything works:"
echo "      west --version"
echo "      renode --version"
echo "      robot --version"
echo "================================================"
