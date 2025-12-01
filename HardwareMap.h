#ifndef HARDWARE_MAP_H
#define HARDWARE_MAP_H

// ---------------------------
// Switch Definitions
// ---------------------------
enum Switch {
    SW_RIGHT_OUTLANE = 1,
    SW_RIGHT_RETURN_LANE,
    SW_LEFT_OUTLANE,
    SW_LEFT_RETURN_LANE,
    SW_TARGET_1_5BANK,
    SW_CREDIT_BUTTON,
    SW_TILT,
    SW_OUTHOLE,
    SW_COIN_3,
    SW_COIN_1,
    SW_COIN_2,
    SW_STANDUP_TARGET,
    SW_TARGET_2_5BANK,
    SW_TARGET_3_3BANK,
    SW_SPIN_TARGET,
    SW_SLAM_TILT,
    SW_ROLLOVER_BUTTON,
    SW_ADV_BONUS_300,
    SW_SCORE_10,
    SW_UNUSED_20,
    SW_TARGET_3_5BANK,
    SW_TARGET_2_3BANK,
    SW_UNUSED_23,
    SW_KICKER,
    SW_ADV_BONUS_1000,
    SW_UNUSED_26,
    SW_UNUSED_27,
    SW_UNUSED_28,
    SW_TARGET_4_5BANK,
    SW_TARGET_3_3BANK,
    SW_UNUSED_31,
    SW_UNUSED_32,
    SW_THUMPER_CENTER,
    SW_THUMPER_RIGHT,
    SW_THUMPER_LEFT,
    SW_UNUSED_36,
    SW_TARGET_5_5BANK,
    SW_RIGHT_SLINGSHOT,
    SW_LEFT_SLINGSHOT,
    SW_SAUCER
};

// ---------------------------
// Solenoid Definitions
// ---------------------------
enum Solenoid {
    SOL_LEFT_THUMPER = 7,
    SOL_RIGHT_THUMPER,
    SOL_CENTER_THUMPER,
    SOL_OUTHOLE,
    SOL_RIGHT_SLINGSHOT,
    SOL_KICKER,
    SOL_DROP_TARGET_3BANK_RESET,
    SOL_SAUCER,
    SOL_DROP_TARGET_5BANK_RESET,
    SOL_LEFT_SLINGSHOT,
    SOL_KNOCKER,
    SOL_FLIPPERS,
    SOL_COIN_LOCKOUT
};

// ---------------------------
// Lamp Definitions
// ---------------------------

// Playfield Lamps (J3)
enum Lamp {
    LAMP_COLLECT_3000 = 26,
    LAMP_COLLECT_7000 = 25,
    LAMP_ADV_BONUS_3 = 17,
    LAMP_LEFT_RETURN_9000 = 21,
    LAMP_5X = 20,
    LAMP_2X = 22,
    LAMP_COLLECT_4000 = 1,
    LAMP_COLLECT_8000 = 12,
    LAMP_ADV_BONUS_4 = 11,
    LAMP_QUINTUPLE = 10,
    LAMP_EXTRA_BALL = 18,
    LAMP_3X = 13,

    // Backbox Lamps (J2) — Offset by 100 to avoid collision
    LAMP_BALL_IN_PLAY = 122,
    LAMP_PLAYER_1 = 116,
    LAMP_PLAYER_UP_1 = 114,
    LAMP_MATCH = 108,
    LAMP_HIGH_SCORE_TO_DATE = 123,
    LAMP_PLAYER_2 = 120,
    LAMP_PLAYER_UP_2 = 115,
    LAMP_GAME_OVER = 111,
    LAMP_PLAYER_3 = 106,
    LAMP_PLAYER_UP_3 = 102,
    LAMP_TILT = 110,
    LAMP_PLAYER_4 = 107,
    LAMP_PLAYER_UP_4 = 101
};

#endif // HARDWARE_MAP_H