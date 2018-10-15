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

#define HEM_LOFI_PCM_BUFFER_SIZE 2048
#define HEM_LOFI_PCM_SPEED 8
#define LOFI_PCM2CV(S) ((uint32_t)S << 8) - 32767;

class LoFiPCM : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "LoFi Tape";
    }

    void Start() {
        countdown = HEM_LOFI_PCM_SPEED;
        for (int i = 0; i < HEM_LOFI_PCM_BUFFER_SIZE; i++) pcm[i] = 127;
    }

    void Controller() {
        play = !Gate(0); // Continuously play unless gated
        gated_record = Gate(1);

        countdown--;
        if (countdown == 0) {
            if (play || record || gated_record) head++;
            if (head >= length) {
                head = 0;
                record = 0;
                ClockOut(1);
            }

            if (record || gated_record) {
                uint32_t s = (In(0) + 32767) >> 8;
                pcm[head] = (char)s;
            }

            uint32_t s = LOFI_PCM2CV(pcm[head]);
            int SOS = In(1); // Sound-on-sound
            int live = Proportion(SOS, HEMISPHERE_MAX_CV, In(0));
            int loop = play ? Proportion(HEMISPHERE_MAX_CV - SOS, HEMISPHERE_MAX_CV, s) : 0;
            Out(0, live + loop);
            countdown = HEM_LOFI_PCM_SPEED;
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawTransportBar();
        DrawWaveform();
    }

    void OnButtonPress() {
        record = 1 - record;
        play = 0;
        head = 0;
    }

    void OnEncoderMove(int direction) {
        length = constrain(length += (direction * 32), 32, HEM_LOFI_PCM_BUFFER_SIZE);
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        return data;
    }

    void OnDataReceive(uint32_t data) {
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "Gate 1=Pause 2=Rec";
        help[HEMISPHERE_HELP_CVS]      = "1=Audio 2=SOS";
        help[HEMISPHERE_HELP_OUTS]     = "A=Audio B=EOC Trg";
        help[HEMISPHERE_HELP_ENCODER]  = "T=End Pt P=Rec";
        //                               "------------------" <-- Size Guide
    }
    
private:
    char pcm[HEM_LOFI_PCM_BUFFER_SIZE];
    bool record = 0; // Record activated via button
    bool gated_record = 0; // Record gated via digital in
    bool play = 0;
    int head = 0; // Locatioon of play/record head
    int countdown = HEM_LOFI_PCM_SPEED;
    int length = HEM_LOFI_PCM_BUFFER_SIZE;
    
    void DrawTransportBar() {
        DrawStop(3, 15);
        DrawPlay(26, 15);
        DrawRecord(50, 15);
    }
    
    void DrawWaveform() {
        int inc = HEM_LOFI_PCM_BUFFER_SIZE / 256;
        int disp[32];
        int high = 1;
        int pos = head - (inc * 15) - random(1,3); // Try to center the head
        if (head < 0) head += length;
        for (int i = 0; i < 32; i++)
        {
            int v = (int)pcm[pos] - 127;
            if (v < 0) v = 0;
            if (v > high) high = v;
            pos += inc;
            if (pos >= HEM_LOFI_PCM_BUFFER_SIZE) pos -= length;
            disp[i] = v;
        }
        
        for (int x = 0; x < 32; x++)
        {
            int height = Proportion(disp[x], high, 30);
            int margin = (32 - height) / 2;
            gfxLine(x * 2, 30 + margin, x * 2, height + 30 + margin);
        }
    }
    
    void DrawStop(int x, int y) {
        if (record || play || gated_record) gfxFrame(x, y, 11, 11);
        else gfxRect(x, y, 11, 11);
    }
    
    void DrawPlay(int x, int y) {
        if (play) {
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
        if (record || gated_record) {
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
///  Once you run the find-and-replace to make these refer to LoFiPCM,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
LoFiPCM LoFiPCM_instance[2];

void LoFiPCM_Start(bool hemisphere) {
    LoFiPCM_instance[hemisphere].BaseStart(hemisphere);
}

void LoFiPCM_Controller(bool hemisphere, bool forwarding) {
    LoFiPCM_instance[hemisphere].BaseController(forwarding);
}

void LoFiPCM_View(bool hemisphere) {
    LoFiPCM_instance[hemisphere].BaseView();
}

void LoFiPCM_OnButtonPress(bool hemisphere) {
    LoFiPCM_instance[hemisphere].OnButtonPress();
}

void LoFiPCM_OnEncoderMove(bool hemisphere, int direction) {
    LoFiPCM_instance[hemisphere].OnEncoderMove(direction);
}

void LoFiPCM_ToggleHelpScreen(bool hemisphere) {
    LoFiPCM_instance[hemisphere].HelpScreen();
}

uint32_t LoFiPCM_OnDataRequest(bool hemisphere) {
    return LoFiPCM_instance[hemisphere].OnDataRequest();
}

void LoFiPCM_OnDataReceive(bool hemisphere, uint32_t data) {
    LoFiPCM_instance[hemisphere].OnDataReceive(data);
}
