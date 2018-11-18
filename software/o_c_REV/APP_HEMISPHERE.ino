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

#include "OC_DAC.h"
#include "OC_digital_inputs.h"
#include "OC_visualfx.h"
#include "OC_patterns.h"
#include "src/drivers/FreqMeasure/OC_FreqMeasure.h"
namespace menu = OC::menu;

#include "hemisphere_config.h"
#include "HemisphereApplet.h"
#include "HSicons.h"
#include "HSMIDI.h"
#include "HSClockManager.h"

#define DECLARE_APPLET(id, categories, class_name) \
{ id, categories, class_name ## _Start, class_name ## _Controller, class_name ## _View, \
  class_name ## _OnButtonPress, class_name ## _OnEncoderMove, class_name ## _ToggleHelpScreen, \
  class_name ## _OnDataRequest, class_name ## _OnDataReceive \
}

#define HEMISPHERE_DOUBLE_CLICK_TIME 8000

typedef struct Applet {
  int id;
  uint8_t categories;
  void (*Start)(bool); // Initialize when selected
  void (*Controller)(bool, bool);  // Interrupt Service Routine
  void (*View)(bool);  // Draw main view
  void (*OnButtonPress)(bool); // Encoder button has been pressed
  void (*OnEncoderMove)(bool, int); // Encoder has been rotated
  void (*ToggleHelpScreen)(bool); // Help Screen has been requested
  uint32_t (*OnDataRequest)(bool); // Get a data int from the applet
  void (*OnDataReceive)(bool, uint32_t); // Send a data int to the applet
} Applet;

// The settings specify the selected applets, and 32 bits of data for each applet
enum HEMISPHERE_SETTINGS {
    HEMISPHERE_SELECTED_LEFT_ID,
    HEMISPHERE_SELECTED_RIGHT_ID,
    HEMISPHERE_LEFT_DATA_L,
    HEMISPHERE_RIGHT_DATA_L,
    HEMISPHERE_LEFT_DATA_H,
    HEMISPHERE_RIGHT_DATA_H,
    HEMISPHERE_SETTING_LAST
};

////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Manager
////////////////////////////////////////////////////////////////////////////////

class HemisphereManager : public SystemExclusiveHandler,
    public settings::SettingsBase<HemisphereManager, HEMISPHERE_SETTING_LAST> {
public:
    void Init() {
        select_mode = -1; // Not selecting
        filter_select_mode = -1; // Not selecting filter
        midi_in_hemisphere = -1; // No MIDI In
        Applet applets[] = HEMISPHERE_APPLETS;
        memcpy(&available_applets, &applets, sizeof(applets));
        forwarding = 0;
        help_hemisphere = -1;

        SetApplet(0, get_applet_index_by_id(8)); // ADSR
        SetApplet(1, get_applet_index_by_id(26)); // Scale Duet

        // Set up category filtering
        const char* category_list[] = {"Modulator", "Sequencer",
                                       "Clocking", "Quantizer",
                                       "Utility", "MIDI",
                                       "Logic", "Other", "ALL"};
        for (int i = 0; i < 9; i++) category_name[i] = category_list[i];
        filter[0] = 8; // All
        filter[1] = 8;
    }

    void Resume() {
        for (int h = 0; h < 2; h++)
        {
            int index = get_applet_index_by_id(values_[h]);
            SetApplet(h, index);
            uint32_t data = (values_[4 + h] << 16) + values_[2 + h];
            available_applets[index].OnDataReceive(h, data);
        }
    }

    void SetApplet(int hemisphere, int index) {
        my_applet[hemisphere] = index;
        if (midi_in_hemisphere == hemisphere) midi_in_hemisphere = -1;
        if (available_applets[index].id & 0x80) midi_in_hemisphere = hemisphere;
        available_applets[index].Start(hemisphere);
        apply_value(hemisphere, available_applets[index].id);
    }

    void ChangeApplet(int dir) {
        if (SelectModeEnabled() and help_hemisphere == -1) {
            int index = get_next_applet_index(my_applet[select_mode], dir);
            SetApplet(select_mode, index);
        }
    }

    bool SelectModeEnabled() {
        return select_mode > -1;
    }

    void ExecuteControllers() {
        if (midi_in_hemisphere == -1) {
            // Only one ISR can look for MIDI messages at a time, so we need to check
            // for another MIDI In applet before looking for sysex. Note that applets
            // that use MIDI In should check for sysex themselves; see Midi In for an
            // example.
            if (usbMIDI.read() && usbMIDI.getType() == 7) {
                OnReceiveSysEx();
            }
        }

        // Turn off clock forwarding if Metronome is running
        if (clock_m->IsRunning()) forwarding = 0;

        for (int h = 0; h < 2; h++)
        {
            int index = my_applet[h];
            available_applets[index].Controller(h, (bool)forwarding);
        }
    }

    void DrawViews() {
        if (help_hemisphere > -1) {
            int index = my_applet[help_hemisphere];
            available_applets[index].View(help_hemisphere);
        } else {
            for (int h = 0; h < 2; h++)
            {
                if (filter_select_mode == h) {
                    DrawFilterSelector(h);
                } else {
                    int index = my_applet[h];
                    available_applets[index].View(h);
                }
            }

            if (select_mode == LEFT_HEMISPHERE) graphics.drawFrame(0, 0, 64, 64);
            if (select_mode == RIGHT_HEMISPHERE) graphics.drawFrame(64, 0, 64, 64);
        }
    }

    void DelegateEncoderPush(const UI::Event &event) {
        int h = (event.control == OC::CONTROL_BUTTON_L) ? LEFT_HEMISPHERE : RIGHT_HEMISPHERE;
        if (select_mode == h) {
            select_mode = -1; // Pushing a button for the selected side turns off select mode
            filter_select_mode = -1; // and filter select mode
        } else {
            int index = my_applet[h];
            if (event.type == UI::EVENT_BUTTON_PRESS) {
                available_applets[index].OnButtonPress(h);
            }
        }
    }

    void DelegateSelectButtonPush(int hemisphere) {
        if (filter_select_mode == hemisphere) {
            // Exit filter select mode if the corresponding button is pressed
            select_mode = filter_select_mode;
            filter_select_mode = -1;
        } else if (filter_select_mode > -1) {
            // Move to the other hemisphere if the opposite button is pressed
            select_mode = hemisphere;
            filter_select_mode = hemisphere;
        } else {
            if (OC::CORE::ticks - click_tick < HEMISPHERE_DOUBLE_CLICK_TIME && hemisphere == first_click) {
                // This is a double-click, so activate corresponding help screen, leave
                // Select Mode, and reset the double-click timer
                SetHelpScreen(hemisphere);
                select_mode = -1;
                click_tick = 0;
            } else {
                // This is a single click. If a help screen is already selected, and the
                // button is for the opposite one, go to the other help screen
                if (help_hemisphere > -1) {
                    if (help_hemisphere != hemisphere) SetHelpScreen(hemisphere);
                    else SetHelpScreen(-1); // Leave help screen if corresponding button is clicked
                } else {
                    // If Select Mode was on, and the same button was pushed, leave Select Mode
                    if (hemisphere == select_mode) select_mode = -1;
                    else select_mode = hemisphere; // Otherwise, set select mode
                    click_tick = OC::CORE::ticks;
                }
                first_click = hemisphere;
            }
        }
    }

    void DelegateEncoderMovement(const UI::Event &event) {
        int h = (event.control == OC::CONTROL_ENCODER_L) ? LEFT_HEMISPHERE : RIGHT_HEMISPHERE;
        if (filter_select_mode == h) {
            filter[filter_select_mode] += event.value;
            if (filter[filter_select_mode] > 8) filter[filter_select_mode] = 0;
            if (filter[filter_select_mode] < 0) filter[filter_select_mode] = 8;
        } else {
            if (select_mode == h) ChangeApplet(event.value);
            else {
                int index = my_applet[h];
                available_applets[index].OnEncoderMove(h, event.value);
            }
        }
    }

    void ToggleForwarding() {
        if (clock_m->IsRunning()) clock_m->Pause();
        else if (clock_m->IsPaused()) clock_m->Start();
        else forwarding = forwarding ? 0 : 1;
    }

    void SetHelpScreen(int hemisphere) {
        if (help_hemisphere > -1) { // Turn off the previous help screen
            int index = my_applet[help_hemisphere];
            available_applets[index].ToggleHelpScreen(help_hemisphere);
        }

        if (hemisphere > -1) { // Turn on the next hemisphere's screen
            int index = my_applet[hemisphere];
            available_applets[index].ToggleHelpScreen(hemisphere);
        }

        help_hemisphere = hemisphere;
    }

    void RequestAppletData() {
        for (int h = 0; h < 2; h++)
        {
            int index = my_applet[h];
            uint32_t data = available_applets[index].OnDataRequest(h);
            apply_value(2 + h, data & 0xffff);
            apply_value(4 + h, (data >> 16) & 0xffff);
        }
    }

    void EnableFilterSelect() {
        if (help_hemisphere < 0) {
            if (select_mode < 0) select_mode = 0; // Default to the left hemisphere
            filter_select_mode = select_mode;
        }
    }

    void OnSendSysEx() {
        // Set the values_ array prior to packing it
        RequestAppletData();

        // Describe the data structure for the audience
        uint8_t V[10];
        V[0] = (uint8_t)values_[HEMISPHERE_SELECTED_LEFT_ID];
        V[1] = (uint8_t)values_[HEMISPHERE_SELECTED_RIGHT_ID];
        V[2] = (uint8_t)(values_[HEMISPHERE_LEFT_DATA_L] & 0xff);
        V[3] = (uint8_t)((values_[HEMISPHERE_LEFT_DATA_L] >> 8) & 0xff);
        V[4] = (uint8_t)(values_[HEMISPHERE_RIGHT_DATA_L] & 0xff);
        V[5] = (uint8_t)((values_[HEMISPHERE_RIGHT_DATA_L] >> 8) & 0xff);
        V[6] = (uint8_t)(values_[HEMISPHERE_LEFT_DATA_H] & 0xff);
        V[7] = (uint8_t)((values_[HEMISPHERE_LEFT_DATA_H] >> 8) & 0xff);
        V[8] = (uint8_t)(values_[HEMISPHERE_RIGHT_DATA_H] & 0xff);
        V[9] = (uint8_t)((values_[HEMISPHERE_RIGHT_DATA_H] >> 8) & 0xff);

        // Pack it up, ship it out
        UnpackedData unpacked;
        unpacked.set_data(10, V);
        PackedData packed = unpacked.pack();
        SendSysEx(packed, 'H');
    }

    void OnReceiveSysEx() {
        uint8_t V[10];
        if (ExtractSysExData(V, 'H')) {
            values_[HEMISPHERE_SELECTED_LEFT_ID] = V[0];
            values_[HEMISPHERE_SELECTED_RIGHT_ID] = V[1];
            values_[HEMISPHERE_LEFT_DATA_L] = ((uint16_t)V[3] << 8) + V[2];
            values_[HEMISPHERE_RIGHT_DATA_L] = ((uint16_t)V[5] << 8) + V[4];
            values_[HEMISPHERE_LEFT_DATA_H] = ((uint16_t)V[7] << 8) + V[6];
            values_[HEMISPHERE_RIGHT_DATA_H] = ((uint16_t)V[9] << 8) + V[8];
            Resume();
        }
    }

private:
    Applet available_applets[HEMISPHERE_AVAILABLE_APPLETS];
    int my_applet[2]; // Indexes to available_applets
    const char* category_name[9];
    int16_t filter[2];
    int filter_select_mode;
    int select_mode;
    int forwarding;
    int help_hemisphere; // Which of the hemispheres (if any) is in help mode, or -1 if none
    int midi_in_hemisphere; // Which of the hemispheres (if any) is using MIDI In
    uint32_t click_tick; // Measure time between clicks for double-click
    int first_click; // The first button pushed of a double-click set, to see if the same one is pressed
    ClockManager *clock_m = clock_m->get();

    void DrawFilterSelector(int h) {
        int offset = h * 64;

        // Header
        graphics.setPrintPos(1 + offset, 2);
        graphics.print("Categories");
        graphics.drawLine(0 + offset, 10, 62 + offset, 10);
        graphics.drawLine(0 + offset, 12, 62 + offset, 12);

        // Draw selector list
        int o = filter[h] - 2;
        if (o < 0) o += 9;
        for (int c = 0; c < 6; c++) // Draw five categories on the screen
        {
            int i = c + o;
            if (i > 8) i -= 9; // Roll back around to make infinitely scroll-able
            graphics.setPrintPos(8 + offset, 15 + (8 * c));
            graphics.print(category_name[i]);
        }

        graphics.drawBitmap8(1 + offset, 30, 8, CHECK_ICON);
    }

    int get_applet_index_by_id(int id) {
        int index = 0;
        for (int i = 0; i < HEMISPHERE_AVAILABLE_APPLETS; i++)
        {
            if (available_applets[i].id == id) index = i;
        }
        return index;
    }

    int get_next_applet_index(int index, int dir) {
        index += dir;
        if (index >= HEMISPHERE_AVAILABLE_APPLETS) index = 0;
        if (index < 0) index = HEMISPHERE_AVAILABLE_APPLETS - 1;

        // If an applet uses MIDI In, it can only be selected in one
        // hemisphere, and is designated by bit 7 set in its id.
        if (available_applets[index].id & 0x80) {
            if (midi_in_hemisphere == (1 - select_mode)) {
                return get_next_applet_index(index, dir);
            }
        }

        // If a filter is on, show only selected applets
        if (filter[select_mode] < 8) {
            uint8_t f = 0x01 << filter[select_mode];
            if (!(available_applets[index].categories & f)) {
                return get_next_applet_index(index, dir);
            }
        }

        return index;
    }
};

SETTINGS_DECLARE(HemisphereManager, HEMISPHERE_SETTING_LAST) {
    {0, 0, 255, "Applet ID L", NULL, settings::STORAGE_TYPE_U8},
    {0, 0, 255, "Applet ID R", NULL, settings::STORAGE_TYPE_U8},
    {0, 0, 65535, "Data L low", NULL, settings::STORAGE_TYPE_U16},
    {0, 0, 65535, "Data R low", NULL, settings::STORAGE_TYPE_U16},
    {0, 0, 65535, "Data L high", NULL, settings::STORAGE_TYPE_U16},
    {0, 0, 65535, "Data R high", NULL, settings::STORAGE_TYPE_U16},
};

HemisphereManager manager;

void ReceiveManagerSysEx() {
    manager.OnReceiveSysEx();
}

////////////////////////////////////////////////////////////////////////////////
//// O_C App Functions
////////////////////////////////////////////////////////////////////////////////

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
    size_t s = manager.Restore(storage);
    manager.Resume();
    return s;
}

void FASTRUN HEMISPHERE_isr() {
    manager.ExecuteControllers();
}

void HEMISPHERE_handleAppEvent(OC::AppEvent event) {
    if (event == OC::APP_EVENT_SUSPEND) {
        manager.OnSendSysEx();
    }
}

void HEMISPHERE_loop() {} // Essentially deprecated in favor of ISR

void HEMISPHERE_menu() {
    manager.DrawViews();
}

void HEMISPHERE_screensaver() {} // Deprecated in favor of screen blanking

void HEMISPHERE_handleButtonEvent(const UI::Event &event) {
    if (event.type == UI::EVENT_BUTTON_PRESS) {
        if (event.control == OC::CONTROL_BUTTON_UP || event.control == OC::CONTROL_BUTTON_DOWN) {
            int hemisphere = (event.control == OC::CONTROL_BUTTON_UP) ? LEFT_HEMISPHERE : RIGHT_HEMISPHERE;
            manager.DelegateSelectButtonPush(hemisphere);
        } else {
            manager.DelegateEncoderPush(event);
        }
    }

    if (event.type == UI::EVENT_BUTTON_LONG_PRESS) {
        if (event.control == OC::CONTROL_BUTTON_DOWN) manager.EnableFilterSelect();
        if (event.control == OC::CONTROL_BUTTON_L) manager.ToggleForwarding();
    }
}

void HEMISPHERE_handleEncoderEvent(const UI::Event &event) {
    manager.DelegateEncoderMovement(event);
}


