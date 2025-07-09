// Lectronamo2025.ino
// Main Arduino Sketch for Lectronamo Recharged (Minimalist RPU Format)

#include "GameRules.h"

void setup() {
  // Initialize hardware, lamps, sounds, and state
  initializeGame();
}

void loop() {
  // Poll switches and update game state
  processSwitches();

  // Run game logic and modes
  updateGameLogic();

  // Refresh lamp states
  updateLamps();

  // Refresh solenoid state
  updateSolenoids();

  // Delay or yield CPU cycle
  delay(5);  // Small delay to control loop timing
}
