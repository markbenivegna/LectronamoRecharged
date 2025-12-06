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

* **Pop Bumpers & Slingshots:** Score **100 points**[cite: 97].
* **Rebound Rubber (SW 18):** Scores **300 points** and advances bonus one step[cite: 97, 153].
* **Outlanes:** Score **3,000 points** and **3 bonus advances**[cite: 205].
* **Turnaround Rollover (SW 17):** Scores **100 points**, then lights the **Left Return Lane**[cite: 193].
* **Left Return Lane Value:** When lit, the **Left Return Lane** awards **9,000 points**[cite: 194].
* **Spinner:** Scores **100 points** per spin. Lights at **10,000 bonus** accumulated. Every **4th spin** advances bonus by 1,000[cite: 189, 190, 152].
* **Stationary Target (SW 12):** Scores **5,000 points** and advances bonus one step. Lites the Eject Pocket (Saucer) and Side Lane for extra bonus steps[cite: 196].
* **Lit Saucer / Side Lane:** If the Stationary Target has been made, the **Eject Pocket (Saucer)** awards **5,000 points and three bonus advances**[cite: 198].
* **Drop Targets:**
    * **3-Bank Completion:** Awards **6,000 points** and advances the Bonus Multiplier (2X, 3X, 5X)[cite: 165].
    * **5-Bank Completion:** Awards **10,000 points**[cite: 184]. Second completion lites **Extra Ball**, third completion lites **Special**.
* **Extra Ball / Special:** Awarded when lit and collected via lane/switch.
* **Tilt:** Disqualifies the current ball only (adjustable warnings)[cite: 110].

### B. Recharged (Modernized) Rules

These features represent the new, additive logic introduced in the Recharged firmware:

* **Ball Save:** **15-second timer** begins after playfield validation. Pulsing "Shoot Again" lamp indicates active save.
* **Skill Shot (Saucer - SW 40):** Awards **5,000 points** if hit on the initial plunge before any other switch.
* **Spinner Lite Value:** When bonus is 10,000 or higher, the spinner awards **1,000 points** per spin count (additive scoring, overriding the 100 pt score).
* **Arc Surge Combo (New Feature):** A timed combo started by the **Right Inlane (SW 2)**. Player has **5 seconds** to complete the sequence:
    * **Hit Target 1 (SW 25):** Scores a moderate bonus.
    * **Hit Saucer (SW 40):** Scores the **Super Value** (50,000 points) and awards **3 bonus advances**.