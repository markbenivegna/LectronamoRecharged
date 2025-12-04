 /**************************************************************************
 *     This file is part of the RPU OS for Arduino Project.

    I, Dick Hamill, the author of this program disclaim all copyright
    in order to make this program freely available in perpetuity to
    anyone who would like to use it. Dick Hamill, 6/1/2020

    RPU OS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    RPU OS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <https://www.gnu.org/licenses/>.
 */

#include <Arduino.h>
#include "RPU_Config.h"
#include "RPU.h"
#include "Lectronamo.h" // Include for friendly names
#include "SelfTestAndAudit.h"

#define MACHINE_STATE_ATTRACT         0

unsigned long LastSolTestTime = 0; 
unsigned long LastSelfTestChange = 0;
unsigned long SavedValue = 0;
unsigned long ResetHold = 0;
unsigned long NextSpeedyValueChange = 0;
unsigned long NumSpeedyChanges = 0;
unsigned long LastResetPress = 0;
byte CurValue = 0;
byte CurSound = 0x01;
byte SoundPlaying = 0;
byte SoundToPlay = 0;
boolean SolenoidCycle = true;

// --- MPU Switch Adjustment Definitions (based on Lectronamo Manual) ---
#define NUM_MPU_SWITCHES 32

struct MpuSwitchSetting {
    byte id;
    byte eepromAddr;
    const char* name;
    const char* option0;
    const char* option1;
};

MpuSwitchSetting mpuSwitches[NUM_MPU_SWITCHES] = {
    // ID, EEPROM Addr, Name,                  Option 0 (Switch OFF), Option 1 (Switch ON)
    { 1,  0x14, "S1: Chute 1 Coins", "1 Coin", "2 Coins" },
    { 2,  0x15, "S2: Chute 1 Coins", "1 Coin", "3 Coins" },
    { 3,  0x16, "S3: Chute 1 Coins", "1 Coin", "4 Coins" },
    { 4,  0x17, "S4: Chute 1 Coins", "1 Coin", "5 Coins" },
    { 5,  0x18, "S5: Chute 1 Coins", "1 Coin", "6 Coins" },
    { 6,  0x19, "S6: HS Award", "Extra Ball", "Replay" },
    { 7,  ADDR_BALLS_PER_GAME, "S7: Balls Per Game", "3 Balls", "5 Balls" }, // Corrected
    { 8,  0x1B, "S8: Chute 1 Bonus", "No Bonus", "Bonus Credit" },
    { 9,  0x1C, "S9: Chute 2 Coins", "1 Coin", "2 Coins" },
    { 10, 0x1D, "S10: Chute 2 Memory", "No Memory", "Memory" },
    { 11, 0x1E, "S11: Chute 2 Bonus", "No Bonus", "Bonus Credit" },
    { 12, 0x1F, "S12: Not Used", "Off", "On" }, // Restored
    { 13, 0x20, "S13: Not Used", "Off", "On" }, // Restored
    { 14, 0x21, "S14: Not Used", "Off", "On" }, // Restored
    { 15, 0x22, "S15: Not Used", "Off", "On" }, // Restored
    { 16, 0x23, "S16: Not Used", "Off", "On" }, // Restored
    { 17, ADDR_MAX_CREDITS_17, "S17-19: Max Credits", "See S18/19", "See S18/19" }, // Handled as a group
    { 18, ADDR_MAX_CREDITS_18, "S17-19: Max Credits", "See S18/19", "See S18/19" }, // Handled as a group
    { 19, ADDR_MAX_CREDITS_19, "S17-19: Max Credits", "See S18/19", "See S18/19" }, // Handled as a group
    { 20, 0x24, "S20: Game Mode", "Normal", "Novelty" },
    { 21, 0x25, "S21: Match Feature", "Off", "On" }, // Corrected
    { 22, 0x26, "S22: Exceed Score", "No Award", "Replay" },
    { 23, ADDR_SPECIAL_LIMIT, "S23: Special Award", "Extra Ball", "Credit" },
    { 24, 0x27, "S24: Bonus Countdown", "Multiple Steps", "1,000 Steps" }, // Corrected
    { 25, ADDR_FREE_PLAY_ADJUSTMENT, "S25: Free Play", "Off", "On" }, // Free Play Setting
    { 26, 0x29, "S26: Replay Scores", "Conservative", "Liberal" },
    { 27, 0x2A, "S27: Not Used", "Off", "On" }, // Restored
    { 28, 0x2B, "S28: Chute 1 Plays", "1 Play", "2 Plays" }, // Corrected
    { 29, 0x2C, "S29: Chute 1 Plays", "1 Play", "3 Plays" }, // Corrected
    { 30, 0x2D, "S30: Chute 1 Plays", "1 Play", "4 Plays" }, // Corrected
    { 31, 0x2E, "S31-32: Special", "See S32", "See S32" }, // Handled as a group
    { 32, 0x2F, "S31-32: Special", "See S32", "See S32" }  // Handled as a group
};

// Structure to hold friendly names for solenoids and switches
struct NamedHardware {
    byte id;
    const char* name;
};

// Helper function to find a friendly name from an ID
const char* findName(byte id, const NamedHardware* hardware, int size) {
    for (int i = 0; i < size; ++i) {
        if (hardware[i].id == id) return hardware[i].name;
    }
    return "???";
}

#ifndef RPU_OS_DISABLE_CPC_FOR_SPACE
boolean CPCSelectionsHaveBeenRead = false;
#define NUM_CPC_PAIRS 9
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
#endif

#ifdef RPU_OS_USE_7_DIGIT_DISPLAYS
#ifdef RPU_OS_USE_6_DIGIT_CREDIT_DISPLAY_WITH_7_DIGIT_DISPLAYS
#define TOTAL_DISPLAY_DIGITS 34
#else
#define TOTAL_DISPLAY_DIGITS 35
#endif
#else
#define TOTAL_DISPLAY_DIGITS 30
#endif

int RunBaseSelfTest(int curState, boolean curStateChanged, unsigned long CurrentTime, byte resetSwitch, byte slamSwitch) {
  byte curSwitch = RPU_PullFirstFromSwitchStack();
  int returnState = curState;
  boolean resetDoubleClick = false;
  unsigned short savedScoreStartByte = 0;
  unsigned short auditNumStartByte = 0;
#ifndef RPU_OS_DISABLE_CPC_FOR_SPACE  
  unsigned short cpcSelectorStartByte = 0;
#endif

  if (curSwitch==resetSwitch) {
    ResetHold = CurrentTime;
    if ((CurrentTime-LastResetPress)<400) {
      resetDoubleClick = true;
      curSwitch = SWITCH_STACK_EMPTY;
    }
    LastResetPress = CurrentTime;
    SoundToPlay += 1;
    if (SoundToPlay>31) SoundToPlay = 0;
  }

  if (ResetHold!=0 && !RPU_ReadSingleSwitchState(resetSwitch)) {
    ResetHold = 0;
    NextSpeedyValueChange = 0;
  }

  boolean resetBeingHeld = false;
  if (ResetHold!=0 && (CurrentTime-ResetHold)>1300) {
    resetBeingHeld = true;
    if (NextSpeedyValueChange==0) {
      NextSpeedyValueChange = CurrentTime;
      NumSpeedyChanges = 0;
    }
  }

  if (slamSwitch!=0xFF && curSwitch==slamSwitch) {
    returnState = MACHINE_STATE_ATTRACT;
  }
  
  if (curSwitch==SW_SELF_TEST_SWITCH && (CurrentTime-LastSelfTestChange)>250) {
    if (RPU_GetUpDownSwitchState()) returnState -= 1;
    else returnState += 1;
//    if (returnState==MACHINE_STATE_TEST_DONE) returnState = MACHINE_STATE_ATTRACT;
    LastSelfTestChange = CurrentTime;
  }

  if (curStateChanged) {
    RPU_SetCoinLockout(false);
    
    for (int count=0; count<4; count++) {
      RPU_SetDisplay(count, 0);
      RPU_SetDisplayBlank(count, 0x00);        
    }

#if (RPU_MPU_ARCHITECTURE<10)
    if (curState<=MACHINE_STATE_TEST_SCORE_LEVEL_1) {
      RPU_SetDisplayCredits(0, false);
      RPU_SetDisplayBallInPlay(MACHINE_STATE_TEST_SOUNDS-curState);
    } else {
      RPU_SetDisplayCredits(0 - curState, true);
      RPU_SetDisplayBallInPlay(0, false);      
    }
#else
    if (curState<=MACHINE_STATE_TEST_HISCR) {
      RPU_SetDisplayCredits(0, false);
      RPU_SetDisplayBallInPlay(MACHINE_STATE_TEST_BOOT-curState, true);
    } else {
      RPU_SetDisplayCredits(curState - MACHINE_STATE_TEST_BOOT, true);
      RPU_SetDisplayBallInPlay(0, false);      
    }
#endif      
  }

  if (curState==MACHINE_STATE_TEST_LAMPS) {
    if (curStateChanged) {
      RPU_DisableSolenoidStack();        
      RPU_SetDisableFlippers(true);
      RPU_TurnOffAllLamps();
      for (int count=0; count<RPU_MAX_LAMPS; count++) {
        RPU_SetLampState(count, 1, 0, 500);
      }
      CurValue = 99;
      RPU_SetDisplay(0, CurValue, true);  
    }
    if (curSwitch==resetSwitch || resetDoubleClick) {
      if (RPU_GetUpDownSwitchState()) {
        CurValue += 1;
        if (CurValue==RPU_MAX_LAMPS) CurValue = 99;
        else if (CurValue>99) CurValue = 0;    
      } else {
        if (CurValue>0) CurValue -= 1;
        else CurValue = 99;
        if (CurValue==98) CurValue = RPU_MAX_LAMPS - 1;
      }
      if (CurValue==99) {
        for (int count=0; count<RPU_MAX_LAMPS; count++) {
          RPU_SetLampState(count, 1, 0, 500);
        }
      } else {
        RPU_TurnOffAllLamps();
        RPU_SetLampState(CurValue, 1);
      }      
      RPU_SetDisplay(0, CurValue, true);  
    }    
  } else if (curState==MACHINE_STATE_TEST_DISPLAYS) {
    if (curStateChanged) {
      RPU_TurnOffAllLamps();
      for (int count=0; count<4; count++) {
        RPU_SetDisplayBlank(count, RPU_OS_ALL_DIGITS_MASK);        
      }
      CurValue = 0;
    }
    if (curSwitch==resetSwitch || resetDoubleClick) {
      if (RPU_GetUpDownSwitchState()) {
        CurValue += 1;
        if (CurValue>TOTAL_DISPLAY_DIGITS) {
          for (int count=0; count<4; count++) {
            RPU_SetDisplayBlank(count, RPU_OS_ALL_DIGITS_MASK);        
          }
          CurValue = 0;
        }
      } else {
        if (CurValue>0) CurValue -= 1;
        else CurValue = TOTAL_DISPLAY_DIGITS;
      }
    }    
    RPU_CycleAllDisplays(CurrentTime, CurValue);
  } else if (curState==MACHINE_STATE_TEST_SOLENOIDS) {
    if (curStateChanged) {
      RPU_TurnOffAllLamps();
      LastSolTestTime = CurrentTime;
      RPU_EnableSolenoidStack(); 
      RPU_SetDisableFlippers(false);
      //RPU_SetDisplayBlank(4, 0);
      // --- Lectronamo Customization: Use friendly names ---
      RPU_SetDisplayText(1, "SOLENOID", true);
      SolenoidCycle = true;
      SavedValue = 0;
      RPU_PushToSolenoidStack(SavedValue, 10);
    } 
    if (curSwitch==resetSwitch || resetDoubleClick) {
      // --- Lectronamo Customization: Cycle through defined solenoids ---
      if (RPU_GetUpDownSwitchState()) {
        SavedValue++;
        if (SavedValue > 19) SavedValue = 7; // Cycle from SOL_LEFT_THUMPER to SOL_COIN_LOCKOUT
      } else {
        if (SavedValue > 7) SavedValue--;
        else SavedValue = 19;
      }
      // Skip unused solenoid numbers
      if (SavedValue > 8 && SavedValue < 10) SavedValue = (RPU_GetUpDownSwitchState()) ? 10 : 8;

      SolenoidCycle = false; // Disable auto-cycle when manually stepping
      RPU_PushToSolenoidStack(SavedValue, 10);
      RPU_SetDisplay(0, SavedValue, true);

      // Display friendly name
      const NamedHardware solenoidNames[] = {
        {SOL_LEFT_THUMPER, "L THUMP"}, {SOL_RIGHT_THUMPER, "R THUMP"}, {SOL_CENTER_THUMPER, "C THUMP"},
        {SOL_OUTHOLE, "OUTHOLE"}, {SOL_RIGHT_SLINGSHOT, "R SLING"}, {SOL_KICKER, "KICKER"},
        {SOL_DROP_TARGET_3BANK_RESET, "3-BANK"}, {SOL_SAUCER, "SAUCER"}, {SOL_DROP_TARGET_5BANK_RESET, "5-BANK"},
        {SOL_LEFT_SLINGSHOT, "L SLING"}, {SOL_KNOCKER, "KNOCKER"}, {SOL_FLIPPERS, "FLIPPERS"}, {SOL_COIN_LOCKOUT, "COIN LCK"}
      };
      RPU_SetDisplayText(1, (char*)findName(SavedValue, solenoidNames, sizeof(solenoidNames)/sizeof(NamedHardware)), true);

    } else if (SolenoidCycle && (CurrentTime-LastSolTestTime)>1000) {
        SavedValue += 1;
#if (RPU_MPU_ARCHITECTURE<10)
        if (SavedValue>14) SavedValue = 0;
#else        
        if (SavedValue>21) SavedValue = 0;
#endif        
      RPU_PushToSolenoidStack(SavedValue, 10);
      RPU_SetDisplay(0, SavedValue, true);
      LastSolTestTime = CurrentTime;
    }
    /* Original logic replaced by Lectronamo specific logic above
      SolenoidCycle = (SolenoidCycle) ? false : true;
    }

    if ((CurrentTime-LastSolTestTime)>1000) {
      if (SolenoidCycle) {
        SavedValue += 1;
#if (RPU_MPU_ARCHITECTURE<10)
        if (SavedValue>14) SavedValue = 0;
#else        
        if (SavedValue>21) SavedValue = 0;
#endif        
      }
      RPU_PushToSolenoidStack(SavedValue, 10);
      RPU_SetDisplay(0, SavedValue, true);
      LastSolTestTime = CurrentTime;
    }
    */
    
  } else if (curState==MACHINE_STATE_TEST_SWITCHES) {
    if (curStateChanged) {
      RPU_TurnOffAllLamps();
      RPU_DisableSolenoidStack(); 
      RPU_SetDisableFlippers(true);
      RPU_SetDisplayText(1, "SWITCH", true); // Lectronamo Customization
    }

    byte displayOutput = 0;
    for (byte switchCount=0; switchCount<64 && displayOutput<4; switchCount++) {
      if (RPU_ReadSingleSwitchState(switchCount)) {
        RPU_SetDisplay(displayOutput, switchCount, true);
        displayOutput += 1;

        // --- Lectronamo Customization: Display first closed switch name ---
        if (displayOutput == 1) {
            const NamedHardware switchNames[] = {
                {SW_TILT, "TILT"}, {SW_SLAM_TILT, "SLAM"}, {SW_OUTHOLE, "OUTHOLE"},
                {SW_RIGHT_SLINGSHOT, "R SLING"}, {SW_LEFT_SLINGSHOT, "L SLING"},
                {SW_THUMPER_CENTER, "C THUMP"}, {SW_THUMPER_RIGHT, "R THUMP"}, {SW_THUMPER_LEFT, "L THUMP"},
                {SW_SAUCER, "SAUCER"}, {SW_CREDIT_BUTTON, "START"}, {SW_COIN_1, "COIN 1"},
                {SW_COIN_2, "COIN 2"}, {SW_COIN_3, "COIN 3"}, {SW_SPIN_TARGET, "SPINNER"},
                {SW_TARGET_1_3BANK, "3BANK 1"}, {SW_TARGET_2_3BANK, "3BANK 2"}, {SW_TARGET_3_3BANK, "3BANK 3"},
                {SW_TARGET_1_5BANK, "5BANK 1"}, {SW_TARGET_2_5BANK, "5BANK 2"}, {SW_TARGET_3_5BANK, "5BANK 3"},
                {SW_TARGET_4_5BANK, "5BANK 4"}, {SW_TARGET_5_5BANK, "5BANK 5"}
            };
            RPU_SetDisplayText(1, (char*)findName(switchCount, switchNames, sizeof(switchNames)/sizeof(NamedHardware)), true);
        }
      }
      // If no switches are closed, clear the name display
      if (displayOutput == 0) {
        RPU_SetDisplayText(1, "SWITCH", true);
      }
    }

    if (displayOutput<4) {
      for (int count=displayOutput; count<4; count++) {
        RPU_SetDisplayBlank(count, 0x00);
      }
    }

  } else if (curState==MACHINE_STATE_TEST_SOUNDS) {
#ifdef RPU_OS_USE_SB100    
    byte soundToPlay = 0x01 << (((CurrentTime-LastSelfTestChange)/750)%8);
    if (SoundPlaying!=soundToPlay) {
      RPU_PlaySB100(soundToPlay);
      SoundPlaying = soundToPlay;
      RPU_SetDisplay(0, (unsigned long)soundToPlay, true);
      LastSolTestTime = CurrentTime; // Time the sound started to play
    }
    // If the sound play call was more than 300ms ago, turn it off
//    if ((CurrentTime-LastSolTestTime)>300) RPU_PlaySB100(128);
#elif defined (RPU_OS_USE_S_AND_T)
    byte soundToPlay = ((CurrentTime-LastSelfTestChange)/2000)%256;
    if (SoundPlaying!=soundToPlay) {
      RPU_PlaySoundSAndT(soundToPlay);
      SoundPlaying = soundToPlay;
      RPU_SetDisplay(0, (unsigned long)soundToPlay, true);
      LastSolTestTime = CurrentTime; // Time the sound started to play
    }
#elif defined (RPU_OS_USE_DASH51) 
    byte soundToPlay = ((CurrentTime-LastSelfTestChange)/2000)%32;
    if (SoundPlaying!=soundToPlay) {
      if (soundToPlay==17) soundToPlay = 0;
      RPU_PlaySoundDash51(soundToPlay);
      SoundPlaying = soundToPlay;
      RPU_SetDisplay(0, (unsigned long)soundToPlay, true);
      LastSolTestTime = CurrentTime; // Time the sound started to play
    }
#elif defined (RPU_OS_USE_WTYPE_1_SOUND)
    byte soundToPlay = (((CurrentTime-LastSelfTestChange)/2000)%31)+1;
    if (SoundPlaying!=soundToPlay) {
      RPU_PushToSoundStack(soundToPlay*256, 8);
      SoundPlaying = soundToPlay;
      RPU_SetDisplay(0, (unsigned long)soundToPlay, true);
      LastSolTestTime = CurrentTime; // Time the sound started to play
    }
#elif defined (RPU_OS_USE_WTYPE_2_SOUND) 
//    byte soundToPlay = (((CurrentTime-LastSelfTestChange)/1000)%32);
    if (SoundPlaying!=SoundToPlay) {
      RPU_PushToSoundStack(SoundToPlay, 8);
      SoundPlaying = SoundToPlay  ;
      RPU_SetDisplay(0, (unsigned long)SoundToPlay, true);
      LastSolTestTime = CurrentTime; // Time the sound started to play
    }
#endif
  } else if (curState==MACHINE_STATE_TEST_BOOT) {
    if (curStateChanged) {
      for (int count=0; count<4; count++) {
        RPU_SetDisplay(count, 8007, true);
      }
    }
    if (curSwitch==resetSwitch || resetDoubleClick) {
      returnState = MACHINE_STATE_ATTRACT;
    }
    for (int count=0; count<4; count++) {
#ifdef RPU_OS_USE_7_DIGIT_DISPLAYS
      RPU_SetDisplayBlank(count, ((CurrentTime/500)%2)?0x78:0x00);
#else      
      RPU_SetDisplayBlank(count, ((CurrentTime/500)%2)?0x3C:0x00);
#endif      
    }
  } else if (curState==MACHINE_STATE_TEST_SCORE_LEVEL_1) {
#ifdef RPU_OS_USE_SB100    
    if (curStateChanged) RPU_PlaySB100(0);
#endif
    savedScoreStartByte = RPU_AWARD_SCORE_1_EEPROM_START_BYTE;
  } else if (curState==MACHINE_STATE_TEST_SCORE_LEVEL_2) {
    savedScoreStartByte = RPU_AWARD_SCORE_2_EEPROM_START_BYTE;
  } else if (curState==MACHINE_STATE_TEST_SCORE_LEVEL_3) {
    savedScoreStartByte = RPU_AWARD_SCORE_3_EEPROM_START_BYTE;
  } else if (curState==MACHINE_STATE_TEST_HISCR) {
    savedScoreStartByte = RPU_HIGHSCORE_EEPROM_START_BYTE;
  } else if (curState==MACHINE_STATE_TEST_CREDITS) {
    if (curStateChanged) {
      SavedValue = RPU_ReadByteFromEEProm(RPU_CREDITS_EEPROM_BYTE);
      RPU_SetDisplay(0, SavedValue, true);
    }
    if (curSwitch==resetSwitch || resetDoubleClick) {
      if (RPU_GetUpDownSwitchState()) {
        SavedValue += 1;
        if (SavedValue>99) SavedValue = 0;
      } else {
        if (SavedValue>0) SavedValue -= 1;
        else SavedValue = 99;
      }
      RPU_SetDisplay(0, SavedValue, true);
      RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, SavedValue & 0x000000FF);
    }
  } else if (curState==MACHINE_STATE_TEST_TOTAL_PLAYS) {
    auditNumStartByte = RPU_TOTAL_PLAYS_EEPROM_START_BYTE;
  } else if (curState==MACHINE_STATE_TEST_TOTAL_REPLAYS) {
    auditNumStartByte = RPU_TOTAL_REPLAYS_EEPROM_START_BYTE;
  } else if (curState==MACHINE_STATE_TEST_HISCR_BEAT) {
    auditNumStartByte = RPU_TOTAL_HISCORE_BEATEN_START_BYTE;
  } else if (curState==MACHINE_STATE_TEST_CHUTE_2_COINS) {
    auditNumStartByte = RPU_CHUTE_2_COINS_START_BYTE;
  } else if (curState==MACHINE_STATE_TEST_CHUTE_1_COINS) {
    auditNumStartByte = RPU_CHUTE_1_COINS_START_BYTE;
  } else if (curState==MACHINE_STATE_TEST_CHUTE_3_COINS) {
    auditNumStartByte = RPU_CHUTE_3_COINS_START_BYTE;
#ifndef RPU_OS_DISABLE_CPC_FOR_SPACE      
  } else if (curState==MACHINE_STATE_ADJUST_CPC_CHUTE_1) {
    cpcSelectorStartByte = RPU_CPC_CHUTE_1_SELECTION_BYTE;
  } else if (curState==MACHINE_STATE_ADJUST_CPC_CHUTE_2) {
    cpcSelectorStartByte = RPU_CPC_CHUTE_2_SELECTION_BYTE;
  } else if (curState==MACHINE_STATE_ADJUST_CPC_CHUTE_3) {
    cpcSelectorStartByte = RPU_CPC_CHUTE_3_SELECTION_BYTE;
#endif
  // --- Lectronamo Custom Adjustments Menu ---
  } else if (curState == MACHINE_STATE_ADJUSTMENTS_MENU) {
    if (curStateChanged) {
      CurValue = 0; // Index of the current MPU switch setting
    }

    if (curSwitch == resetSwitch || resetDoubleClick) {
      // Credit button was pressed, change the value of the current setting
      MpuSwitchSetting* setting = &mpuSwitches[CurValue];
      if (setting->id >= 17 && setting->id <= 19) { // Special case for Max Credits
          byte s17 = RPU_ReadByteFromEEProm(ADDR_MAX_CREDITS_17);
          byte s18 = RPU_ReadByteFromEEProm(ADDR_MAX_CREDITS_18);
          byte s19 = RPU_ReadByteFromEEProm(ADDR_MAX_CREDITS_19);
          // Cycle 8 -> 10 -> 15 -> 25 -> 8
          if (s17==0 && s18==0 && s19==0) { s19=1; } // 8 -> 10
          else if (s17==0 && s18==0 && s19==1) { s18=1; s19=0; } // 10 -> 15
          else if (s17==0 && s18==1 && s19==0) { s19=1; } // 15 -> 25
          else { s17=0; s18=0; s19=0; } // 25 -> 8
          RPU_WriteByteToEEProm(ADDR_MAX_CREDITS_17, s17);
          RPU_WriteByteToEEProm(ADDR_MAX_CREDITS_18, s18);
          RPU_WriteByteToEEProm(ADDR_MAX_CREDITS_19, s19);
      } else {
          byte currentValue = RPU_ReadByteFromEEProm(setting->eepromAddr);
          RPU_WriteByteToEEProm(setting->eepromAddr, !currentValue); // Toggle 0 to 1 or 1 to 0
      }
    } else if (curSwitch == SW_SELF_TEST_SWITCH) {
      // Self-test button was pressed, move to next/prev adjustment
      if (RPU_GetUpDownSwitchState()) { // Down
        CurValue++;
        if (CurValue >= NUM_MPU_SWITCHES) CurValue = 0;
      } else { // Up
        if (CurValue == 0) CurValue = NUM_MPU_SWITCHES - 1;
        else CurValue--;
      }
      if (mpuSwitches[CurValue].id >= 17 && mpuSwitches[CurValue].id <= 18) {
        CurValue = (RPU_GetUpDownSwitchState()) ? 19 : 16; // Skip to S19 or S16
      }
    }

    // Display the current adjustment
    MpuSwitchSetting* setting = &mpuSwitches[CurValue];
    RPU_SetDisplay(0, setting->id, true); // Show adjustment number (S1-S32)
    RPU_SetDisplayText(1, setting->name, true);

    if (setting->id >= 17 && setting->id <= 19) { // Special case for Max Credits
        byte s17 = RPU_ReadByteFromEEProm(ADDR_MAX_CREDITS_17);
        byte s18 = RPU_ReadByteFromEEProm(ADDR_MAX_CREDITS_18);
        byte s19 = RPU_ReadByteFromEEProm(ADDR_MAX_CREDITS_19);
        if (s17==0 && s18==0 && s19==0) RPU_SetDisplay(2, 8, true);
        else if (s17==0 && s18==0 && s19==1) RPU_SetDisplay(2, 10, true);
        else if (s17==0 && s18==1 && s19==0) RPU_SetDisplay(2, 15, true);
        else RPU_SetDisplay(2, 25, true);
    } else {
        byte value = RPU_ReadByteFromEEProm(setting->eepromAddr);
        RPU_SetDisplayText(2, (char*)(value ? setting->option1 : setting->option0), true);
    }
#endif    
  }

  if (savedScoreStartByte) {
    if (curStateChanged) {
      SavedValue = RPU_ReadULFromEEProm(savedScoreStartByte);
      RPU_SetDisplay(0, SavedValue, true);  
    }

    if (curSwitch==resetSwitch) {
      if (RPU_GetUpDownSwitchState()) {
        SavedValue += 1000;
      } else {
        if (SavedValue>1000) SavedValue -= 1000;
        else SavedValue = 0;
      }
      RPU_SetDisplay(0, SavedValue, true);  
      RPU_WriteULToEEProm(savedScoreStartByte, SavedValue);
    }

    if (resetBeingHeld && (CurrentTime>=NextSpeedyValueChange)) {
      if (RPU_GetUpDownSwitchState()) {
        SavedValue += 1000;
      } else {
        if (SavedValue>1000) SavedValue -= 1000;
        else SavedValue = 0;
      }
      RPU_SetDisplay(0, SavedValue, true);  
      if (NumSpeedyChanges<6) NextSpeedyValueChange = CurrentTime + 400;
      else if (NumSpeedyChanges<50) NextSpeedyValueChange = CurrentTime + 50;
      else NextSpeedyValueChange = CurrentTime + 10;
      NumSpeedyChanges += 1;
    }

    if (!resetBeingHeld && NumSpeedyChanges>0) {
      RPU_WriteULToEEProm(savedScoreStartByte, SavedValue);
      NumSpeedyChanges = 0;
    }
    
    if (resetDoubleClick) {
      SavedValue = 0;
      RPU_SetDisplay(0, SavedValue, true);  
      RPU_WriteULToEEProm(savedScoreStartByte, SavedValue);
    }
  }

  if (auditNumStartByte) {
    if (curStateChanged) {
      SavedValue = RPU_ReadULFromEEProm(auditNumStartByte);
      RPU_SetDisplay(0, SavedValue, true);
    }

    if (resetDoubleClick) {
      SavedValue = 0;
      RPU_SetDisplay(0, SavedValue, true);  
      RPU_WriteULToEEProm(auditNumStartByte, SavedValue);
    }
    
  }

#ifndef RPU_OS_DISABLE_CPC_FOR_SPACE  
  if (cpcSelectorStartByte) {
    if (curStateChanged) {
      SavedValue = RPU_ReadByteFromEEProm(cpcSelectorStartByte);
      if (SavedValue>NUM_CPC_PAIRS) SavedValue = 4;
      RPU_SetDisplay(0, CPCPairs[SavedValue][0], true);
      RPU_SetDisplay(1, CPCPairs[SavedValue][1], true);
    }

    if (curSwitch==resetSwitch) {
      byte lastValue = (byte)SavedValue;
      if (RPU_GetUpDownSwitchState()) {
        SavedValue += 1;
        if (SavedValue>=NUM_CPC_PAIRS) SavedValue = 0;
      } else {
        if (SavedValue>0) SavedValue -= 1;
      }
      RPU_SetDisplay(0, CPCPairs[SavedValue][0], true);
      RPU_SetDisplay(1, CPCPairs[SavedValue][1], true);
      if (lastValue!=SavedValue) {
        RPU_WriteByteToEEProm(cpcSelectorStartByte, (byte)SavedValue);
        if (cpcSelectorStartByte==RPU_CPC_CHUTE_1_SELECTION_BYTE) CPCSelection[0] = (byte)SavedValue;
        else if (cpcSelectorStartByte==RPU_CPC_CHUTE_2_SELECTION_BYTE) CPCSelection[1] = (byte)SavedValue;
        else if (cpcSelectorStartByte==RPU_CPC_CHUTE_3_SELECTION_BYTE) CPCSelection[2] = (byte)SavedValue;
      }
    }
  }
#endif  
  
  return returnState;
}

unsigned long GetLastSelfTestChangedTime() {
  return LastSelfTestChange;
}


void SetLastSelfTestChangedTime(unsigned long setSelfTestChange) {
  LastSelfTestChange = setSelfTestChange;
}