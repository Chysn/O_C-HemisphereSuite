#include "OC_DAC.h"
#include "OC_digital_inputs.h"
#include "OC_visualfx.h"
#include "OC_patterns.h"
namespace menu = OC::menu;

#include "hemisphere_config.h"
#include "HemisphereApplet.h"

#define DECLARE_APPLET(id, categories, class_name) \
{ id, categories, class_name ## _Start, class_name ## _Controller, class_name ## _View, class_name ## _Screensaver, \
  class_name ## _OnButtonPress, class_name ## _OnEncoderMove, class_name ## _ToggleHelpScreen, \
  class_name ## _OnDataRequest, class_name ## _OnDataReceive \
}

#define HEMISPHERE_DOUBLE_CLICK_TIME 8000

typedef struct Applet {
  int id;
  uint8_t categories;
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
    HEMISPHERE_LEFT_DATA_L,
    HEMISPHERE_RIGHT_DATA_L,
    HEMISPHERE_LEFT_DATA_H,
    HEMISPHERE_RIGHT_DATA_H,
    HEMISPHERE_SETTING_LAST
};

////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Manager
////////////////////////////////////////////////////////////////////////////////

class HemisphereManager : public settings::SettingsBase<HemisphereManager, HEMISPHERE_SETTING_LAST> {
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
                                       "Audio", "Other", "ALL"};
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
                ProcessSystemExclusive();
            }
        }

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

    void DrawScreensavers() {
        if (help_hemisphere > -1) DrawViews();
        else {
            for (int h = 0; h < 2; h++)
            {
                int index = my_applet[h];
                available_applets[index].Screensaver(h);
            }
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
        forwarding = forwarding ? 0 : 1;
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

    void SendSystemExclusive() {
        RequestAppletData();
        uint8_t packet[29];
        int ix = 0;
        packet[ix++] = 0xf0; // Start of SysEx
        packet[ix++] = 0x7d; // Non-Commercial Manufacturer
        packet[ix++] = 0x62; // Beige Maze
        packet[ix++] = 0x48; // Hemisphere
        for (int i = 0; i <  6; i++)
        {
            uint16_t v = values_[i];
            for (int n = 0; n < 4; n++)
            {
                packet[ix++] = (v >> (4 * n)) & 0x0f;
            }
        }
        packet[ix++] = 0xf7; // End of SysEx
        usbMIDI.sendSysEx(29, packet);
        usbMIDI.send_now();
    }

    void ProcessSystemExclusive() {
        const uint8_t *sysex = usbMIDI.getSysExArray();
        if (sysex[1] == 0x7d && sysex[2] == 0x62 && sysex[3] == 0x48) {
            int ix = 4;
            for (int i = 0; i < 6; i++)
            {
                uint16_t v = 0;
                for (int n = 0; n < 4; n++)
                {
                    v += sysex[ix++] << (4 * n);
                }
                values_[i] = v;
            }
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

        const uint8_t check[8] = {0x00,0xf0,0x40,0x20,0x10,0x08,0x04,0x00};
        graphics.drawBitmap8(1 + offset, 29, 8, check);
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
    size_t s = manager.Restore(storage);
    manager.Resume();
    return s;
}

void HEMISPHERE_isr() {
    manager.ExecuteControllers();
}

void HEMISPHERE_handleAppEvent(OC::AppEvent event) {
    if (event == OC::APP_EVENT_SUSPEND) {
        manager.SendSystemExclusive();
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
