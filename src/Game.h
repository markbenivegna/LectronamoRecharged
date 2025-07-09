#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "HardwareMap.h"
#include "Adjustments.h"
#include "RPU.h"

// Define Game States
enum GameState {
    ATTRACT_MODE,
    GAME_START,
    BALL_IN_PLAY,
    BONUS_COUNT,
    GAME_OVER
};

class LectronamoRecharged {
public:
    void Initialize();
    void AdvanceState();
    void HandleSwitches();
    void UpdateLamps();
    void UpdateDisplay();
    void AddBonus(int value);
    void HandleSaucer();
    void DrainBall();
    GameState GetGameState() const;

private:
    GameState gameState;
    int currentBonus;
    int bonusMultiplier;
    bool extraBallLit;
    bool saucerLocked;
};

extern LectronamoRecharged game;

#endif // GAME_H
