#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"

class Sequence5 : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Sequence5";
    }

    void Start() {
        for (int s = 0; s < 5; s++) note[s] = random(0, 30);
        quantizer.Init();
        quantizer.Configure(OC::Scales::GetScale(5), 0xffff); // Semi-tone
    }

    void Controller() {
        // Reset sequencer
        if (Clock(1)) {
            step = 0;
            ClockOut(1);
        }

        int transpose = Proportion(In(0), HEMISPHERE_MAX_CV / 2, 12);
        transpose = constrain(transpose, -12, 12);

        if (Clock(0)) StartADCLag();

        if (EndOfADCLag()) {
            Out(0, quantizer.Lookup(note[step] + 48 + transpose));
            Advance(step);
            if (step == 0) ClockOut(1);
        }

        if (play) {
            Out(0, quantizer.Lookup(note[step] + 48 + transpose));
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawPanel();
    }

    void ScreensaverView() {
        DrawPanel();
    }

    void OnButtonPress() {
        if (++cursor == 5) cursor = 0;
    }

    void OnEncoderMove(int direction) {
        if (note[cursor] + direction < 0 && cursor > 0) {
            // If turning past zero, set the mute bit for this step
            muted |= (0x01 << cursor);
        } else {
            note[cursor] = constrain(note[cursor] += direction, 0, 30);
            muted &= ~(0x01 << cursor);
        }
        play = 1; // Replay the changed step in the controller, so it can be heard
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        for (int s = 0; s < 5; s++)
        {
            Pack(data, PackLocation {s * 5,5}, note[s]);
        }
        Pack(data, PackLocation{25,5}, muted);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        for (int s = 0; s < 5; s++)
        {
            note[s] = Unpack(data, PackLocation {s * 5,5});
        }
        muted = Unpack(data, PackLocation {25,5});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "1=Transpose";
        help[HEMISPHERE_HELP_OUTS]     = "A=CV B=Clk Step 1";
        help[HEMISPHERE_HELP_ENCODER]  = "Note";
        //                               "------------------" <-- Size Guide
    }
    
private:
    braids::Quantizer quantizer;
    int cursor = 0;
    char muted = 0; // Bitfield for muted steps; ((muted >> step) & 1) means muted
    int note[5]; // Sequence value (0 - 30)
    int step = 0; // Current sequencer step
    bool play; // Play the note

    void Advance(int starting_point) {
        if (++step == 5) step = 0;
        // If all the steps have been muted, stay where we were
        if (step_is_muted(step) && step != starting_point) Advance(starting_point);
    }

    void DrawPanel() {
        // Sliders
        for (int s = 0; s < 5; s++)
        {
            int x = 6 + (12 * s);
            if (!step_is_muted(s)) {
                gfxLine(x, 25, x, 63);

                // When cursor, there's a heavier bar and a solid slider
                if (s == cursor) {
                    gfxLine(x + 1, 25, x + 1, 63);
                    gfxRect(x - 4, BottomAlign(note[s]), 9, 3);
                } else gfxFrame(x - 4, BottomAlign(note[s]), 9, 3);

                // When on this step, there's an indicator circle
                if (s == step) {gfxCircle(x, 20, 3);}
            } else if (s == cursor) {
                gfxLine(x, 25, x, 63);
                gfxLine(x + 1, 25, x + 1, 63);
             }
        }
    }

    bool step_is_muted(int step) {
        return (muted & (0x01 << step));
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Sequence5,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Sequence5 Sequence5_instance[2];

void Sequence5_Start(int hemisphere) {
    Sequence5_instance[hemisphere].BaseStart(hemisphere);
}

void Sequence5_Controller(int hemisphere, bool forwarding) {
    Sequence5_instance[hemisphere].BaseController(forwarding);
}

void Sequence5_View(int hemisphere) {
    Sequence5_instance[hemisphere].BaseView();
}

void Sequence5_Screensaver(int hemisphere) {
    Sequence5_instance[hemisphere].BaseScreensaverView();
}

void Sequence5_OnButtonPress(int hemisphere) {
    Sequence5_instance[hemisphere].OnButtonPress();
}

void Sequence5_OnEncoderMove(int hemisphere, int direction) {
    Sequence5_instance[hemisphere].OnEncoderMove(direction);
}

void Sequence5_ToggleHelpScreen(int hemisphere) {
    Sequence5_instance[hemisphere].HelpScreen();
}

uint32_t Sequence5_OnDataRequest(int hemisphere) {
    return Sequence5_instance[hemisphere].OnDataRequest();
}

void Sequence5_OnDataReceive(int hemisphere, uint32_t data) {
    Sequence5_instance[hemisphere].OnDataReceive(data);
}
