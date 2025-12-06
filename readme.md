# ⚡ Lectronamo Recharged (Modernized Pinball Control)

This repository contains the complete firmware for **Lectronamo Recharged**, a project to modernize the game logic for the classic 1978 Stern Lectronamo pinball machine using the **RPU (Retro Pin Upgrade) Microcontroller System**.

This firmware implements a complex, ruleset-driven logic on modern, reliable hardware, replacing the original MPU and Driver Boards.

---

## 🏗️ Development Status: Lite Version Complete

The project is currently in the **Lite Version** stage. All core game rules, scoring, and state transitions for a complete, four-player game are implemented and ready for physical hardware testing and bug verification.

### Project Files Overview

The Lite Version firmware consists primarily of the following file types:

* **`.ino` File (Main Sketch):** Contains all global variables, helper functions, and the complete `RPU_Callback_GameLogic()` state machine.
* **`.h` Header File (Game Configuration):** Defines all game constants, scoring values, switch/solenoid/lamp mappings, and global variable declarations (e.g., `Lectronamo.h`).
* **External RPU Library Files:** Necessary library files for hardware abstraction (`RPU.h`, `RPU_Config.h`, etc.).

---

## 🕹️ Lite Version Ruleset (Feature List)

This ruleset details the game logic implemented in the Lite Version firmware, strictly adhering to the core Lectronamo rules while adding the modern 'Recharged' features.

### A. Core (1978) Rules

These features are the foundational, unaltered elements of the original Lectronamo ruleset:

* **Pop Bumpers & Slingshots:** Score **100 points**.
* **Rebound Rubber:** Scores **300 points** and advances bonus one step.
* **Outlanes:** Score **3,000 points** and **3 bonus advances**.
* **Turnaround Rollover:** Scores **100 points**, then lights the **Left Return Lane**.
* **Left Return Lane Value:** When lit, the **Left Return Lane** awards **9,000 points**.
* **Spinner:** Scores **100 points** per spin. Lights at **10,000 bonus** accumulated. Every **4th spin** advances bonus by 1,000.
* **Stationary Target (SW 12):** Scores **5,000 points** and advances bonus one step. Lites the Eject Pocket (Saucer) and Side Lane for extra bonus steps.
* **Lit Saucer / Side Lane:** If the Stationary Target has been made, the **Eject Pocket (Saucer)** awards **5,000 points and three bonus advances**.
* **Drop Targets:**
    * **3-Bank Completion:** Awards **6,000 points** and advances the Bonus Multiplier (2X, 3X, 5X).
    * **5-Bank Completion:** Awards **10,000 points**. Second completion lites **Extra Ball**, third completion lites **Special**.
* **Extra Ball / Special:** Awarded when lit and collected via lane/switch.
* **Tilt:** Disqualifies the current ball only (adjustable warnings).

#### B. Recharged (Modernized) Rules

These features represent the new, additive logic introduced in the Recharged firmware:

* **Free Play:** A game adjustment setting is available via EEPROM to enable Free Play operation, eliminating the need for coinage to start a game.
* **Ball Save:** **15-second timer** begins after playfield validation. Pulsing "Shoot Again" lamp indicates active save.
* **Skill Shot (Saucer - SW 40):** Awards **5,000 points** if hit on the initial plunge before any other switch.
* **Spinner Lite Value:** When bonus is 10,000 or higher, the spinner awards **1,000 points** per spin count (additive scoring, overriding the 100 pt score).
* **Display Enhancement:** The current **Spinner Hit Count** is actively displayed on the Credit/Player Display (Display 0).
* **Arc Surge Combo (New Feature):** A timed combo started by the **Right Inlane (SW 2)**. Player has **8 seconds** to complete the sequence:
    * **Hit Target 1 (SW 25):** Scores a moderate bonus.
    * **Hit Saucer (SW 40):** Scores the **Super Value** (50,000 points) and awards **3 bonus advances**.

---

## ⚙️ Hardware Requirements (RPU - Retro Pin Upgrade)

This firmware is designed to run exclusively on the **RPU (Retro Pin Upgrade)**, a custom open-source pinball controller built on the Arduino platform (typically a Teensy or similar microcontroller) compatible with classic Stern/Bally hardware interfaces.

## 🛠️ Getting Started (Flashing and Setup)

### Prerequisites

1.  **Arduino IDE / VS Code PlatformIO:** Must be installed.
2.  **RPU Library:** The core `RPU.h` library must be present in the project folder or installed in your library path.

### Flashing the Firmware

1.  Clone this repository to your local machine.
2.  Open the project in your IDE (VS Code with PlatformIO recommended).
3.  Ensure your RPU-compatible board is selected.
4.  Upload the firmware to the microcontroller.

### Initial Configuration (EEPROM)

Upon first boot, the system will initialize default values in EEPROM. Critical adjustments (e.g., TILT warnings, Free Play, Balls Per Game) are controlled by the MPU Adjustments defined in **`Lectronamo.h`**.
