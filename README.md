# super-ws-overlay

A Hyprland plugin that shows a workspace number overlay on each monitor when you hold the Super key.

![Hyprland](https://img.shields.io/badge/Hyprland-v0.54+-blue)
![Build](https://github.com/qwexvf/super-ws-overlay/actions/workflows/build.yml/badge.svg)

## Install

```bash
hyprpm add https://github.com/qwexvf/super-ws-overlay
hyprpm enable super-ws-overlay
```

## Features

- Shows active workspace number centered on each monitor
- Configurable hold delay, colors, border radius, and font size
- Cancels if Super is used as a modifier (e.g. Super+1)
- Uses eww for rendering overlays

## Dependencies

- Hyprland >= 0.54
- eww

## Setup

### 1. Add eww widgets

Copy the overlay definitions from `src/eww/ws-overlay.yuck` and `src/eww/ws-overlay.scss` into your eww config.

Add one `defwindow ws-overlay-N` per monitor (examples for monitors 0 and 1 are included).

### 2. Configure in hyprland.conf

```conf
plugin {
    super-ws-overlay {
        hold_ms = 100
        bg_color = rgba(10, 8, 16, 0.85)
        text_color = #7888a0
        border_color = rgba(255, 255, 255, 0.08)
        border_radius = 16
        font_size = 48
    }
}
```

### 3. Optional: blur the overlay

```conf
layerrule {
    name = eww-ws-overlay-blur
    match:namespace = eww-ws-overlay
    blur = on
    animation = fade
}
```

## Configuration

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `hold_ms` | int | `800` | Milliseconds to hold Super before showing |
| `bg_color` | string | `rgba(10, 8, 16, 0.85)` | Background color (CSS) |
| `text_color` | string | `#7888a0` | Text color (CSS) |
| `border_color` | string | `rgba(255, 255, 255, 0.08)` | Border color (CSS) |
| `border_radius` | int | `16` | Border radius in px |
| `font_size` | int | `48` | Font size in px |

All options reload with `hyprctl reload`.

## Manual build

```bash
git clone https://github.com/qwexvf/super-ws-overlay.git
cd super-ws-overlay
make
make install
# Then add to hyprland.conf:
# plugin = $HOME/.config/hypr/plugins/super-ws-overlay.so
```

## License

MIT
