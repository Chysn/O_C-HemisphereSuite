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

#define HEM_BURST_NUMBER_MAX 12
#define HEM_BURST_SPACING_MAX 500
#define HEM_BURST_SPACING_MIN 8
#define HEM_BURST_CLOCKDIV_MAX 8

class Burst : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Burst";
    }

    void Start() {
        cursor = 0;
        number = 4;
        div = 1;
        spacing = 50;
        bursts_to_go = 0;
        clocked = 0;
        last_number_cv_tick = 0;
    }

    void Controller() {
        // Settings and modulation over CV
        if (DetentedIn(0) > 0) {
            number = ProportionCV(In(0), HEM_BURST_NUMBER_MAX + 1);
            number = constrain(number, 1, HEM_BURST_NUMBER_MAX);
            last_number_cv_tick = OC::CORE::ticks;
        }
        int spacing_mod = clocked ? 0 : Proportion(DetentedIn(1), HEMISPHERE_MAX_CV, 500);

        // Get timing information
        if (Clock(0)) {
            if (clocked) {
                // Get a tempo, if this is the second tick or later since the last clock
                spacing = (ticks_since_clock / number) / 17;
                ticks_since_clock = 0;
            } else clocked = 1;
        }
        ticks_since_clock++;

        // Get spacing with clock division or multiplication calculated
        int effective_spacing = get_effective_spacing();

        // Handle a burst set in progress
        if (bursts_to_go > 0) {
            if (--burst_countdown <= 0) {
                int modded_spacing = effective_spacing + spacing_mod;
                if (modded_spacing < HEM_BURST_SPACING_MIN) modded_spacing = HEM_BURST_SPACING_MIN;
                ClockOut(0);
                if (--bursts_to_go > 0) burst_countdown = modded_spacing * 17; // Reset for next burst
                else GateOut(1, 0); // Turn off the gate
            }
        }

        // Handle the triggering of a new burst set.
        //
        // number_is_changing: If Number is being changed via CV, employ the ADC Lag mechanism
        // so that Number can be set and gated with a sequencer (or something). Otherwise, if
        // Number is not being changed via CV, fire the set of bursts right away. This is done so that
        // the applet can adapt to contexts that involve (1) the need to accurately interpret rapidly-
        // changing CV values or (2) the need for tight timing when Number is static-ish.
        bool number_is_changing = (OC::CORE::ticks - last_number_cv_tick < 80000);
        if (Clock(1) && number_is_changing) StartADCLag();

        if (EndOfADCLag() || (Clock(1) && !number_is_changing)) {
            ClockOut(0);
            GateOut(1, 1);
            bursts_to_go = number - 1;
            burst_countdown = effective_spacing * 17;
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
        DrawIndicator();
    }

    void OnButtonPress() {
        cursor += 1;
        if (cursor > 2) cursor = 0;
        if (cursor > 1 && !clocked) cursor = 0;
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) number = constrain(number += direction, 1, HEM_BURST_NUMBER_MAX);
        if (cursor == 1) {
            spacing = constrain(spacing += direction, HEM_BURST_SPACING_MIN, HEM_BURST_SPACING_MAX);
            clocked = 0;
        }
        if (cursor == 2) {
            div += direction;
            if (div > HEM_BURST_CLOCKDIV_MAX) div = HEM_BURST_CLOCKDIV_MAX;
            if (div < -HEM_BURST_CLOCKDIV_MAX) div = -HEM_BURST_CLOCKDIV_MAX;
            if (div == 0) div = direction > 0 ? 1 : -2; // No such thing as 1/1 Multiple
            if (div == -1) div = 1; // Must be moving up to hit -1 (see previous line)
        }
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,8}, number);
        Pack(data, PackLocation {8,8}, spacing);
        Pack(data, PackLocation {16,8}, div + 8);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        number = Unpack(data, PackLocation {0,8});
        spacing = Unpack(data, PackLocation {8,8});
        div = Unpack(data, PackLocation {16,8}) - 8;
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Burst";
        help[HEMISPHERE_HELP_CVS]      = "1=Number 2=Spacing";
        help[HEMISPHERE_HELP_OUTS]     = "1=Burst 2=Gate";
        help[HEMISPHERE_HELP_ENCODER]  = "Number/Spacing/Div";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor; // Number and Spacing
    int burst_countdown; // Number of ticks to the next expected burst
    int bursts_to_go; // Counts down to end of burst set
    bool clocked; // When a clock signal is received at Digital 1, clocked is activated, and the
                  // spacing of a new burst is number/clock length.
    int ticks_since_clock; // When clocked, this is the time since the last clock.
    int last_number_cv_tick; // The last time the number was changed via CV. This is used to
                             // decide whether the ADC delay should be used when clocks come in.

    // Settings
    int number; // How many bursts fire at each trigger
    int spacing; // How many ms pass between each burst
    int div; // Divide or multiply the clock tempo

    void DrawSelector() {
        // Number
        gfxPrint(1, 15, number);
        gfxPrint(28, 15, "bursts");

        // Spacing
        gfxPrint(1, 25, clocked ? get_effective_spacing() : spacing);
        gfxPrint(28, 25, "ms");

        // Div
        if (clocked) {
            gfxBitmap(1, 35, 8, CLOCK_ICON);
            gfxPrint(11, 35, div < 0 ? "x" : "/");
            gfxPrint(div < 0 ? -div : div);
            gfxPrint(div < 0 ? " Mult" : " Div");
        }

        // Cursor
        gfxCursor(1, 23 + (cursor * 10), 62);
    }

    void DrawIndicator() {
        for (int i = 0; i < bursts_to_go; i++)
        {
            gfxFrame(1 + (i * 5), 46, 4, 12);
        }
    }

    int get_effective_spacing() {
        int effective_spacing = spacing;
        if (clocked) {
            if (div > 1) effective_spacing *= div;
            if (div < 0) effective_spacing /= -div;
        }
        return effective_spacing;
    }

};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Burst,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Burst Burst_instance[2];

void Burst_Start(bool hemisphere) {
    Burst_instance[hemisphere].BaseStart(hemisphere);
}

void Burst_Controller(bool hemisphere, bool forwarding) {
    Burst_instance[hemisphere].BaseController(forwarding);
}

void Burst_View(bool hemisphere) {
    Burst_instance[hemisphere].BaseView();
}

void Burst_OnButtonPress(bool hemisphere) {
    Burst_instance[hemisphere].OnButtonPress();
}

void Burst_OnEncoderMove(bool hemisphere, int direction) {
    Burst_instance[hemisphere].OnEncoderMove(direction);
}

void Burst_ToggleHelpScreen(bool hemisphere) {
    Burst_instance[hemisphere].HelpScreen();
}

uint32_t Burst_OnDataRequest(bool hemisphere) {
    return Burst_instance[hemisphere].OnDataRequest();
}

void Burst_OnDataReceive(bool hemisphere, uint32_t data) {
    Burst_instance[hemisphere].OnDataReceive(data);
}
