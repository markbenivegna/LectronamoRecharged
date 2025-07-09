// RPU.cpp - Lectronamo Recharged
// Core gameplay logic for updated Lectronamo ruleset using Retro Pin Upgrade

#include "RPU.h"

// Global Variables
GameState game;

void setupGame() {
  game.reset();
  setupLamps();
  setupSounds();
  setupModes();
}

void GameState::reset() {
  ballSaveTimer = 0;
  skillShotAvailable = true;
  mysteryAvailable = false;
  for (int i = 0; i < MODE_COUNT; i++) {
    modeActive[i] = false;
    modeCompleted[i] = false;
  }
  wizardReady = false;
  comboActive = false;
  rightInlaneTime = 0;
  drop3xHits = 0;
  dropsReset = true;
  bonusMultiplier = 1;
  bonusHeld = false;
}

// --------------------------- INPUT SWITCHES ---------------------------

void onSwitchHit(uint8_t sw) {
  switch (sw) {
    case SW_START:
      startGame();
      break;
    case SW_SHOOTER:
      lightSkillShot();
      break;
    case SW_SAUCER:
      processSaucer();
      break;
    case SW_RIGHT_INLANE:
      activateCombo();
      break;
    case SW_25:
      checkComboToSaucer();
      break;
    case SW_STANDUP_12:
      handleModeTrigger();
      break;
    case SW_DROP_3BANK_1:
    case SW_DROP_3BANK_2:
    case SW_DROP_3BANK_3:
      handle3BankDrop(sw);
      break;
    case SW_DROP_5BANK_1:
    case SW_DROP_5BANK_5:
      handle5BankDrop(sw);
      break;
    case SW_POP:
      handlePopHit();
      break;
    case SW_LEFT_KICKER:
      collectBonus();
      break;
  }
}

// --------------------------- SKILL SHOT ---------------------------

void lightSkillShot() {
  game.skillShotAvailable = true;
  setLamp(LMP_SAUCER, true);
}

void processSaucer() {
  if (game.skillShotAvailable) {
    awardPoints(5000); // or configurable value
    playSound(SND_SKILL_SHOT);
    game.skillShotAvailable = false;
  }
  else if (game.comboActive && millis() - game.rightInlaneTime < COMBO_WINDOW_MS) {
    awardComboPoints();
    triggerMystery();
  }
  else if (game.mysteryAvailable) {
    triggerMystery();
  }
  setLamp(LMP_SAUCER, false);
}

// --------------------------- COMBO LOGIC ---------------------------

void activateCombo() {
  game.comboActive = true;
  game.rightInlaneTime = millis();
  setLamp(LMP_COMBO, true);
}

void checkComboToSaucer() {
  if (game.comboActive && millis() - game.rightInlaneTime < COMBO_WINDOW_MS) {
    // Waiting for saucer hit
  } else {
    game.comboActive = false;
    setLamp(LMP_COMBO, false);
  }
}

void awardComboPoints() {
  awardPoints(COMBO_POINTS);
  playSound(SND_COMBO);
  setLamp(LMP_COMBO, false);
  game.comboActive = false;
  game.mysteryAvailable = true;
}

// --------------------------- MYSTERY ---------------------------

void triggerMystery() {
  int roll = random(100);
  if (roll < 5) awardExtraBall();
  else if (roll < 30) advancePops();
  else if (roll < 35) awardPoints(25000);
  else if (roll < 95) awardPoints(100);
  else if (roll < 97) advanceBonusMultiplier();
  else if (roll < 99) game.bonusHeld = true;
  else endGameByTilt();
  game.mysteryAvailable = false;
}

// --------------------------- MODES ---------------------------

void handleModeTrigger() {
  static uint8_t modeHits = 0;
  modeHits++;
  if (modeHits == 2) activateRandomMode();
  if (modeHits == 4 && game.wizardReady) startWizardMode();
}

void activateRandomMode() {
  int index;
  do {
    index = random(MODE_COUNT);
  } while (game.modeCompleted[index]);
  game.modeActive[index] = true;
  startMode(index);
}

void completeMode(uint8_t mode) {
  game.modeActive[mode] = false;
  game.modeCompleted[mode] = true;
  checkWizardEligibility();
}

void checkWizardEligibility() {
  for (int i = 0; i < MODE_COUNT; i++) {
    if (!game.modeCompleted[i]) return;
  }
  game.wizardReady = true;
}

// --------------------------- PLACEHOLDER ---------------------------

void setupLamps() {}
void setupSounds() {}
void setupModes() {}
void startGame() {}
void playSound(uint8_t sound) {}
void awardPoints(uint16_t value) {}
void awardExtraBall() {}
void advancePops() {}
void advanceBonusMultiplier() {}
void collectBonus() {}
void endGameByTilt() {}
void startMode(uint8_t mode) {}
void startWizardMode() {}
void handle3BankDrop(uint8_t sw) {}
void handle5BankDrop(uint8_t sw) {}
void handlePopHit() { awardPoints(100); }