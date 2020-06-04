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

// See https://www.pjrc.com/teensy/td_midi.html

// The functions available for each output
#define HEM_MIDI_CC_IN 0
#define HEM_MIDI_AT_IN 1
#define HEM_MIDI_PB_IN 2
#define HEM_MIDI_VEL_IN 3

class hMIDIOut : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "MIDIOut";
    }

    void Start() {
        channel = 0; // Default channel 1
        last_channel = 0;
        function = 0;
        gated = 0;
        transpose = 0;
        legato = 1;
        log_index = 0;

        const char * fn_name_list[] = {"Mod", "Aft", "Bend", "Veloc"};
        for (int i = 0; i < 4; i++) fn_name[i] = fn_name_list[i];
    }

    void Controller() {
        bool read_gate = Gate(0);

        // Handle MIDI notes

        // Prepare to read pitch and send gate in the near future; there's a slight
        // lag between when a gate is read and when the CV can be read.
        if (read_gate && !gated) StartADCLag();

        bool note_on = EndOfADCLag(); // If the ADC lag has ended, a note will always be sent
        if (note_on || legato_on) {
            // Get a new reading when gated, or when checking for legato changes
            uint8_t midi_note = MIDIQuantizer::NoteNumber(In(0), transpose);

            if (legato_on && midi_note != last_note) {
                // Send note off if the note has changed
                usbMIDI.sendNoteOff(last_note, 0, last_channel + 1);
                UpdateLog(HEM_MIDI_NOTE_OFF, midi_note, 0);
                note_on = 1;
            }

            if (note_on) {
                int velocity = 0x64;
                if (function == HEM_MIDI_VEL_IN) {
                    velocity = ProportionCV(In(1), 127);
                }
                last_velocity = velocity;

                usbMIDI.sendNoteOn(midi_note, velocity, channel + 1);
                usbMIDI.send_now();
                last_note = midi_note;
                last_channel = channel;
                last_tick = OC::CORE::ticks;
                if (legato) legato_on = 1;

                UpdateLog(HEM_MIDI_NOTE_ON, midi_note, velocity);
            }
        }

        if (!read_gate && gated) { // A note off message should be sent
            usbMIDI.sendNoteOff(last_note, 0, last_channel + 1);
            usbMIDI.send_now();
            UpdateLog(HEM_MIDI_NOTE_OFF, last_note, 0);
            last_tick = OC::CORE::ticks;
        }

        gated = read_gate;
        if (!gated) legato_on = 0;

        // Handle other messages
        if (function != HEM_MIDI_VEL_IN) {
            if (Changed(1)) {
                // Modulation wheel
                if (function == HEM_MIDI_CC_IN) {
                    int value = ProportionCV(In(1), 127);
                    usbMIDI.sendControlChange(1, value, channel + 1);
                    usbMIDI.send_now();
                    UpdateLog(HEM_MIDI_CC, value, 0);
                    last_tick = OC::CORE::ticks;
                }

                // Aftertouch
                if (function == HEM_MIDI_AT_IN) {
                    int value = ProportionCV(In(1), 127);
                    usbMIDI.sendAfterTouch(value, channel + 1);
                    usbMIDI.send_now();
                    UpdateLog(HEM_MIDI_AFTERTOUCH, value, 0);
                    last_tick = OC::CORE::ticks;
                }

                // Pitch Bend
                if (function == HEM_MIDI_PB_IN) {
                    uint16_t bend = Proportion(In(1) + HEMISPHERE_3V_CV, HEMISPHERE_3V_CV * 2, 16383);
                    bend = constrain(bend, 0, 16383);
                    usbMIDI.sendPitchBend(bend, channel + 1);
                    usbMIDI.send_now();
                    UpdateLog(HEM_MIDI_PITCHBEND, bend - 8192, 0);
                    last_tick = OC::CORE::ticks;
                }
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawMonitor();
        if (cursor == 4) DrawLog();
        else DrawSelector();
    }

    void OnButtonPress() {
        if (++cursor > 4) cursor = 0;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) channel = constrain(channel += direction, 0, 15);
        if (cursor == 1) transpose = constrain(transpose += direction, -24, 24);
        if (cursor == 2) function = constrain(function += direction, 0, 3);
        if (cursor == 3) legato = direction > 0 ? 1 : 0;
        ResetCursor();
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,4}, channel);
        Pack(data, PackLocation {4,3}, function);
        Pack(data, PackLocation {7,1}, legato);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        channel = Unpack(data, PackLocation {0,4});
        function = Unpack(data, PackLocation {4,3});
        legato = Unpack(data, PackLocation {7,1});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Gate";
        help[HEMISPHERE_HELP_CVS]      = "1=Pitch 2=Assign";
        help[HEMISPHERE_HELP_OUTS]     = "";
        help[HEMISPHERE_HELP_ENCODER]  = "Ch/Trp/Assign/Log";
        //                               "------------------" <-- Size Guide
    }
    
private:
    // Settings
    int channel; // MIDI Out channel
    int function; // Function of B/D output
    int transpose; // Semitones of transposition
    int legato; // New notes are sent when note is changed

    // Housekeeping
    int cursor; // 0=MIDI channel, 1=Transpose, 2=CV 2 function, 3=Legato
    int last_note; // Last MIDI note number awaiting not off
    int last_velocity;
    int last_channel; // The last Note On channel, just in case the channel is changed before release
    bool gated; // The most recent gate status
    bool legato_on; // The note handler may currently respond to legato note changes
    int last_tick; // Most recent MIDI message sent
    int adc_lag_countdown;
    const char* fn_name[4];

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

        // Transpose
        gfxPrint(1, 25, "Tr:");
        if (transpose > -1) gfxPrint(24, 25, "+");
        gfxPrint(30, 25, transpose);

        // Input 2 function
        gfxPrint(1, 35, "i2:");
        gfxPrint(24, 35, fn_name[function]);

        // Legato
        gfxPrint(1, 45, "Legato ");
        if (cursor != 3 || CursorBlink()) gfxIcon(54, 45, legato ? CHECK_ON_ICON : CHECK_OFF_ICON);

        // Cursor
        if (cursor < 3) gfxCursor(24, 23 + (cursor * 10), 39);

        // Last note log
        if (last_velocity) {
            gfxBitmap(1, 56, 8, NOTE_ICON);
            gfxPrint(10, 56, midi_note_numbers[last_note]);
            gfxPrint(40, 56, last_velocity);
        }
        gfxInvert(0, 55, 63, 9);
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
            gfxPrint(10, y, log[index].data1);
        }

        if (log[index].message == HEM_MIDI_AFTERTOUCH) {
            gfxBitmap(1, y, 8, AFTERTOUCH_ICON);
            gfxPrint(10, y, log[index].data1);
        }

        if (log[index].message == HEM_MIDI_PITCHBEND) {
            int data = log[index].data1;
            gfxBitmap(1, y, 8, BEND_ICON);
            gfxPrint(10, y, data);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to hMIDIOut,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
hMIDIOut hMIDIOut_instance[2];

void hMIDIOut_Start(bool hemisphere) {
    hMIDIOut_instance[hemisphere].BaseStart(hemisphere);
}

void hMIDIOut_Controller(bool hemisphere, bool forwarding) {
    hMIDIOut_instance[hemisphere].BaseController(forwarding);
}

void hMIDIOut_View(bool hemisphere) {
    hMIDIOut_instance[hemisphere].BaseView();
}

void hMIDIOut_OnButtonPress(bool hemisphere) {
    hMIDIOut_instance[hemisphere].OnButtonPress();
}

void hMIDIOut_OnEncoderMove(bool hemisphere, int direction) {
    hMIDIOut_instance[hemisphere].OnEncoderMove(direction);
}

void hMIDIOut_ToggleHelpScreen(bool hemisphere) {
    hMIDIOut_instance[hemisphere].HelpScreen();
}

uint32_t hMIDIOut_OnDataRequest(bool hemisphere) {
    return hMIDIOut_instance[hemisphere].OnDataRequest();
}

void hMIDIOut_OnDataReceive(bool hemisphere, uint32_t data) {
    hMIDIOut_instance[hemisphere].OnDataReceive(data);
}
