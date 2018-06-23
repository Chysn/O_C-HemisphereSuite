#include "hemisphere_config.h"
#include "HemisphereApplet.h"

#define DECLARE_APPLET(id, prefix) \
{ id, prefix ## _Start, prefix ## _Controller, \
  prefix ## _View, prefix ## _Screensaver, \
  prefix ## _OnButtonPress, prefix ## _OnButtonLongPress, \
  prefix ## _OnEncoderMove }

typedef struct Applet {
  int id;
  void (*Start)(int); // Initialize when selected
  void (*Controller)(int);  // Interrupt Service Routine
  void (*View)(int);  // Draw main view
  void (*Screensaver)(int); // Draw screensaver view
  void (*OnButtonPress)(int);
  void (*OnButtonLongPress)(int);
  void (*OnEncoderMove)(int, int);
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
        if (SelectModeEnabled()) {
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
        for (int a = 0; a < 2; a++)
        {
            int idx = my_applets[a];
            available_applets[idx].Controller(a);
        }
    }

    void DrawViews() {

        for (int a = 0; a < 2; a++)
        {
            int idx = my_applets[a];
            available_applets[idx].View(a);
        }

        if (select_mode == LEFT_HEMISPHERE) {
            graphics.drawFrame(0, 0, 64, 64);
        }

        if (select_mode == RIGHT_HEMISPHERE) {
            graphics.drawFrame(64, 0, 64, 64);
        }
    }

    void DrawScreensavers() {
        for (int a = 0; a < 2; a++)
        {
            int idx = my_applets[a];
            available_applets[idx].Screensaver(a);
        }
    }

    void DelegateButtonPush(const UI::Event &event) {
        int a = (event.control == OC::CONTROL_BUTTON_L) ? 0 : 1;
        int idx = my_applets[a];
            if (event.type == UI::EVENT_BUTTON_PRESS) {
                available_applets[idx].OnButtonPress(a);
            }
            if (event.type == UI::EVENT_BUTTON_LONG_PRESS) {
                available_applets[idx].OnButtonLongPress(a);
            }
    }

    void DelegateEncoderMovement(const UI::Event &event) {
        int a = (event.control == OC::CONTROL_ENCODER_L) ? 0 : 1;
        int idx = my_applets[a];
        available_applets[idx].OnEncoderMove(a, event.value > 0 ? 1 : -1);
    }

private:
    Applet available_applets[HEMISPHERE_AVAILABLE_APPLETS];
    int my_applets[2]; // Indexes to available_applets
    int select_mode;
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

void HEMISPHERE_loop() {} // Deprecated

void HEMISPHERE_menu() {
    manager.DrawViews();
}

void HEMISPHERE_screensaver() {
    manager.DrawScreensavers();
}

void HEMISPHERE_handleButtonEvent(const UI::Event &event) {
    if (UI::EVENT_BUTTON_PRESS == event.type) {
        if (event.control == OC::CONTROL_BUTTON_UP || event.control == OC::CONTROL_BUTTON_DOWN) {
            int hemisphere = (event.control == OC::CONTROL_BUTTON_UP) ? LEFT_HEMISPHERE : RIGHT_HEMISPHERE;
            manager.ToggleSelectMode(hemisphere);
        } else {
            // It's one of the encoder buttons, so delegate via manager
            manager.DelegateButtonPush(event);
        }
    }
}

void HEMISPHERE_handleEncoderEvent(const UI::Event &event) {
    if (manager.SelectModeEnabled()) {
        manager.ChangeApplet(event.value > 0 ? 1 : -1);
    } else {
        manager.DelegateEncoderMovement(event);
    }
}
