// LectronamoRecharged.ino
// Main Arduino Sketch for Lectronamo Recharged (RPU Format, Stern MPU-100 Compatible)

#include "RPU.h"
#include "Game.h"

LectronamoRecharged game; // Instantiate the global game object

void setup() {
  // Initialize hardware, lamps, sounds, and state
  RPU_InitializeMPU();
  game.Initialize();
}

void loop() {
  // RPU.Update() scans switches/lamps and manages hardware I/O
  RPU_Update(); 

  // game.HandleSwitches() processes the switch events from the RPU
  game.HandleSwitches();

  // game.GameFlowUpdate() runs the main state machine (attract, game over, etc.)
  game.GameFlowUpdate();

  // Allow a brief delay to prevent overwhelming the CPU
  delay(1);
}
