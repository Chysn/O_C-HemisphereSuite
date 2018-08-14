/*
 * Turing Machine based on https://thonk.co.uk/documents/random%20sequencer%20documentation%20v2-1%20THONK%20KIT_LargeImages.pdf
 *
 * Thanks to Tom Whitwell
 */

#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"
#include "bjorklund.h"
const uint8_t TM_MAX_SCALE = 63;

class TM : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Turing";
    }

    void Start() {
        reg = random(0, 65535);
        p = 0;
        length = 16;
        cursor = 0;
        quantizer.Init();
        scale = OC::Scales::SCALE_SEMI;
        quantizer.Configure(OC::Scales::GetScale(scale), 0xffff); // Semi-tone
    }

    void Controller() {
        if (Clock(0)) {
            // If the cursor is not on the p value, the sequence remains the same
            int prob = (cursor == 1) ? p : 0;

            // Grab the bit that's about to be shifted away
            int last = (reg >> (length - 1)) & 0x01;

            // Does it change?
            if (random(0, 99) < prob) last = 1 - last;

            // Shift left, then potentially add the bit from the other side
            reg = (reg << 1) + last;
        }

        // Send 5-bit quantized CV
        int note = reg & 0x1f;
        Out(0, quantizer.Lookup(note + 48));

        // Send 8-bit proportioned CV
        int cv = Proportion(reg & 0x00ff, 255, HEMISPHERE_MAX_CV);
        Out(1, cv);
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
        DrawIndicator();
    }

    void ScreensaverView() {
        DrawSelector();
        DrawIndicator();
    }

    void OnButtonPress() {
        if (++cursor > 2) cursor = 0;
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) length = constrain(length += direction, 2, 16);
        if (cursor == 1) p = constrain(p += direction, 0, 100);
        if (cursor == 2) {
            scale += direction;
            if (scale >= TM_MAX_SCALE) scale = 0;
            if (scale < 0) scale = TM_MAX_SCALE - 1;
            quantizer.Configure(OC::Scales::GetScale(scale), 0xffff);
        }
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,16}, reg);
        Pack(data, PackLocation {16,7}, p);
        Pack(data, PackLocation {23,4}, length - 1);
        Pack(data, PackLocation {27,6}, scale);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        reg = Unpack(data, PackLocation {0,16});
        p = Unpack(data, PackLocation {16,7});
        length = Unpack(data, PackLocation {23,4}) + 1;
        scale = Unpack(data, PackLocation {27,6});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock";
        help[HEMISPHERE_HELP_CVS]      = "";
        help[HEMISPHERE_HELP_OUTS]     = "A=Quant5-bit B=CV8";
        help[HEMISPHERE_HELP_ENCODER]  = "Length/Prob/Scale";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int length; // Sequence length
    int cursor;  // 0 = length, 1 = p, 2 = scale
    braids::Quantizer quantizer;

    // Settings
    uint16_t reg; // 16-bit sequence register
    int p; // Probability of bit 15 changing on each cycle
    int8_t scale; // Scale used for quantized output

    void DrawSelector() {
        gfxBitmap(1, 14, 8, NOTE_ICON);
        gfxPrint(12, 15, length);
        gfxPrint(32, 15, "p=");
        if (cursor == 1) {
            gfxCursor(32, 23, 30);
            gfxPrint(p);
        } else {
            gfxPrint(" -");
        }
        gfxPrint(1, 25, OC::scale_names_short[scale]);
        if (cursor == 0) gfxCursor(1, 23, 30);
        if (cursor == 2) gfxCursor(1, 33, 62);
    }

    void DrawIndicator() {
        gfxLine(0, 40, 63, 40);
        gfxLine(0, 62, 63, 62);
        for (int b = 0; b < 16; b++)
        {
            int v = (reg >> b) & 0x01;
            if (v) gfxRect(4 * b, 42, 3, 18);
        }
    }

    void AdvanceRegister(int prob) {
        // Before shifting, determine the fate of the last bit
        int last = (reg >> 15) & 0x01;
        if (random(0, 99) < prob) last = 1 - last;

        // Shift left, then potentially add the bit from the other side
        reg = (reg << 1) + last;
    }

};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to TM,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
TM TM_instance[2];

void TM_Start(int hemisphere) {
    TM_instance[hemisphere].BaseStart(hemisphere);
}

void TM_Controller(int hemisphere, bool forwarding) {
    TM_instance[hemisphere].BaseController(forwarding);
}

void TM_View(int hemisphere) {
    TM_instance[hemisphere].BaseView();
}

void TM_Screensaver(int hemisphere) {
    TM_instance[hemisphere].BaseScreensaverView();
}

void TM_OnButtonPress(int hemisphere) {
    TM_instance[hemisphere].OnButtonPress();
}

void TM_OnEncoderMove(int hemisphere, int direction) {
    TM_instance[hemisphere].OnEncoderMove(direction);
}

void TM_ToggleHelpScreen(int hemisphere) {
    TM_instance[hemisphere].HelpScreen();
}

uint32_t TM_OnDataRequest(int hemisphere) {
    return TM_instance[hemisphere].OnDataRequest();
}

void TM_OnDataReceive(int hemisphere, uint32_t data) {
    TM_instance[hemisphere].OnDataReceive(data);
}
