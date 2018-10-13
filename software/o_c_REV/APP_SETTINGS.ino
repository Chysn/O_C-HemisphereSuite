// Copyright (c) 2018, Jason Justian
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "HSApplication.h"

class Settings : public HSApplication {
public:
	void Start() {
	}
	
	void Resume() {
	}

    void Controller() {
    }

    void View() {
        gfxHeader("Setup / About");
        gfxPrint(0, 15, "Hemisphere Suite");
        gfxPrint(0, 25, OC_VERSION);
        gfxPrint(0, 55, "[CALIBRATE]   [RESET]");
    }

    /////////////////////////////////////////////////////////////////
    // Control handlers
    /////////////////////////////////////////////////////////////////
    void OnLeftButtonPress() {
        OC::ui.Calibrate();
    }

    void OnLeftButtonLongPress() {
    }

    void OnRightButtonPress() {
        OC::apps::Init(1);
    }

    void OnUpButtonPress() {
    }

    void OnDownButtonPress() {
    }

    void OnDownButtonLongPress() {
    }

    void OnLeftEncoderMove(int direction) {
    }

    void OnRightEncoderMove(int direction) {
    }

private:
    int8_t cursor;

    /* Example private screen-drawing method
    void DrawInterface() {
    }
    */
};

Settings Settings_instance;

// App stubs
void Settings_init() {
    Settings_instance.BaseStart();
}

// Not using O_C Storage
size_t Settings_storageSize() {return 0;}
size_t Settings_save(void *storage) {return 0;}
size_t Settings_restore(const void *storage) {return 0;}

void Settings_isr() {
	return Settings_instance.BaseController();
}

void Settings_handleAppEvent(OC::AppEvent event) {
    if (event ==  OC::APP_EVENT_RESUME) {
        Settings_instance.Resume();
    }
}

void Settings_loop() {} // Deprecated

void Settings_menu() {
    Settings_instance.BaseView();
}

void Settings_screensaver() {} // Deprecated

void Settings_handleButtonEvent(const UI::Event &event) {
    // For left encoder, handle press and long press
    if (event.control == OC::CONTROL_BUTTON_L) {
        if (event.type == UI::EVENT_BUTTON_LONG_PRESS) Settings_instance.OnLeftButtonLongPress();
        else Settings_instance.OnLeftButtonPress();
    }

    // For right encoder, only handle press (long press is reserved)
    if (event.control == OC::CONTROL_BUTTON_R && event.type == UI::EVENT_BUTTON_PRESS) Settings_instance.OnRightButtonPress();

    // For up button, handle only press (long press is reserved)
    if (event.control == OC::CONTROL_BUTTON_UP) Settings_instance.OnUpButtonPress();

    // For down button, handle press and long press
    if (event.control == OC::CONTROL_BUTTON_DOWN) {
        if (event.type == UI::EVENT_BUTTON_PRESS) Settings_instance.OnDownButtonPress();
        if (event.type == UI::EVENT_BUTTON_LONG_PRESS) Settings_instance.OnDownButtonLongPress();
    }
}

void Settings_handleEncoderEvent(const UI::Event &event) {
    // Left encoder turned
    if (event.control == OC::CONTROL_ENCODER_L) Settings_instance.OnLeftEncoderMove(event.value);

    // Right encoder turned
    if (event.control == OC::CONTROL_ENCODER_R) Settings_instance.OnRightEncoderMove(event.value);
}
