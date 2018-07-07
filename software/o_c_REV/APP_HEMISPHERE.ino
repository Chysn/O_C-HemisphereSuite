#include "hemisphere_config.h"
#include "HemisphereApplet.h"

#define DECLARE_APPLET(id, prefix) \
{ id, prefix ## _Start, prefix ## _Controller, prefix ## _View, prefix ## _Screensaver, \
  prefix ## _OnButtonPress, prefix ## _OnEncoderMove, prefix ## _ToggleHelpScreen, \
  prefix ## _OnDataRequest, prefix ## _OnDataReceive \
}

typedef struct Applet {
  int id;
  void (*Start)(int); // Initialize when selected
  void (*Controller)(int, bool);  // Interrupt Service Routine
  void (*View)(int);  // Draw main view
  void (*Screensaver)(int); // Draw screensaver view
  void (*OnButtonPress)(int); // Encoder button has been pressed
  void (*OnEncoderMove)(int, int); // Encoder has been rotated
  void (*ToggleHelpScreen)(int); // Help Screen has been requested
  uint32_t (*OnDataRequest)(int); // Get a data int from the applet
  void (*OnDataReceive)(int, uint32_t); // Send a data int to the applet
} Applet;

// The settings specify the selected applets, and 32 bits of data for each applet
enum HEMISPHERE_SETTINGS {
    HEMISPHERE_SELECTED_LEFT_ID,
    HEMISPHERE_SELECTED_RIGHT_ID,
    HEMISPHERE_LEFT_DATA,
    HEMISPHERE_RIGHT_DATA,
    HEMISPHERE_SETTING_LAST
};

////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Manager
////////////////////////////////////////////////////////////////////////////////

class HemisphereManager : public settings::SettingsBase<HemisphereManager, HEMISPHERE_SETTING_LAST> {
public:
    void Init() {
        select_mode = -1; // Not selecting
        Applet applets[] = HEMISPHERE_APPLETS;
        memcpy(&available_applets, &applets, sizeof(applets));
        forwarding = 0;
        help_hemisphere = -1;

        SetApplet(0, 0);
        SetApplet(1, 1);
    }

    void Resume() {
        for (int h = 0; h < 2; h++)
        {
            int index = GetAppletIndexByID(values_[h]);
            SetApplet(h, index);
            available_applets[index].OnDataReceive(h, values_[2 + h]);
        }
    }

    void SetApplet(int hemisphere, int index) {
        my_applets[hemisphere] = index;
        available_applets[index].Start(hemisphere);
        apply_value(hemisphere, available_applets[index].id);
    }

    void ChangeApplet(int dir) {
        if (SelectModeEnabled() and help_hemisphere == -1) {
            int index = my_applets[select_mode];
            index += dir;
            if (index >= HEMISPHERE_AVAILABLE_APPLETS) index = 0;
            if (index < 0) index = HEMISPHERE_AVAILABLE_APPLETS - 1;
            SetApplet(select_mode, index);
        }
    }

    void ToggleSelectMode(int hemisphere) {
        if (hemisphere == select_mode) {
            select_mode = -1;
        } else {
            select_mode = hemisphere;
        }
    }

    bool SelectModeEnabled() {
        return select_mode > -1;
    }

    void ExecuteControllers() {
        for (int h = 0; h < 2; h++)
        {
            int index = my_applets[h];
            available_applets[index].Controller(h, (bool)forwarding);
        }
    }

    void DrawViews() {
        if (help_hemisphere > -1) {
            int index = my_applets[help_hemisphere];
            available_applets[index].View(help_hemisphere);
        } else {
            for (int h = 0; h < 2; h++)
            {
                int index = my_applets[h];
                available_applets[index].View(h);
            }

            if (select_mode == LEFT_HEMISPHERE) graphics.drawFrame(0, 0, 64, 64);
            if (select_mode == RIGHT_HEMISPHERE) graphics.drawFrame(64, 0, 64, 64);
        }
    }

    void DrawScreensavers() {
        if (help_hemisphere > -1) DrawViews();
        else {
            for (int h = 0; h < 2; h++)
            {
                int index = my_applets[h];
                available_applets[index].Screensaver(h);
            }
        }
    }

    void DelegateButtonPush(const UI::Event &event) {
        int h = (event.control == OC::CONTROL_BUTTON_L) ? 0 : 1;
        if (select_mode == h) select_mode = -1; // Pushing a button for the selected side turns off select mode
        else {
            int index = my_applets[h];
            if (event.type == UI::EVENT_BUTTON_PRESS) {
                available_applets[index].OnButtonPress(h);
            }
        }
    }

    void DelegateEncoderMovement(const UI::Event &event) {
        int h = (event.control == OC::CONTROL_ENCODER_L) ? LEFT_HEMISPHERE : RIGHT_HEMISPHERE;
        if (select_mode == h) ChangeApplet(event.value);
        else {
            int index = my_applets[h];
            available_applets[index].OnEncoderMove(h, event.value);
        }
    }

    void ToggleForwarding() {
        forwarding = forwarding ? 0 : 1;
    }

    void ToggleHelpScreen() {
        if (help_hemisphere > -1) { // Turn off the previous help screen
            int index = my_applets[help_hemisphere];
            available_applets[index].ToggleHelpScreen(help_hemisphere);
        }

        // If the same applet is in both hemispheres, skip the second help screen
        if (my_applets[help_hemisphere] == my_applets[1 - help_hemisphere]) help_hemisphere = 1;

        help_hemisphere++; // Move to the next help screen
        if (help_hemisphere > 1) help_hemisphere = -1; // Turn them all off

        if (help_hemisphere > -1) { // Turn on the next hemisphere's screen
            int index = my_applets[help_hemisphere];
            available_applets[index].ToggleHelpScreen(help_hemisphere);
        }
    }

    void RequestAppletData() {
        for (int h = 0; h < 2; h++)
        {
            int index = my_applets[h];
            uint32_t applet_data = available_applets[index].OnDataRequest(h);
            apply_value(2 + h, applet_data);
        }
    }

private:
    Applet available_applets[HEMISPHERE_AVAILABLE_APPLETS];
    int my_applets[2]; // Indexes to available_applets
    int select_mode;
    int forwarding;
    int help_hemisphere; // Which of the hemispheres (if any) is in help mode, or -1 if none

    int GetAppletIndexByID(int id)
    {
        int index = 0;
        for (int i = 0; i < HEMISPHERE_AVAILABLE_APPLETS; i++)
        {
            if (available_applets[i].id == id) index = i;
        }
        return index;
    }
};

SETTINGS_DECLARE(HemisphereManager, HEMISPHERE_SETTING_LAST) {
    {0, 0, 255, "Applet ID L", NULL, settings::STORAGE_TYPE_U8},
    {1, 0, 255, "Applet ID R", NULL, settings::STORAGE_TYPE_U8},
    {0x00003c3c, 0, 2147483647, "Data L", NULL, settings::STORAGE_TYPE_U32},
    {0x0f2e0f0a, 0, 2147483647, "Data R", NULL, settings::STORAGE_TYPE_U32},
};


////////////////////////////////////////////////////////////////////////////////
//// O_C App Functions
////////////////////////////////////////////////////////////////////////////////

HemisphereManager manager;

// App stubs
void HEMISPHERE_init() {
    manager.Init();
}

size_t HEMISPHERE_storageSize() {
    return HemisphereManager::storageSize();
}

size_t HEMISPHERE_save(void *storage) {
    manager.RequestAppletData();
    return manager.Save(storage);
}

size_t HEMISPHERE_restore(const void *storage) {
    return manager.Restore(storage);
}

void HEMISPHERE_isr() {
    manager.ExecuteControllers();
}

void HEMISPHERE_handleAppEvent(OC::AppEvent event) {
    if (event ==  OC::APP_EVENT_RESUME) {
        manager.Resume();
    }
}

void HEMISPHERE_loop() {} // Essentially deprecated in favor of ISR

void HEMISPHERE_menu() {
    manager.DrawViews();
}

void HEMISPHERE_screensaver() {
    manager.DrawScreensavers();
}

void HEMISPHERE_handleButtonEvent(const UI::Event &event) {
    if (event.type == UI::EVENT_BUTTON_PRESS) {
        if (event.control == OC::CONTROL_BUTTON_UP || event.control == OC::CONTROL_BUTTON_DOWN) {
            int hemisphere = (event.control == OC::CONTROL_BUTTON_UP) ? LEFT_HEMISPHERE : RIGHT_HEMISPHERE;
            manager.ToggleSelectMode(hemisphere);
        } else {
            manager.DelegateButtonPush(event);
        }
    }

    if (event.type == UI::EVENT_BUTTON_LONG_PRESS) {
        if (event.control == OC::CONTROL_BUTTON_DOWN) manager.ToggleForwarding();
        if (event.control == OC::CONTROL_BUTTON_L) manager.ToggleHelpScreen();
    }
}

void HEMISPHERE_handleEncoderEvent(const UI::Event &event) {
    manager.DelegateEncoderMovement(event);
}
