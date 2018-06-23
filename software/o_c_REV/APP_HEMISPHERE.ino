#include "hemisphere_config.h"

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

const int LEFT_HEMISPHERE = 0;
const int RIGHT_HEMISPHERE = 1;
const int HEMISPHERE_MAX_CV = 7800;

////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Base Class
////////////////////////////////////////////////////////////////////////////////

class HemisphereApplet {
public:
    void IO() {
        for (int i = 0; i < 2; i++)
        {
            ADC_CHANNEL channel = (ADC_CHANNEL)(i + io_offset);
            inputs[i] = OC::ADC::raw_pitch_value(channel);
        }
    }

    /* Assign the child class instance to a left or right hemisphere */
    void SetHemisphere(int h) {
        hemisphere = h;
        gfx_offset = h * 65;
        io_offset = h * 2;
    }

    /* Proportion CV values for display purposes */
    int ProportionCV(int value, int max) {
        int divisions = HEMISPHERE_MAX_CV / max; // Divide the CV into little pieces
        int proportion = value / divisions;
        if (proportion > max) proportion = max;
        return proportion;
    }

    /* Offset Graphics Methods */
    void gfxPrint(int x, int y, const char *str) {
        graphics.setPrintPos(x + gfx_offset, y);
        graphics.print(str);
    }

    void gfxPrint(int x, int y, int num) {
        graphics.setPrintPos(x + gfx_offset, y);
        graphics.print(num);
    }

    void gfxPixel(int x, int y) {
        graphics.setPixel(x + gfx_offset, y);
    }

    void gfxFrame(int x, int y, int w, int h) {
        graphics.drawFrame(x + gfx_offset, y, w, h);
    }

    void gfxRect(int x, int y, int w, int h) {
        graphics.drawRect(x + gfx_offset, y, w, h);
    }

    void gfxInvert(int x, int y, int w, int h) {
        graphics.invertRect(x + gfx_offset, y, w, h);
    }

    void gfxLine(int x, int y, int x2, int y2) {
        graphics.drawLine(x + gfx_offset, y, x2 + gfx_offset, y);
    }

    /* Hemisphere-specific graphics methods */
    void gfxOutputBar(int ch, bool screensaver) {
        int width = ProportionCV(outputs[ch], 60);
        if (width < 0) {width = 0;}
        int height = screensaver ? 2 : 12;
        int x = (hemisphere == 0) ? 64 - width : 0;
        gfxRect(x, 35 + (ch * 15), width, height);
    }

    void gfxInputBar(int ch, bool screensaver) {
        int width = ProportionCV(inputs[ch], 63);
        if (width < 0) {width = 0;}
        int height = screensaver ? 1 : 6;
        int x = (hemisphere == 0) ? 63 - width : 0;
        gfxFrame(x, 15 + (ch * 10), width, height);
    }

    void gfxButterfly(bool screensaver) {
        for (int ch = 0; ch < 2; ch++)
        {
            gfxOutputBar(ch, screensaver);
            gfxInputBar(ch, screensaver);
        }
    }

    void gfxHeader(const char *str) {
        gfxPrint(2, 2, str);
        gfxLine(0, 10, 62, 10);
        gfxLine(0, 12, 62, 12);
    }

    /* Offset I/O Methods */
    int In(int i) {
        return inputs[i];
    }

    void Out(int o, int value, int octave) {
        DAC_CHANNEL channel = (DAC_CHANNEL)(o + io_offset);
        OC::DAC::set_pitch(channel, value, octave);
        outputs[o] = value;
    }

    void Out(int o, int value) {
        Out(o, value, 0);
    }

    bool Clock(int i) {
        bool clocked = 0;
        if (hemisphere == 0) {
            if (i == 0) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_1>();
            if (i == 1) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_2>();
        }
        if (hemisphere == 1) {
            if (i == 0) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_3>();
            if (i == 1) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_4>();
        }
        return clocked;
    }

    int Gate(int i) {
        bool gated = 0;
        if (hemisphere == 0) {
            if (i == 0) gated = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_1>();
            if (i == 1) gated = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_2>();
        }
        if (hemisphere == 1) {
            if (i == 0) gated = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_3>();
            if (i == 1) gated = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_4>();
        }
        return gated;
    }

private:
    int hemisphere; // Which hemisphere (0, 1) this applet uses
    int gfx_offset; // Graphics offset, based on the side
    int io_offset; // Input/Output offset, based on the side
    int inputs[2];
    int outputs[2];
};

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
