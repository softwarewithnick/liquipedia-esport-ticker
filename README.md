# Liquipedia-Public-LED

Tournament display for a 64x32 RGB LED matrix. Fetches upcoming and live esports tournament data from Firebase Realtime Database and scrolls it on the display.
<p align="center">
  <img src="https://github.com/user-attachments/assets/b7461026-ee0c-419d-a461-321208bbd294" width="30%" />
  <img src="https://github.com/user-attachments/assets/d2654ae9-2eee-4131-abfd-500b22f1e18d" width="30%" />
  <img src="https://github.com/user-attachments/assets/a2697120-fe1a-4972-9883-693ec3f56e0b" width="30%" />
</p>



## Hardware

- **Board**: ESP32
- **Display**: 64x32 RGB LED matrix (PxMatrix compatible)
- **Wiring** (ESP32):

| Pin   | GPIO |

|-------|------|
| P_LAT | 22   |
| P_A   | 19   |
| P_B   | 23   |
| P_C   | 18   |
| P_D   | 5    |
| P_E   | 15   |
| P_OE  | 16   |

## Required Libraries

Install via Arduino Library Manager:

- **PxMatrix** – LED matrix driver
- **GFX_Layer** (or GFX Layer) – Scrolling text
- **ArduinoJson** – JSON parsing
- **Firebase** – Firebase Realtime Database (same library as game_ticker_pxmatrix)
- **WiFiManager** – Optional; used when `WIFI_USE_MANAGER` is 1

## Setup

1. Copy `config.h.example` to `config.h`
2. Edit `config.h` with your settings (WiFi, Firebase URL, games, etc.)
3. Open `Liquipedia-Public-LED.ino` in Arduino IDE
4. Select board: **ESP32 Dev Module**
5. Upload

## Configuration

| Option | Description |
|--------|-------------|
| `WIFI_SSID` / `WIFI_PASSWORD` | WiFi credentials (used when `WIFI_USE_MANAGER` is 0) |
| `WIFI_USE_MANAGER` | `1` = AP portal for first-time setup, `0` = static credentials |
| `FIREBASE_URL` | Firebase Realtime Database base URL |
| `GAMES` | Comma-separated game IDs to display |
| `TIER_FILTER` | Tiers to show (e.g. `"S,A,B"`) |
| `CYCLES` | Repeat count per tournament (1–10) |
| `COLOR_INDEX` | Accent color: 0=White, 1=Blue, 2=Red, 3=Green, 4=Purple, 5=Orange |

### Supported Game IDs

`halo`, `ageofempires`

## Firebase Data Structure

Uses the same paths as game_ticker_pxmatrix:

| Temporal | Path |
|----------|------|
| Upcoming | `notLive/game/{game}/upcoming` |
| Live     | `live/game/{game}/live` |

Expected JSON keys (arrays, same length):

- `League` – Tournament/league name
- `Tier` – Tier (e.g. S, A, B)
- `Prize` – Prize pool (e.g. $100,000)
- `Date` – Start date string

Example:

```json
{
  "League": ["League Championship 2025"],
  "Tier": ["S"],
  "Prize": ["$100,000"],
  "Date": ["2025-03-01"]
}
```

Firebase rules must allow read access (e.g. `".read": "true"` for test).

## Project Structure

```
Liquipedia-Public-LED/
├── Liquipedia-Public-LED.ino   # Main sketch
├── config.h                     # Your config (copy from config.h.example)
├── config.h.example             # Config template
├── game_logos.h                 # RGB565 game logos
├── data_provider.h              # Data interface
├── data_provider.cpp            # Firebase implementation
└── README.md
```

## Debug Output

Serial debug output includes:

- Firebase path being fetched
- First 300 characters of response
- Loaded tournament count
- JSON parse errors (if any)

## License

MIT
