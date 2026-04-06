#!/bin/bash
set -euo pipefail

REPO="qwexvf/super-ws-overlay"
PLUGIN_DIR="${HOME}/.config/hypr/plugins"
EWW_DIR="${HOME}/.config/eww"

echo ":: super-ws-overlay installer"

# Detect Hyprland version
HYPR_TAG=$(hyprctl version -j 2>/dev/null | jq -r '.tag // empty' || true)
if [ -z "$HYPR_TAG" ]; then
    echo "error: Hyprland not running or hyprctl not found" >&2
    exit 1
fi
echo "   Hyprland: $HYPR_TAG"

# Get latest release matching Hyprland version
ASSET_URL=$(curl -sf "https://api.github.com/repos/${REPO}/releases" | \
    jq -r --arg tag "$HYPR_TAG" '
        [.[] | select(.tag_name | startswith($tag))] | first |
        .assets[] | select(.name == "super-ws-overlay.so") | .browser_download_url // empty')

if [ -z "$ASSET_URL" ]; then
    echo "   No prebuilt binary for $HYPR_TAG, building from source..."
    TMPDIR=$(mktemp -d)
    trap 'rm -rf "$TMPDIR"' EXIT
    git clone --depth 1 "https://github.com/${REPO}.git" "$TMPDIR/src"
    cd "$TMPDIR/src"
    make
    mkdir -p "$PLUGIN_DIR"
    cp super-ws-overlay.so "$PLUGIN_DIR/"
else
    echo "   Downloading prebuilt binary..."
    mkdir -p "$PLUGIN_DIR"
    curl -sfL "$ASSET_URL" -o "${PLUGIN_DIR}/super-ws-overlay.so"
    chmod 755 "${PLUGIN_DIR}/super-ws-overlay.so"
fi

echo "   Installed: ${PLUGIN_DIR}/super-ws-overlay.so"

# Install eww widget files if eww config exists
if [ -d "$EWW_DIR" ]; then
    echo ""
    echo ":: eww config detected at ${EWW_DIR}"
    echo "   You need to add the overlay widgets to your eww config."
    echo "   Copy the contents of eww/ws-overlay.yuck into your eww.yuck"
    echo "   Copy the contents of eww/ws-overlay.scss into your eww.scss"
    echo ""
    echo "   Or if your eww supports includes:"
    echo "     (include \"./ws-overlay.yuck\") in eww.yuck"
    echo "     @import \"ws-overlay\";          in eww.scss"
fi

echo ""
echo ":: Add to hyprland.conf:"
echo ""
echo "   plugin = \$HOME/.config/hypr/plugins/super-ws-overlay.so"
echo ""
echo "   plugin {"
echo "       super-ws-overlay {"
echo "           hold_ms = 100"
echo "           bg_color = rgba(10, 8, 16, 0.85)"
echo "           text_color = #7888a0"
echo "           border_color = rgba(255, 255, 255, 0.08)"
echo "           border_radius = 16"
echo "           font_size = 48"
echo "       }"
echo "   }"
echo ""
echo ":: Done! Reload Hyprland: hyprctl reload"
