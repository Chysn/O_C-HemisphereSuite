#include "hem_arp_chord.h"
#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"

#define HEM_CARPEGGIO_ANIMATION_SPEED 500

class Carpeggio : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Carpeggio";
    }

    void Start() {
        step = 0;
        replay = 0;
        transpose = 0;
        quantizer.Init();
        quantizer.Configure(OC::Scales::GetScale(5), 0xffff); // Semi-tone
        ImprintChord(2);
    }

    void Controller() {
        if (Clock(1)) step = 0; // Reset

        if (Clock(0)) {
            // Are the X or Y position being set? If so, get step coordinates. Otherwise,
            // simply play current step and advance it. This way, the applet can be used as
            // a more conventional arpeggiator as well as a Cartesian one.
            if (DetentedIn(0) || DetentedIn(1)) {
                int x = ProportionCV(In(0), 3);
                int y = ProportionCV(In(1), 3);
                step = (y * 4) + x;
            }
            int note = sequence[step] + 48 + transpose;
            Out(0, quantizer.Lookup(constrain(note, 0, 126)));
            if (++step > 15) step = 0;
        }

        if (replay) {
            int note = sequence[step] + 48 + transpose;
            Out(0, quantizer.Lookup(constrain(note, 0, 126)));
            replay = 0;
        }

        // Handle imprint confirmation animation
        if (--confirm_animation_countdown < 0) {
            confirm_animation_position--;
            confirm_animation_countdown = HEM_CARPEGGIO_ANIMATION_SPEED;
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
        DrawGrid();
    }

    void ScreensaverView() {
        DrawSelector();
        DrawGrid();
    }

    void OnButtonPress() {
        // Set a chord imprint if a new chord is picked
        if (cursor == 1 && chord != sel_chord) ImprintChord(chord);
        if (++cursor > 2) cursor = 0;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) {
            sequence[step] = constrain(sequence[step] += direction, 0, 60);
            replay = 1;
        }
        if (cursor == 1) chord = constrain(chord += direction, 0, Nr_of_arp_chords - 1);
        if (cursor == 2) transpose = constrain(transpose += direction, -24, 24);
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,8}, sel_chord);
        Pack(data, PackLocation {8,8}, transpose + 24);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        ImprintChord(Unpack(data, PackLocation {0,8}));
        transpose = Unpack(data, PackLocation {8,8}) - 24;
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "1=X 2=Y";
        help[HEMISPHERE_HELP_OUTS]     = "A=Quant";
        help[HEMISPHERE_HELP_ENCODER]  = "Note/Chord/Trnspos";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor; // 0=notes, 1=chord
    braids::Quantizer quantizer;

    // Sequencer state
    uint8_t step; // Current step number
    int16_t sequence[16];
    bool replay; // When the encoder is moved, re-quantize the output

    // Settings
    int chord; // Selected chord
    int sel_chord; // Most recently-imprinted chord
    int transpose; // Transposition setting (-24 ~ +24)

    // Variables to handle imprint confirmation animation
    int confirm_animation_countdown;
    int confirm_animation_position;

    void DrawSelector() {
        // Chord selector
        gfxPrint(1, 15, Arp_Chords[chord].chord_name);
        if (cursor == 1) {
            gfxCursor(1, 23, 62);
            if (chord == sel_chord) {
                const uint8_t check[8] = {0x00,0xf0,0x40,0x20,0x10,0x08,0x04,0x00};
                gfxBitmap(55, 13, 8, check);
            }
        }

        // Transpose editor
        gfxPrint(32, 25, "Tr");
        gfxPrint(transpose < 0 ? "" : "+");
        gfxPrint(transpose);
        if (cursor == 2) gfxCursor(32, 33, 30);

        // Coordinates and cursor for tone editing
        if (cursor == 0) {
            // x,y
            gfxPrint(32, 40, "(");
            gfxPrint((step % 4) + 1);
            gfxPrint(",");
            gfxPrint((step / 4) + 1);
            gfxPrint(")");

            gfxPrint(49 + (sequence[step] < 10 ? 8 : 0), 50, sequence[step]);
            gfxCursor(32, 58, 30);
        }
    }

    void DrawGrid() {
        // Draw the Cartesian plane
        for (int s = 0; s < 16; s++) gfxFrame(1 + (8 * (s % 4)), 26 + (8 * (s / 4)), 5, 5);

        // Crosshairs for play position
        int cxy = step / 4;
        int cxx = step % 4;
        gfxDottedLine(3 + (8 * cxx), 26, 3 + (8 * cxx), 58, 2);
        gfxDottedLine(1, 28 + (8 * cxy), 32, 28 + (8 * cxy), 2);
        gfxRect(1 + (8 * cxx), 26 + (8 * cxy), 5, 5);

        // Draw confirmation animation, if necessary
        if (confirm_animation_position > -1) {
            int progress = 16 - confirm_animation_position;
            for (int s = 0; s < progress; s++)
            {
                gfxRect(1 + (8 * (s / 4)), 26 + (8 * (s % 4)), 7, 7);
            }
        }
    }

    void ImprintChord(int new_chord) {
        int num = Arp_Chords[new_chord].nr_notes;
        for (int s = 0; s < 16; s++)
        {
            int oct = (s / num) % 4; // Increase one octave each time this cycle repeats to a max of 4
            int tone = s % num;
            sequence[s] = Arp_Chords[new_chord].chord_tones[tone] + (12 * oct);
        }
        sel_chord = new_chord;
        chord = new_chord;
        confirm_animation_position = 16;
        confirm_animation_countdown = HEM_CARPEGGIO_ANIMATION_SPEED;
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Carpeggio,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Carpeggio Carpeggio_instance[2];

void Carpeggio_Start(int hemisphere) {
    Carpeggio_instance[hemisphere].BaseStart(hemisphere);
}

void Carpeggio_Controller(int hemisphere, bool forwarding) {
    Carpeggio_instance[hemisphere].BaseController(forwarding);
}

void Carpeggio_View(int hemisphere) {
    Carpeggio_instance[hemisphere].BaseView();
}

void Carpeggio_Screensaver(int hemisphere) {
    Carpeggio_instance[hemisphere].BaseScreensaverView();
}

void Carpeggio_OnButtonPress(int hemisphere) {
    Carpeggio_instance[hemisphere].OnButtonPress();
}

void Carpeggio_OnEncoderMove(int hemisphere, int direction) {
    Carpeggio_instance[hemisphere].OnEncoderMove(direction);
}

void Carpeggio_ToggleHelpScreen(int hemisphere) {
    Carpeggio_instance[hemisphere].HelpScreen();
}

uint32_t Carpeggio_OnDataRequest(int hemisphere) {
    return Carpeggio_instance[hemisphere].OnDataRequest();
}

void Carpeggio_OnDataReceive(int hemisphere, uint32_t data) {
    Carpeggio_instance[hemisphere].OnDataReceive(data);
}
