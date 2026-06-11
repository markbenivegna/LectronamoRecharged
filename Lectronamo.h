

#define LAMP_5_BANK_TARGET_SPECIAL      32 // Q1
#define LAMP_DROP_TARGET_2X             36 // Q2  (physically: drop target 2X indicator)
#define LAMP_SHOOT_AGAIN                40 // Q3
#define LAMP_SPINNER                    28 // Q4
#define LAMP_PLAYER_1                   52 // Q5
#define LAMP_1ST_PLAYER_UP              56 // Q6
// #define LAMP_UNUSED                  44 // Q7
#define LAMP_ADVANCE_BONUS_1            12 // Q8
// #define LAMP_UNUSED                  16 // Q9
// #define LAMP_UNUSED                  20 // Q10
#define LAMP_XTRA_BALL                  24 // Q11 — physically: right return lane (extra ball collect)
#define LAMP_BONUS_5K                   4  // Q12
#define LAMP_BONUS_9K                   8  // Q13
#define LAMP_BONUS_1K                   0  // Q14
#define LAMP_HIGHEST_SCORE              49 // Q15
#define LAMP_BALL_IN_PLAY               48 // Q16
#define LAMP_DROP_TARGET_3X             37 // Q17 (physically: drop target 3X indicator)
#define LAMP_PLAYER_2                   53 // Q18
#define LAMP_2ND_PLAYER_UP              57 // Q19
// #define LAMP_UNUSED                  30 // Q20
// #define LAMP_UNUSED                  45 // Q21
// #define LAMP_UNUSED                  21 // Q22
#define LAMP_MATCH                      41 // Q23
#define LAMP_ROLLOVER_BUTTON            29 // Q25 (physically swapped)
#define LAMP_SAUCER                     33 // Q24 (physically swapped)
// #define LAMP_UNUSED                  25 // Q26
#define LAMP_BONUS_6K                   5  // Q27
#define LAMP_BONUS_10K                  9  // Q28
#define LAMP_BONUS_2K                   1  // Q29
#define LAMP_PLAYER_3                   54 // Q30
#define LAMP_3RD_PLAYER_UP              58 // Q31
// #define LAMP_UNUSED                  26 // Q32
#define LAMP_GAME_OVER                  50 // Q33
// #define LAMP_UNUSED                  17 // Q34
#define LAMP_ADVANCE_BONUS_2            13 // Q35
#define LAMP_BONUS_3K                   2  // Q36
// #define LAMP_UNUSED                  22 // Q37
#define LAMP_BONUS_7K                   6  // Q38
// #define LAMP_UNUSED                  46 // Q39
#define LAMP_BONUS_MULTIPLIER_2X        42 // Q40 (physically: bonus multiplier 2X)
#define LAMP_DROP_TARGET_5X             38 // Q41 (physically: drop target 5X indicator)
#define LAMP_LEFT_RETURN                34 // Q42
#define LAMP_PLAYER_4                   55 // Q43
// #define LAMP_UNUSED                  10 // Q44
#define LAMP_4TH_PLAYER_UP              59 // Q45
#define LAMP_EXTRA_BALL_LANE            39 // Q46 — physically: near 5-bank (1st completion indicator) — needs lamp test verification
#define LAMP_TILT                       51 // Q47
// #define LAMP_UNUSED                  18 // Q48
#define LAMP_ADVANCE_BONUS_3            14 // Q49
#define LAMP_BONUS_8K                   7  // Q50
// #define LAMP_UNUSED                  11 // Q51
#define LAMP_BONUS_MULTIPLIER_3X        43 // Q52 (physically: bonus multiplier 3X)
// #define LAMP_UNUSED                  47 // Q53
#define LAMP_ADVANCE_BONUS_4            15 // Q54
// #define LAMP_UNUSED                  19 // Q55
#define LAMP_BONUS_MULTIPLIER_5X        35 // Q56 (physically: bonus multiplier 5X)
#define LAMP_BONUS_4K                   3  // Q57
// #define LAMP_UNUSED                  31 // Q58
// #define LAMP_UNUSED                  27 // Q59
// #define LAMP_UNUSED                  23 // Q60



#define SW_RIGHT_OUTLANE            0
#define SW_RIGHT_INLANE             1
#define SW_LEFT_OUTLANE             2
#define SW_LEFT_INLANE              3
#define SW_TARGET_1_5BANK           4
#define SW_CREDIT_BUTTON            5
#define SW_TILT                     6
#define SW_OUTHOLE                  7
#define SW_COIN_3                 255  // Not wired on this machine
#define SW_COIN_1                   9
#define SW_COIN_2                  10
#define SW_STANDUP_TARGET          11
#define SW_TARGET_2_5BANK          12
#define SW_TARGET_3_3BANK          13
#define SW_SPINNER                 14
#define SW_SLAM_TILT               15
#define SW_ROLLOVER_BUTTON         16
#define SW_ADV_BONUS_300           17
#define SW_SCORE_10                18
// #define SW_                     19
#define SW_TARGET_3_5BANK          20
#define SW_TARGET_2_3BANK          21
// #define SW_                     22
#define SW_KICKER                  23
#define SW_ADV_BONUS_1000          24
// #define SW_                     25
// #define SW_                     26
// #define SW_                     27
#define SW_TARGET_4_5BANK          28
#define SW_TARGET_1_3BANK          29
// #define SW_                     30
// #define SW_                     31
// #define SW_                     32 (not used)
#define SW_TARGET_5_5BANK          36
#define SW_RIGHT_SLING             37
#define SW_LEFT_SLING              38
#define SW_SAUCER                  39
#define SW_BOTTOM_POP              32
#define SW_RIGHT_POP               33
#define SW_LEFT_POP                34

// Solenoid numbering: 4-14, verified mappings from hardware testing
#define SOL_DROP_TARGET_3BANK_RESET   4
#define SOL_KNOCKER                   5
#define SOL_OUTHOLE                   6
#define SOL_SAUCER                    7
#define SOL_KICKER                    8
#define SOL_DROP_TARGET_5BANK_RESET   9
#define SOL_LEFT_THUMPER             10
#define SOL_RIGHT_THUMPER            11
#define SOL_CENTER_THUMPER           12
#define SOL_RIGHT_SLINGSHOT          13
#define SOL_LEFT_SLINGSHOT           14

//----------------------------
// Sound Definitions (SND_) - Stern SB-100 Sound Board (bitmask)
//----------------------------
#define SND_10_POINTS       (1 << 0)  // Solenoid 1 — highest pitch
#define SND_100_POINTS      (1 << 1)  // Solenoid 2
#define SND_1000_POINTS     (1 << 2)  // Solenoid 3
#define SND_10000_POINTS    (1 << 3)  // Solenoid 4 — lowest pitch / dramatic
#define SND_ADD_BONUS       (1 << 4)  // Solenoid 5 — bonus collect chime
#define SND_POP_BUMPER      (1 << 5)  // Solenoid 6 — pop bumper thwack

// ── Framework compatibility aliases ──────────────────────────────────────────
// Head lamp names used by the RPU framework internals
#define LAMP_HEAD_TILT          LAMP_TILT           // 51
#define LAMP_HEAD_HIGH_SCORE    LAMP_HIGHEST_SCORE  // 49
#define LAMP_HEAD_MATCH         LAMP_MATCH          // 41
#define LAMP_HEAD_GAME_OVER     LAMP_GAME_OVER      // 50

// Player-up lamp names expected by framework display/lamp routines
#define LAMP_HEAD_PLAYER_1_UP   LAMP_1ST_PLAYER_UP  // 56
#define LAMP_HEAD_PLAYER_2_UP   LAMP_2ND_PLAYER_UP  // 57
#define LAMP_HEAD_PLAYER_3_UP   LAMP_3RD_PLAYER_UP  // 58
#define LAMP_HEAD_PLAYER_4_UP   LAMP_4TH_PLAYER_UP  // 59

// Bonus multiplier short-names used in ShowBonusXLamps()
#define LAMP_2X  LAMP_BONUS_MULTIPLIER_2X  // 42
#define LAMP_3X  LAMP_BONUS_MULTIPLIER_3X  // 43
#define LAMP_5X  LAMP_BONUS_MULTIPLIER_5X  // 35

// Switch name aliases
#define SW_CREDIT_RESET     SW_CREDIT_BUTTON   // 5
#define SW_CENTER_THUMPER   SW_BOTTOM_POP      // 32
#define SW_RIGHT_THUMPER    SW_RIGHT_POP       // 33
#define SW_LEFT_THUMPER     SW_LEFT_POP        // 34
#define SW_RIGHT_SLINGSHOT  SW_RIGHT_SLING     // 37
#define SW_LEFT_SLINGSHOT   SW_LEFT_SLING      // 38
// ─────────────────────────────────────────────────────────────────────────────



#define NUM_SWITCHES_WITH_TRIGGERS          5 // total number of solenoid/switch pairs
#define NUM_PRIORITY_SWITCHES_WITH_TRIGGERS 2 // slingshots only — thumpers use validated (debounced) path

struct PlayfieldAndCabinetSwitch SolenoidAssociatedSwitches[] = {
  { SW_RIGHT_SLING, SOL_RIGHT_SLINGSHOT, 4},
  { SW_LEFT_SLING,  SOL_LEFT_SLINGSHOT,  4},
  { SW_BOTTOM_POP,  SOL_CENTER_THUMPER,  6},
  { SW_RIGHT_POP,   SOL_RIGHT_THUMPER,   6},
  { SW_LEFT_POP,    SOL_LEFT_THUMPER,    6}
};
