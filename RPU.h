// === RPU.h (Header for Lectronamo Recharged) ===
#ifndef RPU_H
#define RPU_H

#include "RPULite_Config.h"

// Mode States
enum Mode {
  MODE_NONE,
  MODE_SPINNER_RIPS,
  MODE_POP_FRENZY,
  MODE_DROP_ASSAULT,
  MODE_MINI_WIZARD,
  MODE_WIZARD
};

// Game State Flags
extern bool skillShotActive;
extern bool ballSaveActive;
extern bool comboActive;
extern bool modeReady;
extern bool wizardReady;
extern Mode currentMode;

// Timer Variables
extern unsigned long skillShotTimer;
extern unsigned long ballSaveTimer;
extern unsigned long comboTimer;
extern unsigned long modeTimer;

// Scoring + Tracking
extern int bonusMultiplier; // 1x, 2x, 3x, 5x max
extern int bonusPoints;
extern int popHitCount;
extern int drop3Hits;
extern int drop5Hits;
extern int comboSuccesses;

// Function Prototypes
void startSkillShot();
void checkSkillShot();
void startBallSave();
void checkBallSave();
void startCombo();
void checkCombo();
void triggerMysteryAward();
void startMode(Mode newMode);
void updateMode();
void completeMode();
void startMiniWizard();
void startWizardMode();
void resetGameState();
void scorePoints(int pts);

#endif
