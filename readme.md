# Lectronamo Recharged

Complete firmware for the **Lectronamo Recharged** project — a modernization of the classic 1978 Stern Lectronamo pinball machine using the **RPU (Retro Pin Upgrade)** microcontroller system.

This firmware implements a full ruleset-driven game engine on the RPU — a daughter board that sits on top of the original MPU and takes over game control — preserving the original gameplay feel while adding new "Recharged" features.

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

The project is currently in the **Lite Version** stage. All core game rules, scoring, lamp states, and state transitions for a complete four-player game are implemented and running stably on the physical machine (see the `baseline` tag for the current known-good build).

### Project Files

| File | Purpose |
|------|---------|
| `LectronamoRecharged.ino` | Main sketch — game logic, state machine, switch/lamp/solenoid handling |
| `Lectronamo.h` | Hardware map — all lamp, switch, and solenoid definitions |
| `LampAnimations.h` | Attract mode lamp animation data (6 animations) |
| `RPU_Config.h` | RPU framework configuration (architecture, hardware rev, sound board) |
| `RPU.h / RPU.cpp` | RPU hardware abstraction library |
| `AudioHandler.h / .cpp` | Audio routing layer — manages SB-100 sound board queue and playback |
| `SoundSequences.h / .cpp` | Unified sound sequencer — queues multi-step audio sequences with precise timing |
| `DisplayHandler.h / .cpp` | Display management layer |
| `OperatorMenus.h / .cpp` | Operator adjustment and diagnostics menus |
| `DropTargets.h` | Drop target bank management |
| `ALB-Communication.h / .cpp` | Auxiliary lamp board / display communication |

---

## Ruleset

### Core (1978) Rules

| Feature | Rule |
|---------|------|
| Pop Bumpers | 1,000 points on 3-ball / 100 points on 5-ball (per original rules) |
| Slingshots | 100 points |
| Rebound Rubber | 300 points, +1 bonus advance |
| Outlanes | 3,000 points, +3 bonus advances |
| Turnaround Rollover | 100 points, lights Left Return Lane (9,000 when collected) |
| Spinner (unlit) | 100 points per spin; every 4th spin advances bonus |
| Spinner (lit, bonus ≥ 10K) | 1,000 points per spin |
| Stationary Target | 5,000 points, +1 bonus advance, lights Saucer |
| Saucer (lit) | 5,000 points, +3 bonus advances |
| Saucer (unlit) | 500 points, +1 bonus advance |
| 3-Bank Completion | 6,000 points; advances bonus multiplier (2X → 3X → 5X) |
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
| Skill Shot | First plunge to Saucer awards 25,000 points + 3 bonus advances |
| Arc Surge Combo | Right Inlane starts 8-second timed combo; hitting Saucer during combo awards 50,000 + 3 bonus advances |
| 3-Bank Sweep | All 3 targets cleared within 0.5 seconds (one clean pass): +10,000 bonus points + 1.5-second lamp flash on the bank targets |
| Extended Multiplier | Bonus multiplier extends beyond 5X to 7X, 8X, 10X (max). 3-Bank target lamps show next multiplier; go dark at 10X (max reached) |
| Attract Mode Animations | 6 lamp animations including Classic Flow and Arc Surge Showcase sequences |

---

## Operator Adjustments

Accessible via the self-test/operator switch on the machine. Adjustments are stored in EEPROM and persist across power cycles.

Two buttons drive the menus: the **self-test switch** (inside coin door) and the **Start button**. The self-test switch advances through top-level menus (menu number shows in the credit display). Press Start to enter a menu, then the self-test switch steps through its items (item number shows in the ball-in-play display, starting at 1). Start changes the current value — press repeatedly or hold for faster changes. On score-valued adjustments Start can only increase the value; **double-click Start to reset a score adjustment to zero** (e.g. High Score To Date).

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

Item numbers below match the ball-in-play display.

| # | Adjustment | Options | Default | Notes |
|---|------------|---------|---------|-------|
| 1 | Free Play | 0=Off / 1=On | 0 | Disables coin requirement when On |
| 2 | Ball Save | 0 / 10 / 15 / 20 / 25 sec | 15 | Timer starts on first playfield switch |
| 3 | Tilt Warnings | 0–2 | 2 | Warnings before tilt penalty |
| 4 | Music Volume | 0–10 | 10 | |
| 5 | Sound FX Volume | 0–10 | 10 | |
| 6 | Callouts Volume | 0–10 | 10 | |
| 7 | Balls Per Game | 3–10 | 3 | |
| 8 | Tournament Mode | 0=Off / 1=On | 0 | Converts replays/specials to score |
| 9 | Extra Ball Value | Score (tournament mode only) | 20,000 | Points awarded instead of extra ball |
| 10 | Special Value | Score (tournament mode only) | 40,000 | Points awarded instead of special |
| 11 | Reset During Game | 0 / 1 / 2 / 3 / 99 sec | 2 | Hold time for credit button to reset game; 99=disabled |
| 12 | Score Award Level 1 | Score | 100,000 | First score threshold for replay |
| 13 | Score Award Level 2 | Score | 200,000 | Second score threshold for replay |
| 14 | Score Award Level 3 | Score | 350,000 | Third score threshold for replay |
| 15 | Score Awards | 0–7 bitmask | 7 | Which score levels are active |
| 16 | Scrolling Scores | 0=Off / 1=On | 1 | Scroll all scores during attract mode |
| 17 | High Score To Date | Score | 10,000 | Start adds 1,000 (hold for faster); **double-click Start to reset to 0** |
| 18 | Credits | 0–40 | 4 | Current credit count |
| 19 | Coins Per Credit (Chute 1) | 9 coin:credit pairs | 1:1 | Coin-to-credit conversion for chute 1 |
| 20 | Coins Per Credit (Chute 2) | 9 coin:credit pairs | 1:1 | Coin-to-credit conversion for chute 2 |
| 21 | Coins Per Credit (Chute 3) | 9 coin:credit pairs | 1:1 | Coin-to-credit conversion for chute 3 |
| 22 | Match Feature | 0=Off / 1=On | 1 | End-of-game match digit for free ball |

---

### Game Rules (Menu 4)

| Display # | Rule Set | Ball Save | Notes |
|-----------|----------|-----------|-------|
| 1 | Easy | 20 sec | Longer ball save; Special open-ended |
| 2 | Medium | 10 sec | Default |
| 3 | Hard | 0 sec | No ball save; Extra Ball Lane disabled |

Installing a rule set overwrites Ball Save (menu 3), Special Open-Ended, Extra Ball Lane, and Special Award Type (menu 5) with the preset's values — re-check those adjustments after changing rule sets. All presets set Special Award Type to points.

---

### Game Adjustments (Menu 5)

Item numbers below match the ball-in-play display.

| # | Adjustment | Options | Default | Notes |
|---|------------|---------|---------|-------|
| 1 | Trough Eject Strength | 2 / 3 / 4 / 5 / 6 / 7 / 8 | — | Ball serve solenoid power; tune until ball reliably reaches shooter lane |
| 2 | Saucer Eject Strength | 5–15 | — | Saucer ejection solenoid power |
| 3 | Slingshot Strength | 4–8 | — | Applied to both left and right slingshots |
| 4 | Pop Bumper Strength | 4–8 | — | Applied to all three pop bumpers |
| 5 | Saucer Light Persistence | 0=Off after scored / 1=Stays lit whole ball | 1 | Original SW14: when 0, saucer lamp extinguishes after being collected once |
| 6 | High Game Free Games | 0=Novelty / 1 / 2 / 3 | 3 | Original SW15–16: free games awarded for new high score; 0 = novelty (no award) |
| 7 | Special Open-Ended | 0=1 per ball / 1=Open-ended | 0 | Original SW22: when 1, Special can be collected repeatedly each ball |
| 8 | Bonus Countdown Mode | 0=Flat 1,000/step / 1=Multiplied | 0 | Original SW24: when 1, each countdown step scores 1,000 × bonus multiplier |
| 9 | Extra Ball Lane | 0=Bypass / 1=Award | 1 | Original SW26: when 0, 2nd 5-bank completion does not light or award extra ball |
| 10 | Special Award Type | 0=100K pts / 1=Free Ball / 2=Free Game / 3=Both | 2 | Original SW31–32: what is awarded when Special is collected |

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
