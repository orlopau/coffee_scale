#include <string.h>

#include "unity.h"
#include "mode_manager.h"
#include "millis.h"
#include "mocks.h"

class MockMode : public Mode
{
public:
    MockMode(const char *name) : updateCalled(false), name(name){};
    ~MockMode(){};
    void update()
    {
        updateCalled = true;
    };
    bool canSwitchMode()
    {
        return switchable;
    };
    const char *getName()
    {
        return name;
    };
    bool updateCalled;
    const char *name;
    bool switchable = true;
};

MockBattery *battery;
MockDisplay *display;
MockButtons *input;
ModeManager *modeManager;
Mode *modes[3];
MockMode *mockModes[3];

void setUp(void)
{
    battery = new MockBattery();
    display = new MockDisplay();
    input = new MockButtons();
    modes[0] = new MockMode("Mock Mode 1");
    modes[1] = new MockMode("Mock Mode 2");
    modes[2] = new MockMode("Mock Mode 3");
    mockModes[0] = (MockMode *)modes[0];
    mockModes[1] = (MockMode *)modes[1];
    mockModes[2] = (MockMode *)modes[2];
    modeManager = new ModeManager(modes, 3, *display, *input, *battery);
}

void tearDown(void)
{
    delete battery;
    delete display;
    delete input;
    delete modeManager;
    delete modes[0];
    delete modes[1];
    delete modes[2];
}

void enterSelection()
{
    input->encoderClick = ClickType::LONG;
    modeManager->update();
    input->encoderClick = ClickType::NONE;
    modeManager->update();
}

void test_mode_manager_updates_current_mode_by_default(void)
{
    modeManager->update();
    TEST_ASSERT_TRUE(mockModes[0]->updateCalled);
}

void test_mode_manager_shows_current_mode_after_long_click(void)
{
    enterSelection();
    TEST_ASSERT_EQUAL_STRING("Mock Mode 1", display->lastModeText);
}

void test_mode_manager_shows_next_and_previous_mode_after_rotation(void)
{
    enterSelection();
    TEST_ASSERT_EQUAL_STRING("Mock Mode 1", display->lastModeText);

    input->encoderDirection = EncoderDirection::CW;
    modeManager->update();
    TEST_ASSERT_EQUAL_STRING("Mock Mode 2", display->lastModeText);

    input->encoderDirection = EncoderDirection::CCW;
    modeManager->update();
    TEST_ASSERT_EQUAL_STRING("Mock Mode 1", display->lastModeText);
}

void test_mode_manager_modes_lower_bound(void)
{
    enterSelection();
    TEST_ASSERT_EQUAL_STRING("Mock Mode 1", display->lastModeText);

    input->encoderDirection = EncoderDirection::CCW;
    modeManager->update();
    TEST_ASSERT_EQUAL_STRING("Mock Mode 1", display->lastModeText);
}

void test_mode_manager_modes_upper_bound(void)
{
    enterSelection();
    TEST_ASSERT_EQUAL_STRING("Mock Mode 1", display->lastModeText);

    input->encoderDirection = EncoderDirection::CW;
    modeManager->update();
    TEST_ASSERT_EQUAL_STRING("Mock Mode 2", display->lastModeText);

    input->encoderDirection = EncoderDirection::CW;
    modeManager->update();
    TEST_ASSERT_EQUAL_STRING("Mock Mode 3", display->lastModeText);

    input->encoderDirection = EncoderDirection::CW;
    modeManager->update();
    TEST_ASSERT_EQUAL_STRING("Mock Mode 3", display->lastModeText);
}

void test_mode_manager_does_not_call_update_when_changing(void)
{
    enterSelection();
    mockModes[0]->updateCalled = false;

    // some updates
    modeManager->update();
    modeManager->update();
    modeManager->update();

    TEST_ASSERT_FALSE(mockModes[0]->updateCalled);
    TEST_ASSERT_FALSE(mockModes[1]->updateCalled);
    TEST_ASSERT_FALSE(mockModes[2]->updateCalled);
}

void test_mode_manager_selects_mode_with_single_click(void)
{
    enterSelection();
    TEST_ASSERT_EQUAL_STRING("Mock Mode 1", display->lastModeText);

    input->encoderDirection = EncoderDirection::CW;
    modeManager->update();
    input->encoderDirection = EncoderDirection::NONE;
    TEST_ASSERT_EQUAL_STRING("Mock Mode 2", display->lastModeText);

    input->encoderClick = ClickType::SINGLE;
    modeManager->update();
    input->encoderClick = ClickType::NONE;
    modeManager->update();
    TEST_ASSERT_TRUE(mockModes[1]->updateCalled);
}

void test_mode_manager_can_only_switch_when_mode_allows_it(void)
{
    mockModes[0]->switchable = false;
    enterSelection();
    TEST_ASSERT_NULL(display->lastModeText);
    TEST_ASSERT_TRUE(mockModes[0]->updateCalled);
}

void test_mode_manager_updates_next_mode_only_at_next_tick(void)
{
    enterSelection();
    TEST_ASSERT_EQUAL_STRING("Mock Mode 1", display->lastModeText);

    input->encoderDirection = EncoderDirection::CW;
    modeManager->update();
    input->encoderDirection = EncoderDirection::NONE;
    TEST_ASSERT_EQUAL_STRING("Mock Mode 2", display->lastModeText);

    input->encoderClick = ClickType::SINGLE;
    modeManager->update();
    TEST_ASSERT_FALSE(mockModes[1]->updateCalled);
    modeManager->update();
    TEST_ASSERT_TRUE(mockModes[1]->updateCalled);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_mode_manager_updates_current_mode_by_default);
    RUN_TEST(test_mode_manager_shows_current_mode_after_long_click);
    RUN_TEST(test_mode_manager_shows_next_and_previous_mode_after_rotation);
    RUN_TEST(test_mode_manager_modes_lower_bound);
    RUN_TEST(test_mode_manager_modes_upper_bound);
    RUN_TEST(test_mode_manager_does_not_call_update_when_changing);
    RUN_TEST(test_mode_manager_selects_mode_with_single_click);
    RUN_TEST(test_mode_manager_can_only_switch_when_mode_allows_it);
    UNITY_END();
}