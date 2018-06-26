#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"

class DualQuant : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "DualQuant";
    }

    void Start() {
        selected = 0;
        for (int ch = 0; ch < 2; ch++)
        {
            quantizer[ch].Init();
            scale[ch] = ch + 5;
            quantizer[ch].Configure(OC::Scales::GetScale(scale[ch]), 0xffff);
            last_note[ch] = 0;
            MoveLittleNote(ch);
        }
    }

    void Controller() {
        for (int ch = 0; ch < 2; ch++)
        {
            if (Clock(ch)) {
                int32_t pitch = In(ch);
                int32_t quantized = quantizer[ch].Process(pitch, 0, 0);
                Out(ch, quantized);
                last_note[ch] = quantized;
                note_y[ch] = random(15, 54);
                last_dir[ch] = note_y[ch] > 35 ? -1 : 1;
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
    }

    void ScreensaverView() {
        DrawLittleNotes();
    }

    void OnButtonPress() {
        selected = 1 - selected;
    }

    void OnEncoderMove(int direction) {
        scale[selected] += direction;
        if (scale[selected] == OC::Scales::NUM_SCALES) scale[selected] = 0;
        if (scale[selected] < 0) scale[selected] = OC::Scales::NUM_SCALES - 1;
        quantizer[selected].Configure(OC::Scales::GetScale(scale[selected]), 0xffff);
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "Clk 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_CVS] = "CV 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_OUTS] = "Pitch 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_ENCODER] = "T=Scale P=Sel Ch";
    }
    
private:
    braids::Quantizer quantizer[2];
    int scale[2]; // Scale per channel
    int last_note[2]; // Last quantized note
    int note_y[2]; // Last location of graphic
    int last_dir[2]; // Last direction of graphic
    int selected;
    const uint8_t notes[2][8] = {{0xc0, 0xe0, 0xe0, 0xe0, 0x7f, 0x02, 0x14, 0x08},
                                 {0xc0, 0xa0, 0xa0, 0xa0, 0x7f, 0x00, 0x00, 0x00}};

    void DrawSelector()
    {
        for (int ch = 0; ch < 2; ch++)
        {
            gfxBitmap(0 + (31 * ch), 15, 8, notes[ch]);
            gfxPrint(0 + (31 * ch), 25, OC::scale_names_short[scale[ch]]);
            if (ch == selected) gfxCursor(0 + (31 * ch), 33, 30);

            // Little notes
            gfxBitmap(ProportionCV(last_note[ch], 54), 41 + (10 * ch), 8, notes[ch]);
        }
    }

    void DrawLittleNotes()
    {
        for (int ch = 0; ch < 2; ch++)
        {
            int x = ProportionCV(last_note[ch], 54);
            gfxBitmap(x, note_y[ch], 8, notes[ch]);
        }
    }

    void MoveLittleNote(int ch) {
        if (random(0, 100) > 85) {
            // Randomly change direction
            last_dir[ch] = -last_dir[ch];
        }
        note_y[ch] += last_dir[ch];
        if (note_y[ch] >= 53) last_dir[ch] = -1;
        if (note_y[ch] <= 15) last_dir[ch] = 1;
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to DualQuant,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
DualQuant DualQuant_instance[2];

void DualQuant_Start(int hemisphere) {
    DualQuant_instance[hemisphere].BaseStart(hemisphere);
}

void DualQuant_Controller(int hemisphere, bool forwarding) {
    DualQuant_instance[hemisphere].BaseController(forwarding);
}

void DualQuant_View(int hemisphere) {
    DualQuant_instance[hemisphere].BaseView();
}

void DualQuant_Screensaver(int hemisphere) {
    DualQuant_instance[hemisphere].ScreensaverView();
}

void DualQuant_OnButtonPress(int hemisphere) {
    DualQuant_instance[hemisphere].OnButtonPress();
}

void DualQuant_OnEncoderMove(int hemisphere, int direction) {
    DualQuant_instance[hemisphere].OnEncoderMove(direction);
}

void DualQuant_ToggleHelpScreen(int hemisphere) {
    DualQuant_instance[hemisphere].HelpScreen();
}
