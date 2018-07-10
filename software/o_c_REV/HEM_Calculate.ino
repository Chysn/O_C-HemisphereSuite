// Arithmetic functions and typedef to function pointer
#define HEMISPHERE_NUMBER_OF_CALC 7
int hem_MIN(int v1, int v2) {return (v1 < v2) ? v1 : v2;}
int hem_MAX(int v1, int v2) {return (v1 > v2) ? v1 : v2;}
int hem_SUM(int v1, int v2) {return constrain(v1 + v2, 0, HEMISPHERE_MAX_CV);}
int hem_DIFF(int v1, int v2) {return hem_MAX(v1, v2) - hem_MIN(v1, v2);}
int hem_MEAN(int v1, int v2) {return (v1 + v2) / 2;}
typedef int(*CalcFunction)(int, int);

class Calculate : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Dual Calc";
    }

    void Start() {
        selected = 0;
        ForEachChannel(ch)
        {
            operation[ch] = 0;
            rand_clocked[ch] = 0;
        }
        const char * op_name_list[] = {"Min", "Max", "Sum", "Diff", "Mean", "S&H", "Rnd"};
        // 0 goes in the Rand and S&H slots, because those are handled in Controller()
        CalcFunction calc_fn_list[] = {hem_MIN, hem_MAX, hem_SUM, hem_DIFF, hem_MEAN, 0, 0};
        for(int i = 0; i < HEMISPHERE_NUMBER_OF_CALC; i++)
        {
            op_name[i] = op_name_list[i];
            calc_fn[i] = calc_fn_list[i];
        }
    }

    void Controller() {
        int v1 = In(0); // Set CV in values
        int v2 = In(1);
        
        ForEachChannel(ch)
        {
            int idx = operation[ch];

            if (idx == 5) { // S&H
                if (Clock(ch)) Out(ch, In(ch));
            } else if (idx == 6) { // Rand
                // The first time a clock comes in, Rand becomes clocked, freezing a random
                // value with each clock pulse. Otherwise, Rand is unclocked, and outputs
                // a random value with each tick.
                if (Clock(ch)) {
                    Out(ch, random(0, HEMISPHERE_MAX_CV));
                    rand_clocked[ch] = 1;
                }
                else if (!rand_clocked[ch]) Out(ch, random(0, HEMISPHERE_MAX_CV));
            } else {
                int result = calc_fn[idx](v1, v2);
                Out(ch, result);
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
        gfxSkyline();
    }

    void ScreensaverView() {
        DrawSelector();
        gfxSkyline();
    }

    void OnButtonPress() {
        selected = 1 - selected;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        operation[selected] += direction;
        if (operation[selected] == HEMISPHERE_NUMBER_OF_CALC) operation[selected] = 0;
        if (operation[selected] < 0) operation[selected] = HEMISPHERE_NUMBER_OF_CALC - 1;
        rand_clocked[selected] = 0;
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0, 8}, operation[0]);
        Pack(data, PackLocation {8, 8}, operation[1]);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        operation[0] = Unpack(data, PackLocation {0, 8});
        operation[1] = Unpack(data, PackLocation {8, 8});
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "Hold 1=CV1 2=CV2";
        help[HEMISPHERE_HELP_CVS] = "1=CV1 2=CV2";
        help[HEMISPHERE_HELP_OUTS] = "A=Result1 B=Res2";
        help[HEMISPHERE_HELP_ENCODER] = "Operation";
    }
    
private:
    const char* op_name[HEMISPHERE_NUMBER_OF_CALC];
    CalcFunction calc_fn[HEMISPHERE_NUMBER_OF_CALC];
    int hold[2];
    int operation[2];
    int selected;
    bool rand_clocked[2];
    const uint8_t clock[8] = {0x9c, 0xa2, 0xc1, 0xcf, 0xc9, 0xa2, 0x9c, 0x00};
    
    void DrawSelector()
    {
        ForEachChannel(ch)
        {
            gfxPrint(31 * ch, 15, op_name[operation[ch]]);
            if (ch == selected) gfxCursor(0 + (31 * ch), 23, 30);

            // Show the icon if this random calculator is clocked
            if (operation[ch] == 6 && rand_clocked[ch])
                gfxBitmap(24 + (31 * ch), 15, 8, clock);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Calculate,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Calculate Calculate_instance[2];

void Calculate_Start(int hemisphere) {
    Calculate_instance[hemisphere].BaseStart(hemisphere);
}

void Calculate_Controller(int hemisphere, bool forwarding) {
    Calculate_instance[hemisphere].BaseController(forwarding);
}

void Calculate_View(int hemisphere) {
    Calculate_instance[hemisphere].BaseView();
}

void Calculate_Screensaver(int hemisphere) {
    Calculate_instance[hemisphere].BaseScreensaverView();
}

void Calculate_OnButtonPress(int hemisphere) {
    Calculate_instance[hemisphere].OnButtonPress();
}

void Calculate_OnEncoderMove(int hemisphere, int direction) {
    Calculate_instance[hemisphere].OnEncoderMove(direction);
}

void Calculate_ToggleHelpScreen(int hemisphere) {
    Calculate_instance[hemisphere].HelpScreen();
}

uint32_t Calculate_OnDataRequest(int hemisphere) {
    return Calculate_instance[hemisphere].OnDataRequest();
}

void Calculate_OnDataReceive(int hemisphere, uint32_t data) {
    Calculate_instance[hemisphere].OnDataReceive(data);
}
