# Lectronamo Recharged

Complete firmware for the **Lectronamo Recharged** project — a modernization of the classic 1978 Stern Lectronamo pinball machine using the **RPU (Retro Pin Upgrade)** microcontroller system.

This firmware implements a full ruleset-driven game engine on modern, reliable hardware, replacing the original MPU and Driver Boards while preserving the original gameplay feel and adding new "Recharged" features.

This code runs on the original machine with the addition of the RPU board:
https://www.pinballrefresh.com/retro-pin-upgrade-rpu

---

## Hardware

- **RPU Hardware Rev 3** — Arduino Mega 2560 Pro
- **MPU Architecture 1** — Bally/Stern -17, -35, MPU-100/200 compatible
- **Sound** — Stern SB-100 sound board (6-tone bitmask)
- **Displays** — 6-digit player score displays (up to 4 players)

---

## Development Status: Lite Version

The project is currently in the **Lite Version** stage. All core game rules, scoring, lamp states, and state transitions for a complete four-player game are implemented and ready for physical hardware testing.

### Project Files

| File | Purpose |
|------|---------|
| `LectronamoRecharged.ino` | Main sketch — game logic, state machine, switch/lamp/solenoid handling |
| `Lectronamo.h` | Hardware map — all lamp, switch, and solenoid definitions |
| `LampAnimations.h` | Attract mode lamp animation data (6 animations) |
| `RPU_Config.h` | RPU framework configuration (architecture, hardware rev, sound board) |
| `RPU.h / RPU.cpp` | RPU hardware abstraction library |
| `AudioHandler.h / .cpp` | Audio routing layer |
| `DisplayHandler.h / .cpp` | Display management layer |
| `OperatorMenus.h / .cpp` | Operator adjustment and diagnostics menus |
| `DropTargets.h` | Drop target bank management |

---

## Ruleset

### Core (1978) Rules

| Feature | Rule |
|---------|------|
| Pop Bumpers & Slingshots | 100 points |
| Rebound Rubber | 300 points, +1 bonus advance |
| Outlanes | 3,000 points, +3 bonus advances |
| Turnaround Rollover | 100 points, lights Left Return Lane (9,000 when collected) |
| Spinner (unlit) | 100 points per spin; every 4th spin advances bonus |
| Spinner (lit, bonus ≥ 10K) | 1,000 points per spin |
| Stationary Target | 5,000 points, +1 bonus advance, lights Saucer |
| Saucer (lit) | 5,000 points, +3 bonus advances |
| Saucer (unlit) | 500 points, +1 bonus advance |
| 3-Bank Completion | 6,000 points; advances bonus multiplier (2X → 3X → 5X → 7X → 8X → 10X) |
| 5-Bank Completion (1st) | 10,000 points |
| 5-Bank Completion (2nd) | Lights Extra Ball Lane (collect via Right Inlane) |
| 5-Bank Completion (3rd+) | Special |
| Kicker (Side Lane) | Collects bonus countdown |
| Bonus Countdown | 1,000 points per bonus step × multiplier |
| Tilt | Cancels current ball only (adjustable warnings) |

### Recharged (Modernized) Rules

| Feature | Rule |
|---------|------|
| Free Play | Operator-adjustable via EEPROM; disables coin requirement |
| Ball Save | Configurable timer begins after first playfield switch; pulsing Shoot Again lamp |
| Skill Shot | First plunge to Saucer awards 5,000 points + 3 bonus advances |
| Arc Surge Combo | Right Inlane starts 8-second timed combo; hitting Saucer during combo awards 50,000 + 3 bonus advances |
| 3-Bank Sweep | All 3 targets cleared within 1.5 seconds: +10,000 bonus points + 1.5-second lamp flash on the bank targets |
| Extended Multiplier | Bonus multiplier extends beyond 5X to 7X, 8X, 10X (max). 3-Bank target lamps show next multiplier; go dark at 10X (max reached) |
| Attract Mode Animations | 6 lamp animations including Classic Flow and Arc Surge Showcase sequences |

---

## Operator Adjustments

Accessible via the self-test/operator switch on the machine. Adjustments are stored in EEPROM and persist across power cycles.

Press the **self-test switch** (inside coin door) repeatedly to advance through top-level menus. The display shows the menu number. Use the **credit button** to step through sub-adjustments within a menu. The **coin door button** increments the value; hold to change faster.

### Top-Level Menus

| Display # | Menu |
|-----------|------|
| 1 | Self Test (lamps, solenoids, switches, displays, sounds) |
| 2 | Audits (play counts, coin counts, replay counts, high score beats) |
| 3 | Basic Adjustments |
| 4 | Game Rules |
| 5 | Game Adjustments |

---

### Basic Adjustments (Menu 3)

| # | Adjustment | Options | Default | Notes |
|---|------------|---------|---------|-------|
| 0 | Free Play | 0=Off / 1=On | 0 | Disables coin requirement when On |
| 1 | Ball Save | 0 / 5 / 10 / 15 / 20 sec | 15 | Timer starts on first playfield switch |
| 2 | Tilt Warnings | 0–2 | 2 | Warnings before tilt penalty |
| 3 | Music Volume | 0–10 | 10 | |
| 4 | Sound FX Volume | 0–10 | 10 | |
| 5 | Callouts Volume | 0–10 | 10 | |
| 6 | Balls Per Game | 3–10 | 3 | |
| 7 | Tournament Mode | 0=Off / 1=On | 0 | Converts replays/specials to score |
| 8 | Extra Ball Value | Score (tournament mode only) | 20,000 | Points awarded instead of extra ball |
| 9 | Special Value | Score (tournament mode only) | 40,000 | Points awarded instead of special |
| 10 | Reset During Game | 0 / 1 / 2 / 3 / 99 sec | 2 | Hold time for credit button to reset game; 99=disabled |
| 11 | Score Award Level 1 | Score | 1,000,000 | First score threshold for replay |
| 12 | Score Award Level 2 | Score | 3,000,000 | Second score threshold for replay |
| 13 | Score Award Level 3 | Score | 5,000,000 | Third score threshold for replay |
| 14 | Score Awards | 0–7 bitmask | 7 | Which score levels are active |
| 15 | Scrolling Scores | 0=Off / 1=On | 1 | Scroll all scores during attract mode |
| 16 | High Score | Score | 10,000 | Resets only on first EEPROM initialization |
| 17 | Credits | 0–40 | 4 | Current credit count |
| 21 | Match Feature | 0=Off / 1=On | 1 | End-of-game match digit for free ball |

---

### Game Rules (Menu 4)

| Display # | Rule Set | Ball Save | Notes |
|-----------|----------|-----------|-------|
| 1 | Easy | 20 sec | Longer ball save |
| 2 | Medium | 10 sec | Default |
| 3 | Hard | 0 sec | No ball save |

---

### Game Adjustments (Menu 5)

| # | Adjustment | Options | Default | Notes |
|---|------------|---------|---------|-------|
| 0 | Trough Eject Strength | 2 / 3 / 4 / 5 / 6 / 7 / 8 | — | Ball serve solenoid power; tune until ball reliably reaches shooter lane |
| 1 | Saucer Eject Strength | 5–15 | — | Saucer ejection solenoid power |
| 2 | Slingshot Strength | 4–8 | — | Applied to both left and right slingshots |
| 3 | Pop Bumper Strength | 4–8 | — | Applied to all three pop bumpers |
| 4 | Saucer Light Persistence | 0=Off after scored / 1=Stays lit whole ball | 1 | Original SW14: when 0, saucer lamp extinguishes after being collected once |
| 5 | High Game Free Games | 0=Novelty / 1 / 2 / 3 | 3 | Original SW15–16: free games awarded for new high score; 0 = novelty (no award) |
| 6 | Special Open-Ended | 0=1 per ball / 1=Open-ended | 0 | Original SW22: when 1, Special can be collected repeatedly each ball |
| 7 | Bonus Countdown Mode | 0=Flat 1,000/step / 1=Multiplied | 0 | Original SW24: when 1, each countdown step scores 1,000 × bonus multiplier |
| 8 | Extra Ball Lane | 0=Bypass / 1=Award | 1 | Original SW26: when 0, 2nd 5-bank completion does not light or award extra ball |
| 9 | Special Award Type | 0=100K pts / 1=Free Ball / 2=Free Game / 3=Both | 2 | Original SW31–32: what is awarded when Special is collected |

---

## Getting Started

### Prerequisites

1. Arduino IDE (or VS Code with Arduino extension)
2. Arduino Mega 2560 Pro board support installed
3. RPU hardware board installed in the machine

### Flashing

1. Open `LectronamoRecharged.ino` in the Arduino IDE
2. Select board: **Arduino Mega or Mega 2560**
3. Select the correct COM port for your RPU board
4. Upload

### First Boot

On first boot the system initializes default EEPROM values. Use the operator menu (self-test switch) to verify and adjust solenoid strengths before gameplay.
