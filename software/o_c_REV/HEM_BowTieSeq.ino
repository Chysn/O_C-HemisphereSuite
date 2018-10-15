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

class BowTieSeq : public HemisphereApplet {
public:

    const char* applet_name() {
        return "BowTieSeq";
    }

    void Start() {
        step = -1;
        end = 7;
    }

    void Controller() {
        if (Clock(1)) step = 0; // Reset
        
        uint16_t cv = (1000 * step) + 400;
        Out(0, cv);

        if (Clock(0)) if (++step > end) step = 0;
    }

    void View() {
        gfxHeader(applet_name());
        DrawIndicator();
    }

    void OnButtonPress() {
        step = 0;
    }

    void OnEncoderMove(int direction) {
        end = constrain(end += direction, 0, 7);
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,4}, end);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        end = Unpack(data, PackLocation {0,4});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "";
        help[HEMISPHERE_HELP_OUTS]     = "A=CV";
        help[HEMISPHERE_HELP_ENCODER]  = "T=Length P=Reset";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int8_t step;
    int8_t end;
    
    void DrawIndicator() {
        for (uint8_t s = 0; s < end + 1; s++)
        {
            if (s == step) gfxRect(8 * s, 30, 7, 7);
            else gfxFrame(8 * s, 30, 7, 7);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to BowTieSeq,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
BowTieSeq BowTieSeq_instance[2];

void BowTieSeq_Start(bool hemisphere) {
    BowTieSeq_instance[hemisphere].BaseStart(hemisphere);
}

void BowTieSeq_Controller(bool hemisphere, bool forwarding) {
    BowTieSeq_instance[hemisphere].BaseController(forwarding);
}

void BowTieSeq_View(bool hemisphere) {
    BowTieSeq_instance[hemisphere].BaseView();
}

void BowTieSeq_OnButtonPress(bool hemisphere) {
    BowTieSeq_instance[hemisphere].OnButtonPress();
}

void BowTieSeq_OnEncoderMove(bool hemisphere, int direction) {
    BowTieSeq_instance[hemisphere].OnEncoderMove(direction);
}

void BowTieSeq_ToggleHelpScreen(bool hemisphere) {
    BowTieSeq_instance[hemisphere].HelpScreen();
}

uint32_t BowTieSeq_OnDataRequest(bool hemisphere) {
    return BowTieSeq_instance[hemisphere].OnDataRequest();
}

void BowTieSeq_OnDataReceive(bool hemisphere, uint32_t data) {
    BowTieSeq_instance[hemisphere].OnDataReceive(data);
}
