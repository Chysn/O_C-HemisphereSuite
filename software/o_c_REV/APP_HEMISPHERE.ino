#include "hemisphere_config.h"
#include "HemisphereApplet.h"

#define DECLARE_APPLET(id, prefix) \
{ id, prefix ## _Start, prefix ## _Controller, prefix ## _View, prefix ## _Screensaver, \
  prefix ## _OnButtonPress, prefix ## _OnEncoderMove, prefix ## _ToggleHelpScreen \
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
} Applet;

////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Manager
////////////////////////////////////////////////////////////////////////////////

class HemisphereManager {
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
    }

    void SetApplet(int hemisphere, int applet_index) {
        my_applets[hemisphere] = applet_index;
        available_applets[applet_index].Start(hemisphere);
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
            int idx = my_applets[h];
            available_applets[idx].Controller(h, (bool)forwarding);
        }
    }

    void DrawViews() {
        if (help_hemisphere > -1) {
            int idx = my_applets[help_hemisphere];
            available_applets[idx].View(help_hemisphere);
        } else {
            for (int h = 0; h < 2; h++)
            {
                int idx = my_applets[h];
                available_applets[idx].View(h);
            }

            if (select_mode == LEFT_HEMISPHERE) {
                graphics.drawFrame(0, 0, 64, 64);
            }

            if (select_mode == RIGHT_HEMISPHERE) {
                graphics.drawFrame(64, 0, 64, 64);
            }
        }
    }

    void DrawScreensavers() {
        for (int h = 0; h < 2; h++)
        {
            int idx = my_applets[h];
            available_applets[idx].Screensaver(h);
        }
    }

    void DelegateButtonPush(const UI::Event &event) {
        int h = (event.control == OC::CONTROL_BUTTON_L) ? 0 : 1;
        int idx = my_applets[h];
        if (event.type == UI::EVENT_BUTTON_PRESS) {
            available_applets[idx].OnButtonPress(h);
        }
    }

    void DelegateEncoderMovement(const UI::Event &event) {
        int h = (event.control == OC::CONTROL_ENCODER_L) ? LEFT_HEMISPHERE : RIGHT_HEMISPHERE;
        int idx = my_applets[h];
        available_applets[idx].OnEncoderMove(h, event.value > 0 ? 1 : -1);
    }

    void ToggleForwarding() {
        forwarding = forwarding ? 0 : 1;
    }

    void ToggleHelpScreen() {
        if (help_hemisphere > -1) { // Turn off the previous help screen
            int idx = my_applets[help_hemisphere];
            available_applets[idx].ToggleHelpScreen(help_hemisphere);
        }

        help_hemisphere++; // Move to the next help screen
        if (help_hemisphere > 1) help_hemisphere = -1; // Turn them all off

        if (help_hemisphere > -1) { // Turn on the next hemisphere's screen
            int idx = my_applets[help_hemisphere];
            available_applets[idx].ToggleHelpScreen(help_hemisphere);
        }
    }

private:
    Applet available_applets[HEMISPHERE_AVAILABLE_APPLETS];
    int my_applets[2]; // Indexes to available_applets
    int select_mode;
    int forwarding;
    int help_hemisphere; // Which of the hemispheres (if any) is in help mode, or -1 if none
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
    return 0;
}

size_t HEMISPHERE_save(void *storage) {
    return 0;
}

size_t HEMISPHERE_restore(const void *storage) {
    return 0;
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
    if (manager.SelectModeEnabled()) {
        manager.ChangeApplet(event.value > 0 ? 1 : -1);
    } else {
        manager.DelegateEncoderMovement(event);
    }
}
