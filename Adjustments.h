// Adjustments.h — Operator Settings for Lectronamo Recharged
// This file defines all tunable parameters the operator can adjust via the menu system

#ifndef ADJUSTMENTS_H
#define ADJUSTMENTS_H

// Skill Shot
#define ADJ_SKILL_SHOT_ENABLED       true        // Enable/Disable skill shot feature
#define ADJ_SKILL_SHOT_POINTS        5000        // Points for successful skill shot
#define ADJ_SKILL_SHOT_TIMEOUT_MS    0           // Lamp pulses until switch hit (0 = no timeout)

// Ball Save
#define ADJ_BALL_SAVE_TIME_SEC       20          // Duration of ball save (seconds)

// Combo Timing
#define ADJ_ARC_SURGE_COMBO_TIME_MS  5000        // Time window for right inlane → saucer combo (ms)

// Mode Timers
#define ADJ_SPINNER_MODE_TIME_SEC    30          // Spinner Rips Mode Duration
#define ADJ_POP_MODE_TIME_SEC        30          // Pop Bumper Frenzy Duration
#define ADJ_DROP_MODE_TIME_SEC       60          // Drop Target Assault Duration
#define ADJ_HURRY_UP_TIME_SEC        15          // Hurry-up countdown variant

// Mini Wizard
#define ADJ_MINI_WIZARD_RETRIES      true        // Allow retries on Mini Wizard if failed
#define ADJ_MINI_WIZARD_SCORE        100000      // Points awarded on successful completion

// Wizard Mode
#define ADJ_WIZARD_MODE_SCORE        500000      // Points awarded on successful completion
#define ADJ_WIZARD_MODE_RETRIES      true        // Allow replay of Wizard Mode if failed

// Mystery Award Odds (values are percentage chances, must total 100%)
#define ODDS_LIGHT_EXTRA_BALL        5
#define ODDS_ADVANCE_POPS            20
#define ODDS_BIG_POINTS              5
#define ODDS_PUNY_POINTS             40
#define ODDS_ADVANCE_MULTIPLIER      10
#define ODDS_HOLD_BONUS              15
#define ODDS_TILT_ENDS_GAME          5

// Extra Ball Rules
#define ADJ_MAX_EXTRA_BALLS_PER_BALL 1           // Max EBs per ball

// Bonus
#define ADJ_BONUS_CAP_POINTS         30000       // Cap max bonus value
#define ADJ_BONUS_HOLD_ENABLED       true        // Enable Hold Bonus feature

#endif // ADJUSTMENTS_H
