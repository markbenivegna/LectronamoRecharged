/*
 * Lectronamo Recharged - Alpha Build for Retro Pin Upgrade
 * Based on original Stern Lectronamo rules with Lite rule enhancements
 * By: [Your Name / Team Name]
 * Date: [Insert Date]
 *
 * Features in this version:
 * - Original rules preserved (scoring, bonus, drop banks, saucer, spinner, etc.)
 * - Skill Shot (timed saucer light)
 * - Ball Save timer (20s)
 * - Right inlane → saucer combo logic ("Arc Surge")
 * - Mystery Award trigger (placeholders)
 * - Mode starter logic (placeholder for Spinner Rips mode)
 */

#include "RPU.h"

// === GLOBAL VARIABLES === //
uint8_t bonusMultiplier = 1;
uint8_t drop3BankHits = 0;
uint8_t drop5BankHits = 0;
bool ballSaveActive = false;
uint32_t ballSaveTimer = 0;
bool skillShotActive = false;
bool arcSurgeActive = false;
uint32_t arcSurgeTimer = 0;
bool mysteryAvailable = false;
uint8_t currentMode = 0;
bool modeInProgress = false;

// === LAMP DEFINITIONS === //
#define LAMP_SKILL_SHOT 40
#define LAMP_ARC_SURGE 25
#define LAMP_BALL_SAVE 55

// === SWITCH DEFINITIONS === //
#define SWITCH_SAUCER 40
#define SWITCH_RIGHT_INLANE 2
#define SWITCH_ARC_SURGE 25
#define SWITCH_STANDUP_MODE 12

void setupGame() {
    bonusMultiplier = 1;
    drop3BankHits = 0;
    drop5BankHits = 0;
    ballSaveActive = true;
    ballSaveTimer = millis() + 20000; // 20 seconds
    skillShotActive = true;
    arcSurgeActive = false;
    arcSurgeTimer = 0;
    mysteryAvailable = false;
    currentMode = 0;
    modeInProgress = false;
    lamps[LAMP_SKILL_SHOT] = 1;
    lamps[LAMP_BALL_SAVE] = 1;
}

void loopGame() {
    // Ball Save logic
    if (ballSaveActive && millis() > ballSaveTimer) {
        ballSaveActive = false;
        lamps[LAMP_BALL_SAVE] = 0;
    }

    // Arc Surge timer
    if (arcSurgeActive && millis() > arcSurgeTimer) {
        arcSurgeActive = false;
        lamps[LAMP_ARC_SURGE] = 0;
    }
}

void switchHit(uint8_t sw) {
    switch (sw) {
        case SWITCH_SAUCER:
            if (skillShotActive) {
                addScore(5000);
                skillShotActive = false;
                lamps[LAMP_SKILL_SHOT] = 0;
                debug("Skill Shot Achieved!");
            } else if (arcSurgeActive) {
                addScore(10000); // Super Saucer Value
                arcSurgeActive = false;
                lamps[LAMP_ARC_SURGE] = 0;
                // Chance to trigger mystery
                if (!mysteryAvailable) {
                    mysteryAvailable = true;
                    triggerMysteryAward();
                }
            } else {
                addScore(1000); // Base saucer value
            }
            break;

        case SWITCH_RIGHT_INLANE:
            arcSurgeActive = true;
            arcSurgeTimer = millis() + 6000; // 6s window for combo
            lamps[LAMP_ARC_SURGE] = 1;
            break;

        case SWITCH_ARC_SURGE:
            if (arcSurgeActive) {
                // Will finish in saucer if combo is successful
                // Actual saucer logic handled above
            }
            break;

        case SWITCH_STANDUP_MODE:
            if (!modeInProgress) {
                if (currentMode == 0) {
                    currentMode = 1; // Spinner Rips for now
                    startMode(currentMode);
                }
            }
            break;

        default:
            if (skillShotActive) {
                skillShotActive = false;
                lamps[LAMP_SKILL_SHOT] = 0;
            }
            break;
    }
}

void startMode(uint8_t modeId) {
    modeInProgress = true;
    switch (modeId) {
        case 1: // Spinner Rips mode
            debug("Spinner Rips Mode Started");
            // Placeholder: Activate spinner lights and set timer
            break;
        // Add additional modes here
    }
}

void triggerMysteryAward() {
    uint8_t rng = random(0, 100);
    if (rng < 5) {
        debug("Mystery: Extra Ball");
    } else if (rng < 25) {
        debug("Mystery: Advance Pops");
    } else if (rng < 30) {
        debug("Mystery: Big Points");
        addScore(15000);
    } else if (rng < 90) {
        debug("Mystery: Puny Points");
        addScore(100);
    } else if (rng < 95) {
        debug("Mystery: Advance Multiplier");
    } else if (rng < 98) {
        debug("Mystery: Hold Bonus");
    } else {
        debug("Mystery: TILT Ends Game!!!");
        // Simulated humorous penalty
    }
    mysteryAvailable = false;
}