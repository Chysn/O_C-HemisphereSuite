// See https://www.pjrc.com/teensy/td_midi.html

#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"

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
        quantizer.Init();
        quantizer.Configure(OC::Scales::GetScale(5), 0xffff); // Semi-tone
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
            quantizer.Process(In(0), 0, 0);
            uint8_t midi_note = quantizer.NoteNumber() + transpose - 4;
            midi_note = constrain(midi_note, 0, 127);

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
            int this_cv = In(1);
            if (cv_has_changed(this_cv, last_cv)) {
                last_cv = this_cv;

                // Modulation wheel
                if (function == HEM_MIDI_CC_IN) {
                    int value = ProportionCV(this_cv, 127);
                    usbMIDI.sendControlChange(1, value, channel + 1);
                    usbMIDI.send_now();
                    UpdateLog(HEM_MIDI_CC, value, 0);
                    last_tick = OC::CORE::ticks;
                }

                // Aftertouch
                if (function == HEM_MIDI_AT_IN) {
                    int value = ProportionCV(this_cv, 127);
                    usbMIDI.sendAfterTouch(value, channel + 1);
                    usbMIDI.send_now();
                    UpdateLog(HEM_MIDI_AFTERTOUCH, value, 0);
                    last_tick = OC::CORE::ticks;
                }

                // Pitch Bend
                if (function == HEM_MIDI_PB_IN) {
                    uint16_t bend = ProportionCV(this_cv + (HEMISPHERE_MAX_CV / 2), 16383);
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
        DrawSelector();
    }

    void ScreensaverView() {
        DrawLog();
    }

    void OnButtonPress() {
        if (++cursor > 3) cursor = 0;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) channel = constrain(channel += direction, 0, 15);
        if (cursor == 1) transpose = constrain(transpose += direction, -24, 24);
        if (cursor == 2) function = constrain(function += direction, 0, 3);
        if (cursor == 3) legato = direction > 0 ? 1 : 0;
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
        help[HEMISPHERE_HELP_ENCODER]  = "Ch/Trnspose/Assign";
        //                               "------------------" <-- Size Guide
    }
    
private:
    // Quantizer for note numbers
    braids::Quantizer quantizer;

    // Icons
    const uint8_t note[8] = {0xc0, 0xe0, 0xe0, 0xe0, 0x7f, 0x02, 0x14, 0x08};
    const uint8_t mod[8]  = {0x30, 0x08, 0x04, 0x08, 0x10, 0x20, 0x10, 0x0c};
    const uint8_t pb[8]   = {0x20, 0x70, 0x70, 0x3f, 0x20, 0x14, 0x0c, 0x1c};
    const uint8_t at[8]   = {0x00, 0x00, 0x20, 0x42, 0xf5, 0x48, 0x20, 0x00};
    const uint8_t midi[8] = {0x3c, 0x42, 0x91, 0x45, 0x45, 0x91, 0x42, 0x3c};

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
    int last_cv; // For checking for changes
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
            gfxBitmap(46, 1, 8, midi);
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
        gfxPrint(legato ? "On" : "Off");

        // Cursor
        if (cursor < 3) gfxCursor(24, 23 + (cursor * 10), 39);
        else gfxCursor(44, 53, 19);

        // Last note log
        if (last_velocity) {
            gfxDottedLine(1, 55, 62, 55, 2);
            gfxBitmap(1, 56, 8, note);
            gfxPrint(10, 56, midi_note_numbers[last_note]);
            gfxPrint(40, 56, last_velocity);
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

    bool cv_has_changed(int this_cv, int last_cv) {
        int diff = this_cv - last_cv;
        return (diff > 50 || diff < -50) ? 1 : 0;
    }

    void log_entry(int y, int index) {
        if (log[index].message == HEM_MIDI_NOTE_ON) {
            gfxBitmap(1, y, 8, note);
            gfxPrint(10, y, midi_note_numbers[log[index].data1]);
            gfxPrint(40, y, log[index].data2);
        }

        if (log[index].message == HEM_MIDI_NOTE_OFF) {
            gfxPrint(1, y, "-");
            gfxPrint(10, y, midi_note_numbers[log[index].data1]);
        }

        if (log[index].message == HEM_MIDI_CC) {
            gfxBitmap(1, y, 8, mod);
            gfxPrint(10, y, log[index].data1);
        }

        if (log[index].message == HEM_MIDI_AFTERTOUCH) {
            gfxBitmap(1, y, 8, at);
            gfxPrint(10, y, log[index].data1);
        }

        if (log[index].message == HEM_MIDI_PITCHBEND) {
            int data = log[index].data1;
            gfxBitmap(1, y, 8, pb);
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

void hMIDIOut_Start(int hemisphere) {
    hMIDIOut_instance[hemisphere].BaseStart(hemisphere);
}

void hMIDIOut_Controller(int hemisphere, bool forwarding) {
    hMIDIOut_instance[hemisphere].BaseController(forwarding);
}

void hMIDIOut_View(int hemisphere) {
    hMIDIOut_instance[hemisphere].BaseView();
}

void hMIDIOut_Screensaver(int hemisphere) {
    hMIDIOut_instance[hemisphere].BaseScreensaverView();
}

void hMIDIOut_OnButtonPress(int hemisphere) {
    hMIDIOut_instance[hemisphere].OnButtonPress();
}

void hMIDIOut_OnEncoderMove(int hemisphere, int direction) {
    hMIDIOut_instance[hemisphere].OnEncoderMove(direction);
}

void hMIDIOut_ToggleHelpScreen(int hemisphere) {
    hMIDIOut_instance[hemisphere].HelpScreen();
}

uint32_t hMIDIOut_OnDataRequest(int hemisphere) {
    return hMIDIOut_instance[hemisphere].OnDataRequest();
}

void hMIDIOut_OnDataReceive(int hemisphere, uint32_t data) {
    hMIDIOut_instance[hemisphere].OnDataReceive(data);
}
