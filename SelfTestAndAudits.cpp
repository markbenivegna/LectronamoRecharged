#include "SelfTestAndAudits.h"
#include "RPU.h"
#include "HardwareMap.h"
#include "Adjustments.h"

namespace SelfTest {

enum TestMode {
    TEST_SWITCHES,
    TEST_LAMPS,
    TEST_SOLENOIDS,
    TEST_AUDITS,
    TEST_ADJUSTMENTS,
    TEST_COMPLETE
};

static TestMode currentMode = TEST_SWITCHES;
static bool testActive = false;
static int testIndex = 0;
static unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;

void StartTestMode() {
    currentMode = TEST_SWITCHES;
    testIndex = 0;
    testActive = true;
    RPU::DisplayMessage("ENTER TEST MODE");
}

void ExitTestMode() {
    testActive = false;
    RPU::DisplayMessage("EXIT TEST MODE");
}

void AdvanceMode() {
    currentMode = static_cast<TestMode>((currentMode + 1) % TEST_COMPLETE);
    testIndex = 0;
    ShowCurrentTestName();
}

void ShowCurrentTestName() {
    switch (currentMode) {
        case TEST_SWITCHES:
            RPU::DisplayMessage("SWITCH TEST");
            break;
        case TEST_LAMPS:
            RPU::DisplayMessage("LAMP TEST");
            break;
        case TEST_SOLENOIDS:
            RPU::DisplayMessage("SOL TEST");
            break;
        case TEST_AUDITS:
            RPU::DisplayMessage("AUDITS");
            break;
        case TEST_ADJUSTMENTS:
            RPU::DisplayMessage("ADJUSTMENTS");
            break;
        default:
            break;
    }
}

void HandleInput() {
    if (!testActive) return;

    if (RPU::SwitchClosed(SW_LEFT_FLIPPER) && millis() - lastDebounceTime > debounceDelay) {
        testIndex--;
        lastDebounceTime = millis();
    }

    if (RPU::SwitchClosed(SW_RIGHT_FLIPPER) && millis() - lastDebounceTime > debounceDelay) {
        testIndex++;
        lastDebounceTime = millis();
    }

    if (RPU::SwitchClosed(SW_CREDIT_BUTTON) && millis() - lastDebounceTime > debounceDelay) {
        AdvanceMode();
        lastDebounceTime = millis();
    }

    RunCurrentTest();
}

void RunCurrentTest() {
    switch (currentMode) {
        case TEST_SWITCHES:
            RunSwitchTest();
            break;
        case TEST_LAMPS:
            RunLampTest();
            break;
        case TEST_SOLENOIDS:
            RunSolenoidTest();
            break;
        case TEST_AUDITS:
            ShowAudits();
            break;
        case TEST_ADJUSTMENTS:
            AdjustSettings();
            break;
        default:
            break;
    }
}

void RunSwitchTest() {
    for (int sw = 1; sw <= MAX_SWITCHES; ++sw) {
        if (RPU::SwitchClosed(sw)) {
            String label = RPU::GetSwitchLabel(sw);
            RPU::DisplayMessage("SWITCH " + String(sw) + ": " + label);
            delay(250);
        }
    }
}

void RunLampTest() {
    RPU::DisplayMessage("LAMP " + String(testIndex));
    for (int i = 0; i < NUM_LAMPS; ++i) {
        RPU::SetLamp(i, i == testIndex);
    }
}

void RunSolenoidTest() {
    RPU::DisplayMessage("SOL " + String(testIndex));
    RPU::PulseSolenoid(testIndex);
    delay(250);
}

void ShowAudits() {
    RPU::DisplayMessage("AUDITS");
    RPU::DisplayMessage("GAMES: " + String(RPU::GetAuditValue("GAMES")));
    RPU::DisplayMessage("PLAYS: " + String(RPU::GetAuditValue("PLAYS")));
    RPU::DisplayMessage("EXTRAS: " + String(RPU::GetAuditValue("EXTRAS")));
    delay(3000);
}

void AdjustSettings() {
    RPU::DisplayMessage("ADJUST MODE");
    RPU::DisplayMessage("FLIPPERS = NAV");
    RPU::DisplayMessage("START = EDIT");
    // Placeholder for real menu logic
}

} // namespace SelfTest