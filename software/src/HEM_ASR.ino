// Copyright (c) 2018, Jason Justian
//
// Based on Braids Quantizer, Copyright 2015 Émilie Gillet.
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

#include "HSRingBufferManager.h" // Singleton Ring Buffer manager

class ASR : public HemisphereApplet {
public:

    const char* applet_name() {
        return "\"A\"SR";
    }

    void Start() {
        scale = OC::Scales::SCALE_SEMI;
        buffer_m->SetIndex(1);
        quantizer.Configure(OC::Scales::GetScale(scale), 0xffff); // Semi-tone
    }

    void Controller() {
        buffer_m->Register(hemisphere);

        if (Clock(0)) StartADCLag();

        if (EndOfADCLag() || buffer_m->Ready(hemisphere)) {
            if (!Gate(1) && !buffer_m->IsLinked(hemisphere)) {
                int cv = In(0);
                buffer_m->WriteValueToBuffer(cv, hemisphere);
            }
            index_mod = Proportion(DetentedIn(1), HEMISPHERE_MAX_CV, 32);
            ForEachChannel(ch)
            {
                int cv = buffer_m->ReadNextValue(ch, hemisphere, index_mod);
                int quantized = quantizer.Process(cv, 0, 0);
                Out(ch, quantized);
            }
            buffer_m->Advance();
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
        DrawData();
    }

    void OnButtonPress() {
        if (++cursor > 1) cursor = 0;
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) { // Index
            byte ix = buffer_m->GetIndex();
            buffer_m->SetIndex(ix + direction);
        }
        if (cursor == 1) { // Scale selection
            scale += direction;
            if (scale >= OC::Scales::NUM_SCALES) scale = 0;
            if (scale < 0) scale = OC::Scales::NUM_SCALES - 1;
            quantizer.Configure(OC::Scales::GetScale(scale), 0xffff);
        }
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        byte ix = buffer_m->GetIndex();
        Pack(data, PackLocation {0,8}, ix);
        Pack(data, PackLocation {8,8}, scale);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        buffer_m->SetIndex(Unpack(data, PackLocation {0,8}));
        scale = Unpack(data, PackLocation {8,8});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Freeze";
        help[HEMISPHERE_HELP_CVS]      = "1=CV 2=Mod index";
        help[HEMISPHERE_HELP_OUTS]     = "Outputs";
        help[HEMISPHERE_HELP_ENCODER]  = "Index/Scale";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor;
    RingBufferManager *buffer_m = buffer_m->get();
    braids::Quantizer quantizer;
    int scale;
    int index_mod; // Effect of modulation
    
    void DrawInterface() {
        // Show Link icon if linked with another ASR
        if (buffer_m->IsLinked(hemisphere)) gfxIcon(56, 1, LINK_ICON);

        // Index (shared between all instances of ASR)
        byte ix = buffer_m->GetIndex() + index_mod;
        gfxPrint(1, 15, "Index: ");
        gfxPrint(pad(100, ix), ix);
        if (index_mod != 0) gfxIcon(54, 26, CV_ICON);

        // Scale
        gfxBitmap(1, 24, 8, SCALE_ICON);
        gfxPrint(12, 25, OC::scale_names_short[scale]);

        // Cursor
        if (cursor == 0) gfxCursor(43, 23, 18); // Index Cursor
        if (cursor == 1) gfxCursor(13, 33, 30); // Scale Cursor
    }

    void DrawData() {
        for (byte x = 0; x < 64; x++)
        {
            int y = buffer_m->GetYAt(x, hemisphere) + 40;
            gfxPixel(x, y);
        }
    }

};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to ASR,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
ASR ASR_instance[2];

void ASR_Start(bool hemisphere) {ASR_instance[hemisphere].BaseStart(hemisphere);}
void ASR_Controller(bool hemisphere, bool forwarding) {ASR_instance[hemisphere].BaseController(forwarding);}
void ASR_View(bool hemisphere) {ASR_instance[hemisphere].BaseView();}
void ASR_OnButtonPress(bool hemisphere) {ASR_instance[hemisphere].OnButtonPress();}
void ASR_OnEncoderMove(bool hemisphere, int direction) {ASR_instance[hemisphere].OnEncoderMove(direction);}
void ASR_ToggleHelpScreen(bool hemisphere) {ASR_instance[hemisphere].HelpScreen();}
uint32_t ASR_OnDataRequest(bool hemisphere) {return ASR_instance[hemisphere].OnDataRequest();}
void ASR_OnDataReceive(bool hemisphere, uint32_t data) {ASR_instance[hemisphere].OnDataReceive(data);}
