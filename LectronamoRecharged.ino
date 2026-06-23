/**************************************************************************
    This pinball code is distributed in the hope that it
    will be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <https://www.gnu.org/licenses/>.
*/

#include "RPU_Config.h"
#include "RPU.h"
#include "DropTargets.h"
#include "Lectronamo.h"
#include "OperatorMenus.h"
#include "AudioHandler.h"
#include "DisplayHandler.h"
#include "LampAnimations.h"
#include "SoundSequences.h"
#include <EEPROM.h>

// switch test not showing switches in player displays
// player up lamps versus number of player lamps

#define GAME_MAJOR_VERSION  2026
#define GAME_MINOR_VERSION  1
#define DEBUG_MESSAGES  1
#define DEBUG_SWITCH_LOGGING  1

#if (DEBUG_MESSAGES==1)
#define DEBUG_SHOW_LOOPS_PER_SECOND
#endif

#ifdef RPU_SIMPLIFY_DISPLAY_FOR_7VOLUTION
#define DISPLAY_DASH_STYLE  DISPLAY_DASH_INTERMITTENT_FLASH
#else
#define DISPLAY_DASH_STYLE  DISPLAY_DASH_ROLLING_BLANK
#endif

/*********************************************************************

    Game specific code

*********************************************************************/

// MachineState
//  0 - Attract Mode
//  negative - no longer used
//  positive - game play
boolean InOperatorMenu = false;
char MachineState = 0;
boolean MachineStateChanged = true;
#define MACHINE_STATE_ATTRACT         0
#define MACHINE_STATE_INIT_GAMEPLAY   1
#define MACHINE_STATE_INIT_NEW_BALL   2
#define MACHINE_STATE_NORMAL_GAMEPLAY 4
#define MACHINE_STATE_COUNTDOWN_BONUS 99
#define MACHINE_STATE_BALL_OVER       100
#define MACHINE_STATE_MATCH_MODE      110
#define MACHINE_STATE_DIAGNOSTICS     120

// Indices of EEPROM save locations
#define EEPROM_RPOS_INIT_PROOF_UL                 90

// This value needs to be set to a UNIQUE value for the 
// game code. 
#define RPOS_INIT_PROOF                           0x454D3031
#define EEPROM_BALL_SAVE_BYTE                     100
#define EEPROM_FREE_PLAY_BYTE                     101
#define EEPROM_TILT_WARNING_BYTE                  104
#define EEPROM_AWARD_OVERRIDE_BYTE                105
#define EEPROM_BALLS_OVERRIDE_BYTE                106
#define EEPROM_TOURNAMENT_SCORING_BYTE            107
#define EEPROM_SFX_VOLUME_BYTE                    108
#define EEPROM_MUSIC_VOLUME_BYTE                  109
#define EEPROM_SCROLLING_SCORES_BYTE              110
#define EEPROM_CALLOUTS_VOLUME_BYTE               111
#define EEPROM_CRB_HOLD_TIME                      118
#define EEPROM_TROUGH_EJECT_STRENGTH              131
#define EEPROM_SAUCER_EJECT_STRENGTH              134
#define EEPROM_SLINGSHOT_STRENGTH                 135 
#define EEPROM_POP_BUMPER_STRENGTH                136
#define EEPROM_GAME_RULES_SELECTION               137
#define EEPROM_MATCH_FEATURE_BYTE                 138
#define EEPROM_EXTRA_BALL_SCORE_UL                160
#define EEPROM_SPECIAL_SCORE_UL                   164
#define EEPROM_SAUCER_LIGHT_PERSISTENCE           180
#define EEPROM_HIGHSCORE_REPLAY_AWARD             181
#define EEPROM_SPECIAL_OPEN_ENDED                 182
#define EEPROM_BONUS_COUNTDOWN_MULTIPLE_STEPS     183
#define EEPROM_EXTRA_BALL_LANE_ENABLED            184
#define EEPROM_SPECIAL_AWARD_TYPE                 185



#define GAME_MODE_SKILL_SHOT                        1
#define GAME_MODE_UNSTRUCTURED_PLAY                 2
//#define GAME_MODE_EXAMPLE_1                         3


#define SOUND_EFFECT_NONE                     0
#define SOUND_EFFECT_BONUS_COUNT              1
#define SOUND_EFFECT_SPINNER                  2
#define SOUND_EFFECT_TILT                     3
#define SOUND_EFFECT_TILT_WARNING             4
#define SOUND_EFFECT_SCORE_TICK               5
#define SOUND_EFFECT_POP_BUMPER               6
#define SOUND_EFFECT_LEFT_SLING               7
#define SOUND_EFFECT_RIGHT_SLING              8
#define SOUND_EFFECT_SLINGSHOT                16
#define SOUND_EFFECT_GAME_OVER                21
#define SOUND_EFFECT_MATCH_SPIN               28
#define SOUND_EFFECT_DROP_TARGET_SOUND_1      30
#define SOUND_EFFECT_DROP_TARGET_SOUND_2      31
#define SOUND_EFFECT_DROP_TARGET_SOUND_3      32
#define SOUND_EFFECT_DROP_TARGET_SOUND_4      33
#define SOUND_EFFECT_DROP_TARGET_SOUND_5      34
#define SOUND_EFFECT_DROP_TARGET_SOUND_6      35
#define SOUND_EFFECT_DROP_TARGET_SOUND_7      36
#define SOUND_EFFECT_DROP_TARGET_SOUND_8      37
#define SOUND_EFFECT_DROP_TARGET_COMPLETE     40
#define SOUND_EFFECT_BONUS_1                  81
#define SOUND_EFFECT_BONUS_2                  82
#define SOUND_EFFECT_BONUS_3                  83
#define SOUND_EFFECT_BONUS_4                  84
#define SOUND_EFFECT_BONUS_5                  85
#define SOUND_EFFECT_BONUS_6                  86
#define SOUND_EFFECT_BONUS_7                  87
#define SOUND_EFFECT_BONUS_8                  88
#define SOUND_EFFECT_STARTUP_1                100
#define SOUND_EFFECT_STARTUP_2                101

#define SOUND_EFFECT_COIN_DROP_1              105
#define SOUND_EFFECT_COIN_DROP_2              106
#define SOUND_EFFECT_COIN_DROP_3              107

#define SOUND_EFFECT_BACKGROUND_SONG_1                400
#define SOUND_EFFECT_BACKGROUND_SONG_2                401
#define SOUND_EFFECT_BACKGROUND_SONG_3                402
#define SOUND_EFFECT_BACKGROUND_SONG_4                403
#define SOUND_EFFECT_BACKGROUND_SONG_5                404
#define SOUND_EFFECT_BACKGROUND_SONG_6                405
#define SOUND_EFFECT_BACKGROUND_SONG_7                406
#define SOUND_EFFECT_BALL_MUSIC_1                     450
#define SOUND_EFFECT_BALL_MUSIC_2                     451
#define SOUND_EFFECT_BALL_MUSIC_3                     452
#define SOUND_EFFECT_BALL_MUSIC_4                     453
#define SOUND_EFFECT_BALL_MUSIC_5                     454


// Game play status callouts
#define SOUND_EFFECT_VP_PLAYER_1_UP                   301
#define SOUND_EFFECT_VP_PLAYER_2_UP                   302
#define SOUND_EFFECT_VP_PLAYER_3_UP                   303
#define SOUND_EFFECT_VP_PLAYER_4_UP                   304
#define SOUND_EFFECT_VP_EXTRA_BALL                    305

#define SOUND_EFFECT_VP_ADD_PLAYER_1        306
#define SOUND_EFFECT_VP_ADD_PLAYER_2        (SOUND_EFFECT_VP_ADD_PLAYER_1+1)
#define SOUND_EFFECT_VP_ADD_PLAYER_3        (SOUND_EFFECT_VP_ADD_PLAYER_1+2)
#define SOUND_EFFECT_VP_ADD_PLAYER_4        (SOUND_EFFECT_VP_ADD_PLAYER_1+3)
#define SOUND_EFFECT_VP_SHOOT_AGAIN         310

#define SOUND_EFFECT_VP_BALL_SAVE                       326

#define SOUND_EFFECT_DIAG_START                   1900
#define SOUND_EFFECT_DIAG_CREDIT_RESET_BUTTON     1900
#define SOUND_EFFECT_DIAG_SELECTOR_SWITCH_ON      1901
#define SOUND_EFFECT_DIAG_SELECTOR_SWITCH_OFF     1902
#define SOUND_EFFECT_DIAG_STARTING_ORIGINAL_CODE  1903
#define SOUND_EFFECT_DIAG_STARTING_NEW_CODE       1904
#define SOUND_EFFECT_DIAG_ORIGINAL_CPU_DETECTED   1905
#define SOUND_EFFECT_DIAG_ORIGINAL_CPU_RUNNING    1906
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_U10         1907
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_U11         1908
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_1           1909
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_2           1910
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_3           1911
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_4           1912
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_5           1913
#define SOUND_EFFECT_DIAG_STARTING_DIAGNOSTICS    1914


#define MAX_DISPLAY_BONUS     19
#define TILT_WARNING_DEBOUNCE_TIME      1000

#define BALL_SAVE_GRACE_PERIOD            3000

/*********************************************************************

    Machine state and options

*********************************************************************/

// Lamp state structure for bonus multiplier encoding
struct LampState {
  byte lamp2X;
  byte lamp3X;
  byte lamp5X;
};

byte Credits = 0;
byte BallSaveNumSeconds = 0;
byte MaximumCredits = 40;
byte BallsPerGame = 3;
byte ScoreAwardReplay = 0;
byte MusicVolume = 6;
byte SoundEffectsVolume = 8;
byte CalloutsVolume = 10;
byte ChuteCoinsInProgress[3];
byte TotalBallsLoaded = 1;
byte TimeRequiredToResetGame = 1;
boolean FreePlayMode = false;
boolean MatchEnabled = true;
byte HighGameFreeGames = 3;
boolean MatchFeature = true;
boolean TournamentScoring = false;
boolean ScrollingScores = true;
unsigned long ExtraBallValue = 0;
unsigned long SpecialValue = 0;
unsigned long CurrentTime = 0;
unsigned long HighScore = 0;
unsigned long AwardScores[3];
unsigned long CreditResetPressStarted = 0;
unsigned long OperatorSwitchPressStarted = 0;

#define NUM_CPC_PAIRS 9
boolean CPCSelectionsHaveBeenRead = false;
byte CPCPairs[NUM_CPC_PAIRS][2] = {
  {1, 5},
  {1, 4},
  {1, 3},
  {1, 2},
  {1, 1},
  {2, 3},
  {2, 1},
  {3, 1},
  {4, 1}
};
byte CPCSelection[3];

AudioHandler  Audio;
OperatorMenus Menus;

// Solenoid/switch pairs for RPU framework
struct PlayfieldAndCabinetSwitch SolenoidAssociatedSwitches[] = {
  { SW_RIGHT_SLING, SOL_RIGHT_SLINGSHOT, 4},
  { SW_LEFT_SLING,  SOL_LEFT_SLINGSHOT,  4},
  { SW_BOTTOM_POP,  SOL_CENTER_THUMPER,  6},
  { SW_RIGHT_POP,   SOL_RIGHT_THUMPER,   6},
  { SW_LEFT_POP,    SOL_LEFT_THUMPER,    6}
};



/*********************************************************************

    Game State

*********************************************************************/
byte CurrentPlayer = 0;
byte CurrentBallInPlay = 1;
byte CurrentNumPlayers = 0;
byte NumberOfBallsLocked;
byte NumberOfBallsInPlay;
byte Bonus[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte BonusX[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte GameMode = GAME_MODE_SKILL_SHOT;
byte LastGameMode = 0;
byte MaxTiltWarnings = 2;
byte NumTiltWarnings = 0;
byte CurrentAchievements[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte NumberOfBallSavesRemaining;

boolean SamePlayerShootsAgain = false;
boolean BallSaveUsed = false;
boolean SpecialAvailable = false;
boolean SpecialCollected = false;
boolean TimersPaused = true;
boolean CollectBonusViaKicker = false;
boolean KickerBonusCollect = false;
boolean SaucerLightPersists = true;
boolean SpecialOpenEnded = false;
boolean BonusCountdownMultipleSteps = false;
unsigned long BonusCollectionEndTime = 0;
boolean ExtraBallLaneEnabled = true;
byte SpecialAwardType = 2;

unsigned long CurrentScores[RPU_NUMBER_OF_PLAYERS_ALLOWED];

// Pending score updates - synced with audio playback
struct PendingScoreUpdate {
  byte playerNum;
  unsigned long scoreIncrement;      // Amount to add per tone
  unsigned long nextUpdateTime;      // When the next increment should apply
  unsigned long updateIntervalMs;    // Gap between tones
  byte remainingIncrements;          // How many tones are left
  boolean isActive;
};
#define MAX_PENDING_SCORE_UPDATES 4   // One per player at a time
PendingScoreUpdate PendingScores[MAX_PENDING_SCORE_UPDATES];

unsigned long BallFirstSwitchHitTime = 0;
unsigned long BallTimeInTrough = 0;
unsigned long GameModeStartTime = 0;
unsigned long GameModeEndTime = 0;
unsigned long LastTiltWarningTime;
unsigned long PlayfieldMultiplier;
unsigned long LastTimeThroughLoop;
unsigned long LastSwitchHitTime;
unsigned long BallSaveEndTime;
unsigned long SaucerClosedStart = 0;
unsigned long KickerClosedStart = 0;
unsigned long KickerEjectTime = 0;
unsigned long KickerSwitchReleaseTime = 0;

#define STUCK_BALL_SETTLE_TIME_MS 500
#define KICKER_EJECT_LOCKOUT_MS 1000
#define KICKER_RELEASE_DEBOUNCE_MS 100

/*********************************************************************

    Game Specific State Variables

*********************************************************************/
#define SCORE_SPINNER_BASE      100
#define SCORE_SPINNER_LIT       1000
#define SCORE_DROP_TARGET_BASE  500
#define SCORE_STANDUP_TARGET    5000
#define SCORE_3BANK_COMPLETION  6000
#define SCORE_3BANK_SWEEP_BONUS 10000
#define TIME_3BANK_SWEEP_WINDOW_MS 1500
#define SCORE_5BANK_COMPLETION  10000
#define SCORE_OUTLANE           3000
#define SCORE_ARC_SURGE_T1      10000
#define SCORE_ARC_SURGE_SUPER   50000
#define SCORE_SKILL_SHOT        5000

const uint16_t TIME_MATCH_SEQUENCE_MS = 3000;
const uint16_t TIME_BALL_SAVE_DURATION_MS = 15000;
const uint16_t TIME_ARC_SURGE_COMBO_MS = 8000;

const byte ATTRACT_PHASE_1_CLASSIC_FLOW = 1;
const byte ATTRACT_PHASE_2_ARC_SURGE = 2;
const byte ATTRACT_PHASE_3_WAVE = 3;

byte ExtraBallsAvailable[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte GameRulesSelection;
byte BallServeSolenoidStrength = 3;
byte SaucerSolenoidStrength = 4;
byte TempSlingStrength = 4;
byte TempPopStrength = 4;
unsigned long ThumperEnableTime = 0;

#define GAME_RULES_EASY         1
#define GAME_RULES_MEDIUM       2
#define GAME_RULES_HARD         3
#define GAME_RULES_PROGRESSIVE  4
#define GAME_RULES_CUSTOM       5

unsigned long PlayfieldMultiplierTimeLeft;
unsigned long BonusChangedTime;
unsigned long BonusXAnimationStart;
unsigned long BonusAnimationStart;
byte BonusBefore;
unsigned long SkillShotAnimationStart;
unsigned long protectedSoundUntilTime = 0;  // Grace period for protected sequences (drop targets, drain, etc.)
unsigned long LastTimeBallServed;
unsigned long LastSpinnerHitTime = 0;

// Per-player game state (indexed by player number 0-3)
boolean isSaucerLit[RPU_NUMBER_OF_PLAYERS_ALLOWED];
boolean isArcSurgeActive[RPU_NUMBER_OF_PLAYERS_ALLOWED];
boolean arcSurgeT1Hit[RPU_NUMBER_OF_PLAYERS_ALLOWED];
boolean isLeftReturnLaneLit[RPU_NUMBER_OF_PLAYERS_ALLOWED];
boolean ExtraBallCollectedThisBall[RPU_NUMBER_OF_PLAYERS_ALLOWED];
boolean ExtraBallLaneAvailable[RPU_NUMBER_OF_PLAYERS_ALLOWED];
boolean firstHitMade[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte threeBankCompleteCount[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte fiveBankCompleteCount[RPU_NUMBER_OF_PLAYERS_ALLOWED];
byte spinnerHitCount[RPU_NUMBER_OF_PLAYERS_ALLOWED];
unsigned long arcSurgeTimerStart[RPU_NUMBER_OF_PLAYERS_ALLOWED];
unsigned long arcSurgeCompleteTime = 0;
unsigned long threeBankSweepStartTime[RPU_NUMBER_OF_PLAYERS_ALLOWED];
unsigned long threeBankSweepAnimationStart[RPU_NUMBER_OF_PLAYERS_ALLOWED];

DropTargetBank ThreeBank(3, 1, DROP_TARGET_TYPE_STRN_1, 8);
DropTargetBank FiveBank(5, 1, DROP_TARGET_TYPE_STRN_1, 8);

#define DROP_TARGET_RESET_STRENGTH    10
#define KNOCKER_SOLENOID_STRENGTH     5

byte PlayerUpLamps[4] = {LAMP_HEAD_PLAYER_1_UP, LAMP_HEAD_PLAYER_2_UP, LAMP_HEAD_PLAYER_3_UP, LAMP_HEAD_PLAYER_4_UP};


/******************************************************

   Adjustments Serialization

*/


void SetAllParameterDefaults() {

  // In the event that the EEPROM has not been initialized,
  // these are the values that will be used
  HighScore = 10000;
  Credits = 4;
  FreePlayMode = false;
  BallSaveNumSeconds = 5;
  MusicVolume = 10;
  SoundEffectsVolume = 10;
  CalloutsVolume = 10;
  AwardScores[0] = 100000;
  AwardScores[1] = 200000;
  AwardScores[2] = 350000;
  TournamentScoring = false;
  MaxTiltWarnings = 2;
  ScoreAwardReplay = 0x00;     // no replays — free play home machine
  BallsPerGame = 3;
  ScrollingScores = true;
  MatchFeature = true;
  ExtraBallValue = 20000;
  SpecialValue = 50000;
  TimeRequiredToResetGame = 2;
  CPCSelection[0] = 4;
  CPCSelection[1] = 4;
  CPCSelection[2] = 4;

  SaucerLightPersists = true;
  HighGameFreeGames = 3;
  SpecialOpenEnded = false;
  BonusCountdownMultipleSteps = false;
  ExtraBallLaneEnabled = true;
  SpecialAwardType = 2;

  // EASY / MEDIUM / HARD rules
  GameRulesSelection = GAME_RULES_MEDIUM;
}


boolean LoadRuleDefaults(byte ruleLevel) {
  if (ruleLevel==GAME_RULES_EASY) {
    BallSaveNumSeconds = 10;
    ExtraBallLaneEnabled = true;
    SpecialOpenEnded = true;
    SpecialAwardType = 0;        // points (home use)
  } else if (ruleLevel==GAME_RULES_MEDIUM) {
    BallSaveNumSeconds = 5;
    ExtraBallLaneEnabled = true;
    SpecialOpenEnded = false;
    SpecialAwardType = 0;        // points (home use)
  } else if (ruleLevel==GAME_RULES_HARD) {
    BallSaveNumSeconds = 0;
    ExtraBallLaneEnabled = false;
    SpecialOpenEnded = false;
    SpecialAwardType = 0;        // points (home use)
  } else {
    return false;
  }

  return true;
}


void WriteParameters(boolean onlyWriteRulesParameters = true) {
  if (!onlyWriteRulesParameters) {
    RPU_WriteULToEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, HighScore);
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    RPU_WriteByteToEEProm(RPU_CPC_CHUTE_1_SELECTION_BYTE, CPCSelection[0]);
    RPU_WriteByteToEEProm(RPU_CPC_CHUTE_2_SELECTION_BYTE, CPCSelection[1]);
    RPU_WriteByteToEEProm(RPU_CPC_CHUTE_3_SELECTION_BYTE, CPCSelection[2]);
        
    RPU_WriteByteToEEProm(EEPROM_FREE_PLAY_BYTE, FreePlayMode);
    RPU_WriteByteToEEProm(EEPROM_MUSIC_VOLUME_BYTE, MusicVolume);
    RPU_WriteByteToEEProm(EEPROM_SFX_VOLUME_BYTE, SoundEffectsVolume);
    RPU_WriteByteToEEProm(EEPROM_CALLOUTS_VOLUME_BYTE, CalloutsVolume);

    RPU_WriteULToEEProm(RPU_AWARD_SCORE_1_EEPROM_START_BYTE, AwardScores[0]);
    RPU_WriteULToEEProm(RPU_AWARD_SCORE_2_EEPROM_START_BYTE, AwardScores[1]);
    RPU_WriteULToEEProm(RPU_AWARD_SCORE_3_EEPROM_START_BYTE, AwardScores[2]);

    RPU_WriteByteToEEProm(EEPROM_TOURNAMENT_SCORING_BYTE, TournamentScoring);
    RPU_WriteByteToEEProm(EEPROM_AWARD_OVERRIDE_BYTE, ScoreAwardReplay);
    RPU_WriteByteToEEProm(EEPROM_BALLS_OVERRIDE_BYTE, BallsPerGame);
    RPU_WriteByteToEEProm(EEPROM_SCROLLING_SCORES_BYTE, ScrollingScores);
    RPU_WriteByteToEEProm(EEPROM_MATCH_FEATURE_BYTE, MatchFeature);
    
    RPU_WriteULToEEProm(EEPROM_EXTRA_BALL_SCORE_UL, ExtraBallValue);
    RPU_WriteULToEEProm(EEPROM_SPECIAL_SCORE_UL, SpecialValue);
    RPU_WriteByteToEEProm(EEPROM_CRB_HOLD_TIME, TimeRequiredToResetGame);
    RPU_WriteByteToEEProm(EEPROM_SAUCER_LIGHT_PERSISTENCE, SaucerLightPersists);
    RPU_WriteByteToEEProm(EEPROM_HIGHSCORE_REPLAY_AWARD, HighGameFreeGames);
    RPU_WriteByteToEEProm(EEPROM_SPECIAL_OPEN_ENDED, SpecialOpenEnded);
    RPU_WriteByteToEEProm(EEPROM_BONUS_COUNTDOWN_MULTIPLE_STEPS, BonusCountdownMultipleSteps);
    RPU_WriteByteToEEProm(EEPROM_EXTRA_BALL_LANE_ENABLED, ExtraBallLaneEnabled);
    RPU_WriteByteToEEProm(EEPROM_SPECIAL_AWARD_TYPE, SpecialAwardType);

    // Set baseline for audits
    RPU_WriteByteToEEProm(RPU_CHUTE_1_COINS_START_BYTE, 0);
    RPU_WriteByteToEEProm(RPU_CHUTE_2_COINS_START_BYTE, 0);
    RPU_WriteByteToEEProm(RPU_CHUTE_3_COINS_START_BYTE, 0);
    RPU_WriteULToEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE, 0);
    RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, 0);
    RPU_WriteULToEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE, 0);
  }

  RPU_WriteByteToEEProm(EEPROM_BALL_SAVE_BYTE, BallSaveNumSeconds);
  RPU_WriteByteToEEProm(EEPROM_TILT_WARNING_BYTE, MaxTiltWarnings); 
  
}

void ReadStoredParameters() {
  for (byte count = 0; count < 3; count++) {
    ChuteCoinsInProgress[count] = 0;
  }

  // The first time the EEPROM has been written with good values for this game,
  // the EEPROM_RPOS_INIT_PROOF_UL will be written to a known state (RPOS_INIT_PROOF)
  // if that value hasn't been written, then we load defaults and save them to EEPROM.
  // This should only happen the first time a device is run with this game code.
  unsigned long RPUProofValue = RPU_ReadULFromEEProm(EEPROM_RPOS_INIT_PROOF_UL, 0);
  if (RPUProofValue!=RPOS_INIT_PROOF) {
    // Doesn't look like this memory has been initialized
    RPU_WriteULToEEProm(EEPROM_RPOS_INIT_PROOF_UL, RPOS_INIT_PROOF);
    SetAllParameterDefaults();
    WriteParameters(false);
  } else {

    // Read machine settings
    HighScore = RPU_ReadULFromEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, 10000);
    Credits = RPU_ReadByteFromEEProm(RPU_CREDITS_EEPROM_BYTE);
    if (Credits > MaximumCredits) Credits = MaximumCredits;
  
    FreePlayMode = ReadSetting(EEPROM_FREE_PLAY_BYTE, true, true);
    MusicVolume = ReadSetting(EEPROM_MUSIC_VOLUME_BYTE, 10, 10);
    SoundEffectsVolume = ReadSetting(EEPROM_SFX_VOLUME_BYTE, 10, 10);
    CalloutsVolume = ReadSetting(EEPROM_CALLOUTS_VOLUME_BYTE, 10, 10);
    Audio.SetMusicVolume(MusicVolume);
    Audio.SetSoundFXVolume(SoundEffectsVolume);
    Audio.SetNotificationsVolume(CalloutsVolume);

    AwardScores[0] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_1_EEPROM_START_BYTE);
    AwardScores[1] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_2_EEPROM_START_BYTE);
    AwardScores[2] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_3_EEPROM_START_BYTE);
  
    TournamentScoring = ReadSetting(EEPROM_TOURNAMENT_SCORING_BYTE, false, true);
    ScoreAwardReplay = ReadSetting(EEPROM_AWARD_OVERRIDE_BYTE, 0x03, 0x07);
    BallsPerGame = ReadSetting(EEPROM_BALLS_OVERRIDE_BYTE, 3, 10);
    ScrollingScores = ReadSetting(EEPROM_SCROLLING_SCORES_BYTE, true, true);
    MatchFeature = ReadSetting(EEPROM_MATCH_FEATURE_BYTE, true, true);

    CPCSelection[0] = ReadSetting(RPU_CPC_CHUTE_1_SELECTION_BYTE, 4, 8);
    CPCSelection[1] = ReadSetting(RPU_CPC_CHUTE_2_SELECTION_BYTE, 4, 8);
    CPCSelection[2] = ReadSetting(RPU_CPC_CHUTE_3_SELECTION_BYTE, 4, 8);
    CPCSelectionsHaveBeenRead = true;

    ExtraBallValue = RPU_ReadULFromEEProm(EEPROM_EXTRA_BALL_SCORE_UL);
    if (ExtraBallValue % 1000 || ExtraBallValue > 1000000) ExtraBallValue = 20000;
  
    SpecialValue = RPU_ReadULFromEEProm(EEPROM_SPECIAL_SCORE_UL);
    if (SpecialValue % 1000 || SpecialValue > 1000000) SpecialValue = 40000;
  
    TimeRequiredToResetGame = ReadSetting(EEPROM_CRB_HOLD_TIME, 1, 99);
    if (TimeRequiredToResetGame > 3 && TimeRequiredToResetGame != 99) TimeRequiredToResetGame = 1;
    
    // Read game rules
    GameRulesSelection = ReadSetting(EEPROM_GAME_RULES_SELECTION, GAME_RULES_MEDIUM, GAME_RULES_CUSTOM);

    BallSaveNumSeconds = ReadSetting(EEPROM_BALL_SAVE_BYTE, 5, 20);
    MaxTiltWarnings = ReadSetting(EEPROM_TILT_WARNING_BYTE, 2, 3);
    TempSlingStrength = ReadSetting(EEPROM_SLINGSHOT_STRENGTH, 4, 8);
    TempPopStrength = ReadSetting(EEPROM_POP_BUMPER_STRENGTH, 4, 8);
    SolenoidAssociatedSwitches[0].solenoidHoldTime = TempSlingStrength;
    SolenoidAssociatedSwitches[1].solenoidHoldTime = TempSlingStrength;

    SaucerLightPersists = ReadSetting(EEPROM_SAUCER_LIGHT_PERSISTENCE, true, true);
    HighGameFreeGames = ReadSetting(EEPROM_HIGHSCORE_REPLAY_AWARD, 3, 3);
    SpecialOpenEnded = ReadSetting(EEPROM_SPECIAL_OPEN_ENDED, false, true);
    BonusCountdownMultipleSteps = ReadSetting(EEPROM_BONUS_COUNTDOWN_MULTIPLE_STEPS, false, true);
    ExtraBallLaneEnabled = ReadSetting(EEPROM_EXTRA_BALL_LANE_ENABLED, true, true);
    SpecialAwardType = ReadSetting(EEPROM_SPECIAL_AWARD_TYPE, 2, 3);

  }
}



byte GetCPCSelection(byte chuteNumber) {
  if (chuteNumber>2) return 0xFF;

  if (CPCSelectionsHaveBeenRead==false) {
    CPCSelection[0] = RPU_ReadByteFromEEProm(RPU_CPC_CHUTE_1_SELECTION_BYTE);
    if (CPCSelection[0]>=NUM_CPC_PAIRS) {
      CPCSelection[0] = 4;
      RPU_WriteByteToEEProm(RPU_CPC_CHUTE_1_SELECTION_BYTE, 4);
    }
    CPCSelection[1] = RPU_ReadByteFromEEProm(RPU_CPC_CHUTE_2_SELECTION_BYTE);  
    if (CPCSelection[1]>=NUM_CPC_PAIRS) {
      CPCSelection[1] = 4;
      RPU_WriteByteToEEProm(RPU_CPC_CHUTE_2_SELECTION_BYTE, 4);
    }
    CPCSelection[2] = RPU_ReadByteFromEEProm(RPU_CPC_CHUTE_3_SELECTION_BYTE);  
    if (CPCSelection[2]>=NUM_CPC_PAIRS) {
      CPCSelection[2] = 4;
      RPU_WriteByteToEEProm(RPU_CPC_CHUTE_3_SELECTION_BYTE, 4);
    }
    CPCSelectionsHaveBeenRead = true;
  }
  
  return CPCSelection[chuteNumber];
}


byte GetCPCCoins(byte cpcSelection) {
  if (cpcSelection>=NUM_CPC_PAIRS) return 1;
  return CPCPairs[cpcSelection][0];
}


byte GetCPCCredits(byte cpcSelection) {
  if (cpcSelection>=NUM_CPC_PAIRS) return 1;
  return CPCPairs[cpcSelection][1];
}


void QueueDIAGNotification(unsigned short notificationNum) {
  // This is optional, but the machine can play an audio message at boot
  // time to indicate any errors and whether it's going to boot to original
  // or new code.
  //Audio.QueuePrioritizedNotification(notificationNum, 0, 10, CurrentTime);
  (void)notificationNum;
}


// I'm doing this as a function instead of an array because 
// memory is short and spending 44 bytes on a converstion array
// seems wasteful when the board has tons and tons of code space.
// There's a way to store this data in code space and then convert
// it when needed, but that's slow compared to this (ugly) method.
byte LampConvertDisplayNumberToIndex(byte displayNumber) {
  if (displayNumber>60) return OPERATOR_MENU_VALUE_OUT_OF_RANGE;
  return displayNumber - 1;
}


byte SoundTestFunction(byte soundCommand) {
  Audio.PlaySound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  Audio.PlaySound(soundCommand, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  return 1;
}


byte DisplayTestFunction(byte testMode, byte digitPosition, byte numberToDisplay) {

  unsigned long allDigits = RPU_OS_MAX_DISPLAY_SCORE;
  allDigits /= 9;
  allDigits *= (unsigned long)numberToDisplay;
  for (byte count=0; count<4; count++) {
    RPU_SetDisplay(count, allDigits);
  }

  (void)testMode;
  (void)digitPosition;

  return 0; 
}


unsigned short SolenoidConvertDisplayNumberToIndex(byte displayNumber) {
  switch (displayNumber) {
    case  4: return SOL_DROP_TARGET_3BANK_RESET;
    case  5: return SOL_KNOCKER;
    case  6: return SOL_OUTHOLE;
    case  7: return SOL_SAUCER;
    case  8: return SOL_KICKER;
    case  9: return SOL_DROP_TARGET_5BANK_RESET;
    case 10: return SOL_LEFT_THUMPER;
    case 11: return SOL_RIGHT_THUMPER;
    case 12: return SOL_CENTER_THUMPER;
    case 13: return SOL_RIGHT_SLINGSHOT;
    case 14: return SOL_LEFT_SLINGSHOT;
    default: return OPERATOR_MENU_VALUE_UNUSED;
  }
}


byte SolenoidConvertDisplayNumberToTestStrength(byte displayNumber) {
  switch (displayNumber) {
    case  0: return OPERATOR_MENU_VALUE_UNUSED;
    case  1: return OPERATOR_MENU_VALUE_UNUSED;
    case  2: return OPERATOR_MENU_VALUE_UNUSED;
    case  3: return OPERATOR_MENU_VALUE_UNUSED;
    case  4: return OPERATOR_MENU_VALUE_UNUSED;
    case  5: return 4;
    case  6: return 4;
    case  7: return 4;
    case  8: return 4;
    case  9: return 4;
    case 10: return 4;
    case 11: return 4;
    case 12: return 4;
    case 13: return 4;
    case 14: return OPERATOR_MENU_VALUE_UNUSED;
    case 15: return OPERATOR_MENU_VALUE_UNUSED;
    case 16: return OPERATOR_MENU_VALUE_UNUSED;
    case 17: return OPERATOR_MENU_VALUE_UNUSED;
    case 18: return OPERATOR_MENU_VALUE_UNUSED;
    case 19: return OPERATOR_MENU_VALUE_UNUSED; 
    default: return OPERATOR_MENU_VALUE_OUT_OF_RANGE;
  }
}




////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////
// Score Display Sync Helpers
////////////////////////////////////////////////////////////////////////////

void QueuePendingScoreUpdate(byte playerNum, unsigned long scoreValue, byte seqID, unsigned long startOffsetMs = 0) {
  byte toneCount = GetSequenceToneCount(seqID);
  if (toneCount == 0) {
    // Single-tone sequence or no sequence - add score immediately
    CurrentScores[playerNum] += scoreValue;
    return;
  }

  unsigned int toneSpacing = GetSequenceToneSpacing(seqID);
  if (toneSpacing == 0) {
    // No spacing (single tone) - add immediately
    CurrentScores[playerNum] += scoreValue;
    return;
  }

  // Multi-tone sequence - queue incremental updates
  unsigned long scoreIncrement = scoreValue / toneCount;

  // Find an inactive pending score slot
  for (byte i = 0; i < MAX_PENDING_SCORE_UPDATES; i++) {
    if (!PendingScores[i].isActive) {
      PendingScores[i].playerNum = playerNum;
      PendingScores[i].scoreIncrement = scoreIncrement;
      PendingScores[i].nextUpdateTime = CurrentTime + startOffsetMs;  // Start at first tone (with offset)
      PendingScores[i].updateIntervalMs = toneSpacing;
      PendingScores[i].remainingIncrements = toneCount;
      PendingScores[i].isActive = true;
      return;
    }
  }

  // No inactive slots - queue is full. This shouldn't happen in normal play.
  // As a fallback, add the score immediately.
  if (DEBUG_MESSAGES) {
    char buf[64];
    sprintf(buf, "PendingScore queue full for player %d - adding %lu immediately\n", playerNum, scoreValue);
    Serial.write(buf);
  }
  CurrentScores[playerNum] += scoreValue;
}

void ProcessPendingScoreUpdates() {
  for (byte i = 0; i < MAX_PENDING_SCORE_UPDATES; i++) {
    if (PendingScores[i].isActive && PendingScores[i].playerNum == CurrentPlayer) {
      if (CurrentTime >= PendingScores[i].nextUpdateTime) {
        // Apply this increment
        CurrentScores[PendingScores[i].playerNum] += PendingScores[i].scoreIncrement;
        PendingScores[i].remainingIncrements--;

        if (PendingScores[i].remainingIncrements > 0) {
          // Schedule next increment
          PendingScores[i].nextUpdateTime = CurrentTime + PendingScores[i].updateIntervalMs;
        } else {
          // All increments applied
          PendingScores[i].isActive = false;
        }
      }
    }
  }
}

//  Setup
//    Arduino calls this function at power up and reset.
//    It's used to initialize the hardware and
//    certain variables and structures used by
//    the code.
//
////////////////////////////////////////////////////////////////////////////

void setup() {
  // Initialize Serial early to prevent audio initialization issues
  Serial.begin(115200);

  if (DEBUG_MESSAGES) {
    Serial.write("Starting\n");
  }

  // Give hardware time to stabilize
  delay(500);

  // Configure drop target banks
  ThreeBank.DefineSwitch(0, SW_TARGET_1_3BANK);
  ThreeBank.DefineSwitch(1, SW_TARGET_2_3BANK);
  ThreeBank.DefineSwitch(2, SW_TARGET_3_3BANK);
  ThreeBank.DefineResetSolenoid(0, SOL_DROP_TARGET_3BANK_RESET);

  FiveBank.DefineSwitch(0, SW_TARGET_1_5BANK);
  FiveBank.DefineSwitch(1, SW_TARGET_2_5BANK);
  FiveBank.DefineSwitch(2, SW_TARGET_3_5BANK);
  FiveBank.DefineSwitch(3, SW_TARGET_4_5BANK);
  FiveBank.DefineSwitch(4, SW_TARGET_5_5BANK);
  FiveBank.DefineResetSolenoid(0, SOL_DROP_TARGET_5BANK_RESET);

  // Tell the OS about game-specific switches
  // (this is for software-controlled pop bumpers and slings)
#if (RPU_MPU_ARCHITECTURE<10)
  // Machines with a -17, -35, 100, and 200 architecture
  // almost always have software based switch-triggered solenoids.
  // For those, you can define an array of solenoids and the switches
  // that will trigger them:
  RPU_SetupGameSwitches(NUM_SWITCHES_WITH_TRIGGERS, NUM_PRIORITY_SWITCHES_WITH_TRIGGERS, SolenoidAssociatedSwitches);

#endif

  // Set up the chips and interrupts
  unsigned long initResult = 0;
  if (DEBUG_MESSAGES) Serial.write("Initializing MPU\n");

  // If the hardware has the ability to switch on the Credit/Reset button (requires Rev 4 or greater)
  // then that can be used to choose Original or New code. Otherwise, the hardware switch
  // will choose Original if open, and New if closed
  initResult = RPU_InitializeMPU(   RPU_CMD_BOOT_ORIGINAL_IF_CREDIT_RESET | RPU_CMD_BOOT_ORIGINAL_IF_NOT_SWITCH_CLOSED |
                                    RPU_CMD_INIT_AND_RETURN_EVEN_IF_ORIGINAL_CHOSEN | RPU_CMD_PERFORM_MPU_TEST, SW_CREDIT_RESET);

  if (DEBUG_MESSAGES) {
    char buf[128];
    sprintf(buf, "Return from init = 0x%04lX\n", initResult);
    Serial.write(buf);
    if (initResult & RPU_RET_6800_DETECTED) Serial.write("Detected 6800 clock\n");
    else if (initResult & RPU_RET_6802_OR_8_DETECTED) Serial.write("Detected 6802/8 clock\n");
    Serial.write("Back from init\n");
  }

  if (initResult & RPU_RET_SELECTOR_SWITCH_ON) QueueDIAGNotification(SOUND_EFFECT_DIAG_SELECTOR_SWITCH_ON);
  else QueueDIAGNotification(SOUND_EFFECT_DIAG_SELECTOR_SWITCH_OFF);

  if (initResult & RPU_RET_CREDIT_RESET_BUTTON_HIT) QueueDIAGNotification(SOUND_EFFECT_DIAG_CREDIT_RESET_BUTTON);

  if (initResult & RPU_RET_DIAGNOSTIC_REQUESTED) {
    QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_DIAGNOSTICS);
    // Run diagnostics here:
  }

  if (initResult & RPU_RET_ORIGINAL_CODE_REQUESTED) {
    if (DEBUG_MESSAGES) Serial.write("Asked to run original code\n");
    delay(100);
    QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_ORIGINAL_CODE);
    delay(100);
    while (Audio.Update(millis()));
    // Arduino should hang if original code is running
    while (1);
  }
  QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_NEW_CODE);

  RPU_DisableSolenoidStack();
  RPU_SetDisableFlippers(true);

  // Now initialize audio after RPU is ready
  CurrentTime = millis();
  Audio.InitDevices(AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  Audio.StopAllAudio();
  Audio.ClearSoundQueue();

  // Read parameters from EEProm
  ReadStoredParameters();

  CurrentScores[0] = GAME_MAJOR_VERSION;
  CurrentScores[1] = GAME_MINOR_VERSION;
  CurrentScores[2] = RPU_OS_MAJOR_VERSION;
  CurrentScores[3] = RPU_OS_MINOR_VERSION;

  CurrentAchievements[0] = 0;
  CurrentAchievements[1] = 0;
  CurrentAchievements[2] = 0;
  CurrentAchievements[3] = 0;

  CurrentTime = millis();

  // 4-note ascending chime x2 on boot (with audio update to ensure sounds play)
  for (byte rep = 0; rep < 2; rep++) {
    Audio.PlaySound(SND_10000_POINTS, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
    for (int i = 0; i < 15; i++) { Audio.Update(millis()); delay(10); }

    Audio.PlaySound(SND_1000_POINTS, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
    for (int i = 0; i < 15; i++) { Audio.Update(millis()); delay(10); }

    Audio.PlaySound(SND_100_POINTS, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
    for (int i = 0; i < 15; i++) { Audio.Update(millis()); delay(10); }

    Audio.PlaySound(SND_10_POINTS, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
    for (int i = 0; i < 15; i++) { Audio.Update(millis()); delay(10); }
  }
  Audio.PlaySound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  OperatorSwitchPressStarted = 0;
  InOperatorMenu = false;
  Menus.SetNavigationButtons(SWITCH_STACK_EMPTY, SWITCH_STACK_EMPTY, SW_CREDIT_RESET, SW_SELF_TEST_SWITCH);
  Menus.SetLampsLookupCallback(LampConvertDisplayNumberToIndex);
  Menus.SetSolenoidIDLookupCallback(SolenoidConvertDisplayNumberToIndex);
  Menus.SetSolenoidStrengthLookupCallback(SolenoidConvertDisplayNumberToTestStrength);
  Menus.SetDisplayTestCallback(DisplayTestFunction);
  Menus.SetSoundCallbackFunction(SoundTestFunction);
  Menus.SetMenuButtonDebounce(250);
}

byte ReadSetting(byte setting, byte defaultValue, byte maxValue) {
  byte value = EEPROM.read(setting);
  if (value == 0xFF || value>maxValue) {
    EEPROM.write(setting, defaultValue);
    return defaultValue;
  }
  return value;
}

// This function is useful for checking the status of drop target switches
byte CheckSequentialSwitches(byte startingSwitch, byte numSwitches) {
  byte returnSwitches = 0;
  for (byte count = 0; count < numSwitches; count++) {
    returnSwitches |= (RPU_ReadSingleSwitchState(startingSwitch + count) << count);
  }
  return returnSwitches;
}


////////////////////////////////////////////////////////////////////////////
//
//  Lamp Management functions
//    These functions are called each time through the gameplay loop.
//    They use the current status variables to set each lamp to 
//    on, off, dim (depending on hardware), or flashing. The lamps are
//    actually set in hardware by the Interrupt Service Routine, so
//    these functions simply update the state arrays used by the ISR.
//
//    If a special animation is required (sweeps and such), then these
//    functions are turned off and one of the special functions in
//    "LampAnimations.h" is used to set those lamps.
//
////////////////////////////////////////////////////////////////////////////
void SetGeneralIlluminationOn(boolean setGIOn = true) {
  // Since this machine doesn't have GI control,
  // this line prevents compiler warnings.
  (void)setGIOn;
}

void ShowPlayerLamps() {
  byte playerCountLamps[4] = {LAMP_PLAYER_1, LAMP_PLAYER_2, LAMP_PLAYER_3, LAMP_PLAYER_4};

  for (byte count = 0; count < 4; count++) {
    // Player-up lamps: current player flashes, others solid if registered
    if (count == CurrentPlayer) RPU_SetLampState(PlayerUpLamps[count], 1, 0, 250);
    else if (count < CurrentNumPlayers) RPU_SetLampState(PlayerUpLamps[count], 1);
    else RPU_SetLampState(PlayerUpLamps[count], 0);

    // Player count lamps: solid for each registered player
    RPU_SetLampState(playerCountLamps[count], count < CurrentNumPlayers ? 1 : 0);
  }

  RPU_SetLampState(LAMP_BALL_IN_PLAY, 1);
  RPU_SetLampState(LAMP_GAME_OVER, 0);
}

byte BonusLampAssignments[10] = {LAMP_BONUS_1K, LAMP_BONUS_2K, LAMP_BONUS_3K, LAMP_BONUS_4K, LAMP_BONUS_5K, LAMP_BONUS_6K, LAMP_BONUS_7K, LAMP_BONUS_8K, LAMP_BONUS_9K, LAMP_BONUS_10K};
const byte SpinnerAdvanceLamps[4] = {LAMP_ADVANCE_BONUS_1, LAMP_ADVANCE_BONUS_2, LAMP_ADVANCE_BONUS_3, LAMP_ADVANCE_BONUS_4};

void ShowBonusLamps() {
  if (arcSurgeCompleteTime && (CurrentTime - arcSurgeCompleteTime) < 1500) {
    // Arc Surge completion flash: all bonus lamps strobe rapidly for 1.5 seconds
    byte flashOn = (CurrentTime / 75) % 2;
    for (byte count = 0; count < 10; count++) RPU_SetLampState(BonusLampAssignments[count], flashOn);
  } else if (arcSurgeCompleteTime && (CurrentTime - arcSurgeCompleteTime) >= 1500) {
    arcSurgeCompleteTime = 0;
  } else if (isArcSurgeActive[CurrentPlayer]) {
    // Arc Surge active: fast chase at 50ms
    byte chasePhase = (CurrentTime / 50) % 10;
    for (byte count = 0; count < 10; count++) RPU_SetLampState(BonusLampAssignments[count], count == chasePhase);
  } else {
    byte bonusValue = Bonus[CurrentPlayer];
    byte numLampsLit;

    // Bonus 1-10: light lamps 1-10 sequentially
    // Bonus 11-19: keep lamp 10 lit + light lamps 1-9 to show position in 11-19
    if (bonusValue <= 10) {
      numLampsLit = bonusValue;
    } else {
      numLampsLit = bonusValue - 10;  // Second cycle: 1 (lamp 10) + (bonus-11) more lamps
      if (numLampsLit > 9) numLampsLit = 9;  // Cap at 9
    }

    // During +3 advance animation, show lamps lighting up one-by-one
    if (BonusAnimationStart && (CurrentTime - BonusAnimationStart) < 550) {
      unsigned long elapsed = CurrentTime - BonusAnimationStart;
      byte prevBonus = BonusBefore;
      byte prevLampsLit;
      if (prevBonus <= 10) {
        prevLampsLit = prevBonus;
      } else {
        prevLampsLit = prevBonus - 10;
        if (prevLampsLit > 9) prevLampsLit = 9;
      }

      byte finalLampsLit = numLampsLit;

      byte animLamps = 1;
      if (elapsed >= 200) animLamps = 2;
      if (elapsed >= 400) animLamps = 3;

      numLampsLit = prevLampsLit + animLamps;
      if (numLampsLit > 19) numLampsLit = 19;  // Cap at 10 + 9
    } else if (BonusAnimationStart) {
      BonusAnimationStart = 0;
    }

    for (byte count = 0; count < 10; count++) {
      boolean lampOn = false;
      if (bonusValue <= 10) {
        // Cycle 1 (Bonus 1-10): light lamps 0 through (numLampsLit-1)
        lampOn = (count < numLampsLit);
      } else {
        // Cycle 2 (Bonus 11-19): always light lamp 9 (10th), then light lamps 0 through (numLampsLit-1)
        lampOn = (count == 9) || (count < numLampsLit);
      }
      RPU_SetLampState(BonusLampAssignments[count], lampOn);
    }
  }
}

void ShowBonusXLamps() {
  if (CurrentTime > (BonusXAnimationStart + 2000)) {
    BonusXAnimationStart = 0;
  }

  LampState state = GetMultiplierLampState(BonusX[CurrentPlayer]);
  boolean isAnimating = (BonusXAnimationStart != 0);
  int flashPeriod = isAnimating ? 175 : 0;

  // Debug disabled - clean output for sound testing
  RPU_SetLampState(LAMP_2X, state.lamp2X, 0, flashPeriod);
  RPU_SetLampState(LAMP_3X, state.lamp3X, 0, flashPeriod);
  RPU_SetLampState(LAMP_5X, state.lamp5X, 0, flashPeriod);
}





// Helper: Encode multiplier value to 3-lamp states using dual-lamp encoding
LampState GetMultiplierLampState(byte multiplier) {
  LampState state = {0, 0, 0};

  switch(multiplier) {
    case 2: state.lamp2X = 1; break;
    case 3: state.lamp3X = 1; break;
    case 5: state.lamp5X = 1; break;
    case 7: state.lamp2X = 1; state.lamp5X = 1; break;
    case 8: state.lamp3X = 1; state.lamp5X = 1; break;
    case 10: state.lamp2X = 1; state.lamp3X = 1; state.lamp5X = 1; break;
  }
  return state;
}

void ShowShootAgainLamp() {

  if ( (BallFirstSwitchHitTime == 0 && BallSaveNumSeconds) || (BallSaveEndTime && CurrentTime < BallSaveEndTime) ) {
    unsigned long msRemaining = 5000;
    if (BallSaveEndTime != 0) msRemaining = BallSaveEndTime - CurrentTime;
    RPU_SetLampState(LAMP_SHOOT_AGAIN, 1, 0, (msRemaining < 5000) ? 100 : 500);
  } else {
    RPU_SetLampState(LAMP_SHOOT_AGAIN, SamePlayerShootsAgain);
  }
}

void ShowThreeBankTargetLamps() {
  // Display next target multiplier on 3-bank lamps, or OFF if at max (10X)
  byte targetMultiplier = BonusX[CurrentPlayer];

  // Calculate next multiplier in progression
  if (targetMultiplier < 10) {
    if (targetMultiplier == 1) targetMultiplier = 2;
    else if (targetMultiplier == 2) targetMultiplier = 3;
    else if (targetMultiplier == 3) targetMultiplier = 5;
    else if (targetMultiplier == 5) targetMultiplier = 7;
    else if (targetMultiplier == 7) targetMultiplier = 8;
    else if (targetMultiplier == 8) targetMultiplier = 10;
  }

  if (DEBUG_MESSAGES) {
  }

  // At max (10X), turn off target lamps; otherwise show next target with 500ms pulse
  if (BonusX[CurrentPlayer] >= 10) {
    RPU_SetLampState(LAMP_DROP_TARGET_2X, 0);
    RPU_SetLampState(LAMP_DROP_TARGET_3X, 0);
    RPU_SetLampState(LAMP_DROP_TARGET_5X, 0);
  } else {
    LampState state = GetMultiplierLampState(targetMultiplier);
    if (state.lamp2X) RPU_SetLampState(LAMP_DROP_TARGET_2X, 1, 0, 500);
    else RPU_SetLampState(LAMP_DROP_TARGET_2X, 0);
    if (state.lamp3X) RPU_SetLampState(LAMP_DROP_TARGET_3X, 1, 0, 500);
    else RPU_SetLampState(LAMP_DROP_TARGET_3X, 0);
    if (state.lamp5X) RPU_SetLampState(LAMP_DROP_TARGET_5X, 1, 0, 500);
    else RPU_SetLampState(LAMP_DROP_TARGET_5X, 0);
  }
}

void ShowGameplayLamps() {
  // LAMP_SAUCER: pulse fast during Arc Surge, pulse during skill shot, solid when lit
  if (SkillShotAnimationStart != 0 && (CurrentTime - SkillShotAnimationStart) < 1000) {
    RPU_SetLampState(LAMP_SAUCER, 1, 0, 200);
  } else if (isArcSurgeActive[CurrentPlayer]) {
    RPU_SetLampState(LAMP_SAUCER, 1, 0, 250);
  } else if (!firstHitMade[CurrentPlayer]) {
    RPU_SetLampState(LAMP_SAUCER, 1, 0, 500);
  } else {
    RPU_SetLampState(LAMP_SAUCER, isSaucerLit[CurrentPlayer] ? 1 : 0);
    SkillShotAnimationStart = 0;
  }

  // LAMP_SPINNER: lit when bonus ladder is full (>= 10 steps)
  RPU_SetLampState(LAMP_SPINNER, Bonus[CurrentPlayer] >= 10 ? 1 : 0);

  // LAMP_LEFT_RETURN: lit after rollover button hit, cleared by left inlane
  RPU_SetLampState(LAMP_LEFT_RETURN, isLeftReturnLaneLit[CurrentPlayer] ? 1 : 0);

  // LAMP_EXTRA_BALL_LANE (46): physically near 5-bank, lights on 1st completion
  RPU_SetLampState(LAMP_EXTRA_BALL_LANE, (ExtraBallLaneEnabled && fiveBankCompleteCount[CurrentPlayer] == 1) ? 1 : 0);

  // Advance bonus lamps: one lamp cycles per spin, 4 spins = bonus advance
  byte spinnerProgress = spinnerHitCount[CurrentPlayer] % 4;
  for (byte i = 0; i < 4; i++) {
    RPU_SetLampState(SpinnerAdvanceLamps[i], i == spinnerProgress ? 1 : 0);
  }

  // Drop target multiplier indicator lamps
  int bankCount = threeBankCompleteCount[CurrentPlayer];
  if (threeBankSweepAnimationStart[CurrentPlayer] != 0) {
    if ((CurrentTime - threeBankSweepAnimationStart[CurrentPlayer]) < 1500) {
      // Sweep animation: all three flash rapidly together
      byte flashOn = (CurrentTime / 100) % 2;
      RPU_SetLampState(LAMP_DROP_TARGET_2X, flashOn);
      RPU_SetLampState(LAMP_DROP_TARGET_3X, flashOn);
      RPU_SetLampState(LAMP_DROP_TARGET_5X, flashOn);
    } else {
      threeBankSweepAnimationStart[CurrentPlayer] = 0;
    }
  }
  // Note: Drop target lamps now used for bonus multiplier target display in ShowThreeBankTargetLamps()

  // LAMP_ROLLOVER_BUTTON: on when available; turns off after rolled (value moves to left inlane)
  RPU_SetLampState(LAMP_ROLLOVER_BUTTON, isLeftReturnLaneLit[CurrentPlayer] ? 0 : 1);

  // LAMP_XTRA_BALL (24): physically in the right return lane, lights on 2nd completion for collection
  RPU_SetLampState(LAMP_XTRA_BALL, (ExtraBallLaneEnabled && ExtraBallLaneAvailable[CurrentPlayer] && !ExtraBallCollectedThisBall[CurrentPlayer]) ? 1 : 0);

  // LAMP_5_BANK_TARGET_SPECIAL: lit from 2nd completion; off once collected (unless open-ended)
  RPU_SetLampState(LAMP_5_BANK_TARGET_SPECIAL, (fiveBankCompleteCount[CurrentPlayer] >= 2 && (!SpecialCollected || SpecialOpenEnded)) ? 1 : 0);
}


////////////////////////////////////////////////////////////////////////////
//
//  Machine State Helper functions
//
////////////////////////////////////////////////////////////////////////////
boolean AddPlayer(boolean resetNumPlayers = false) {

  if (Credits < 1 && !FreePlayMode) return false;
  if (resetNumPlayers) CurrentNumPlayers = 0;
  if (CurrentNumPlayers >= RPU_NUMBER_OF_PLAYERS_ALLOWED) return false;

  CurrentNumPlayers += 1;
  RPU_SetDisplay(CurrentNumPlayers - 1, 0, true, 2);
  
  if (CurrentNumPlayers > 1) {
    // 4-note ascending chime x2
    for (byte rep = 0; rep < 2; rep++) {
      Audio.PlaySound(SND_10000_POINTS, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS); delay(200);
      Audio.PlaySound(SND_1000_POINTS,  AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS); delay(200);
      Audio.PlaySound(SND_100_POINTS,   AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS); delay(200);
      Audio.PlaySound(SND_10_POINTS,    AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS); delay(200);
    }
    Audio.PlaySound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  }

  for (byte count = 0; count < 4; count++) {
    if (count==CurrentPlayer) RPU_SetLampState(PlayerUpLamps[count], 1, 0, 250);
    else if (count<CurrentNumPlayers) RPU_SetLampState(PlayerUpLamps[count], 1);
    else RPU_SetLampState(PlayerUpLamps[count], 0);
  }

  if (!FreePlayMode) {
    Credits -= 1;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
//    RPU_SetCoinLockout(false);
  }

  RPU_WriteULToEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE) + 1);

  return true;
}


unsigned short ChuteAuditByte[] = {RPU_CHUTE_1_COINS_START_BYTE, RPU_CHUTE_2_COINS_START_BYTE, RPU_CHUTE_3_COINS_START_BYTE};
void AddCoinToAudit(byte chuteNum) {
  if (chuteNum > 2) return;
  unsigned short coinAuditStartByte = ChuteAuditByte[chuteNum];
  RPU_WriteULToEEProm(coinAuditStartByte, RPU_ReadULFromEEProm(coinAuditStartByte) + 1);
}


void AddCredit(boolean playSound = false, byte numToAdd = 1) {
  if (Credits < MaximumCredits) {
    Credits += numToAdd;
    if (Credits > MaximumCredits) Credits = MaximumCredits;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    if (playSound) {
      //PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
      RPU_PushToSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, true);
    }
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
//    RPU_SetCoinLockout(false);
  } else {
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
//    RPU_SetCoinLockout(true);
  }

}

byte SwitchToChuteNum(byte switchHit) {
  (void)switchHit;
  byte chuteNum = 0;
  return chuteNum;
}

boolean AddCoin(byte chuteNum) {
  boolean creditAdded = false;
  if (chuteNum > 2) return false;
  byte cpcSelection = GetCPCSelection(chuteNum);

  // Find the lowest chute num with the same ratio selection
  // and use that ChuteCoinsInProgress counter
  byte chuteNumToUse;
  for (chuteNumToUse = 0; chuteNumToUse <= chuteNum; chuteNumToUse++) {
    if (GetCPCSelection(chuteNumToUse) == cpcSelection) break;
  }

  PlaySoundEffect(SOUND_EFFECT_COIN_DROP_1 + (CurrentTime % 3));

  byte cpcCoins = GetCPCCoins(cpcSelection);
  byte cpcCredits = GetCPCCredits(cpcSelection);
  byte coinProgressBefore = ChuteCoinsInProgress[chuteNumToUse];
  ChuteCoinsInProgress[chuteNumToUse] += 1;

  if (ChuteCoinsInProgress[chuteNumToUse] == cpcCoins) {
    if (cpcCredits > cpcCoins) AddCredit(cpcCredits - (coinProgressBefore));
    else AddCredit(cpcCredits);
    ChuteCoinsInProgress[chuteNumToUse] = 0;
    creditAdded = true;
  } else {
    if (cpcCredits > cpcCoins) {
      AddCredit(1);
      creditAdded = true;
    } else {
    }
  }

  return creditAdded;
}


void AddSpecialCredit() {
  AddCredit(false, 1);
  RPU_PushToTimedSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, CurrentTime, true);
  RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 1);
}

boolean AwardExtraBall(boolean basedOnScore = false);

void AwardSpecial(boolean overrideSpecialCollected = false) {
  if (SpecialCollected && !overrideSpecialCollected && !SpecialOpenEnded) return;
  if (!SpecialOpenEnded) SpecialCollected = true;
  if (TournamentScoring) {
    CurrentScores[CurrentPlayer] += SpecialValue * PlayfieldMultiplier;
  } else {
    if (SpecialAwardType == 0) {
      CurrentScores[CurrentPlayer] += SpecialValue;
    } else if (SpecialAwardType == 1) {
      AwardExtraBall();
    } else if (SpecialAwardType == 2) {
      AddSpecialCredit();
    } else {
      AwardExtraBall();
      AddSpecialCredit();
    }
  }
}

boolean AwardExtraBall(boolean basedOnScore = false) {
  if (ExtraBallsAvailable[CurrentPlayer]) {
    ExtraBallsAvailable[CurrentPlayer] -= 1;
    if (TournamentScoring) {
      if (!basedOnScore) CurrentScores[CurrentPlayer] += ExtraBallValue * PlayfieldMultiplier;
    } else {
      SamePlayerShootsAgain = true;
      RPU_SetLampState(LAMP_SHOOT_AGAIN, SamePlayerShootsAgain);
      PlaySoundEffect(SOUND_EFFECT_BONUS_5);
    }
    return true;
  }
  return false;
}


void IncreasePlayfieldMultiplier(unsigned long duration) {
  PlayfieldMultiplierTimeLeft += duration;

  PlayfieldMultiplier += 1;
  if (PlayfieldMultiplier > 5) {
    PlayfieldMultiplier = 5;
  }
}


void SetBallSave(unsigned long duration, byte numberOfSaves = 0xFF, boolean addToBallSave = false) {

  if (duration == 0) {
    BallSaveEndTime = 0;
    NumberOfBallSavesRemaining = 0;
  } else if (addToBallSave) {
    if (BallSaveEndTime) BallSaveEndTime += duration;
  } else {
    BallSaveEndTime = CurrentTime + duration;
    NumberOfBallSavesRemaining = numberOfSaves;
  }
}




#define SOUND_EFFECT_OM_CPC_VALUES                  180
#define SOUND_EFFECT_OM_CRB_VALUES                  210
#define SOUND_EFFECT_OM_DIFFICULTY_VALUES           220

#define SOUND_EFFECT_AP_TOP_LEVEL_MENU_ENTRY    1700
#define SOUND_EFFECT_AP_TEST_MENU               1701
#define SOUND_EFFECT_AP_AUDITS_MENU             1702
#define SOUND_EFFECT_AP_BASIC_ADJUSTMENTS_MENU  1703
#define SOUND_EFFECT_AP_GAME_RULES_LEVEL        1704
#define SOUND_EFFECT_AP_GAME_SPECIFIC_ADJ_MENU  1705

#define SOUND_EFFECT_AP_TEST_LAMPS              1710
#define SOUND_EFFECT_AP_TEST_DISPLAYS           1711
#define SOUND_EFFECT_AP_TEST_SOLENOIDS          1712
#define SOUND_EFFECT_AP_TEST_SWITCHES           1713
#define SOUND_EFFECT_AP_TEST_SOUNDS             1714
#define SOUND_EFFECT_AP_TEST_EJECT_BALLS        1715

#define SOUND_EFFECT_AP_AUDIT_TOTAL_PLAYS       1720
#define SOUND_EFFECT_AP_AUDIT_CHUTE_1_COINS     1721
#define SOUND_EFFECT_AP_AUDIT_CHUTE_2_COINS     1722
#define SOUND_EFFECT_AP_AUDIT_CHUTE_3_COINS     1723
#define SOUND_EFFECT_AP_AUDIT_TOTAL_REPLAYS     1724
#define SOUND_EFFECT_AP_AUDIT_AVG_BALL_TIME     1725
#define SOUND_EFFECT_AP_AUDIT_HISCR_BEAT        1726
#define SOUND_EFFECT_AP_AUDIT_TOTAL_BALLS       1727
#define SOUND_EFFECT_AP_AUDIT_NUM_MATCHES       1728
#define SOUND_EFFECT_AP_AUDIT_MATCH_PERCENTAGE  1729
#define SOUND_EFFECT_AP_AUDIT_LIFETIME_PLAYS    1730
#define SOUND_EFFECT_AP_AUDIT_MINUTES_ON        1731
#define SOUND_EFFECT_AP_AUDIT_CLEAR_AUDITS      1732

#define OM_BASIC_ADJ_IDS_FREEPLAY               0
#define OM_BASIC_ADJ_IDS_BALL_SAVE              1
#define OM_BASIC_ADJ_IDS_TILT_WARNINGS          2
#define OM_BASIC_ADJ_IDS_MUSIC_VOLUME           3
#define OM_BASIC_ADJ_IDS_SOUNDFX_VOLUME         4
#define OM_BASIC_ADJ_IDS_CALLOUTS_VOLUME        5
#define OM_BASIC_ADJ_IDS_BALLS_PER_GAME         6
#define OM_BASIC_ADJ_IDS_TOURNAMENT_MODE        7
#define OM_BASIC_ADJ_IDS_EXTRA_BALL_VALUE       8
#define OM_BASIC_ADJ_IDS_SPECIAL_VALUE          9 
#define OM_BASIC_ADJ_IDS_RESET_DURING_GAME      10
#define OM_BASIC_ADJ_IDS_SCORE_LEVEL_1          11
#define OM_BASIC_ADJ_IDS_SCORE_LEVEL_2          12
#define OM_BASIC_ADJ_IDS_SCORE_LEVEL_3          13
#define OM_BASIC_ADJ_IDS_SCORE_AWARDS           14
#define OM_BASIC_ADJ_IDS_SCROLLING_SCORES       15
#define OM_BASIC_ADJ_IDS_HISCR                  16
#define OM_BASIC_ADJ_IDS_CREDITS                17
#define OM_BASIC_ADJ_IDS_CPC_1                  18
#define OM_BASIC_ADJ_IDS_CPC_2                  19
#define OM_BASIC_ADJ_IDS_CPC_3                  20
#define OM_BASIC_ADJ_IDS_MATCH_FEATURE          21
#define OM_BASIC_ADJ_FINISHED                   22
#define SOUND_EFFECT_AP_FREEPLAY                (1740 + OM_BASIC_ADJ_IDS_FREEPLAY)
#define SOUND_EFFECT_AP_BALL_SAVE_SECONDS       (1740 + OM_BASIC_ADJ_IDS_BALL_SAVE)
#define SOUND_EFFECT_AP_TILT_WARNINGS           (1740 + OM_BASIC_ADJ_IDS_TILT_WARNINGS)
#define SOUND_EFFECT_AP_MUSIC_VOLUME            (1740 + OM_BASIC_ADJ_IDS_MUSIC_VOLUME)
#define SOUND_EFFECT_AP_SOUNDFX_VOLUME          (1740 + OM_BASIC_ADJ_IDS_SOUNDFX_VOLUME)
#define SOUND_EFFECT_AP_CALLOUTS_VOLUME         (1740 + OM_BASIC_ADJ_IDS_CALLOUTS_VOLUME)
#define SOUND_EFFECT_AP_BALLS_PER_GAME          (1740 + OM_BASIC_ADJ_IDS_BALLS_PER_GAME)
#define SOUND_EFFECT_AP_TOURNAMENT_MODE         (1740 + OM_BASIC_ADJ_IDS_TOURNAMENT_MODE)
#define SOUND_EFFECT_AP_EXTRA_BALL_VALUE        (1740 + OM_BASIC_ADJ_IDS_EXTRA_BALL_VALUE)
#define SOUND_EFFECT_AP_SPECIAL_VALUE           (1740 + OM_BASIC_ADJ_IDS_SPECIAL_VALUE)
#define SOUND_EFFECT_AP_RESET_DURING_GAME       (1740 + OM_BASIC_ADJ_IDS_RESET_DURING_GAME)
#define SOUND_EFFECT_AP_ADJ_SCORE_LEVEL_1       (1740 + OM_BASIC_ADJ_IDS_SCORE_LEVEL_1)
#define SOUND_EFFECT_AP_ADJ_SCORE_LEVEL_2       (1740 + OM_BASIC_ADJ_IDS_SCORE_LEVEL_2)
#define SOUND_EFFECT_AP_ADJ_SCORE_LEVEL_3       (1740 + OM_BASIC_ADJ_IDS_SCORE_LEVEL_3)
#define SOUND_EFFECT_AP_SCORE_AWARDS            (1740 + OM_BASIC_ADJ_IDS_SCORE_AWARDS)
#define SOUND_EFFECT_AP_SCROLLING_SCORES        (1740 + OM_BASIC_ADJ_IDS_SCROLLING_SCORES)
#define SOUND_EFFECT_AP_ADJ_HISCR               (1740 + OM_BASIC_ADJ_IDS_HISCR)
#define SOUND_EFFECT_AP_ADJ_CREDITS             (1740 + OM_BASIC_ADJ_IDS_CREDITS)
#define SOUND_EFFECT_AP_ADJ_CPC_1               (1740 + OM_BASIC_ADJ_IDS_CPC_1)
#define SOUND_EFFECT_AP_ADJ_CPC_2               (1740 + OM_BASIC_ADJ_IDS_CPC_2)
#define SOUND_EFFECT_AP_ADJ_CPC_3               (1740 + OM_BASIC_ADJ_IDS_CPC_3)
#define SOUND_EFFECT_AP_MATCH_FEATURE           (1740 + OM_BASIC_ADJ_IDS_MATCH_FEATURE)

#define SOUND_EFFECT_OM_EASY_RULES_INSTRUCTIONS           1770
#define SOUND_EFFECT_OM_MEDIUM_RULES_INSTRUCTIONS         1771
#define SOUND_EFFECT_OM_HARD_RULES_INSTRUCTIONS           1772
#define SOUND_EFFECT_OM_PROGRESSIVE_RULES_INSTRUCTIONS    1773
#define SOUND_EFFECT_OM_CUSTOM_RULES_INSTRUCTIONS         1774

#define OM_GAME_ADJ_EASY_DIFFICULTY                 0
#define OM_GAME_ADJ_MEDIUM_DIFFICULTY               1
#define OM_GAME_ADJ_HARD_DIFFICULTY                 2
#define OM_GAME_ADJ_PROGRESSIVE_DIFFICULTY          3
#define OM_GAME_ADJ_CUSTOM_DIFFICULTY               4
#define SOUND_EFFECT_AP_DIFFICULTY                  (1790 + OM_GAME_ADJ_EASY_DIFFICULTY)

#define OM_GAME_ADJ_TROUGH_EJECT_STRENGTH           0
#define OM_GAME_ADJ_SAUCER_EJECT_STRENGTH           1
#define OM_GAME_ADJ_SLINGSHOT_STRENGTH              2
#define OM_GAME_ADJ_POP_BUMPER_STRENGTH             3
#define OM_GAME_ADJ_SAUCER_LIGHT_PERSISTENCE        4
#define OM_GAME_ADJ_HIGH_GAME_FREE_GAMES            5
#define OM_GAME_ADJ_SPECIAL_OPEN_ENDED              6
#define OM_GAME_ADJ_BONUS_COUNTDOWN_MODE            7
#define OM_GAME_ADJ_EXTRA_BALL_LANE                 8
#define OM_GAME_ADJ_SPECIAL_AWARD_TYPE              9
#define OM_GAME_ADJ_FINISHED                        10
#define SOUND_EFFECT_AP_LOCK_BEHAVIOR               (1800 + OM_GAME_ADJ_TROUGH_EJECT_STRENGTH)

unsigned long SoundSettingTimeout;
unsigned long SoundTestStart;
byte SoundTestSequence;
  
void RunOperatorMenu() {
  if (!Menus.UpdateMenu(CurrentTime)) {
    // Menu is done
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    Audio.StopAllAudio();
    RPU_TurnOffAllLamps();
    if (MachineState==MACHINE_STATE_ATTRACT) {
      RPU_SetDisplayBallInPlay(0, true);
    } else {
      RPU_SetDisplayBallInPlay(CurrentBallInPlay);
    }
    SoundSettingTimeout = 0;
    return;
  }

  // It's up to this function to eject balls if requested
  if (Menus.BallEjectInProgress()) {
    if (CountBallsInTrough()) {
      if (CurrentTime > (LastTimeBallServed+1500)) {
        LastTimeBallServed = CurrentTime;
        RPU_PushToSolenoidStack(SOL_OUTHOLE, BallServeSolenoidStrength, true);
      }
    }
  } else {
    LastTimeBallServed = 0;
  }
  
  byte topLevel = Menus.GetTopLevel();
  byte subLevel = Menus.GetSubLevel();

  if (Menus.HasTopLevelChanged()) {
    // Play an audio prompt for the top level
    SoundTestStart = 0;
    Audio.StopAllAudio();
    Audio.PlaySound((unsigned short)topLevel + SOUND_EFFECT_AP_TOP_LEVEL_MENU_ENTRY, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
    if (Menus.GetTopLevel()==OPERATOR_MENU_GAME_RULES_LEVEL) Menus.SetNumSubLevels(4);
    if (Menus.GetTopLevel()==OPERATOR_MENU_BASIC_ADJ_MENU) {
      GetCPCSelection(0); // make sure CPC values have been read
      Menus.SetNumSubLevels(OM_BASIC_ADJ_FINISHED);
    }
    if (Menus.GetTopLevel()==OPERATOR_MENU_GAME_ADJ_MENU) Menus.SetNumSubLevels(OM_GAME_ADJ_FINISHED);
  }
  if (Menus.HasSubLevelChanged()) {
    SoundTestStart = 0;
    // Play an audio prompt for the sub level    
    Audio.StopAllAudio();
    if (topLevel==OPERATOR_MENU_SELF_TEST_MENU) {
      Audio.PlaySound((unsigned short)subLevel + SOUND_EFFECT_AP_TEST_LAMPS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);

      if (subLevel==OPERATOR_MENU_TEST_SOUNDS) {
        SoundTestStart = CurrentTime + 1000;
        SoundTestSequence = 0;
      } else {
        SoundTestStart = 0;
      }
    } else if (topLevel==OPERATOR_MENU_AUDITS_MENU) {
      unsigned long *currentAdjustmentUL = NULL;
      byte currentAdjustmentStorageByte = 0;
      byte adjustmentType = OPERATOR_MENU_AUD_CLEARABLE;

      switch (subLevel) {
        case 0:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_TOTAL_PLAYS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_TOTAL_PLAYS_EEPROM_START_BYTE;
          break;
        case 1:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_CHUTE_1_COINS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_CHUTE_1_COINS_START_BYTE;
          break;
        case 2:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_CHUTE_2_COINS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_CHUTE_2_COINS_START_BYTE;
          break;
        case 3:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_CHUTE_3_COINS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_CHUTE_3_COINS_START_BYTE;
          break;
        case 4:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_TOTAL_REPLAYS, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_TOTAL_REPLAYS_EEPROM_START_BYTE;
          break;
        case 5:
          Audio.PlaySound(SOUND_EFFECT_AP_AUDIT_HISCR_BEAT, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
          currentAdjustmentStorageByte = RPU_TOTAL_HISCORE_BEATEN_START_BYTE;
          break;
      }

      Menus.SetAuditControls(currentAdjustmentUL, currentAdjustmentStorageByte, adjustmentType);

    } else if (topLevel==OPERATOR_MENU_BASIC_ADJ_MENU) {
      Audio.PlaySound((unsigned short)subLevel + SOUND_EFFECT_AP_FREEPLAY, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);

      byte *currentAdjustmentByte = NULL;
      byte currentAdjustmentStorageByte = 0;
      byte adjustmentValues[8] = {0};
      byte numAdjustmentValues = 2;
      byte adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
      short parameterCallout = 0;
      unsigned long *currentAdjustmentUL = NULL;
      
      adjustmentValues[1] = 1;

      switch(subLevel) {
        case OM_BASIC_ADJ_IDS_FREEPLAY:
          currentAdjustmentByte = (byte *)&FreePlayMode;
          currentAdjustmentStorageByte = EEPROM_FREE_PLAY_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_BALL_SAVE:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_LIST;
          numAdjustmentValues = 5;
          adjustmentValues[1] = 5;
          adjustmentValues[2] = 10;
          adjustmentValues[3] = 15;
          adjustmentValues[4] = 20;
          currentAdjustmentByte = &BallSaveNumSeconds;
          currentAdjustmentStorageByte = EEPROM_BALL_SAVE_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_TILT_WARNINGS:
          adjustmentValues[1] = 2;
          currentAdjustmentByte = &MaxTiltWarnings;
          currentAdjustmentStorageByte = EEPROM_TILT_WARNING_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_MUSIC_VOLUME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 10;
          currentAdjustmentByte = &MusicVolume;
          currentAdjustmentStorageByte = EEPROM_MUSIC_VOLUME_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SOUNDFX_VOLUME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 10;
          currentAdjustmentByte = &SoundEffectsVolume;
          currentAdjustmentStorageByte = EEPROM_SFX_VOLUME_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_CALLOUTS_VOLUME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 10;
          currentAdjustmentByte = &CalloutsVolume;
          currentAdjustmentStorageByte = EEPROM_CALLOUTS_VOLUME_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_BALLS_PER_GAME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          numAdjustmentValues = 8;
          adjustmentValues[0] = 3;
          adjustmentValues[1] = 10;
          currentAdjustmentByte = &BallsPerGame;
          currentAdjustmentStorageByte = EEPROM_BALLS_OVERRIDE_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_TOURNAMENT_MODE:
          currentAdjustmentByte = (byte *)&TournamentScoring;
          currentAdjustmentStorageByte = EEPROM_TOURNAMENT_SCORING_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_EXTRA_BALL_VALUE:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &ExtraBallValue;
          currentAdjustmentStorageByte = EEPROM_EXTRA_BALL_SCORE_UL;
          break;
        case OM_BASIC_ADJ_IDS_SPECIAL_VALUE:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &SpecialValue;
          currentAdjustmentStorageByte = EEPROM_SPECIAL_SCORE_UL;
          break;
        case OM_BASIC_ADJ_IDS_RESET_DURING_GAME:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_LIST;
          numAdjustmentValues = 5;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 1;
          adjustmentValues[2] = 2;
          adjustmentValues[3] = 3;
          adjustmentValues[4] = 99;
          currentAdjustmentByte = &TimeRequiredToResetGame;
          currentAdjustmentStorageByte = EEPROM_CRB_HOLD_TIME;
          parameterCallout = SOUND_EFFECT_OM_CRB_VALUES;
          break;
        case OM_BASIC_ADJ_IDS_SCORE_LEVEL_1:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &AwardScores[0];
          currentAdjustmentStorageByte = RPU_AWARD_SCORE_1_EEPROM_START_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SCORE_LEVEL_2:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &AwardScores[1];
          currentAdjustmentStorageByte = RPU_AWARD_SCORE_2_EEPROM_START_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SCORE_LEVEL_3:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &AwardScores[2];
          currentAdjustmentStorageByte = RPU_AWARD_SCORE_3_EEPROM_START_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SCORE_AWARDS:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX_DEFAULT;
          adjustmentValues[1] = 7;
          currentAdjustmentByte = &ScoreAwardReplay;
          currentAdjustmentStorageByte = EEPROM_AWARD_OVERRIDE_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_SCROLLING_SCORES:
          currentAdjustmentByte = (byte *)&ScrollingScores;
          currentAdjustmentStorageByte = EEPROM_SCROLLING_SCORES_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_HISCR:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_SCORE_WITH_DEFAULT;
          currentAdjustmentUL = &HighScore;
          currentAdjustmentStorageByte = RPU_HIGHSCORE_EEPROM_START_BYTE;
          break;
        case OM_BASIC_ADJ_IDS_CREDITS:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 40;
          currentAdjustmentByte = &Credits;
          currentAdjustmentStorageByte = RPU_CREDITS_EEPROM_BYTE;
          break;
/*
        case OM_BASIC_ADJ_IDS_CPC_1:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_CPC;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = (NUM_CPC_PAIRS-1);
          currentAdjustmentByte = &(CPCSelection[0]);
          currentAdjustmentStorageByte = RPU_CPC_CHUTE_1_SELECTION_BYTE;
          parameterCallout = SOUND_EFFECT_OM_CPC_VALUES;
          break;
        case OM_BASIC_ADJ_IDS_CPC_2:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_CPC;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = (NUM_CPC_PAIRS-1);
          currentAdjustmentByte = &(CPCSelection[1]);
          currentAdjustmentStorageByte = RPU_CPC_CHUTE_2_SELECTION_BYTE;
          parameterCallout = SOUND_EFFECT_OM_CPC_VALUES;
          break;
        case OM_BASIC_ADJ_IDS_CPC_3:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_CPC;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = (NUM_CPC_PAIRS-1);
          currentAdjustmentByte = &(CPCSelection[2]);
          currentAdjustmentStorageByte = RPU_CPC_CHUTE_3_SELECTION_BYTE;
          parameterCallout = SOUND_EFFECT_OM_CPC_VALUES;
          break;
*/
        case OM_BASIC_ADJ_IDS_MATCH_FEATURE:
          currentAdjustmentByte = (byte *)&MatchFeature;
          currentAdjustmentStorageByte = EEPROM_MATCH_FEATURE_BYTE;
          break;
      }

      Menus.SetParameterControls(   adjustmentType, numAdjustmentValues, adjustmentValues, parameterCallout,
                                    currentAdjustmentStorageByte, currentAdjustmentByte, currentAdjustmentUL );
    } else if (topLevel==OPERATOR_MENU_GAME_RULES_LEVEL) {
      Audio.PlaySound((unsigned short)subLevel + SOUND_EFFECT_AP_DIFFICULTY, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
      byte *currentAdjustmentByte = &GameRulesSelection;
      byte adjustmentValues[8] = {0};
      adjustmentValues[0] = 0;
      // if one of the below parameters is installed, the "HasParameterChanged" 
      // check below will install Easy / Medium / Hard rules

      switch (subLevel) {
        case 0:
          adjustmentValues[1] = 1;
          break;
        case 1:
          adjustmentValues[1] = 2;
          break;
        case 2:
          adjustmentValues[1] = 3;
          break;
        case 3:
          adjustmentValues[1] = 4;
          break;
      }

      Menus.SetParameterControls(   OPERATOR_MENU_ADJ_TYPE_LIST, 2, adjustmentValues, (short)SOUND_EFFECT_OM_EASY_RULES_INSTRUCTIONS-1,
                                    EEPROM_GAME_RULES_SELECTION, currentAdjustmentByte, NULL );
                  
    } else if (topLevel==OPERATOR_MENU_GAME_ADJ_MENU) {
      Audio.PlaySound((unsigned short)subLevel + SOUND_EFFECT_AP_LOCK_BEHAVIOR, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);

      byte *currentAdjustmentByte = NULL;
      byte currentAdjustmentStorageByte = 0;
      byte adjustmentValues[8] = {0};
      byte numAdjustmentValues = 2;
      byte adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
      short parameterCallout = 0;
      unsigned long *currentAdjustmentUL = NULL;
      
      adjustmentValues[1] = 1;

      switch (subLevel) {
        case OM_GAME_ADJ_TROUGH_EJECT_STRENGTH:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_LIST;
          numAdjustmentValues = 7;
          adjustmentValues[0] = 2;
          adjustmentValues[1] = 3;
          adjustmentValues[2] = 4;
          adjustmentValues[3] = 5;
          adjustmentValues[4] = 6;
          adjustmentValues[5] = 7;
          adjustmentValues[6] = 8;
          currentAdjustmentByte = &BallServeSolenoidStrength;
          currentAdjustmentStorageByte = EEPROM_TROUGH_EJECT_STRENGTH;
          break;
        case OM_GAME_ADJ_SAUCER_EJECT_STRENGTH:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 5;
          adjustmentValues[1] = 15;
          currentAdjustmentByte = &SaucerSolenoidStrength;
          currentAdjustmentStorageByte = EEPROM_SAUCER_EJECT_STRENGTH;
          break;
        case OM_GAME_ADJ_SLINGSHOT_STRENGTH:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 4;
          adjustmentValues[1] = 8;
          currentAdjustmentByte = &TempSlingStrength;
          currentAdjustmentStorageByte = EEPROM_SLINGSHOT_STRENGTH;
          break;
        case OM_GAME_ADJ_POP_BUMPER_STRENGTH:
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 4;
          adjustmentValues[1] = 8;
          currentAdjustmentByte = &TempPopStrength;
          currentAdjustmentStorageByte = EEPROM_POP_BUMPER_STRENGTH;
          break;
        case OM_GAME_ADJ_SAUCER_LIGHT_PERSISTENCE:
          // 0=off after scored, 1=stays lit whole ball
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 1;
          currentAdjustmentByte = (byte*)&SaucerLightPersists;
          currentAdjustmentStorageByte = EEPROM_SAUCER_LIGHT_PERSISTENCE;
          break;
        case OM_GAME_ADJ_HIGH_GAME_FREE_GAMES:
          // 0=novelty (no free games), 1-3 = free games awarded for new high score
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 3;
          currentAdjustmentByte = &HighGameFreeGames;
          currentAdjustmentStorageByte = EEPROM_HIGHSCORE_REPLAY_AWARD;
          break;
        case OM_GAME_ADJ_SPECIAL_OPEN_ENDED:
          // 0=1 special per ball, 1=open ended (collectable every time)
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 1;
          currentAdjustmentByte = (byte*)&SpecialOpenEnded;
          currentAdjustmentStorageByte = EEPROM_SPECIAL_OPEN_ENDED;
          break;
        case OM_GAME_ADJ_BONUS_COUNTDOWN_MODE:
          // 0=1000 pts per step, 1=multiplied (multiplier applied each step)
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 1;
          currentAdjustmentByte = (byte*)&BonusCountdownMultipleSteps;
          currentAdjustmentStorageByte = EEPROM_BONUS_COUNTDOWN_MULTIPLE_STEPS;
          break;
        case OM_GAME_ADJ_EXTRA_BALL_LANE:
          // 0=bypass (no extra ball from 5-bank), 1=award extra ball
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_MIN_MAX;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 1;
          currentAdjustmentByte = (byte*)&ExtraBallLaneEnabled;
          currentAdjustmentStorageByte = EEPROM_EXTRA_BALL_LANE_ENABLED;
          break;
        case OM_GAME_ADJ_SPECIAL_AWARD_TYPE:
          // 0=100K pts, 1=free ball, 2=free game, 3=both free ball & free game
          adjustmentType = OPERATOR_MENU_ADJ_TYPE_LIST;
          numAdjustmentValues = 4;
          adjustmentValues[0] = 0;
          adjustmentValues[1] = 1;
          adjustmentValues[2] = 2;
          adjustmentValues[3] = 3;
          currentAdjustmentByte = &SpecialAwardType;
          currentAdjustmentStorageByte = EEPROM_SPECIAL_AWARD_TYPE;
          break;
      }

      Menus.SetParameterControls(   adjustmentType, numAdjustmentValues, adjustmentValues, parameterCallout,
                                    currentAdjustmentStorageByte, currentAdjustmentByte, currentAdjustmentUL );
    }    
  }

  if (Menus.HasParameterChanged()) {
    short parameterCallout = Menus.GetParameterCallout();
    if (parameterCallout) {
      Audio.StopAllAudio();
      Audio.PlaySound((unsigned short)parameterCallout + Menus.GetParameterID(), AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
    }
    if (Menus.GetTopLevel()==OPERATOR_MENU_GAME_RULES_LEVEL) {
      // Install the new rules level
      if (LoadRuleDefaults(GameRulesSelection)) {
        WriteParameters();
      }
    } else if (Menus.GetTopLevel()==OPERATOR_MENU_BASIC_ADJ_MENU) {
      if (Menus.GetSubLevel()==OM_BASIC_ADJ_IDS_MUSIC_VOLUME) {
        if (SoundSettingTimeout) Audio.StopAllAudio();
        Audio.PlaySound(SOUND_EFFECT_BACKGROUND_SONG_1, AUDIO_PLAY_TYPE_WAV_TRIGGER, MusicVolume);
        Audio.SetMusicVolume(MusicVolume);
        SoundSettingTimeout = CurrentTime + 5000;
      } else if (Menus.GetSubLevel()==OM_BASIC_ADJ_IDS_SOUNDFX_VOLUME) {
        if (SoundSettingTimeout) Audio.StopAllAudio();
        Audio.PlaySound(SOUND_EFFECT_SPINNER, AUDIO_PLAY_TYPE_WAV_TRIGGER, SoundEffectsVolume);
        Audio.SetSoundFXVolume(SoundEffectsVolume);
        SoundSettingTimeout = CurrentTime + 5000;
      } else if (Menus.GetSubLevel()==OM_BASIC_ADJ_IDS_CALLOUTS_VOLUME) {
        if (SoundSettingTimeout) Audio.StopAllAudio();
        Audio.PlaySound(SOUND_EFFECT_VP_SHOOT_AGAIN, AUDIO_PLAY_TYPE_WAV_TRIGGER, CalloutsVolume);
        Audio.SetNotificationsVolume(CalloutsVolume);
        SoundSettingTimeout = CurrentTime + 3000;        
      }
    } else if (Menus.GetTopLevel()==OPERATOR_MENU_GAME_ADJ_MENU) {
      if (Menus.GetSubLevel()==OM_GAME_ADJ_SLINGSHOT_STRENGTH) {
        SolenoidAssociatedSwitches[0].solenoidHoldTime = TempSlingStrength;
        SolenoidAssociatedSwitches[1].solenoidHoldTime = TempSlingStrength;
      }
    }
  }

  if (SoundSettingTimeout && CurrentTime>SoundSettingTimeout) {
    SoundSettingTimeout = 0;
    Audio.StopAllAudio();
  }

  if (SoundTestStart && CurrentTime>SoundTestStart) {
/*    
    if (SoundTestSequence==0) {
      PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_1);
      SoundTestSequence = 1;
    } else if (SoundTestSequence==1 && CurrentTime>(SoundTestStart+5000)) {
      PlaySoundEffect(SOUND_EFFECT_SPINNER);
      SoundTestSequence = 2;
    } else if (SoundTestSequence==2 && CurrentTime>(SoundTestStart+10000)) {
      Audio.QueuePrioritizedNotification(SOUND_EFFECT_VP_EXTRA_BALL, 0, 10, CurrentTime);
      SoundTestSequence = 3;
    }
*/
    RPU_SetDisplay(0, SoundTestSequence, true, 0);
    Audio.PlaySound(SoundTestSequence, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
    SoundTestSequence += 1;
    SoundTestStart = CurrentTime + 1000;
    if (SoundTestSequence>31) SoundTestSequence = 0;
        
  }
  
}



////////////////////////////////////////////////////////////////////////////
//
//  Audio Output functions
//
////////////////////////////////////////////////////////////////////////////
void PlayBackgroundSong(unsigned int songNum) {
  // SB-100 does not support background music — no-op until WAV trigger is added
  (void)songNum;
}


unsigned long NextSoundEffectTime = 0;

void PlaySoundEffect(unsigned int soundEffectNum) {

  if (MachineState == MACHINE_STATE_INIT_GAMEPLAY) return;
  if (DEBUG_MESSAGES) {
    char buf[64];
    sprintf(buf, "SND: %d @ %lu ms\n", soundEffectNum, CurrentTime);
    Serial.write(buf);
  }

  // Route all game sounds through the Stern SB-100 sound board.
  // Each SOUND_EFFECT_* maps to one of the six SB-100 tones.
  byte sb100Sound;
  switch (soundEffectNum) {
    case SOUND_EFFECT_SCORE_TICK:
      sb100Sound = SND_10_POINTS;
      break;
    case SOUND_EFFECT_POP_BUMPER:
      // sb100Sound = SND_POP_BUMPER;  // TEMP: disabled to debug
      return;  // Skip pop bumper sounds
      break;
    case SOUND_EFFECT_TILT_WARNING:
      sb100Sound = SND_1000_POINTS;
      break;
    case SOUND_EFFECT_TILT:
    case SOUND_EFFECT_GAME_OVER:
    case SOUND_EFFECT_STARTUP_1:
    case SOUND_EFFECT_STARTUP_2:
      sb100Sound = SND_10000_POINTS;
      break;
    default:
      // Covers: spinner, slings, drop targets, bonus countdown, coin drops,
      // match spin, and any unspecified effect
      sb100Sound = SND_100_POINTS;
      break;
  }
  Audio.PlaySound(sb100Sound, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  Audio.QueueSound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, CurrentTime + 150);

  switch (soundEffectNum) {
      /*
          Placeholder for future WAV trigger cases if added later.
            Audio.PlaySoundCardWhenPossible(19, CurrentTime, 1500, 10, 4);
            break;
          case SOUND_EFFECT_HOOFBEATS:
            Audio.PlaySoundCardWhenPossible(12, CurrentTime, 0, 100, 10);
            break;
          case SOUND_EFFECT_STOP_BACKGROUND:
            Audio.PlaySoundCardWhenPossible(19, CurrentTime, 0, 10, 10);
            break;
          case SOUND_EFFECT_DROP_TARGET_HIT:
            Audio.PlaySoundCardWhenPossible(7, CurrentTime, 0, 150, 5);
            break;
          case SOUND_EFFECT_SPINNER:
            Audio.PlaySoundCardWhenPossible(6, CurrentTime, 0, 25, 2);
            break;
      */
  }
}


void PlayScoreSounds(int score) {
  byte tone;
  int count;

  if (score >= 10000 && score % 10000 == 0) {
    tone = SND_10000_POINTS;
    count = score / 10000;
  } else if (score >= 1000 && score % 1000 == 0) {
    tone = SND_1000_POINTS;
    count = score / 1000;
  } else if (score >= 100 && score % 100 == 0) {
    tone = SND_100_POINTS;
    count = score / 100;
  } else {
    tone = SND_10_POINTS;
    count = score / 10;
  }

  for (int i = 0; i < count; i++) {
    Audio.QueueSound(tone, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, CurrentTime + (i * 100));
    Audio.QueueSound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, CurrentTime + (i * 100) + 150);
  }
}


void QueueNotification(unsigned int soundEffectNum, byte priority) {
  (void)soundEffectNum;
  (void)priority;
  // SB-100 has no voice callouts — uncomment below if WAV trigger is added later
  // if (CalloutsVolume == 0) return;
  // Audio.PlaySound(SND_10000_POINTS, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  // Audio.QueueSound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, CurrentTime + 150);
}


void AlertPlayerUp() {
  QueueNotification(SOUND_EFFECT_VP_PLAYER_1_UP + CurrentPlayer, 1);
}




////////////////////////////////////////////////////////////////////////////
//
//  Diagnostics Mode
//
////////////////////////////////////////////////////////////////////////////

int RunDiagnosticsMode(int curState, boolean curStateChanged) {

  int returnState = curState;

  if (curStateChanged) {

    /*
        char buf[256];
        boolean errorSeen;

        Serial.write("Testing Volatile RAM at IC13 (0x0000 - 0x0080): writing & reading... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        errorSeen = false;
        for (byte valueCount=0; valueCount<0xFF; valueCount++) {
          for (unsigned short address=0x0000; address<0x0080; address++) {
            RPU_DataWrite(address, valueCount);
          }
          for (unsigned short address=0x0000; address<0x0080; address++) {
            byte readValue = RPU_DataRead(address);
            if (readValue!=valueCount) {
              sprintf(buf, "Write/Read failure at address=0x%04X (expected 0x%02X, read 0x%02X)\n", address, valueCount, readValue);
              Serial.write(buf);
              errorSeen = true;
            }
            if (errorSeen) break;
          }
          if (errorSeen) break;
        }
        if (errorSeen) {
          Serial.write("!!! Error in Volatile RAM\n");
        }

        Serial.write("Testing Volatile RAM at IC16 (0x0080 - 0x0100): writing & reading... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        errorSeen = false;
        for (byte valueCount=0; valueCount<0xFF; valueCount++) {
          for (unsigned short address=0x0080; address<0x0100; address++) {
            RPU_DataWrite(address, valueCount);
          }
          for (unsigned short address=0x0080; address<0x0100; address++) {
            byte readValue = RPU_DataRead(address);
            if (readValue!=valueCount) {
              sprintf(buf, "Write/Read failure at address=0x%04X (expected 0x%02X, read 0x%02X)\n", address, valueCount, readValue);
              Serial.write(buf);
              errorSeen = true;
            }
            if (errorSeen) break;
          }
          if (errorSeen) break;
        }
        if (errorSeen) {
          Serial.write("!!! Error in Volatile RAM\n");
        }

        // Check the CMOS RAM to see if it's operating correctly
        errorSeen = false;
        Serial.write("Testing CMOS RAM: writing & reading... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        for (byte valueCount=0; valueCount<0x10; valueCount++) {
          for (unsigned short address=0x0100; address<0x0200; address++) {
            RPU_DataWrite(address, valueCount);
          }
          for (unsigned short address=0x0100; address<0x0200; address++) {
            byte readValue = RPU_DataRead(address);
            if ((readValue&0x0F)!=valueCount) {
              sprintf(buf, "Write/Read failure at address=0x%04X (expected 0x%02X, read 0x%02X)\n", address, valueCount, (readValue&0x0F));
              Serial.write(buf);
              errorSeen = true;
            }
            if (errorSeen) break;
          }
          if (errorSeen) break;
        }

        if (errorSeen) {
          Serial.write("!!! Error in CMOS RAM\n");
        }


        // Check the ROMs
        Serial.write("CMOS RAM dump... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        for (unsigned short address=0x0100; address<0x0200; address++) {
          if ((address&0x000F)==0x0000) {
            sprintf(buf, "0x%04X:  ", address);
            Serial.write(buf);
          }
      //      RPU_DataWrite(address, address&0xFF);
          sprintf(buf, "0x%02X ", RPU_DataRead(address));
          Serial.write(buf);
          if ((address&0x000F)==0x000F) {
            Serial.write("\n");
          }
        }

    */

    //    RPU_EnableSolenoidStack();
    //    RPU_SetDisableFlippers(false);

  }

  return returnState;
}




////////////////////////////////////////////////////////////////////////////
//
//  Attract Mode
//
////////////////////////////////////////////////////////////////////////////
byte AttractLastHeadMode = 255;
byte AttractLastPlayfieldPhase = 255;
boolean AttractCheckedForTrappedBall;
unsigned long AttractModeStartTime;

int RunAttractMode(int curState, boolean curStateChanged) {

  int returnState = curState;

  if (curStateChanged) {
    // Some sound cards have a special index
    // for a "sound" that will turn off
    // the current background drone or currently
    // playing sound
    RPU_DisableSolenoidStack();
    RPU_TurnOffAllLamps();
    RPU_SetDisableFlippers(true);
    AttractLastPlayfieldPhase = 255;
    AttractLastHeadMode = 0;
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    Display_ClearOverride(0xFF);
    Display_UpdateDisplays(0xFF);
    AttractCheckedForTrappedBall = false;
    AttractModeStartTime = CurrentTime;
    RPU_SetLampState(LAMP_GAME_OVER, 1);
    RPU_SetLampState(LAMP_BALL_IN_PLAY, 0);

  }

  if (CurrentTime > (AttractModeStartTime + 5000) && !AttractCheckedForTrappedBall) {
    AttractCheckedForTrappedBall = true;
    if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
      RPU_PushToSolenoidStack(SOL_SAUCER, SaucerSolenoidStrength, true);
    }
  }

  // Alternate displays between high score and blank
  if (CurrentTime < 16000) {
    if (AttractLastHeadMode != 1) {
      RPU_SetDisplayCredits(Credits, !FreePlayMode);
      RPU_SetDisplayBallInPlay(0, true);
      Display_ClearOverride(0xFF);
    }
    AttractLastHeadMode = 1;
    Display_UpdateDisplays(0xFF);
    RPU_SetLampState(LAMP_HEAD_HIGH_SCORE, 0);
  } else if ((CurrentTime / 8000) % 2 == 0) {

    if (AttractLastHeadMode != 2) {
      Display_SetLastTimeScoreChanged(CurrentTime);
    }
    AttractLastHeadMode = 2;
    Display_UpdateDisplays(0xFF, false, false, false, HighScore);
    RPU_SetLampState(LAMP_HEAD_HIGH_SCORE, 1);
    for (byte count=0; count<4; count++) {
      RPU_SetLampState(PlayerUpLamps[count], 0);
    }
  } else {
    if (AttractLastHeadMode != 3) {
      if (CurrentTime < 32000) {
        for (int count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
          CurrentScores[count] = 0;
        }
        CurrentNumPlayers = 0;
      }
      Display_SetLastTimeScoreChanged(CurrentTime);
    }

    RPU_SetLampState(LAMP_HEAD_HIGH_SCORE, 0);
    for (byte count=0; count<4; count++) {
      if (count<CurrentNumPlayers) Display_UpdateDisplays(count, false, false, false, CurrentScores[count]);
      else RPU_SetDisplayBlank(count, 0x00);
    }

    AttractLastHeadMode = 3;
  }

  byte attractPlayfieldPhase = ((CurrentTime / 5000) % 6);

  if (attractPlayfieldPhase != AttractLastPlayfieldPhase) {
    RPU_TurnOffAllLamps();
    AttractLastPlayfieldPhase = attractPlayfieldPhase;
  }
  ShowLampAnimation(attractPlayfieldPhase, 75, CurrentTime, 18, false, false);

  byte switchHit;
  while ( (switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
    if (switchHit == SW_CREDIT_RESET) {
      if (AddPlayer(true)) returnState = MACHINE_STATE_INIT_GAMEPLAY;
    } else if (switchHit == SW_COIN_1 || switchHit == SW_COIN_3 || switchHit == SW_COIN_2) {
      AddCoinToAudit(SwitchToChuteNum(switchHit));
      AddCoin(SwitchToChuteNum(switchHit));
    } else if (switchHit == SW_SELF_TEST_SWITCH) {
      Menus.EnterOperatorMenu();
      Menus.SetCreditAndBIPRestore(FreePlayMode?0xFF:Credits, 0);
    } else {
#ifdef DEBUG_MESSAGES
      char buf[128];
      sprintf(buf, "sw %d\n", switchHit);
      Serial.write(buf);
#endif       
    }
  }

  // If the user was holding the menu button when the game started
  // then kick the balls
  if (CurrentTime < 4000) {
    if (RPU_ReadSingleSwitchState(SW_SELF_TEST_SWITCH)) {
      if (OperatorSwitchPressStarted==0) {
        OperatorSwitchPressStarted = CurrentTime;
      } else if (CurrentTime > (OperatorSwitchPressStarted+500)) {
        Menus.EnterOperatorMenu();
        Menus.SetCreditAndBIPRestore(FreePlayMode?0xFF:Credits, 0);
        Menus.BallEjectInProgress(true);
      }
    } else {
      OperatorSwitchPressStarted = 0;
    }
  }

  return returnState;
}





////////////////////////////////////////////////////////////////////////////
//
//  Game Play functions
//
////////////////////////////////////////////////////////////////////////////
byte CountBits(unsigned short intToBeCounted) {
  byte numBits = 0;

  for (byte count = 0; count < 16; count++) {
    numBits += (intToBeCounted & 0x01);
    intToBeCounted = intToBeCounted >> 1;
  }

  return numBits;
}


void SetGameMode(byte newGameMode) {
  LastGameMode = GameMode;
  GameMode = newGameMode;
  GameModeStartTime = 0;
  GameModeEndTime = 0;

  if (DEBUG_MESSAGES) {
    char buf[128];
    // Debug disabled - clean output for sound testing
  }
}

byte CountBallsInTrough() {

  byte numBalls = RPU_ReadSingleSwitchState(SW_OUTHOLE);

  return numBalls;
}



void AddToBonus(byte bonus) {
  if (bonus == 3) {
    BonusBefore = Bonus[CurrentPlayer];
    BonusAnimationStart = CurrentTime;
  }
  Bonus[CurrentPlayer] += bonus;
  if (Bonus[CurrentPlayer] > MAX_DISPLAY_BONUS) {
    Bonus[CurrentPlayer] = MAX_DISPLAY_BONUS;
  } else {
    BonusChangedTime = CurrentTime;
  }
}

void PlayBonusAdvanceSound() {
  if (DEBUG_SWITCH_LOGGING) {
    Serial.write("BONUS ADVANCE SOUND PLAYED\n");
  }
  Audio.PlaySound(SND_1000_POINTS, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  if (DEBUG_SWITCH_LOGGING) {
    char buf[64];
    sprintf(buf, "  QUEUED: 0x0 @ +80ms, 0x4 @ +120ms\n");
    Serial.write(buf);
  }
  Audio.QueueSound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, CurrentTime + 80);
  Audio.QueueSound(SND_1000_POINTS, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, CurrentTime + 120);
}

void IncreaseBonusX() {
  if (BonusX[CurrentPlayer] < 10) {
    if (BonusX[CurrentPlayer] == 1) {
      BonusX[CurrentPlayer] = 2;
    } else if (BonusX[CurrentPlayer] == 2) {
      BonusX[CurrentPlayer] = 3;
    } else if (BonusX[CurrentPlayer] == 3) {
      BonusX[CurrentPlayer] = 5;
    } else if (BonusX[CurrentPlayer] == 5) {
      BonusX[CurrentPlayer] = 7;
    } else if (BonusX[CurrentPlayer] == 7) {
      BonusX[CurrentPlayer] = 8;
    } else if (BonusX[CurrentPlayer] == 8) {
      BonusX[CurrentPlayer] = 10;
    }
    BonusXAnimationStart = CurrentTime;
  }

}



unsigned long GameStartNotificationTime = 0;
byte GameStartMelodyStep = 0;
byte BallSearchPhase = 0;
byte BallSearchAttempts = 0;
unsigned long LastBallSearchFire = 0;

int InitGamePlay(boolean curStateChanged) {

  if (curStateChanged) {
    RPU_TurnOffAllLamps();
    RPU_SetLampState(LAMP_2X, 0);
    RPU_SetLampState(LAMP_3X, 0);
    RPU_SetLampState(LAMP_5X, 0);
    SetGeneralIlluminationOn(true);
    GameStartNotificationTime = CurrentTime;
    GameStartMelodyStep = 0;
    Audio.StopAllAudio();
    for (byte count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) RPU_SetDisplayBlank(count, 0x00);
    RPU_SetDisplayCredits(0, false);
    RPU_SetDisplayBallInPlay(1, true);
    NumberOfBallsLocked = 0;

    // Initialize custom game state variables for all players
    for (byte p = 0; p < RPU_NUMBER_OF_PLAYERS_ALLOWED; p++) {
      isSaucerLit[p] = false;
      isArcSurgeActive[p] = false;
      arcSurgeT1Hit[p] = false;
      isLeftReturnLaneLit[p] = false;
      firstHitMade[p] = false;
      threeBankCompleteCount[p] = 0;
      fiveBankCompleteCount[p] = 0;
      ExtraBallCollectedThisBall[p] = false;
      ExtraBallLaneAvailable[p] = false;
      spinnerHitCount[p] = 0;
      arcSurgeTimerStart[p] = 0;
      threeBankSweepStartTime[p] = 0;
      threeBankSweepAnimationStart[p] = 0;
    }

    // Clear any balls stuck from a previous game — only check once on entry
    if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
      RPU_PushToSolenoidStack(SOL_SAUCER, SaucerSolenoidStrength, true);
    }
    if (RPU_ReadSingleSwitchState(SW_KICKER)) {
      RPU_PushToSolenoidStack(SOL_KICKER, 16, true);
    }

    RPU_SetCoinLockout((Credits >= MaximumCredits) ? true : false);

    for (int count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
      Bonus[count] = 0;
      BonusX[count] = 1;
      ExtraBallsAvailable[count] = 1;
      CurrentAchievements[count] = 0;
      CurrentScores[count] = 0;
    }

    SamePlayerShootsAgain = false;
    CurrentBallInPlay = 1;
    CurrentNumPlayers = 1;
    CurrentPlayer = 0;
    NumberOfBallsInPlay = 0;
    LastTiltWarningTime = 0;
    Display_ClearOverride(0xFF);
    Display_UpdateDisplays(0xFF);
    RPU_EnableSolenoidStack();
    RPU_SetDisableFlippers(false);

    // Clear sound queue at game start
    Audio.ClearSoundQueue();
  }

  boolean showBIP = (CurrentTime / 100) % 2;
  RPU_SetDisplayBallInPlay(1, showBIP ? true : false);

  return MACHINE_STATE_INIT_NEW_BALL;
}



int InitNewBall(bool curStateChanged) {

  // If we're coming into this mode for the first time
  // then we have to do everything to set up the new ball
  if (curStateChanged) {
    //RPU_FireContinuousSolenoid(0x20, 5);
    RPU_TurnOffAllLamps();
    RPU_EnableSolenoidStack();
    RPU_SetDisableFlippers(false);
    BallFirstSwitchHitTime = 0;

    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    if (CurrentNumPlayers > 1 && (CurrentBallInPlay != 1 || CurrentPlayer != 0) && !SamePlayerShootsAgain) AlertPlayerUp();
    SamePlayerShootsAgain = false;

    RPU_SetDisplayBallInPlay(CurrentBallInPlay);
    RPU_SetLampState(LAMP_HEAD_TILT, 0);
    for (byte count = 0; count < 4; count++) {
      if (count==CurrentPlayer) RPU_SetLampState(PlayerUpLamps[count], 1, 0, 250);
      else if (count<CurrentNumPlayers) RPU_SetLampState(PlayerUpLamps[count], 1);
      else RPU_SetLampState(PlayerUpLamps[count], 0);
      RPU_SetDisplayBlank(count, 0);
    }

    if (BallSaveNumSeconds > 0) {
      RPU_SetLampState(LAMP_SHOOT_AGAIN, 1, 0, 500);
    }

    BallSaveUsed = false;
    BallTimeInTrough = 0;
    NumTiltWarnings = 0;
    
    ExtraBallsAvailable[CurrentPlayer] = 1;

    // Clear sound queue at start of ball to remove any stale data
    Audio.ClearSoundQueue();

    // Initialize game-specific start-of-ball lights & variables
    GameModeStartTime = 0;
    GameModeEndTime = 0;
    GameMode = GAME_MODE_SKILL_SHOT;

    SpecialCollected = false;
    SpecialAvailable = false;
    KickerBonusCollect = false;
    CollectBonusViaKicker = false;
    LastSpinnerHitTime = 0;
    ExtraBallCollectedThisBall[CurrentPlayer] = false;
    ExtraBallLaneAvailable[CurrentPlayer] = false;

    isSaucerLit[CurrentPlayer] = false;
    isArcSurgeActive[CurrentPlayer] = false;
    arcSurgeT1Hit[CurrentPlayer] = false;
    isLeftReturnLaneLit[CurrentPlayer] = false;
    firstHitMade[CurrentPlayer] = false;
    arcSurgeTimerStart[CurrentPlayer] = 0;
    spinnerHitCount[CurrentPlayer] = 0;
    threeBankSweepStartTime[CurrentPlayer] = 0;
    threeBankSweepAnimationStart[CurrentPlayer] = 0;
    // Stagger drop target resets to avoid simultaneous solenoid firing
    if (ThreeBank.GetStatus() != 0) {
      ThreeBank.ResetDropTargets(CurrentTime + 300, true, true);
    }
    if (FiveBank.GetStatus() != 0) {
      FiveBank.ResetDropTargets(CurrentTime + 600, true, true);
    }
    BallSearchPhase = 0;
    BallSearchAttempts = 0;
    LastBallSearchFire = 0;

    PlayfieldMultiplier = 1;
    PlayfieldMultiplierTimeLeft = 0;
    BonusXAnimationStart = 0;
    Bonus[CurrentPlayer] = 1;
    BonusChangedTime = 0;
    BonusX[CurrentPlayer] = 1;
    RPU_SetLampState(LAMP_2X, 0);
    RPU_SetLampState(LAMP_3X, 0);
    RPU_SetLampState(LAMP_5X, 0);
    threeBankCompleteCount[CurrentPlayer] = 0;
    fiveBankCompleteCount[CurrentPlayer] = 0;
    ExtraBallLaneAvailable[CurrentPlayer] = false;
    Display_ResetDisplayTrackingVariables();

    SetBallSave(0);

    RPU_PushToTimedSolenoidStack(SOL_OUTHOLE, BallServeSolenoidStrength, CurrentTime + 950, false);
    LastTimeBallServed = CurrentTime + 1000;
    
    NumberOfBallsInPlay = 1;

    Audio.OutputTracksPlaying();
    PlayBackgroundSong(SOUND_EFFECT_BALL_MUSIC_1 + (CurrentBallInPlay-1));
    Audio.OutputTracksPlaying();
  }

  return MACHINE_STATE_NORMAL_GAMEPLAY;
}











byte GameModeStage;
boolean DisplaysNeedRefreshing = false;
unsigned long LastTimePromptPlayed = 0;
unsigned long LastTimeAwardsChecked = 0;
unsigned long LastTimeJackpotAdjusted = 0;
unsigned long LastLoopTick = 0;






void CheckForStuckBalls() {
  // Check saucer for stuck ball
  if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
    if (SaucerClosedStart == 0) {
      SaucerClosedStart = CurrentTime;
    } else if (SaucerClosedStart != 1) {
      if (CurrentTime > (SaucerClosedStart + STUCK_BALL_SETTLE_TIME_MS)) {
        SaucerClosedStart = 1;
        RPU_PushToSolenoidStack(SOL_SAUCER, SaucerSolenoidStrength, true);
      }
    }
  } else {
    SaucerClosedStart = 0;
  }

  // Check kicker for stuck ball — skip during bonus collect (kicker fires at end of collect)
  // Use dual lockout: either the eject is recent OR the switch hasn't fully released yet
  // This prevents double-fire from bounces during eject motion
  boolean switchOpen = !RPU_ReadSingleSwitchState(SW_KICKER);
  boolean inKickerEjectLockout = (KickerEjectTime != 0 && (CurrentTime - KickerEjectTime) < KICKER_EJECT_LOCKOUT_MS) ||
                                  (KickerSwitchReleaseTime != 0 &&
                                   (CurrentTime - KickerSwitchReleaseTime) < KICKER_RELEASE_DEBOUNCE_MS);

  if (switchOpen) {
    // Switch is open; track when it released
    if (KickerSwitchReleaseTime == 0) {
      KickerSwitchReleaseTime = CurrentTime;
    }
  } else {
    // Switch is closed; reset release timer (it hasn't fully released yet)
    KickerSwitchReleaseTime = 0;
  }

  // Stuck-ball detector: fire kicker if held closed for 500ms, but only if eject lockout is clear
  if (!KickerBonusCollect && !inKickerEjectLockout && RPU_ReadSingleSwitchState(SW_KICKER)) {
    if (KickerClosedStart == 0) {
      KickerClosedStart = CurrentTime;
    } else if (KickerClosedStart != 1) {
      if (CurrentTime > (KickerClosedStart + STUCK_BALL_SETTLE_TIME_MS)) {
        KickerClosedStart = 1;
        RPU_PushToSolenoidStack(SOL_KICKER, 16, true);
        // Mark the eject so future stuck-ball checks know we just fired
        KickerEjectTime = CurrentTime;
      }
    }
  } else {
    KickerClosedStart = 0;
  }
}


// This function manages all timers, flags, and lights
int ManageGameMode() {
  int returnState = MACHINE_STATE_NORMAL_GAMEPLAY;

  boolean specialAnimationRunning = false;
  boolean statusRunning = false;

  if ((CurrentTime - LastSwitchHitTime) > 3000) TimersPaused = true;
  else TimersPaused = false;

  CheckForStuckBalls();

  // Process any pending score updates (synced with audio playback)
  ProcessPendingScoreUpdates();

  // Game start melody: 4-note ascending chime played twice while ball is in shooter lane (ball 1, player 1 only)
  // Matches boot melody timing (150ms spacing)
  if (CurrentBallInPlay == 1 && CurrentPlayer == 0 &&
      GameStartMelodyStep < 9 && BallFirstSwitchHitTime == 0 &&
      GameStartNotificationTime > 0) {
    unsigned long elapsed = CurrentTime - GameStartNotificationTime;
    byte targetStep = (byte)(elapsed / 150);
    if (targetStep > 9) targetStep = 9;
    while (GameStartMelodyStep < targetStep) {
      if (GameStartMelodyStep < 8) {
        static const byte melodyNotes[4] = {SND_10000_POINTS, SND_1000_POINTS, SND_100_POINTS, SND_10_POINTS};
        Audio.PlaySound(melodyNotes[GameStartMelodyStep % 4], AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
      } else {
        Audio.PlaySound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
      }
      GameStartMelodyStep++;
    }
  }

  // Ball search: fire solenoids if no switch hit for 25 seconds while ball is on playfield
  // Skip ball search during bonus collection (give ball time to settle)
  if (CollectBonusViaKicker && CurrentTime > BonusCollectionEndTime) {
    CollectBonusViaKicker = false;
  }

  if (BallFirstSwitchHitTime != 0 && NumTiltWarnings <= MaxTiltWarnings && !CollectBonusViaKicker) {
    unsigned long timeSinceSwitch = CurrentTime - LastSwitchHitTime;
    if (timeSinceSwitch > 25000UL) {
      if (BallSearchAttempts < 12 &&
          (LastBallSearchFire == 0 || (CurrentTime - LastBallSearchFire) > 1000UL)) {
        switch (BallSearchPhase % 4) {
          case 0: RPU_PushToSolenoidStack(SOL_SAUCER, SaucerSolenoidStrength, true); break;
          case 1: RPU_PushToSolenoidStack(SOL_KICKER, 16, true);                     break;
          case 2: RPU_PushToSolenoidStack(SOL_DROP_TARGET_3BANK_RESET, 10, true);    break;
          case 3: RPU_PushToSolenoidStack(SOL_DROP_TARGET_5BANK_RESET, 10, true);    break;
        }
        BallSearchPhase++;
        BallSearchAttempts++;
        LastBallSearchFire = CurrentTime;
      }
    } else if (BallSearchAttempts > 0) {
      // Ball found — reset search state so it can trigger again later if needed
      BallSearchPhase = 0;
      BallSearchAttempts = 0;
      LastBallSearchFire = 0;
    }
  }

  switch ( GameMode ) {
    case GAME_MODE_SKILL_SHOT:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
        LastTimePromptPlayed = CurrentTime;
        GameModeStage = 0;
        SetGeneralIlluminationOn(true);
      }

      // The switch handler will award the skill shot
      // (when applicable) and this mode will move
      // to unstructured play when any valid switch is
      // recorded

      if (CurrentTime > (LastTimePromptPlayed + 20000)) {
        //RPU_FireContinuousSolenoid(0x20, 30);
        AlertPlayerUp();
        LastTimePromptPlayed = CurrentTime;
        Audio.OutputTracksPlaying();
      }
      
      // If we've seen a tilt before plunge, then
      // we can show a countdown timer here
      if (LastTiltWarningTime) {
        if ( CurrentTime > (LastTiltWarningTime + 30000) ) {
          LastTiltWarningTime = 0;
        } else {
          byte secondsSinceWarning = (CurrentTime - LastTiltWarningTime) / 1000;
          for (byte count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
            if (count == CurrentPlayer && !statusRunning) Display_OverrideScoreDisplay(count%RPU_NUMBER_OF_PLAYER_DISPLAYS, 30 - secondsSinceWarning, DISPLAY_OVERRIDE_ANIMATION_CENTER);
          }
          DisplaysNeedRefreshing = true;
        }
      } else if (DisplaysNeedRefreshing) {
        DisplaysNeedRefreshing = false;
        if (!statusRunning) Display_ClearOverride(0xFF);
      } else {
        if ( ((CurrentTime/1000)%10)>7 ) {        
          if (GameModeStage!=2) {
//            for (byte count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
//              if (!statusRunning && count==CurrentPlayer) Display_OverrideScoreDisplay(count%RPU_NUMBER_OF_PLAYER_DISPLAYS, ((unsigned long)CurrentPlayer + 1) * 111111, DISPLAY_OVERRIDE_ANIMATION_FLUTTER);
//            }
            GameModeStage = 2;
          }
        } else {
          if (GameModeStage!=1) {
            if (!statusRunning) Display_ClearOverride(0xFF);
            GameModeStage = 1;
          }
        }
      }

      if (BallFirstSwitchHitTime != 0) {
        Display_ClearOverride(0xFF);
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_UNSTRUCTURED_PLAY:
      // If this is the first time in this mode
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        DisplaysNeedRefreshing = true;
        SetGeneralIlluminationOn(true);
        unsigned short songNum = SOUND_EFFECT_BACKGROUND_SONG_1;
        if (CurrentBallInPlay>1 && CurrentBallInPlay<5) songNum = SOUND_EFFECT_BACKGROUND_SONG_2 + (CurrentTime%3);
        if (CurrentBallInPlay==BallsPerGame) songNum = SOUND_EFFECT_BACKGROUND_SONG_5;
        if (Audio.GetBackgroundSong() != songNum) {
          PlayBackgroundSong(songNum);
        }
        GameModeStage = 0;
        LastTimePromptPlayed = 0;
      }

      // Display Overrides in Unstructured Play
      if (PlayfieldMultiplier > 1) {
        // Playfield X value is only reset during unstructured play
        if (PlayfieldMultiplierTimeLeft && (CurrentTime > LastLoopTick)) {
          unsigned long numTicks = CurrentTime - LastLoopTick;
          if (numTicks > PlayfieldMultiplierTimeLeft) {
            PlayfieldMultiplierTimeLeft = 0;
            PlayfieldMultiplier = 1;
          } else {
            PlayfieldMultiplierTimeLeft -= numTicks;
          }
        } else {
          for (byte count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
            if (count != CurrentPlayer && !statusRunning) Display_OverrideScoreDisplay(count, PlayfieldMultiplier, DISPLAY_OVERRIDE_SYMMETRIC_BOUNCE);
          }
          DisplaysNeedRefreshing = true;
        }
      } else if (DisplaysNeedRefreshing) {
        DisplaysNeedRefreshing = false;
        Display_ClearOverride(0xFF);
      }

      if (isArcSurgeActive[CurrentPlayer] && (CurrentTime - arcSurgeTimerStart[CurrentPlayer]) > TIME_ARC_SURGE_COMBO_MS) {
        isArcSurgeActive[CurrentPlayer] = false;
        arcSurgeT1Hit[CurrentPlayer] = false;
      }

      break;

  }

  if ( !statusRunning && !specialAnimationRunning && NumTiltWarnings <= MaxTiltWarnings ) {
    ShowBonusLamps();
    ShowBonusXLamps();
    ShowThreeBankTargetLamps();
    ShowShootAgainLamp();
    ShowGameplayLamps();
  }
  ShowPlayerLamps();

  // Credit display priority: Arc Surge countdown > spinner count > normal credits
  if (isArcSurgeActive[CurrentPlayer] && arcSurgeTimerStart[CurrentPlayer] > 0) {
    unsigned long arcElapsed = CurrentTime - arcSurgeTimerStart[CurrentPlayer];
    if (arcElapsed < TIME_ARC_SURGE_COMBO_MS) {
      byte secsLeft = (byte)((TIME_ARC_SURGE_COMBO_MS - arcElapsed + 999) / 1000);
      RPU_SetDisplayCredits(secsLeft, true);
    } else {
      RPU_SetDisplayCredits(Credits, !FreePlayMode);
    }
  } else if (LastSpinnerHitTime > 0 && (CurrentTime - LastSpinnerHitTime) < 1500) {
    RPU_SetDisplayCredits(spinnerHitCount[CurrentPlayer], true);
  } else {
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
  }

  Display_UpdateDisplays(0xFF, false, (BallFirstSwitchHitTime == 0) ? true : false, (BallFirstSwitchHitTime > 0 && ((CurrentTime - Display_GetLastTimeScoreChanged()) > 2000)) ? DISPLAY_DASH_STYLE : false);

  // Check to see if ball is in the outhole
  if (CountBallsInTrough() > (TotalBallsLoaded - (NumberOfBallsInPlay + NumberOfBallsLocked))) {

    if (BallTimeInTrough == 0) {
      // If this is the first time we're seeing too many balls in the trough, we'll wait to make sure
      // everything is settled
      BallTimeInTrough = CurrentTime;
    } else {

      // Make sure the ball stays on the sensor for at least
      // 0.5 seconds to be sure that it's not bouncing or passing through
      if ((CurrentTime - BallTimeInTrough) > 750) {

        if ((BallFirstSwitchHitTime == 0 && NumTiltWarnings <= MaxTiltWarnings)) {
          // Nothing hit yet, so return the ball to the player
          RPU_PushToTimedSolenoidStack(SOL_OUTHOLE, BallServeSolenoidStrength, CurrentTime);
          BallTimeInTrough = 0;
          returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
        } else {
          // if we haven't used the ball save, and we're under the time limit, then save the ball
          if (BallSaveEndTime && CurrentTime < (BallSaveEndTime + BALL_SAVE_GRACE_PERIOD)) {
            RPU_PushToTimedSolenoidStack(SOL_OUTHOLE, BallServeSolenoidStrength, CurrentTime + 100);

            RPU_SetLampState(LAMP_SHOOT_AGAIN, 0);
            BallTimeInTrough = CurrentTime;
            returnState = MACHINE_STATE_NORMAL_GAMEPLAY;

            if (NumberOfBallSavesRemaining && NumberOfBallSavesRemaining != 0xFF) {
              NumberOfBallSavesRemaining -= 1;
              if (NumberOfBallSavesRemaining == 0) {
                BallSaveEndTime = 0;
                if (DEBUG_MESSAGES) {
                  Serial.write("Last ball save\n");
                }
                QueueNotification(SOUND_EFFECT_VP_BALL_SAVE, 10);
              } else {
                if (DEBUG_MESSAGES) {
                  Serial.write("Not last ball save\n");
                }                
              }
            }

          } else {

            NumberOfBallsInPlay -= 1;
            if (NumberOfBallsInPlay == 0) {
              Display_ClearOverride(0xFF);
              Audio.StopAllAudio();
              //PlaySoundEffect(SOUND_EFFECT_BALL_OVER);
              returnState = MACHINE_STATE_COUNTDOWN_BONUS;
            }
          }
        }
      }
    }
  } else {
    BallTimeInTrough = 0;
  }

  LastLoopTick = CurrentTime;
  LastTimeThroughLoop = CurrentTime;
  return returnState;
}



unsigned long CountdownStartTime = 0;
unsigned long LastCountdownReportTime = 0;
unsigned long BonusCountDownEndTime = 0;
byte DecrementingBonusCounter;
byte IncrementingBonusXCounter;
byte TotalBonus = 0;
byte TotalBonusX = 0;
byte BonusSoundIncrement;
boolean CountdownBonusHurryUp = false;
int LastBonusSoundPlayed = 0;

int CountDownDelayTimes[] = {175, 175, 175, 175, 175, 175, 175, 175, 175, 175};

int CountdownBonus(boolean curStateChanged) {

  // If this is the first time through the countdown loop
  if (curStateChanged) {
    
    // Turn off solenoids
    //RPU_DisableSolenoidStack();

    if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
      RPU_PushToSolenoidStack(SOL_SAUCER, SaucerSolenoidStrength, true);
    }

    CountdownStartTime = CurrentTime;
    LastCountdownReportTime = CurrentTime;
    CollectBonusViaKicker = true;
    BonusCollectionEndTime = CurrentTime + 35000; // Disable ball search for 35 seconds (covers full bonus countdown including multiplier levels)
    ShowBonusLamps();
    ShowBonusXLamps();
    IncrementingBonusXCounter = 1;
    DecrementingBonusCounter = Bonus[CurrentPlayer];
    TotalBonus = Bonus[CurrentPlayer];
    TotalBonusX = BonusX[CurrentPlayer];
    CountdownBonusHurryUp = false;
    BonusSoundIncrement = 0;
    LastBonusSoundPlayed = 0;

    BonusCountDownEndTime = 0xFFFFFFFF;
    // Some sound cards have a special index
    // for a "sound" that will turn off
    // the current background drone or currently
    // playing sound
    //    PlaySoundEffect(SOUND_EFFECT_STOP_BACKGROUND);
  }

  unsigned long countdownDelayTime = (unsigned long)(CountDownDelayTimes[IncrementingBonusXCounter - 1]);
  if (CountdownBonusHurryUp && countdownDelayTime > ((unsigned long)CountDownDelayTimes[9])) countdownDelayTime = CountDownDelayTimes[9];

  if ((CurrentTime - LastCountdownReportTime) > countdownDelayTime) {

    if (DecrementingBonusCounter) {

      // Only give sound & score if this isn't a tilt
      if (NumTiltWarnings <= MaxTiltWarnings) {
        // Play bonus countdown sound via sequence (includes silence padding to prevent collisions)
        PlaySoundSequence(SEQ_BONUS_COUNT, 0);
        BonusSoundIncrement += 1;
        CurrentScores[CurrentPlayer] += BonusCountdownMultipleSteps ? ((unsigned long)TotalBonusX * 1000) : 1000;
      }

      DecrementingBonusCounter -= 1;
      Bonus[CurrentPlayer] = DecrementingBonusCounter;
      BonusXAnimationStart = CurrentTime;  // Trigger lamp flash to sync with sound
      ShowBonusLamps();

    } else if (BonusCountDownEndTime == 0xFFFFFFFF) {
      IncrementingBonusXCounter += 1;
      if (BonusX[CurrentPlayer] > 1 && !BonusCountdownMultipleSteps) {
        DecrementingBonusCounter = TotalBonus;
        Bonus[CurrentPlayer] = TotalBonus;
        ShowBonusLamps();
        BonusX[CurrentPlayer] -= 1;
        if (BonusX[CurrentPlayer] == 9) BonusX[CurrentPlayer] = 8;
      } else {
        BonusX[CurrentPlayer] = TotalBonusX;
        Bonus[CurrentPlayer] = TotalBonus;
        BonusCountDownEndTime = CurrentTime + 1000;
      }
    }
    LastCountdownReportTime = CurrentTime;
  }

  if (CurrentTime > BonusCountDownEndTime) {

    BonusCountDownEndTime = 0xFFFFFFFF;

    if (KickerBonusCollect) {
      if (DEBUG_MESSAGES) Serial.write("Kicker bonus collect done, ejecting ball\n");
      KickerBonusCollect = false;
      CollectBonusViaKicker = false;
      KickerClosedStart = 0;
      KickerEjectTime = CurrentTime;
      Bonus[CurrentPlayer] = 1;
      RPU_PushToTimedSolenoidStack(SOL_KICKER, 16, CurrentTime + 250, false);
      // Arm lockout at the moment solenoid fires (250ms from now) so it blocks bounces
      // when switch closes ~120ms later. This prevents stuck-ball detector from double-firing.
      KickerSwitchReleaseTime = CurrentTime + 250;
      return MACHINE_STATE_NORMAL_GAMEPLAY;
    }

    if (DEBUG_MESSAGES) Serial.write("Count down over, moving to ball over\n");
    if (LastBonusSoundPlayed != 0) Audio.StopSound(LastBonusSoundPlayed);
    return MACHINE_STATE_BALL_OVER;
  }

  return MACHINE_STATE_COUNTDOWN_BONUS;
}



void CheckHighScores() {
  unsigned long highestScore = 0;
  int highScorePlayerNum = 0;
  for (int count = 0; count < CurrentNumPlayers; count++) {
    if (CurrentScores[count] > highestScore) highestScore = CurrentScores[count];
    highScorePlayerNum = count;
  }

  if (highestScore > HighScore) {
    HighScore = highestScore;
    if (HighGameFreeGames > 0) {
      AddCredit(false, HighGameFreeGames);
      RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + HighGameFreeGames);
    }
    RPU_WriteULToEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, highestScore);
    RPU_WriteULToEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE) + 1);

    for (int count = 0; count < RPU_NUMBER_OF_PLAYERS_ALLOWED; count++) {
      if (count == highScorePlayerNum) {
        RPU_SetDisplay(count, CurrentScores[count], true, 2);
      } else {
        RPU_SetDisplayBlank(count, 0x00);
      }
    }

    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, CurrentTime, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, CurrentTime + 300, true);
    RPU_PushToTimedSolenoidStack(SOL_KNOCKER, KNOCKER_SOLENOID_STRENGTH, CurrentTime + 600, true);
  }
}


unsigned long MatchSequenceStartTime = 0;
unsigned long MatchDelay = 150;
byte MatchDigit = 0;
byte NumMatchSpins = 0;
byte ScoreMatches = 0;
boolean GameOverMelodyPlayed = false;

int ShowMatchSequence(boolean curStateChanged) {
  if (!MatchFeature) return MACHINE_STATE_ATTRACT;

  if (curStateChanged) {
    MatchSequenceStartTime = CurrentTime;
    GameOverMelodyPlayed = false;
    MatchDelay = 2500;
    MatchDigit = CurrentTime % 10;
    NumMatchSpins = 0;
    RPU_SetLampState(LAMP_HEAD_MATCH, 1, 0);
    RPU_SetDisableFlippers(true);
    ScoreMatches = 0;
  }

  // Play game over melody on first entry (hardcoded like boot startup for matching timing)
  if (!GameOverMelodyPlayed) {
    GameOverMelodyPlayed = true;
    for (byte rep = 0; rep < 2; rep++) {
      Audio.PlaySound(SND_10_POINTS, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
      for (int i = 0; i < 15; i++) { Audio.Update(millis()); delay(10); }

      Audio.PlaySound(SND_100_POINTS, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
      for (int i = 0; i < 15; i++) { Audio.Update(millis()); delay(10); }

      Audio.PlaySound(SND_1000_POINTS, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
      for (int i = 0; i < 15; i++) { Audio.Update(millis()); delay(10); }

      Audio.PlaySound(SND_10000_POINTS, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
      for (int i = 0; i < 15; i++) { Audio.Update(millis()); delay(10); }
    }
    Audio.PlaySound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  }

  if (NumMatchSpins < 40) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      MatchDigit += 1;
      if (MatchDigit > 9) MatchDigit = 0;
      //PlaySoundEffect(10+(MatchDigit%2));
      PlaySoundSequence(SEQ_MATCH_SPIN, 0);
      RPU_SetDisplayBallInPlay((int)MatchDigit * 10);
      MatchDelay += 50 + 4 * NumMatchSpins;
      NumMatchSpins += 1;
      RPU_SetLampState(LAMP_HEAD_MATCH, NumMatchSpins % 2, 0);

      if (NumMatchSpins == 40) {
        RPU_SetLampState(LAMP_HEAD_MATCH, 0);
        MatchDelay = CurrentTime - MatchSequenceStartTime;
      }
    }
  }

  if (NumMatchSpins >= 40 && NumMatchSpins <= 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      if ( (CurrentNumPlayers > (NumMatchSpins - 40)) && ((CurrentScores[NumMatchSpins - 40] / 10) % 10) == MatchDigit) {
        ScoreMatches |= (1 << (NumMatchSpins - 40));
        AddSpecialCredit();
        MatchDelay += 1000;
        NumMatchSpins += 1;
        RPU_SetLampState(LAMP_HEAD_MATCH, 1);
      } else {
        NumMatchSpins += 1;
      }
      if (NumMatchSpins == 44) {
        MatchDelay += 5000;
      }
    }
  }

  if (NumMatchSpins > 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      return MACHINE_STATE_ATTRACT;
    }
  }

  for (int count = 0; count < 4; count++) {
    if ((ScoreMatches >> count) & 0x01) {
      // If this score matches, we're going to flash the last two digits
      if ( (CurrentTime / 200) % 2 ) {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) & 0x0F);
      } else {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) | 0x30);
      }
    }
  }

  return MACHINE_STATE_MATCH_MODE;
}




////////////////////////////////////////////////////////////////////////////
//
//  Switch Handling functions
//
////////////////////////////////////////////////////////////////////////////

int HandleSystemSwitches(int curState, byte switchHit) {
  int returnState = curState;
  switch (switchHit) {
    case SW_SELF_TEST_SWITCH:
      Menus.EnterOperatorMenu();
      Menus.SetCreditAndBIPRestore(FreePlayMode?0xFF:Credits, CurrentBallInPlay);
      break;
    case SW_COIN_1:
    case SW_COIN_2:
    case SW_COIN_3:
      if (!FreePlayMode) {
        AddCoinToAudit(SwitchToChuteNum(switchHit));
        AddCoin(SwitchToChuteNum(switchHit));
      }
      break;
    case SW_CREDIT_RESET:
      if (MachineState == MACHINE_STATE_MATCH_MODE) {
        // If the first ball is over, pressing start again resets the game
        if (Credits >= 1 || FreePlayMode) {
          if (!FreePlayMode) {
            Credits -= 1;
            RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
            RPU_SetDisplayCredits(Credits, !FreePlayMode);
          }
          returnState = MACHINE_STATE_INIT_GAMEPLAY;
        }
      } else {
        CreditResetPressStarted = CurrentTime;
      }
      break;
    case SW_OUTHOLE:
      // Some machines have a kicker to move the ball
      // from the outhole to the re-shooter ramp
      break;
    case SW_TILT:
      if (BallFirstSwitchHitTime) {
        if ( CurrentTime > (LastTiltWarningTime + TILT_WARNING_DEBOUNCE_TIME) ) {
          LastTiltWarningTime = CurrentTime;
          NumTiltWarnings += 1;
          if (NumTiltWarnings > MaxTiltWarnings) {
            RPU_DisableSolenoidStack();
            RPU_SetDisableFlippers(true);
            RPU_TurnOffAllLamps();
            Audio.StopAllAudio();
            if (BallSaveEndTime) {
              BallSaveEndTime = 0;
              NumberOfBallSavesRemaining = 0;
            }
            RPU_SetLampState(LAMP_HEAD_TILT, 1);
            {
              unsigned int duration = PlaySoundSequence(SEQ_TILT, 0);
              protectedSoundUntilTime = CurrentTime + duration + 100;
            }
          } else {
            unsigned int duration = PlaySoundSequence(SEQ_TILT_WARNING, 0);
            protectedSoundUntilTime = CurrentTime + duration + 100;
          }
        }
      } else {
        // Tilt before ball is plunged -- show a timer in ManageGameMode if desired
        if ( CurrentTime > (LastTiltWarningTime + TILT_WARNING_DEBOUNCE_TIME) ) {
          PlaySoundSequence(SEQ_TILT_WARNING, 0);
        }
        LastTiltWarningTime = CurrentTime;
      }
      break;
  }

  return returnState;
}






void ValidateAndRegisterPlayfieldSwitch() {
  LastSwitchHitTime = CurrentTime;
  if (BallFirstSwitchHitTime == 0) {
    BallFirstSwitchHitTime = CurrentTime;
    if (BallSaveNumSeconds > 0) SetBallSave((unsigned long)BallSaveNumSeconds * 1000);
  }
}


void Handle3BankCompletion() {
    boolean isSweep = (threeBankSweepStartTime[CurrentPlayer] != 0 &&
                       (CurrentTime - threeBankSweepStartTime[CurrentPlayer]) < TIME_3BANK_SWEEP_WINDOW_MS);
    threeBankSweepStartTime[CurrentPlayer] = 0;

    ThreeBank.ResetDropTargets(CurrentTime + 500);
    threeBankCompleteCount[CurrentPlayer]++;
    IncreaseBonusX();

    if (isSweep) {
        if (DEBUG_MESSAGES) Serial.write("3-BANK SWEEP DETECTED!\n");
        QueuePendingScoreUpdate(CurrentPlayer, SCORE_3BANK_SWEEP_BONUS * PlayfieldMultiplier, SEQ_SCORE_10000, 1075);
        // Last drop target plays SEQ_SCORE_500 (5 tones x 200ms = ~1000ms)
        // Queue 10K tone after drop sounds finish + buffer
        PlaySoundSequence(SEQ_SCORE_10000, 1075);
        threeBankSweepAnimationStart[CurrentPlayer] = CurrentTime;
    } else {
        if (DEBUG_MESSAGES) Serial.write("Regular 3-bank completion - playing scoring sounds!\n");
        QueuePendingScoreUpdate(CurrentPlayer, SCORE_3BANK_COMPLETION * PlayfieldMultiplier, SEQ_SCORE_6000, 975);
        // SCORE_500 duration: 800ms (max gap) + 75ms (Dick's silence) = 875ms
        // Queue SCORE_6000 after SCORE_500 completes with buffer
        PlaySoundSequence(SEQ_SCORE_6000, 975);
    }
}

void Handle5BankCompletion() {
    QueuePendingScoreUpdate(CurrentPlayer, SCORE_5BANK_COMPLETION * PlayfieldMultiplier, SEQ_SCORE_10000, 975);
    FiveBank.ResetDropTargets(CurrentTime + 500);

    fiveBankCompleteCount[CurrentPlayer]++;

    // SCORE_500 duration: 800ms (max gap) + 75ms (Dick's silence) = 875ms
    // Queue completion sound with buffer after SCORE_500 finishes
    unsigned int score500Offset = 875 + 100;  // duration + 100ms buffer

    if (fiveBankCompleteCount[CurrentPlayer] == 1) {
        // 1st: score and reset
        PlaySoundSequence(SEQ_SCORE_10000, score500Offset);
    } else if (fiveBankCompleteCount[CurrentPlayer] == 2) {
        // 2nd: extra ball lane available (if enabled) + special lamp lights
        if (ExtraBallLaneEnabled) {
            ExtraBallLaneAvailable[CurrentPlayer] = true;
        }
        PlaySoundSequence(SEQ_SCORE_10000, score500Offset);
    } else if (fiveBankCompleteCount[CurrentPlayer] >= 3) {
        // 3rd: award special via AwardSpecial() — respects SpecialAwardType, SpecialOpenEnded, SpecialCollected
        boolean specialAwarded = false;
        if (!SpecialCollected || SpecialOpenEnded) {
            AwardSpecial();
            specialAwarded = true;
        }
        fiveBankCompleteCount[CurrentPlayer] = 0;
        if (specialAwarded) {
            PlaySoundSequence(SEQ_FANFARE_5BANK, score500Offset);
        } else {
            PlaySoundSequence(SEQ_BONUS_COUNT, score500Offset);
        }
    }
}

void HandleGamePlaySwitches(byte switchHit) {
    if (DEBUG_MESSAGES) {
      char buf[64];
      sprintf(buf, "SW: %d @ %lu ms\n", switchHit, CurrentTime);
      Serial.write(buf);
    }

    switch (switchHit) {
        case SW_KICKER:
            if (!KickerBonusCollect) {
                isArcSurgeActive[CurrentPlayer] = false;
                arcSurgeT1Hit[CurrentPlayer] = false;
                KickerBonusCollect = true;
                CollectBonusViaKicker = true;
            }
            ValidateAndRegisterPlayfieldSwitch();
            break;

        case SW_TARGET_1_3BANK:
        case SW_TARGET_2_3BANK:
        case SW_TARGET_3_3BANK:
        {
            // Suppress individual target sounds only if already in sweep window
            boolean inSweepWindow = (threeBankSweepStartTime[CurrentPlayer] != 0 && (CurrentTime - threeBankSweepStartTime[CurrentPlayer]) < TIME_3BANK_SWEEP_WINDOW_MS);

            // Track sweep window start
            if (ThreeBank.GetStatus(false) == 0x00 && threeBankSweepStartTime[CurrentPlayer] == 0)
                threeBankSweepStartTime[CurrentPlayer] = CurrentTime;

            ThreeBank.HandleDropTargetHit(switchHit);
            QueuePendingScoreUpdate(CurrentPlayer, SCORE_DROP_TARGET_BASE * PlayfieldMultiplier, SEQ_SCORE_500);

            // Play sound only if NOT in sweep window
            if (!inSweepWindow) {
                unsigned int duration = PlaySoundSequence(SEQ_SCORE_500, 0);
                protectedSoundUntilTime = CurrentTime + duration + 100;  // Grace period after protected sound
            }
            ValidateAndRegisterPlayfieldSwitch();
            break;
        }

        case SW_TARGET_1_5BANK:
        case SW_TARGET_2_5BANK:
        case SW_TARGET_3_5BANK:
        case SW_TARGET_4_5BANK:
        case SW_TARGET_5_5BANK:
            FiveBank.HandleDropTargetHit(switchHit);
            QueuePendingScoreUpdate(CurrentPlayer, SCORE_DROP_TARGET_BASE * PlayfieldMultiplier, SEQ_SCORE_500);
            {
                unsigned int duration = PlaySoundSequence(SEQ_SCORE_500, 0);
                protectedSoundUntilTime = CurrentTime + duration + 100;  // Grace period after protected sound
            }
            ValidateAndRegisterPlayfieldSwitch();
            break;

        case SW_SPINNER: {
            static unsigned long lastSpinnerSoundTime = 0;
            spinnerHitCount[CurrentPlayer]++;
            LastSpinnerHitTime = CurrentTime;
            int spinnerScore = (Bonus[CurrentPlayer] >= 10) ? SCORE_SPINNER_LIT : SCORE_SPINNER_BASE;
            CurrentScores[CurrentPlayer] += spinnerScore * PlayfieldMultiplier;
            if (CurrentTime > (lastSpinnerSoundTime + 150)) {
                // Don't queue spinner if a protected sequence is playing or grace period is active
                if (CurrentTime >= protectedSoundUntilTime) {
                    byte seqID = (spinnerScore == SCORE_SPINNER_LIT) ? SEQ_SCORE_1000 : SEQ_SCORE_100;
                    PlaySoundSequence(seqID, 0);
                    lastSpinnerSoundTime = CurrentTime;
                }
            }
            if (spinnerHitCount[CurrentPlayer] % 4 == 0) { AddToBonus(1); }
            ValidateAndRegisterPlayfieldSwitch();
            break;
        }

        case SW_CENTER_THUMPER:
        case SW_RIGHT_THUMPER:
        case SW_LEFT_THUMPER: {
            // 100 pts on 5-ball, 1,000 pts on 3-ball (per original rules)
            int popScore = (BallsPerGame == 3 ? 1000 : 100) * PlayfieldMultiplier;
            CurrentScores[CurrentPlayer] += popScore;
            // Only play sound during active ball play, not during match/bonus sequences
            if (MachineState == MACHINE_STATE_NORMAL_GAMEPLAY) {
                // Pop bumper first, then score sound with offset to let pop finish
                PlaySoundSequence(SEQ_POP_BUMPER, 0);
                byte scoreSeqID = (popScore == 1000) ? SEQ_SCORE_1000 : SEQ_SCORE_100;
                PlaySoundSequence(scoreSeqID, 250);
            }
            ValidateAndRegisterPlayfieldSwitch();
            break;
        }

        case SW_RIGHT_INLANE:
            QueuePendingScoreUpdate(CurrentPlayer, 3000L * PlayfieldMultiplier, SEQ_SCORE_3000);
            PlaySoundSequence(SEQ_SCORE_3000, 0);
            if (DEBUG_MESSAGES) {
                char buf[64];
                sprintf(buf, "RIGHT_INLANE: 5bank=%d EBEnabled=%d EBAvail=%d\n",
                    fiveBankCompleteCount[CurrentPlayer], ExtraBallLaneEnabled, ExtraBallsAvailable[CurrentPlayer]);
                Serial.write(buf);
            }
            if (ExtraBallLaneAvailable[CurrentPlayer] && !ExtraBallCollectedThisBall[CurrentPlayer] && !CollectBonusViaKicker) {
                PlaySoundSequence(SEQ_FANFARE_ASCENDING, 600);
                AwardExtraBall();
                ExtraBallCollectedThisBall[CurrentPlayer] = true;
                ExtraBallLaneAvailable[CurrentPlayer] = false;
            }
            isArcSurgeActive[CurrentPlayer] = true;
            arcSurgeTimerStart[CurrentPlayer] = CurrentTime;
            ValidateAndRegisterPlayfieldSwitch();
            break;

        case SW_ADV_BONUS_1000: // Side lane wire form — scores T1 during Arc Surge (combo continues to saucer)
            if (isArcSurgeActive[CurrentPlayer]) {
                CurrentScores[CurrentPlayer] += SCORE_ARC_SURGE_T1 * PlayfieldMultiplier;
                arcSurgeT1Hit[CurrentPlayer] = true;
                PlaySoundSequence(SEQ_SCORE_10000, 0);
                // Arc Surge stays active - saucer completes the combo
            } else if (isSaucerLit[CurrentPlayer]) {
                // Lit T1: always play SCORE_5000 + ADVANCE_3 (5000 pts + 3 bonus advances)
                if (DEBUG_MESSAGES) Serial.print("T1 lit: Bonus="); Serial.print(Bonus[CurrentPlayer]); Serial.println(" → 5000 + ADVANCE_3");
                QueuePendingScoreUpdate(CurrentPlayer, 5000L * PlayfieldMultiplier, SEQ_SCORE_5000);
                unsigned int scoreDuration = PlaySoundSequence(SEQ_SCORE_5000, 0);
                PlaySoundSequence(SEQ_ADVANCE_3, scoreDuration + 50);
                AddToBonus(3);
            } else {
                CurrentScores[CurrentPlayer] += 1000L * PlayfieldMultiplier;
                AddToBonus(1);
                PlaySoundSequence(SEQ_SCORE_1000, 0);
            }
            ValidateAndRegisterPlayfieldSwitch();
            break;

        case SW_SAUCER: // Saucer / Eject Pocket
             if (MachineState != MACHINE_STATE_NORMAL_GAMEPLAY) break;  // Only handle during normal gameplay

             if (isArcSurgeActive[CurrentPlayer] && arcSurgeT1Hit[CurrentPlayer]) { // Arc Surge combo complete (both T1 and saucer hit)
                QueuePendingScoreUpdate(CurrentPlayer, SCORE_ARC_SURGE_SUPER * PlayfieldMultiplier, SEQ_FANFARE_ASCENDING, 300);
                AddToBonus(3);
                if (DEBUG_MESSAGES) Serial.write("ARC SURGE COMPLETE - playing fanfare\n");
                PlaySoundSequence(SEQ_FANFARE_ASCENDING, 300);
                isArcSurgeActive[CurrentPlayer] = false;
                arcSurgeT1Hit[CurrentPlayer] = false;
                arcSurgeCompleteTime = CurrentTime;
             } else if (isArcSurgeActive[CurrentPlayer] && !arcSurgeT1Hit[CurrentPlayer]) { // Arc Surge active but T1 not hit - deactivate and treat as normal saucer
                isArcSurgeActive[CurrentPlayer] = false;
                arcSurgeT1Hit[CurrentPlayer] = false;
                if (isSaucerLit[CurrentPlayer]) {
                    CurrentScores[CurrentPlayer] += SCORE_SKILL_SHOT * PlayfieldMultiplier;
                    if (Bonus[CurrentPlayer] < 19) {
                      if (DEBUG_MESSAGES) Serial.print("Saucer (Arc no T1): Bonus="); Serial.print(Bonus[CurrentPlayer]); Serial.println(" < 19 → ADVANCE");
                      PlaySoundSequence(SEQ_ADVANCE_3, 0);
                      AddToBonus(3);
                    } else {
                      if (DEBUG_MESSAGES) Serial.print("Saucer (Arc no T1): Bonus="); Serial.print(Bonus[CurrentPlayer]); Serial.println(" >= 19 → SCORE");
                      PlaySoundSequence(SEQ_SCORE_5000, 0);
                    }
                    if (!SaucerLightPersists) isSaucerLit[CurrentPlayer] = false;
                } else {
                    QueuePendingScoreUpdate(CurrentPlayer, 500L * PlayfieldMultiplier, SEQ_SCORE_500);
                    AddToBonus(1);
                    PlaySoundSequence(SEQ_SCORE_500, 0);
                }
             } else if (!firstHitMade[CurrentPlayer] && !CollectBonusViaKicker) { // Skill shot (not during bonus collect)
                 QueuePendingScoreUpdate(CurrentPlayer, SCORE_SKILL_SHOT * PlayfieldMultiplier, SEQ_FANFARE_ASCENDING, 300);
                 AddToBonus(3);
                 PlaySoundSequence(SEQ_FANFARE_ASCENDING, 300);
                 SkillShotAnimationStart = CurrentTime;
            } else if (isSaucerLit[CurrentPlayer]) {
                 // Lit saucer: always play SCORE_5000 + ADVANCE_3 (5000 pts + 3 bonus advances)
                 if (DEBUG_MESSAGES) Serial.print("Saucer (lit): Bonus="); Serial.print(Bonus[CurrentPlayer]); Serial.println(" → 5000 + ADVANCE_3");
                 QueuePendingScoreUpdate(CurrentPlayer, SCORE_SKILL_SHOT * PlayfieldMultiplier, SEQ_SCORE_5000);
                 unsigned int scoreDuration = PlaySoundSequence(SEQ_SCORE_5000, 0);
                 PlaySoundSequence(SEQ_ADVANCE_3, scoreDuration + 50);
                 AddToBonus(3);
                 if (!SaucerLightPersists) isSaucerLit[CurrentPlayer] = false;
            } else {
                 QueuePendingScoreUpdate(CurrentPlayer, 500L * PlayfieldMultiplier, SEQ_SCORE_500);
                 AddToBonus(1);
                 PlaySoundSequence(SEQ_SCORE_500, 0);
            }
            RPU_PushToTimedSolenoidStack(SOL_SAUCER, SaucerSolenoidStrength, CurrentTime + 500, false);
            ValidateAndRegisterPlayfieldSwitch();
            break;

        case SW_STANDUP_TARGET:
            isSaucerLit[CurrentPlayer] = true;
            QueuePendingScoreUpdate(CurrentPlayer, SCORE_STANDUP_TARGET * PlayfieldMultiplier, SEQ_SCORE_5000);
            AddToBonus(1);
            PlaySoundSequence(SEQ_SCORE_5000, 0);
            ValidateAndRegisterPlayfieldSwitch();
            break;

        case SW_RIGHT_SLINGSHOT:
        case SW_LEFT_SLINGSHOT:
            CurrentScores[CurrentPlayer] += 100L * PlayfieldMultiplier;
            PlaySoundSequence(SEQ_SCORE_100, 0);
            ValidateAndRegisterPlayfieldSwitch();
            break;

        case SW_ADV_BONUS_300:
            QueuePendingScoreUpdate(CurrentPlayer, 300L * PlayfieldMultiplier, SEQ_SCORE_300);
            AddToBonus(1);
            PlaySoundSequence(SEQ_SCORE_300, 0);
            ValidateAndRegisterPlayfieldSwitch();
            break;

        case SW_SCORE_10:
            CurrentScores[CurrentPlayer] += 10L * PlayfieldMultiplier;
            PlaySoundSequence(SEQ_SCORE_100, 0);
            ValidateAndRegisterPlayfieldSwitch();
            break;

        case SW_RIGHT_OUTLANE:
        case SW_LEFT_OUTLANE: {
            QueuePendingScoreUpdate(CurrentPlayer, SCORE_OUTLANE * PlayfieldMultiplier, SEQ_SCORE_3000);
            AddToBonus(3);
            ValidateAndRegisterPlayfieldSwitch();
            // Only play sounds during normal gameplay
            if (MachineState == MACHINE_STATE_NORMAL_GAMEPLAY) {
              // Always: score 3000, then advance +3, then drain (unless ball saved)
              if (DEBUG_MESSAGES) Serial.print("Outlane: Bonus="); Serial.print(Bonus[CurrentPlayer]); Serial.println(" → SCORE + ADVANCE + DRAIN");
              unsigned int scoreDuration = PlaySoundSequence(SEQ_SCORE_3000, 0);
              unsigned int advanceDuration = PlaySoundSequence(SEQ_ADVANCE_3, scoreDuration + 50);
              // Don't play drain sound if ball save is active (check from first switch hit, now set by ValidateAndRegisterPlayfieldSwitch)
              boolean isBallSaveActive = (BallFirstSwitchHitTime != 0 &&
                                          CurrentTime < (BallFirstSwitchHitTime + ((unsigned long)BallSaveNumSeconds * 1000) + BALL_SAVE_GRACE_PERIOD));
              if (!isBallSaveActive) {
                unsigned int drainDuration = PlaySoundSequence(SEQ_DRAIN, scoreDuration + advanceDuration + 300);
              }
            }
            break;
        }

        case SW_ROLLOVER_BUTTON:
            CurrentScores[CurrentPlayer] += SCORE_SPINNER_BASE * PlayfieldMultiplier;
            isLeftReturnLaneLit[CurrentPlayer] = true;
            PlaySoundSequence(SEQ_SCORE_100, 0);
            ValidateAndRegisterPlayfieldSwitch();
            break;

        case SW_LEFT_INLANE:
            if (isLeftReturnLaneLit[CurrentPlayer]) {
                QueuePendingScoreUpdate(CurrentPlayer, 9000L * PlayfieldMultiplier, SEQ_SCORE_9000);
                isLeftReturnLaneLit[CurrentPlayer] = false;
                PlaySoundSequence(SEQ_SCORE_9000, 0);
            } else {
                QueuePendingScoreUpdate(CurrentPlayer, 3000L * PlayfieldMultiplier, SEQ_SCORE_3000);
                PlaySoundSequence(SEQ_SCORE_3000, 0);
            }
            ValidateAndRegisterPlayfieldSwitch();
            break;
    }

    if (!firstHitMade[CurrentPlayer]) {
        firstHitMade[CurrentPlayer] = true;
    }

    // Cancel Arc Surge if any switch other than the combo targets is hit
    if (isArcSurgeActive[CurrentPlayer] &&
        switchHit != SW_RIGHT_INLANE &&
        switchHit != SW_SAUCER &&
        switchHit != SW_ADV_BONUS_1000) {
        isArcSurgeActive[CurrentPlayer] = false;
        arcSurgeT1Hit[CurrentPlayer] = false;
    }
}


int RunGamePlayMode(int curState, boolean curStateChanged) {
  int returnState = curState;
  unsigned long scoreAtTop = CurrentScores[CurrentPlayer];

  // Very first time into gameplay loop
  if (curState == MACHINE_STATE_INIT_GAMEPLAY) {
    returnState = InitGamePlay(curStateChanged);
  } else if (curState == MACHINE_STATE_INIT_NEW_BALL) {
    returnState = InitNewBall(curStateChanged);
  } else if (curState == MACHINE_STATE_NORMAL_GAMEPLAY) {
    returnState = ManageGameMode();
  } else if (curState == MACHINE_STATE_COUNTDOWN_BONUS) {
    Display_ClearOverride(0xFF);
    Display_UpdateDisplays(0xFF, true);
    returnState = CountdownBonus(curStateChanged);
//    ShowPlayerScoresOnTwoDisplays(0xFF, false, false);
  } else if (curState == MACHINE_STATE_BALL_OVER) {
    RPU_SetDisplayCredits(Credits, !FreePlayMode);

    if (SamePlayerShootsAgain) {
      QueueNotification(SOUND_EFFECT_VP_SHOOT_AGAIN, 10);
      returnState = MACHINE_STATE_INIT_NEW_BALL;
    } else {

      CurrentPlayer += 1;
      if (CurrentPlayer >= CurrentNumPlayers) {
        CurrentPlayer = 0;
        CurrentBallInPlay += 1;
      }

      scoreAtTop = CurrentScores[CurrentPlayer];

      if (CurrentBallInPlay > BallsPerGame) {
        CheckHighScores();
        for (int count = 0; count < CurrentNumPlayers; count++) {
          RPU_SetDisplay(count, CurrentScores[count], true, 2);
        }

        for (byte count = 0; count < 4; count++) {
          RPU_SetLampState(PlayerUpLamps[count], 0);
        }
        RPU_SetLampState(LAMP_GAME_OVER, 1);
        RPU_SetLampState(LAMP_BALL_IN_PLAY, 0);

        if (MatchEnabled) {
          returnState = MACHINE_STATE_MATCH_MODE;
        } else {
          returnState = MACHINE_STATE_ATTRACT;
        }
      }
      else returnState = MACHINE_STATE_INIT_NEW_BALL;
    }
  } else if (curState == MACHINE_STATE_MATCH_MODE) {
    returnState = ShowMatchSequence(curStateChanged);
  }

  byte switchHit;
  unsigned long lastBallFirstSwitchHitTime = BallFirstSwitchHitTime;

  while ( (switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
    returnState = HandleSystemSwitches(curState, switchHit);
    if (NumTiltWarnings <= MaxTiltWarnings &&
        switchHit != SW_CREDIT_BUTTON &&
        switchHit != SW_SLAM_TILT &&
        switchHit != SW_TILT &&
        (curState == MACHINE_STATE_NORMAL_GAMEPLAY || curState == MACHINE_STATE_COUNTDOWN_BONUS)) {
      HandleGamePlaySwitches(switchHit);
    }
  }

  if (CollectBonusViaKicker && curState == MACHINE_STATE_NORMAL_GAMEPLAY) {
    CollectBonusViaKicker = false;
    returnState = MACHINE_STATE_COUNTDOWN_BONUS;
  } else {
    CollectBonusViaKicker = false;
  }

  ThreeBank.Update(CurrentTime);
  FiveBank.Update(CurrentTime);
  static unsigned long lastThreeBankClear = 0;
  static unsigned long lastFiveBankClear = 0;
  if (ThreeBank.CheckIfBankCleared() && CurrentTime > (lastThreeBankClear + 750)) {
    lastThreeBankClear = CurrentTime;
    Handle3BankCompletion();
  }
  if (FiveBank.CheckIfBankCleared() && CurrentTime > (lastFiveBankClear + 750)) {
    lastFiveBankClear = CurrentTime;
    Handle5BankCompletion();
  }

  if (CreditResetPressStarted) {
    if (CurrentBallInPlay < 2) {
      // If we haven't finished the first ball, we can add players
      AddPlayer();
      if (DEBUG_MESSAGES) {
        Serial.write("Start game button pressed\n\r");
      }
      CreditResetPressStarted = 0;
    } else {
      if (RPU_ReadSingleSwitchState(SW_CREDIT_RESET)) {
        if (TimeRequiredToResetGame != 99 && (CurrentTime - CreditResetPressStarted) >= ((unsigned long)TimeRequiredToResetGame * 1000)) {
          // If the first ball is over, pressing start again resets the game
          if (Credits >= 1 || FreePlayMode) {
            if (!FreePlayMode) {
              Credits -= 1;
              RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
              RPU_SetDisplayCredits(Credits, !FreePlayMode);
            }
            returnState = MACHINE_STATE_INIT_GAMEPLAY;
            CreditResetPressStarted = 0;
          }
        }
      } else {
        CreditResetPressStarted = 0;
      }
    }

  }

  if (lastBallFirstSwitchHitTime == 0 && BallFirstSwitchHitTime != 0) {
    BallSaveEndTime = BallFirstSwitchHitTime + ((unsigned long)BallSaveNumSeconds) * 1000;
    NumberOfBallSavesRemaining = 1;
  }
  if (CurrentTime > (BallSaveEndTime + BALL_SAVE_GRACE_PERIOD)) {
    BallSaveEndTime = 0;
    NumberOfBallSavesRemaining = 0;
  }

  if (!ScrollingScores && CurrentScores[CurrentPlayer] > RPU_OS_MAX_DISPLAY_SCORE) {
    CurrentScores[CurrentPlayer] -= RPU_OS_MAX_DISPLAY_SCORE;
    if (!TournamentScoring) AddSpecialCredit();
  }

  if (scoreAtTop != CurrentScores[CurrentPlayer]) {
    Display_SetLastTimeScoreChanged(CurrentTime);
    if (!TournamentScoring) {
      for (int awardCount = 0; awardCount < 3; awardCount++) {
        if (AwardScores[awardCount] != 0 && scoreAtTop < AwardScores[awardCount] && CurrentScores[CurrentPlayer] >= AwardScores[awardCount]) {
          // Player has just passed an award score, so we need to award it
          if (((ScoreAwardReplay >> awardCount) & 0x01)) {
            AddSpecialCredit();
          } else {
            AwardExtraBall(true);
          }
        }
      }
    }

  }

  return returnState;
}


#if (RPU_MPU_ARCHITECTURE>=10)
unsigned long LastLEDUpdateTime = 0;
byte LEDPhase = 0;
#endif

#ifdef DEBUG_SHOW_LOOPS_PER_SECOND
unsigned long NumLoops = 0;
unsigned long LastLoopReportTime = 0;

#endif

void loop() {

  CurrentTime = millis();
  int newMachineState = MachineState;

  // Debug: send 'G' for game over, 'S' for startup, 'D' for drain
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'G') PlaySoundSequence(23);
    else if (cmd == 'S') PlaySoundSequence(26);
    else if (cmd == 'D') PlaySoundSequence(27);
  }

#ifdef DEBUG_SHOW_LOOPS_PER_SECOND
  NumLoops += 1;
  if (LastLoopReportTime==0) LastLoopReportTime = CurrentTime;
  if (CurrentTime>(LastLoopReportTime+1000)) {
    LastLoopReportTime = CurrentTime;
    NumLoops = 0;
  }
#endif

  static boolean lastOperatorMenuActive = false;
  boolean operatorMenuActive = Menus.OperatorMenusActive();
  if (operatorMenuActive) {
    RunOperatorMenu();
  } else {
    if (lastOperatorMenuActive) {
      Audio.PlaySound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
    }
    if (MachineState < 0) {
      newMachineState = 0;
    } else if (MachineState == MACHINE_STATE_ATTRACT) {
      newMachineState = RunAttractMode(MachineState, MachineStateChanged);
    } else if (MachineState == MACHINE_STATE_DIAGNOSTICS) {
      newMachineState = RunDiagnosticsMode(MachineState, MachineStateChanged);
    } else {
      newMachineState = RunGamePlayMode(MachineState, MachineStateChanged);
    }
  
    if (newMachineState != MachineState) {
      MachineState = newMachineState;
      MachineStateChanged = true;
    } else {
      MachineStateChanged = false;
    }
  }
  lastOperatorMenuActive = operatorMenuActive;

  RPU_Update(CurrentTime);
  Audio.Update(CurrentTime);

#if (RPU_MPU_ARCHITECTURE>=10)
  if (LastLEDUpdateTime == 0 || (CurrentTime - LastLEDUpdateTime) > 250) {
    LastLEDUpdateTime = CurrentTime;
    RPU_SetBoardLEDs((LEDPhase % 8) == 1 || (LEDPhase % 8) == 3, (LEDPhase % 8) == 5 || (LEDPhase % 8) == 7);
    LEDPhase += 1;
  }
#endif

}
