#!/bin/bash
# Build Debian packages for openambit + libambit0 + libambit-dev.
# Run from the repository root. Produces .deb files in the parent directory.
#
# Usage:
#   ./build-deb.sh             # build binary packages only (faster)
#   ./build-deb.sh --full      # full source + binary build (requires gpg key)

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
PACKAGE_VERSION="$(grep '^openambit (' "$REPO_ROOT/debian/changelog" | head -1 | \
    sed 's/openambit (\(.*\)) .*/\1/')"

echo "=== OpenAmbit Debian packager ==="
echo "Version : $PACKAGE_VERSION"
echo "Repo    : $REPO_ROOT"
echo ""

# Check build dependencies
for dep in debhelper cmake qtbase5-dev qt5-qmake qttools5-dev qttools5-dev-tools libhidapi-dev libudev-dev dpkg-dev; do
    if ! dpkg -l "$dep" >/dev/null 2>&1; then
        echo "Missing build dependency: $dep"
        echo "Install with: sudo apt-get install debhelper cmake qtbase5-dev qt5-qmake qttools5-dev qttools5-dev-tools libhidapi-dev libudev-dev dpkg-dev"
        exit 1
    fi
done

cd "$REPO_ROOT"

if [[ "${1:-}" == "--full" ]]; then
    echo "Running full dpkg-buildpackage (source + binary)..."
    dpkg-buildpackage -us -uc
else
    echo "Running binary-only build (dpkg-buildpackage -b)..."
    dpkg-buildpackage -b -us -uc
fi

echo ""
echo "=== Packages built ==="
ls -lh "$REPO_ROOT"/../openambit_*.deb \
        "$REPO_ROOT"/../libambit0_*.deb \
        "$REPO_ROOT"/../libambit-dev_*.deb 2>/dev/null || true

echo ""
echo "Install with:"
echo "  sudo dpkg -i ../libambit0_${PACKAGE_VERSION}_$(dpkg --print-architecture).deb \\"
echo "               ../openambit_${PACKAGE_VERSION}_$(dpkg --print-architecture).deb"
