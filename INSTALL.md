# Installing OpenAmbit on Debian / Ubuntu

## Option A — Build a .deb package (recommended)

This is the cleanest approach. It produces installable `.deb` packages with
proper dependency tracking and automatic udev rule setup.

### 1. Install build dependencies

```bash
sudo apt-get install \
    debhelper cmake \
    qtbase5-dev qt5-qmake qttools5-dev qttools5-dev-tools \
    qtwebengine5-dev libqt5webchannel5-dev \
    libhidapi-dev libudev-dev \
    python3 python3-requests
```

### 2. Build the packages

```bash
cd ~/dev/openambit2
bash build-deb.sh
```

This produces five files in the **parent directory** (`~/dev/`):

| File | Contents |
|---|---|
| `openambit_0.5-1_amd64.deb` | GUI application + CLI tools |
| `libambit0_0.5-1_amd64.deb` | Shared library (required by the GUI) |
| `libambit-dev_0.5-1_amd64.deb` | Headers for developers |
| `openambit-dbgsym_…deb` | Debug symbols for the GUI |
| `libambit0-dbgsym_…deb` | Debug symbols for the library |

### 3. Install

```bash
cd ~/dev
sudo dpkg -i libambit0_0.5-1_amd64.deb openambit_0.5-1_amd64.deb
sudo apt-get install -f          # resolve any missing runtime deps
```

The postinst script runs `udevadm control --reload-rules && udevadm trigger`
automatically, so udev access to the watch is set up immediately.

### 4. Run

```bash
openambit
```

Plug in your Suunto Ambit via USB — the watch is detected automatically.

---

## Option B — Build and run directly (no install)

Useful for development or testing without touching your system.

### 1. Install build dependencies

```bash
sudo apt-get install \
    cmake build-essential \
    qtbase5-dev qt5-qmake qttools5-dev qttools5-dev-tools \
    qtwebengine5-dev libqt5webchannel5-dev \
    libhidapi-dev libudev-dev \
    python3 python3-requests
```

### 2. Configure and build

```bash
cd ~/dev/openambit2
mkdir build && cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_EXTRAS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_INSTALL_LIBDIR=lib/x86_64-linux-gnu
make -j$(nproc) openambit
```

### 3. Set up udev (once, needs sudo)

```bash
sudo cp src/libambit/libambit.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
```

### 4. Run from the build directory

```bash
cd ~/dev/openambit2/build/src/openambit
LD_LIBRARY_PATH=../movescount:../libambit ./openambit
```

---

## GPS orbit updates

On first launch OpenAmbit automatically downloads fresh GPS orbit data using
the bundled `tools/rinex2ubx_ambit.py` script (IGS broadcast ephemeris,
no account required). You need **Python 3** and the `requests` library:

```bash
sudo apt-get install python3 python3-requests
```

To trigger a manual download at any time: **Settings → GPS Orbit → Download now**.

---

## Uninstall

```bash
sudo dpkg -r openambit libambit0
```

User data (exercise logs, GeoJSON exports, orbit cache, sport mode config)
stays in `~/.openambit/` and is not removed by the package.  
Delete it manually if you want a clean slate:

```bash
rm -rf ~/.openambit
```

---

## Troubleshooting

**Watch not detected**  
Make sure you are in the `plugdev` group:
```bash
sudo usermod -aG plugdev $USER   # then log out and back in
```

**Map shows "map.html not found"**  
The file is installed to `/usr/share/openambit/map.html` by the `.deb`.  
For a manual build it sits next to the binary; check that `map.html` is in the
same directory as the `openambit` executable or one level up in `share/openambit/`.

**WebEngine / OSM map blank**  
Requires `qtwebengine5-dev` at build time and `libqt5webenginewidgets5` at runtime.
Both are pulled in automatically when using the `.deb` packages.

**Python orbit script fails**  
```bash
python3 ~/dev/openambit2/tools/rinex2ubx_ambit.py
```
Check that `python3-requests` is installed and that you have internet access.
The script writes its output to `~/.openambit/orbit_cache.uo`.
