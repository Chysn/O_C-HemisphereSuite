int const LOFI_PCM_BUFFER_SIZE = 2048;
int const LOFI_PCM_SPEED = 8;
#define LOFI_PCM2CV(S) ((uint32_t)S << 8) - 32767;

class LoFiPCM : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "LoFi Tape";
    }

    void Start() {
        countdown = LOFI_PCM_SPEED;
        for (int i = 0; i < LOFI_PCM_BUFFER_SIZE; i++) pcm[i] = 128;
    }

    void Controller() {
        play = Gate(0);
        gated_record = Gate(1);

        countdown--;
        if (countdown == 0) {
            if (record || play || gated_record) {
                head++;
                if (head > length) {
                    head = 0;
                    record = 0;
                }
                
                if (record || gated_record) {
                    uint32_t s = (In(0) + 32767) >> 8;
                    pcm[head] = (char)s;
                    if (record) Out(0, In(0));
                }
                
                if (play) {
                    uint32_t s = LOFI_PCM2CV(pcm[head]);
                    Out(0, s);
                }
            }
            countdown = LOFI_PCM_SPEED;
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawTransportBar(15);
        DrawWaveform();
    }

    void ScreensaverView() {
        DrawWaveform();
    }

    void OnButtonPress() {
        record = 1 - record;
        play = 0;
        head = 0;
    }

    void OnEncoderMove(int direction) {
        length = constrain(length += (direction * 32), 32, LOFI_PCM_BUFFER_SIZE);
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
        help[HEMISPHERE_HELP_DIGITALS] = "Gate 1=Loop 2=Rec";
        help[HEMISPHERE_HELP_CVS]      = "1=Audio";
        help[HEMISPHERE_HELP_OUTS]     = "A=Audio";
        help[HEMISPHERE_HELP_ENCODER]  = "P=Record T=End Pt";
        //                               "------------------" <-- Size Guide
    }
    
private:
    char pcm[LOFI_PCM_BUFFER_SIZE];
    bool record = 0; // Record activated via button
    bool gated_record = 0; // Record gated via digital in
    bool play = 0;
    int head = 0; // Locatioon of play/record head
    int countdown = LOFI_PCM_SPEED;
    int length = LOFI_PCM_BUFFER_SIZE;
    
    void DrawTransportBar(int y) {
        DrawStop(3, 15);
        DrawPlay(26, 15);
        DrawRecord(50, 15);
    }
    
    void DrawWaveform() {
        int inc = LOFI_PCM_BUFFER_SIZE / 256;
        int disp[32];
        int high = 1;
        int pos = head - (inc * 15) - random(1,6); // Try to center the head
        if (head < 0) head += length;
        for (int i = 0; i < 32; i++)
        {
            int v = (int)pcm[pos] - 127;
            if (v < 0) v = 0;
            if (v > high) high = v;
            pos += inc;
            if (pos >= LOFI_PCM_BUFFER_SIZE) pos -= length;
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

void LoFiPCM_Start(int hemisphere) {
    LoFiPCM_instance[hemisphere].BaseStart(hemisphere);
}

void LoFiPCM_Controller(int hemisphere, bool forwarding) {
    LoFiPCM_instance[hemisphere].BaseController(forwarding);
}

void LoFiPCM_View(int hemisphere) {
    LoFiPCM_instance[hemisphere].BaseView();
}

void LoFiPCM_Screensaver(int hemisphere) {
    LoFiPCM_instance[hemisphere].ScreensaverView();
}

void LoFiPCM_OnButtonPress(int hemisphere) {
    LoFiPCM_instance[hemisphere].OnButtonPress();
}

void LoFiPCM_OnEncoderMove(int hemisphere, int direction) {
    LoFiPCM_instance[hemisphere].OnEncoderMove(direction);
}

void LoFiPCM_ToggleHelpScreen(int hemisphere) {
    LoFiPCM_instance[hemisphere].HelpScreen();
}

uint32_t LoFiPCM_OnDataRequest(int hemisphere) {
    return LoFiPCM_instance[hemisphere].OnDataRequest();
}

void LoFiPCM_OnDataReceive(int hemisphere, uint32_t data) {
    LoFiPCM_instance[hemisphere].OnDataReceive(data);
}
