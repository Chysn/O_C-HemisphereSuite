// Arithmetic functions and typedef to function pointer
#define HEMISPHERE_NUMBER_OF_CALC 6
int hem_MIN(int v1, int v2) {return (v1 < v2) ? v1 : v2;}
int hem_MAX(int v1, int v2) {return (v1 > v2) ? v1 : v2;}
int hem_SUM(int v1, int v2) {return constrain(v1 + v2, 0, HEMISPHERE_MAX_CV);}
int hem_DIFF(int v1, int v2) {return hem_MAX(v1, v2) - hem_MIN(v1, v2);}
int hem_MEAN(int v1, int v2) {return (v1 + v2) / 2;}
int hem_RAND(int v1, int v2) {return random(hem_MIN(v1, v2), hem_MAX(v1, v2));}
typedef int(*CalcFunction)(int, int);

class Calculate : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Dual Calc";
    }

    void Start() {
        selected = 0;
        operation[0] = 0;
        operation[1] = 1;
        const char * op_name_list[] = {"Min", "Max", "Sum", "Diff", "Mean", "Rand"};
        CalcFunction calc_fn_list[] = {hem_MIN, hem_MAX, hem_SUM, hem_DIFF, hem_MEAN, hem_RAND};
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
            int result = calc_fn[idx](v1, v2);
            Out(ch, result);
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
        gfxSkyline();
    }

    void ScreensaverView() {
        gfxButterfly();
    }

    void OnButtonPress() {
        selected = 1 - selected;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        operation[selected] += direction;
        if (operation[selected] == HEMISPHERE_NUMBER_OF_CALC) operation[selected] = 0;
        if (operation[selected] < 0) operation[selected] = HEMISPHERE_NUMBER_OF_CALC - 1;
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "";
        help[HEMISPHERE_HELP_CVS] = "1=CV1 2=CV2";
        help[HEMISPHERE_HELP_OUTS] = "A=Result1 B=Res2";
        help[HEMISPHERE_HELP_ENCODER] = "T=Set Op P=Sel Ch";
    }
    
private:
    const char* op_name[HEMISPHERE_NUMBER_OF_CALC];
    CalcFunction calc_fn[HEMISPHERE_NUMBER_OF_CALC];
    int operation[2];
    int selected;
    
    void DrawSelector()
    {
        ForEachChannel(ch)
        {
            gfxPrint(0 + (31 * ch), 15, op_name[operation[ch]]);
            if (ch == selected) gfxCursor(0 + (31 * ch), 23, 30);
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
    Calculate_instance[hemisphere].ScreensaverView();
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
