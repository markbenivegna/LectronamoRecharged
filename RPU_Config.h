#ifndef RPU_CONFIG_H
#define RPU_CONFIG_H

// --- Hardware and Architecture Definitions ---

// Set the hardware revision of the RPU plug-in board. 
// Hardware Rev 3 uses a MEGA 2560 Pro, and nothing else
// Hardware Rev 4 uses a MEGA 2560 Pro (all the pins) on a larger board (display & WIFI)
#define RPU_OS_HARDWARE_REV 4

// Set the MPU architecture for the target machine.
// 1 = -17, -35, 100, 200 compatible boards (includes Stern MPU-100)
#define RPU_MPU_ARCHITECTURE 1

// Set the processor type for the build.
// 1 = 6800 processor
// 0 = 6802/6808 processor
#define RPU_MPU_BUILD_FOR_6800 1

// --- Optional Feature Defines ---
// Uncomment the features you want to include in the build.

// #define RPU_OS_USE_DIP_SWITCHES
// #define RPU_OS_USE_S_AND_T
// #define RPU_OS_USE_DASH51
// #define RPU_OS_USE_SB100
// #define RPU_OS_USE_SB300
// #define RPU_OS_USE_WAV_TRIGGER
#define RPU_OS_USE_WAV_TRIGGER_1p3
// #define RPU_OS_DISABLE_CPC_FOR_SPACE
// #define RPU_OS_USE_AUX_LAMPS
#define RPU_OS_USE_7_DIGIT_DISPLAYS
// #define RPU_USE_EXTENDED_SWITCHES_ON_PB4
// #define RPU_USE_EXTENDED_SWITCHES_ON_PB7
// #define RPU_OS_USE_WTYPE_1_SOUND
// #define RPU_OS_USE_WTYPE_2_SOUND
// #define RPU_OS_USE_W11_SOUND


#if (RPU_MPU_ARCHITECTURE==1)
// --- Settings for -17, -35, 100, 200 MPU boards ---
#define RPU_OS_SOFTWARE_DISPLAY_INTERRUPT_INTERVAL 48 // Approx 318.8 Hz
#define CONTSOL_DISABLE_FLIPPERS 0x40
#define CONTSOL_DISABLE_COIN_LOCKOUT 0x80
#define RPU_OS_SWITCH_DELAY_IN_MICROSECONDS 10
#define RPU_OS_TIMING_LOOP_PADDING_IN_MICROSECONDS 20
#define RPU_NUM_LAMP_BANKS 5
#define RPU_MAX_LAMPS 40
#endif

// --- EEPROM Memory Map ---
#define RPU_CREDITS_EEPROM_BYTE 0
#define RPU_HIGHSCORE_EEPROM_START_BYTE 1
#define RPU_AWARD_SCORE_1_EEPROM_START_BYTE 5
#define RPU_AWARD_SCORE_2_EEPROM_START_BYTE 9
#define RPU_AWARD_SCORE_3_EEPROM_START_BYTE 13
#define RPU_TOTAL_PLAYS_EEPROM_START_BYTE 17
#define RPU_TOTAL_REPLAYS_EEPROM_START_BYTE 21
#define RPU_TOTAL_HISCORE_BEATEN_START_BYTE 25

#endif // RPU_CONFIG_H