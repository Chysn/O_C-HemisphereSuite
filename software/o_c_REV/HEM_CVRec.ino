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

class CVRec : public HemisphereApplet {
public:

    const char* applet_name() {
        return "CVRec";
    }

    void Start() {
        for (int i = 0; i < 128; i++) sequence[i] = 0;
        step = 0;
        record = 0;
        length = 64;
    }

    void Controller() {
        if (Clock(1)) step = 0;

        if (Clock(0)) {
            if (record) {
                ForEachChannel(ch) sequence[step + (ch * 64)] = In(ch);
                if (--punch_in == 0) record = 0;
            }
            ForEachChannel(ch) Out(ch, sequence[step + (ch * 64)]);
            if (++step >= length) step = 0;
        }

    }

    void View() {
        gfxHeader(applet_name());
        DrawTransportBar();
        DrawPositionIndicator();
        DrawLengthIndicator();
        DrawSequence();
    }

    void ScreensaverView() {
        DrawTransportBar();
        DrawPositionIndicator();
        DrawLengthIndicator();
        DrawSequence();
    }

    void OnButtonPress() {
        if (!record) {
            record = 1;
            punch_in = length;
        } else {
            record = 0;
        }
    }

    void OnEncoderMove(int direction) {
        length = constrain(length += direction, 1, 64);
        if (punch_in > length) punch_in = length;
        encoder_last_move = OC::CORE::ticks;
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        // example: pack property_name at bit 0, with size of 8 bits
        // Pack(data, PackLocation {0,8}, property_name); 
        return data;
    }

    void OnDataReceive(uint32_t data) {
        // example: unpack value at bit 0 with size of 8 bits to property_name
        // property_name = Unpack(data, PackLocation {0,8}); 
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "1=Tr1 2=Tr2";
        help[HEMISPHERE_HELP_OUTS]     = "A=Tr1 B=Tr2";
        help[HEMISPHERE_HELP_ENCODER]  = "T=Length P=Record";
        //                               "------------------" <-- Size Guide
    }
    
private:
    uint16_t sequence[128];
    uint8_t step;
    uint8_t punch_in; // Punched in countdown
    int length;
    bool record;
    int encoder_last_move;

    void DrawTransportBar() {
        DrawPlay(26, 15);
        DrawRecord(50, 15);
        if (OC::CORE::ticks - encoder_last_move < 16667) gfxPrint(1, 15, length);
    }

    void DrawPositionIndicator() {
        gfxLine(step, 45, step, 63);
    }

    void DrawLengthIndicator() {
        gfxLine(0, 30, 0, 34);
        gfxLine(0, 30, length - 1, 30);
        gfxLine(length - 1, 30, length - 1, 34);
    }

    void DrawSequence() {
        for (int x = 0; x < 64; x++)
        {
            ForEachChannel(ch)
            {
                gfxPixel(x, BottomAlign(ProportionCV(sequence[x + (ch * 64)], 29)));
            }
        }
    }

    void DrawPlay(int x, int y) {
        if (!record) {
            for (int i = 0; i < 11; i += 2)
            {
                gfxLine(x + i, y + i/2, x + i, y + 10 - i/2);
                gfxLine(x + i + 1, y + i/2, x + i + 1, y + 10 - i/2);
            }
        } else {
            gfxLine(x, y, x, y + 10);
            gfxLine(x, y, x + 10, y + 5);
            gfxLine(x, y + 10, x + 10, y + 5);
        }
    }

    void DrawRecord(int x, int y) {
        gfxCircle(x + 5, y + 5, 5);
        if (record) {
            for (int r = 1; r < 5; r++)
            {
                gfxCircle(x + 5, y + 5, r);
            }
        }
    }

};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to CVRec,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
CVRec CVRec_instance[2];

void CVRec_Start(int hemisphere) {
    CVRec_instance[hemisphere].BaseStart(hemisphere);
}

void CVRec_Controller(int hemisphere, bool forwarding) {
    CVRec_instance[hemisphere].BaseController(forwarding);
}

void CVRec_View(int hemisphere) {
    CVRec_instance[hemisphere].BaseView();
}

void CVRec_Screensaver(int hemisphere) {
    CVRec_instance[hemisphere].BaseScreensaverView();
}

void CVRec_OnButtonPress(int hemisphere) {
    CVRec_instance[hemisphere].OnButtonPress();
}

void CVRec_OnEncoderMove(int hemisphere, int direction) {
    CVRec_instance[hemisphere].OnEncoderMove(direction);
}

void CVRec_ToggleHelpScreen(int hemisphere) {
    CVRec_instance[hemisphere].HelpScreen();
}

uint32_t CVRec_OnDataRequest(int hemisphere) {
    return CVRec_instance[hemisphere].OnDataRequest();
}

void CVRec_OnDataReceive(int hemisphere, uint32_t data) {
    CVRec_instance[hemisphere].OnDataReceive(data);
}
