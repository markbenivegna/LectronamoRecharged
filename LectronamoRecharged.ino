// LectronamoRecharged.ino
// Main Arduino Sketch for Lectronamo Recharged (RPU Format, Stern MPU-100 Compatible)

#include "RPU.h"
#include "Lectronamo.h"

//================================================================
// I. GLOBAL GAME VARIABLES
//================================================================
// These variables are declared as 'extern' in Lectronamo.h
GameState gameState;
long currentScore;
int currentBonus;
int bonusMultiplier;
bool extraBallLit;
unsigned long ballSaveStartTime;
bool isBallSaveActive;
bool firstHitMade;
unsigned long arcSurgeTimerStart;
bool arcSurgeActive;
int attractPhase;
int attractStep;
unsigned long selfTestTimerStart;
int selfTestPressCount;
bool inAuditMode;
int currentTestMode;
int threeBankCompleteCount;
bool threeTargetsDown[3];
int fiveBankCompleteCount;
bool fiveTargetsDown[5];
int spinnerHitCount;
bool holdBonus;
int extraBalls;
int ball;
int player;
int ballsPerGame;
bool thumperScoreIs1000;
bool specialAwardedThisBall;
unsigned long highScore;
int credits;

//================================================================
// II. RPU HARDWARE MAPPING
//================================================================
#define NUM_SWITCHES 9
#define NUM_PRIORITY_SWITCHES 0

PlayfieldAndCabinetSwitch gameSwitchArray[NUM_SWITCHES] = {
    // { Switch ID, Solenoid ID, Hold Time }
    { SW_TILT, CONTSOL_DISABLE_FLIPPERS, 0 },
    { SW_SLAM_TILT, CONTSOL_DISABLE_FLIPPERS, 0 },
    { SW_RIGHT_SLINGSHOT, SOL_RIGHT_SLINGSHOT, 4 },
    { SW_LEFT_SLINGSHOT, SOL_LEFT_SLINGSHOT, 4 },
    { SW_THUMPER_CENTER, SOL_CENTER_THUMPER, 6 },
    { SW_THUMPER_RIGHT, SOL_RIGHT_THUMPER, 6 },
    { SW_THUMPER_LEFT, SOL_LEFT_THUMPER, 6 },
    { SW_OUTHOLE, SOL_OUTHOLE, 8 },
    { SW_SAUCER, SOL_SAUCER, 8 }
};

//================================================================
// III. CORE RPU CALLBACKS & SETUP
//================================================================

void SaveHighScore() {
    if (currentScore > highScore) {
        highScore = currentScore;
        RPU_WriteULToEEProm(ADDR_HIGH_SCORE, highScore);
    }
}

void DrainBall(bool isTilted = false) {
    if (isTilted) { 
        currentBonus = 0; 
    }
    SaveHighScore();
    gameState = BONUS_COUNT;
}

void RPU_Callback_GameLogic() {
    // This function is called repeatedly by RPU_loop()
    // It replaces the old HandleSwitches() and GameFlowUpdate() methods.
    
    byte switchHit;
    while ((switchHit = RPU_PullFirstFromSwitchStack()) != NO_SWITCH_HIT) {
        // --- Coin Switch Logic ---
        bool isCoinSwitch = (switchHit == SW_COIN_1 || switchHit == SW_COIN_2 || switchHit == SW_COIN_3);

        if (isCoinSwitch && gameState == ATTRACT_MODE) {
            byte s17 = RPU_ReadByteFromEEProm(ADDR_MAX_CREDITS_17);
            byte s18 = RPU_ReadByteFromEEProm(ADDR_MAX_CREDITS_18);
            byte s19 = RPU_ReadByteFromEEProm(ADDR_MAX_CREDITS_19);
            
            // Per manual pg. 9, 000=8, 001=10, 010=15, 011=25
            int maxCredits = 8;
            if (s17 == 0 && s18 == 0 && s19 == 1) maxCredits = 10;
            else if (s17 == 0 && s18 == 1 && s19 == 0) maxCredits = 15;
            else if (s17 == 0 && s18 == 1 && s19 == 1) maxCredits = 25;

            if (credits < maxCredits) {
                credits++;
                // RPU_PlaySound(SND_COIN_IN); // Example
            }
        }

        // --- Start Button Logic ---
        if (switchHit == SW_CREDIT_BUTTON && credits > 0 && (gameState == ATTRACT_MODE || gameState == GAME_OVER)) {
            // Start a new game
            credits--;
            gameState = GAME_START;
            // RPU_PlaySound(SND_GAME_START); // Example
            return; // Exit to allow game start logic to run on the next loop
        }

        // --- In-Game Playfield Switch Logic ---
        if (gameState != BALL_IN_PLAY) continue; // Ignore playfield switches if not in a game
        
        switch (switchHit) {
            case SW_OUTHOLE:
                DrainBall();
                break;
            // Add other scoring cases from your ruleset here...
        }
    }
    // Add main game flow logic (attract mode, game over sequences, etc.) here
}

void setup() {
  RPU_init(RPU_Callback_GameLogic);

  // --- One-Time Game Initialization (from LectronamoRecharged::Initialize) ---
  highScore = RPU_ReadULFromEEProm(ADDR_HIGH_SCORE, 100000); // Load high score with a default of 100,000
  byte ballsPerGameSetting = RPU_ReadByteFromEEProm(ADDR_BALLS_PER_GAME);
  ballsPerGame = (ballsPerGameSetting == 1) ? 5 : 3;
  
  RPU_SetupGameSwitches(NUM_SWITCHES, NUM_PRIORITY_SWITCHES, gameSwitchArray);
  
  // Reset all game state variables
  gameState = ATTRACT_MODE;
  credits = 0;
  currentScore = 0;
  currentBonus = 0;
  bonusMultiplier = 1;
  extraBallLit = false;
  isBallSaveActive = false;
  firstHitMade = false;
  arcSurgeActive = false;
  inAuditMode = false;
  threeBankCompleteCount = 0;
  fiveBankCompleteCount = 0;
  spinnerHitCount = 0;
  holdBonus = false;
  extraBalls = 0;
  thumperScoreIs1000 = false;
  specialAwardedThisBall = false;
}

void loop() {
  RPU_loop();
}
