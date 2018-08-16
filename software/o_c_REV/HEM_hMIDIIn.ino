// See https://www.pjrc.com/teensy/td_midi.html

#define HEM_MIDI_NOTE_ON 1
#define HEM_MIDI_NOTE_OFF 0
#define HEM_MIDI_CC 3
#define HEM_MIDI_AFTERTOUCH 5
#define HEM_MIDI_PITCHBEND 6
#define HEM_MIDI_SYSEX 7

// The functions available for each output
#define HEM_MIDI_NOTE_OUT 0
#define HEM_MIDI_TRIG_OUT 1
#define HEM_MIDI_GATE_OUT 2
#define HEM_MIDI_VEL_OUT 3
#define HEM_MIDI_CC_OUT 4
#define HEM_MIDI_AT_OUT 5
#define HEM_MIDI_PB_OUT 6

struct MIDILogEntry {
    int message;
    int data1;
    int data2;
};

class hMIDIIn : public HemisphereApplet {
public:

    const char* applet_name() {
        return "MIDIIn";
    }

    void Start() {
        first_note = -1;
        channel = 0; // Default channel 1

        const char * fn_name_list[] = {"Note#", "Trig", "Gate", "Veloc", "Mod", "Aft", "Bend"};
        for (int i = 0; i < 7; i++) fn_name[i] = fn_name_list[i];

        ForEachChannel(ch)
        {
            function[ch] = 0 + (ch * 2);
            Out(ch, 0);
        }

        log_index = 0;
    }

    void Controller() {
        if (usbMIDI.read()) {
            int message = usbMIDI.getType();
            if (message == HEM_MIDI_SYSEX) {
                ReceiveManagerSysEx();
            }

            if (usbMIDI.getChannel() == (channel + 1)) {
                last_tick = OC::CORE::ticks;
                int data1 = usbMIDI.getData1();
                int data2 = usbMIDI.getData2();
                bool log_this = false;

                if (message == HEM_MIDI_NOTE_ON) { // Note on
                    if (first_note == -1) first_note = data1;

                    // Should this message go out on any channel?
                    ForEachChannel(ch)
                    {
                        if (function[ch] == HEM_MIDI_NOTE_OUT)
                            Out(ch, MIDINoteNumber_CV(data1));

                        if (function[ch] == HEM_MIDI_TRIG_OUT)
                            ClockOut(ch);

                        if (function[ch] == HEM_MIDI_GATE_OUT)
                            GateOut(ch, 1);

                        if (function[ch] == HEM_MIDI_VEL_OUT)
                            Out(ch, Proportion(data2, 127, HEMISPHERE_MAX_CV));
                    }

                    log_this = 1; // Log all MIDI notes. Other stuff is conditional.
                }

                if (message == HEM_MIDI_NOTE_OFF) { // Note off
                    if (data1 == first_note) first_note = -1;

                    // Should this message go out on any channel?
                    ForEachChannel(ch)
                    {
                        if (function[ch] == HEM_MIDI_GATE_OUT) {
                            GateOut(ch, 0);
                            log_this = 1;
                        }
                    }
                }

                if (message == HEM_MIDI_CC) { // Modulation wheel
                    ForEachChannel(ch)
                    {
                        if (function[ch] == HEM_MIDI_CC_OUT && data1 == 1) {
                            int data = data2 << 8;
                            Out(ch, Proportion(data, 0x7fff, HEMISPHERE_MAX_CV));
                            log_this = 1;
                        }
                    }

                }

                if (message == HEM_MIDI_AFTERTOUCH) { // Aftertouch
                    ForEachChannel(ch)
                    {
                        if (function[ch] == HEM_MIDI_AT_OUT) {
                            int data = data2 << 8;
                            Out(ch, Proportion(data, 0x7fff, HEMISPHERE_MAX_CV));
                            log_this = 1;
                        }
                    }

                    UpdateLog(message, data1, data2);
                }

                if (message == HEM_MIDI_PITCHBEND) { // Pitch Bend
                    ForEachChannel(ch)
                    {
                        if (function[ch] == HEM_MIDI_PB_OUT) {
                            int data = (data2 << 7) + data1 - 8192;
                            Out(ch, Proportion(data, 0x7fff, HEMISPHERE_3V_CV));
                            log_this = 1;
                        }
                    }
                }

                if (log_this) UpdateLog(message, data1, data2);
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawMonitor();
        DrawSelector();
    }

    void ScreensaverView() {
        DrawMonitor();
        DrawLog();
    }

    void OnButtonPress() {
        if (++cursor > 2) cursor = 0;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) channel = constrain(channel += direction, 0, 15);
        else {
            int ch = cursor - 1;
            function[ch] = constrain(function[ch] += direction, 0, 6);
        }
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,8}, channel);
        Pack(data, PackLocation {8,3}, function[0]);
        Pack(data, PackLocation {11,3}, function[1]);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        channel = Unpack(data, PackLocation {0,8});
        function[0] = Unpack(data, PackLocation {8,3});
        function[1] = Unpack(data, PackLocation {11,3});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "";
        help[HEMISPHERE_HELP_CVS]      = "";
        help[HEMISPHERE_HELP_OUTS]     = "Assignable";
        help[HEMISPHERE_HELP_ENCODER]  = "MIDI Ch/Assign Out";
        //                               "------------------" <-- Size Guide
    }
    
private:
    // Settings
    int channel; // MIDI channel number
    int function[2]; // Function for each channel

    // Housekeeping
    int cursor; // 0=MIDI channel, 1=A/C function, 2=B/D function
    int last_tick; // Tick of last received message
    int first_note; // First note received, for awaiting Note Off
    const char* fn_name[7];
    
    // Logging
    MIDILogEntry log[7];
    int log_index;

    void UpdateLog(int message, int data1, int data2) {
        log[log_index++] = {message, data1, data2};
        if (log_index == 7) {
            for (int i = 0; i < 6; i++)
            {
                memcpy(&log[i], &log[i+1], sizeof(log[i+1]));
            }
            log_index--;
        }
    }

    void DrawMonitor() {
        if (OC::CORE::ticks - last_tick < 4000) {
            gfxBitmap(46, 1, 8, MIDI_ICON);
        }
    }

    void DrawSelector() {
        // MIDI Channel
        gfxPrint(1, 15, "Ch:");
        gfxPrint(24, 15, channel + 1);

        // Output 1 function
        if (hemisphere == 0) gfxPrint(1, 25, "A :");
        else gfxPrint(1, 25, "C :");
        gfxPrint(24, 25, fn_name[function[0]]);

        // Output 2 function
        if (hemisphere == 0) gfxPrint(1, 35, "B :");
        else gfxPrint(1, 35, "D :");
        gfxPrint(24, 35, fn_name[function[1]]);

        // Cursor
        gfxCursor(24, 23 + (cursor * 10), 39);

        // Last log entry
        if (log_index > 0) {
            gfxDottedLine(1, 55, 62, 55, 2);
            log_entry(56, log_index - 1);
        }
    }

    void DrawLog() {
        if (log_index) {
            for (int i = 0; i < log_index; i++)
            {
                log_entry(15 + (i * 8), i);
            }
        }
    }

    void log_entry(int y, int index) {
        if (log[index].message == HEM_MIDI_NOTE_ON) {
            gfxBitmap(1, y, 8, NOTE_ICON);
            gfxPrint(10, y, midi_note_numbers[log[index].data1]);
            gfxPrint(40, y, log[index].data2);
        }

        if (log[index].message == HEM_MIDI_NOTE_OFF) {
            gfxPrint(1, y, "-");
            gfxPrint(10, y, midi_note_numbers[log[index].data1]);
        }

        if (log[index].message == HEM_MIDI_CC) {
            gfxBitmap(1, y, 8, MOD_ICON);
            gfxPrint(10, y, log[index].data2);
        }

        if (log[index].message == HEM_MIDI_AFTERTOUCH) {
            gfxBitmap(1, y, 8, AFTERTOUCH_ICON);
            gfxPrint(10, y, log[index].data2);
        }

        if (log[index].message == HEM_MIDI_PITCHBEND) {
            int data = (log[index].data2 << 7) + log[index].data1 - 8192;
            gfxBitmap(1, y, 8, BEND_ICON);
            gfxPrint(10, y, data);
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to hMIDIIn,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
hMIDIIn hMIDIIn_instance[2];

void hMIDIIn_Start(int hemisphere) {
    hMIDIIn_instance[hemisphere].BaseStart(hemisphere);
}

void hMIDIIn_Controller(int hemisphere, bool forwarding) {
    hMIDIIn_instance[hemisphere].BaseController(forwarding);
}

void hMIDIIn_View(int hemisphere) {
    hMIDIIn_instance[hemisphere].BaseView();
}

void hMIDIIn_Screensaver(int hemisphere) {
    hMIDIIn_instance[hemisphere].BaseScreensaverView();
}

void hMIDIIn_OnButtonPress(int hemisphere) {
    hMIDIIn_instance[hemisphere].OnButtonPress();
}

void hMIDIIn_OnEncoderMove(int hemisphere, int direction) {
    hMIDIIn_instance[hemisphere].OnEncoderMove(direction);
}

void hMIDIIn_ToggleHelpScreen(int hemisphere) {
    hMIDIIn_instance[hemisphere].HelpScreen();
}

uint32_t hMIDIIn_OnDataRequest(int hemisphere) {
    return hMIDIIn_instance[hemisphere].OnDataRequest();
}

void hMIDIIn_OnDataReceive(int hemisphere, uint32_t data) {
    hMIDIIn_instance[hemisphere].OnDataReceive(data);
}
