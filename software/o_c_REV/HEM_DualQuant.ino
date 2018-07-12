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
        ForEachChannel(ch)
        {
            quantizer[ch].Init();
            scale[ch] = ch + 5;
            quantizer[ch].Configure(OC::Scales::GetScale(scale[ch]), 0xffff);
            last_note[ch] = 0;
            continuous[ch] = 1;
        }
    }

    void Controller() {
        ForEachChannel(ch)
        {
            bool clocked = Clock(ch);
            if (continuous[ch] || clocked) {
                if (clocked) continuous[ch] = 0; // Turn off continuous mode if there's a clock
                int32_t pitch = In(ch);
                int32_t quantized = quantizer[ch].Process(pitch, 0, 0);
                Out(ch, quantized);
                if (last_note[ch] != quantized) {
                    last_note[ch] = quantized;
                }
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
    }

    void ScreensaverView() {
        DrawSelector();
    }

    void OnButtonPress() {
        selected = 1 - selected;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        scale[selected] += direction;
        if (scale[selected] == OC::Scales::NUM_SCALES) scale[selected] = 0;
        if (scale[selected] < 0) scale[selected] = OC::Scales::NUM_SCALES - 1;
        quantizer[selected].Configure(OC::Scales::GetScale(scale[selected]), 0xffff);
        continuous[selected] = 1; // Re-enable continuous mode when scale is changed
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,16}, scale[0]);
        Pack(data, PackLocation {16,16}, scale[1]);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        scale[0] = Unpack(data, PackLocation {0,16});
        scale[1] = Unpack(data, PackLocation {16,16});
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "Clock 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_CVS] = "CV 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_OUTS] = "Pitch A=Ch1 B=Ch2";
        help[HEMISPHERE_HELP_ENCODER] = "Scale";
    }
    
private:
    braids::Quantizer quantizer[2];
    int scale[2]; // Scale per channel
    int last_note[2]; // Last quantized note
    bool continuous[2]; // Each channel starts as continuous and becomes clocked when a clock is received
    int selected;
    const uint8_t notes[2][8] = {{0xc0, 0xe0, 0xe0, 0xe0, 0x7f, 0x02, 0x14, 0x08},
                                 {0xc0, 0xa0, 0xa0, 0xa0, 0x7f, 0x00, 0x00, 0x00}};
    const uint8_t clock[8] = {0x9c, 0xa2, 0xc1, 0xcf, 0xc9, 0xa2, 0x9c, 0x00};

    void DrawSelector()
    {
        ForEachChannel(ch)
        {
            gfxBitmap(0 + (31 * ch), 15, 8, notes[ch]);
            if (!continuous[ch]) gfxBitmap(10 + (31 * ch), 15,  8, clock); // Display icon if clocked

            gfxPrint(0 + (31 * ch), 25, OC::scale_names_short[scale[ch]]);
            if (ch == selected) gfxCursor(0 + (31 * ch), 33, 30);

            // Little notes
            gfxBitmap(ProportionCV(last_note[ch], 54), 41 + (10 * ch), 8, notes[ch]);
        }
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
    DualQuant_instance[hemisphere].BaseScreensaverView();
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

uint32_t DualQuant_OnDataRequest(int hemisphere) {
    return DualQuant_instance[hemisphere].OnDataRequest();
}

void DualQuant_OnDataReceive(int hemisphere, uint32_t data) {
    DualQuant_instance[hemisphere].OnDataReceive(data);
}
