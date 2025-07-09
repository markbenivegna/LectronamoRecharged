#include "Game.h"
#include "HardwareMap.h"
#include "RPU.h"
#include "Adjustments.h"

// Format: { switch, solenoid, debounceTime_ms }
PlayfieldAndCabinetSwitch SolenoidAssociatedSwitches[] = {
    { SW_LEFT_THUMPER,    SOL_LEFT_THUMPER,    6 },
    { SW_CENTER_THUMPER,  SOL_CENTER_THUMPER,  6 },
    { SW_RIGHT_THUMPER,   SOL_RIGHT_THUMPER,   6 },
    { SW_LEFT_SLINGSHOT,  SOL_LEFT_SLINGSHOT,  4 },
    { SW_RIGHT_SLINGSHOT, SOL_RIGHT_SLINGSHOT, 4 },
    { SW_KICKER,          SOL_KICKER,          10 },
    { SW_SAUCER,          SOL_SAUCER,          10 },
    { SW_OUT_HOLE,        SOL_OUTHOLE,         10 }
};

// Internal game state
int currentBonus = 0;
int bonusMultiplier = 1;
bool extraBallLit = false;
bool saucerLocked = false;

LectronamoRecharged::LectronamoRecharged() {
    currentPlayer = 0;
    currentBall = 1;
    totalPlayers = 1;
}

void LectronamoRecharged::StartGame() {
    currentPlayer = 1;
    currentBall = 1;
    ResetBonus();
    ResetLamps();
    ResetSolenoids();
}

void LectronamoRecharged::EndBall() {
    currentBall++;
    if (currentBall > maxBallsPerGame) {
        EndGame();
    } else {
        PrepareNextBall();
    }
}

void LectronamoRecharged::EndGame() {
    SetLamp(LAMP_GAME_OVER, true);
    PlaySound(SOUND_GAME_OVER);
}

void LectronamoRecharged::AdvanceState() {
    // Handle solenoid-triggered switches with debounce timing
    for (auto &entry : SolenoidAssociatedSwitches) {
        if (RPU::SwitchClosed(entry.sw)) {
            RPU::FireSolenoid(entry.solenoid);
            delay(entry.debounceTime);
        }
    }

    HandleSwitches();
    UpdateLamps();
    UpdateDisplay();
}

void LectronamoRecharged::HandleSwitches() {
    for (int i = 1; i <= MAX_SWITCHES; i++) {
        if (RPU::SwitchClosed((Switch)i)) {
            HandleSwitchEvent((Switch)i);
        }
    }
}

void LectronamoRecharged::HandleSwitchEvent(Switch sw) {
    switch (sw) {
        case SW_RIGHT_OUTLANE:
        case SW_LEFT_OUTLANE:
            DrainBall();
            break;

        case SW_OUT_HOLE:
            RPU::PulseSolenoid(SOL_OUTHOLE);
            break;

        case SW_SPIN_TARGET:
            AddScore(100);
            break;

        case SW_KICKER:
            RPU::PulseSolenoid(SOL_KICKER);
            AddScore(1000);
            break;

        case SW_SAUCER:
            HandleSaucer();
            break;

        case SW_ADV_BONUS_300:
            AddBonus(1);
            break;

        case SW_ADV_BONUS_1000:
            AddBonus(3);
            break;

        case SW_STANDUP_TARGET:
            bonusMultiplier = 2;
            break;

        case SW_TARGET_1_5BANK:
        case SW_TARGET_2_5BANK:
        case SW_TARGET_3_5BANK:
        case SW_TARGET_4_5BANK:
        case SW_TARGET_5_5BANK:
            AddScore(500);
            break;

        case SW_CREDIT_BUTTON:
            RPU::StartGameIfReady();
            break;

        default:
            break;
    }
}

void LectronamoRecharged::FireSolenoid(Solenoid sol) {
    RPU::FireSolenoid(sol);
}

void LectronamoRecharged::SetLamp(Lamp lamp, bool state) {
    RPU::SetLamp(lamp, state);
}

void LectronamoRecharged::ResetLamps() {
    for (int i = 0; i < NUM_LAMPS; i++) {
        SetLamp((Lamp)i, false);
    }
}

void LectronamoRecharged::ResetSolenoids() {
    // Placeholder for any solenoid release logic
}

void LectronamoRecharged::ResetBonus() {
    currentBonus = 0;
    bonusMultiplier = 1;
}

void LectronamoRecharged::AddBonus(int value) {
    currentBonus += value;
    if (currentBonus > 20) currentBonus = 20;
    AddScore(1000 * value * bonusMultiplier);
}

void LectronamoRecharged::UpdateLamps() {
    RPU::SetLamp(LAMP_2X, bonusMultiplier >= 2);
    RPU::SetLamp(LAMP_3X, bonusMultiplier >= 3);
    RPU::SetLamp(LAMP_5X, bonusMultiplier >= 5);
    RPU::SetLamp(LAMP_EXTRA_BALL, extraBallLit);
}

void LectronamoRecharged::UpdateDisplay() {
    RPU::UpdateScoreDisplay();
    RPU::UpdateBallInPlayDisplay();
}

void LectronamoRecharged::HandleSaucer() {
    if (!saucerLocked) {
        RPU::PulseSolenoid(SOL_SAUCER);
        AddScore(3000);
        saucerLocked = true;
        RPU::DelayThenUnlock(&saucerLocked, 1500);
    }
}

void LectronamoRecharged::DrainBall() {
    RPU::EndBall();
}

void LectronamoRecharged::AddScore(int value) {
    RPU::AddScore(value);
}