#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"

class ScaleDuet : public HemisphereApplet {
public:

    const char* applet_name() {
        return "ScaleDuet";
    }

    void Start() {
        ForEachChannel(scale)
        {
            mask[scale] = 0xffff;
        }
        quantizer.Init();
        quantizer.Configure(OC::Scales::GetScale(5), mask[0]);
        last_scale = 0;
        adc_lag_countdown = 0;
    }

    void Controller() {
        // Prepare to read pitch and send gate in the near future; there's a slight
        // lag between when a gate is read and when the CV can be read.
        if (Clock(0)) StartADCLag();

        if (EndOfADCLag()) {
            uint8_t scale = Gate(1);
            if (scale != last_scale) {
                quantizer.Configure(OC::Scales::GetScale(5), mask[scale]);
                last_scale = scale;
            }
            int32_t pitch = In(0);
            int32_t quantized = quantizer.Process(pitch, 0, 0);
            Out(0, quantized);
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawKeyboard();
        DrawMaskIndicators();
    }

    void ScreensaverView() {
        DrawKeyboard();
        DrawMaskIndicators();
    }

    void OnButtonPress() {
        uint8_t scale = cursor > 11 ? 1 : 0;
        uint8_t bit = cursor - (scale * 12);

        // Toggle the mask bit at the cursor position
        mask[scale] ^= (0x01 << bit);
        if (scale == last_scale) quantizer.Configure(OC::Scales::GetScale(5), mask[scale]);
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0 && direction == -1) cursor = 1;
        cursor = constrain(cursor += direction, 0, 23);
        ResetCursor();
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,12}, mask[0]);
        Pack(data, PackLocation {12,12}, mask[1]);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        mask[0] = Unpack(data, PackLocation {0,12});
        mask[1] = Unpack(data, PackLocation {12,12});

        if (mask[0] == 0 && mask[1] == 0) Start(); // If empty data, initialize
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=ScaleSel";
        help[HEMISPHERE_HELP_CVS]      = "1=CV";
        help[HEMISPHERE_HELP_OUTS]     = "A=Pitch";
        help[HEMISPHERE_HELP_ENCODER]  = "T=Select P=Toggle";
        //                               "------------------" <-- Size Guide
    }
    
private:
    braids::Quantizer quantizer;
    uint16_t mask[2];
    uint8_t cursor; // 0-11=Scale 1; 12-23=Scale 2
    uint8_t last_scale; // The most-recently-used scale (used to set the mask when necessary)
    int adc_lag_countdown;

    void DrawKeyboard() {
        // Border
        gfxFrame(0, 27, 63, 32);

        // White keys
        for (uint8_t x = 0; x < 8; x++)
        {
            gfxLine(x * 8, 27, x * 8, 58);
        }

        // Black keys
        for (uint8_t i = 0; i < 6; i++)
        {
            if (i != 2) { // Skip the third position
                uint8_t x = (i * 8) + 6;
                gfxRect(x, 27, 5, 16);
            }
        }

        // Which scale
        gfxPrint(10, 15, "Scale ");
        gfxPrint(cursor < 12 ? 1 : 2);
    }

    void DrawMaskIndicators() {
        uint8_t scale = cursor < 12 ? 0 : 1;
        uint8_t x[12] = {2, 7, 10, 15, 18, 26, 31, 34, 39, 42, 47, 50};
        uint8_t p[12] = {0, 1,  0,  1,  0,  0,  1,  0,  1,  0,  1,  0};
        for (uint8_t i = 0; i < 12; i++)
        {
            if ((mask[scale] >> i) & 0x01) gfxInvert(x[i], (p[i] ? 37 : 51), 4 - p[i], 4 - p[i]);
            if (i == (cursor - (scale * 12))) gfxCursor(x[i] - 1, p[i] ? 25 : 60, 6);
        }

        // If C is selcted, display a selector on the higher C, too
        if (mask[scale] & 0x01) gfxInvert(58, 51, 3, 4);
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to ScaleDuet,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
ScaleDuet ScaleDuet_instance[2];

void ScaleDuet_Start(int hemisphere) {
    ScaleDuet_instance[hemisphere].BaseStart(hemisphere);
}

void ScaleDuet_Controller(int hemisphere, bool forwarding) {
    ScaleDuet_instance[hemisphere].BaseController(forwarding);
}

void ScaleDuet_View(int hemisphere) {
    ScaleDuet_instance[hemisphere].BaseView();
}

void ScaleDuet_Screensaver(int hemisphere) {
    ScaleDuet_instance[hemisphere].BaseScreensaverView();
}

void ScaleDuet_OnButtonPress(int hemisphere) {
    ScaleDuet_instance[hemisphere].OnButtonPress();
}

void ScaleDuet_OnEncoderMove(int hemisphere, int direction) {
    ScaleDuet_instance[hemisphere].OnEncoderMove(direction);
}

void ScaleDuet_ToggleHelpScreen(int hemisphere) {
    ScaleDuet_instance[hemisphere].HelpScreen();
}

uint32_t ScaleDuet_OnDataRequest(int hemisphere) {
    return ScaleDuet_instance[hemisphere].OnDataRequest();
}

void ScaleDuet_OnDataReceive(int hemisphere, uint32_t data) {
    ScaleDuet_instance[hemisphere].OnDataReceive(data);
}
