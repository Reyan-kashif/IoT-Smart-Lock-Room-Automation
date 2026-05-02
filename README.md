# 🔐 IoT Smart Lock & Room Automation — ESP32-S3
 
**A fully integrated IoT system combining smart door lock access control with automated room appliance management, built on the ESP32-S3-N16R8.**
 
> Cost: ~4,500 PKR | vs. 15,000+ PKR for commercial solutions
 
---
 
## 📋 Table of Contents
- [Overview](#overview)
- [Hardware](#hardware)
- [Pin Reference](#pin-reference)
- [Features](#features)
- [Blynk Setup](#blynk-setup)
- [Arduino IDE Settings](#arduino-ide-settings)
- [Libraries Required](#libraries-required)
- [How It Works](#how-it-works)
- [Keypad Controls](#keypad-controls)
- [RGB LED Status](#rgb-led-status)
- [Repo Structure](#repo-structure)
- [License](#license)
---
 
## Overview
 
This system replaces both a standalone smart lock and a room automation hub with a single ESP32-S3-N16R8 micro-controller. The ESP32-S3's built-in Wi-Fi eliminates the need for a separate Wi-Fi module, reducing hardware complexity and cost.
 
**Authentication methods:** 4×4 keypad (local) + Blynk mobile app (remote)  
**Automation trigger:** PIR motion sensor — detects when the room is empty and alerts if appliances are left on  
**Appliance control:** 4-channel relay module (light, fan, AC, extra)
 
---
 
## Hardware
 
| Component | Details |
|---|---|
| **Micro-controller** | ESP32-S3-N16R8 (16MB Flash, 8MB OPI PSRAM, dual USB-C) |
| **Keypad** | 4×4 matrix keypad |
| **Motion Sensor** | PIR sensor (2-second software debounce) |
| **Door Lock** | Servo motor — 0° = locked, 90° = unlocked |
| **Relay Module** | 4-channel, active LOW |
| **Buzzer** | KY-012 active buzzer |
| **Status LED** | Built-in RGB NeoPixel on GPIO 48 |
| **Programming** | CH343P USB-to-Serial chip via USB-C |
 
---
 
## Pin Reference
 
| GPIO | Component |
|---|---|
| 4 | Relay 1 — Light |
| 5 | Relay 2 — Fan |
| 6 | KY-012 Buzzer |
| 7 | PIR Motion Sensor |
| 8 | Servo Motor (PWM) |
| 15 | Relay 3 — AC |
| 16 | Relay 4 — Extra |
| 17, 18, 21, 38 | Keypad Rows |
| 39, 40, 41, 42 | Keypad Columns |
| 48 | Built-in RGB NeoPixel LED |
 
---
 
## Features
 
1. **Dual authentication** — Keypad password entry + Blynk app remote control
2. **Smart exit detection** — Monitors motion + door lock status together
3. **Energy alerts** — Notifies if appliances are on 30 seconds after you leave
4. **Remote control** — Door lock + 4 appliances via Blynk mobile app
5. **Security alerts** — Push notifications for unauthorized access attempts
6. **Non-blocking buzzer** — Timer-based audio feedback (no blocking `delay()`)
7. **RGB status LED** — Color-coded system state using the onboard NeoPixel
---
 
## Blynk Setup
 
| Virtual Pin | Function |
|---|---|
| V0 | Lock / Unlock toggle |
| V1 | Lock status display |
| V2 | Motion status display |
| V3 | Relay 1 (Light) |
| V4 | Relay 2 (Fan) |
| V5 | Relay 3 (AC) |
| V6 | Relay 4 (Extra) |
| V7 | Turn OFF all appliances |
| V8 | System log / notification feed |
 
**Blynk Events to configure in your template:**
 
| Event Name | Description |
|---|---|
| `door_locked` | Door was locked |
| `door_unlocked` | Door was unlocked |
| `unauthorized_access` | Wrong password entered |
| `appliances_on` | Appliances left on after exit |
 
> ⚠️ Add your own `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME`, and `BLYNK_AUTH_TOKEN` in the `.ino` file before uploading.
 
---
 
## Arduino IDE Settings
 
| Setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
| PSRAM | OPI PSRAM |
| USB CDC On Boot | Enabled |
| Upload Speed | 921600 |
| Port | COM port (CH343P chip) |
 
---
 
## Libraries Required
 
Install all via Arduino IDE Library Manager:
 
- `Blynk` by Volodymyr Shymanskyy
- `Keypad` by Mark Stanley, Alexander Brevig
- `ESP32Servo` by Kevin Harrington
- `Adafruit NeoPixel` by Adafruit
---
 
## How It Works
 
1. On boot, all relays initialize HIGH (OFF). Door locked. RGB = red.
2. Enter password on keypad then press `#` to unlock.
3. On correct password: servo rotates to 90°, RGB turns green, Blynk notified.
4. PIR sensor tracks motion in the room.
5. When door is locked + no motion detected for 30 seconds:
   - System checks which relays are still ON
   - Sends Blynk push notification listing active appliances
   - RGB turns yellow
6. User can turn off appliances remotely via Blynk.
---
 
## Keypad Controls
 
| Key | Action |
|---|---|
| `0–9` | Enter password digits |
| `#` | Confirm / Submit password |
| `*` | Clear current input |
| `A` | Manually lock the door |
 
Default password: `1111` (change `PASSWORD` constant in code)
 
---
 
## RGB LED Status
 
| Color | Meaning |
|---|---|
| 🔴 Red | Door locked |
| 🟢 Green | Door unlocked |
| 🔵 Blue | Motion detected |
| 🟡 Yellow | Appliances left on (warning) |
| 🟣 Purple | Wrong password entered |
 
---
 
## Repo Structure
 
```
IoT-Smart-Lock-Room-Automation/
│
├── code/
│   └── EspCode.ino              # Main ESP32-S3 firmware
│
├── docs/
│   └── Iot based Smart door lock and dynamic room automation system.pdf                 # IEEE-format review paper (LaTeX)
│
├── simulation/
│   └── ICT_Project.pkt          # Cisco Packet Tracer simulation
│
├── images/
│   ├── Cisco_Packet_Tracer_Circuit.png
│   ├── appendixA_components.png
│   ├── appendixB_flowchart.png
│   └── appendixC_circuit.png
|   └── appendixD_blynk.png.png
│
├── LICENSE
└── README.md
```
 
---
 
## License
 
This project is licensed under the **MIT License** — see [LICENSE](LICENSE) for details.  
Free to use, modify, and share. Credit appreciated.
 
---
 
*Built by Reyan Kashif*
