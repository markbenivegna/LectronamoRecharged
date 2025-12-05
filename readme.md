# ⚡ Lectronamo Recharged (Modernized Pinball Control)

This repository contains the complete firmware for **Lectronamo Recharged**, a project to modernize the game logic for the classic 1978 Stern Lectronamo pinball machine using the **RPU (Retro Pinball Unit) Microcontroller System**.

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

This ruleset details the game logic implemented in the Lite Version firmware, differentiating between the original 1978 rules and the modernized "Recharged" additions.

### A. Core (1978) Rules

These features are foundational elements of the original Lectronamo ruleset:

* **Drop Targets (3-Bank & 5-Bank):** Base score of **500 points** per drop target.
* **Pop Bumpers & Slingshots:** **100 points** per hit.
* **Outlanes:** **3,000 points** plus **3 bonus advances**.
* **Rebound Rubber (SW 18):** **300 points** plus **1 bonus advance**.
* **Spinner:** **100 points** per rotation count.
* **3-Bank Completion:** Awards **15,000 points** and resets the bank.
    * **Bonus Multiplier:** Completing the 3-Bank advances the multiplier from 1X to **2X**, then to **3X**, and finally to **5X**.
* **5-Bank Completion:** Awards **25,000 points** and resets the bank.
    * **Extra Ball/Special:** Awards the **Extra Ball** or **Special** score (configurable via EEPROM) upon subsequent completions.
* **Bonus Count:** Initiated on ball drain. Awards accumulated bonus at the current multiplier.

### B. Recharged (Modernized) Rules

These features represent the new, modernized logic introduced in the Recharged firmware:

* **Skill Shot (Saucer on Plunge):** Awards **5,000 points** on the first hit of the ball.
* **Spinner Lite Value:** If the accumulated bonus is 10,000 or higher, the spinner is lit for a high-value score of **1,000 points** per count.
* **Side Lane / Saucer Combo:** Hitting the **Standup Target (SW 12)** lites the **Saucer** for **5,000 points** and **3 bonus advances**.
* **Left Return Lane Value:** Hitting the **Rollover Button (SW 17)** lites the **Left Return Lane** for a high-value award of **9,000 points**.
* **Arc Surge Combo (New Feature):** A timed combo started by the **Right Return Lane**.
    * **T1 Hit (SW 25):** Awards **10,000 points**.
    * **Saucer Hit (SW 40) within timer:** Awards **Super Score of 50,000 points** and ends the combo.
* **Ball Save:** A modern feature active for the first **15 seconds** of a new ball.
* **Game Utility:** Implements modern **High Score Check** with configurable Replay Awards and **Match Mode**.

---

## ⚙️ Hardware Requirements (RPU System)

This firmware is designed to run exclusively on the **RPU (Retro Pinball Unit)**, a custom open-source pinball controller built on the Arduino platform (typically a Teensy or similar microcontroller) compatible with classic Stern/Bally hardware interfaces.

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