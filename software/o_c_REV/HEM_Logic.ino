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

// Logical gate functions and typedef to function pointer
#define HEMISPHERE_NUMBER_OF_LOGIC 7
bool hem_AND(bool s1, bool s2) {return s1 & s2;}
bool hem_OR(bool s1, bool s2) {return s1 | s2;}
bool hem_XOR(bool s1, bool s2) {return s1 != s2;}
bool hem_NAND(bool s1, bool s2) {return !hem_AND(s1, s2);}
bool hem_NOR(bool s1, bool s2) {return !hem_OR(s1, s2);}
bool hem_XNOR(bool s1, bool s2) {return !hem_XOR(s1, s2);}
bool hem_null(bool s1, bool s2) {return 0;} // Used when the section is under CV control
typedef bool(*LogicGateFunction)(bool, bool);

const uint8_t LOGIC_ICON[6][12] = {
    {0x22, 0x22, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x3e, 0x08, 0x08, 0x08}, // AND
    {0x22, 0x22, 0x63, 0x77, 0x7f, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x08, 0x08}, // OR
    {0x22, 0x22, 0x77, 0x08, 0x77, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x08, 0x08}, // XOR
    {0x22, 0x22, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x3e, 0x08, 0x0a, 0x0c}, // NAND
    {0x22, 0x22, 0x63, 0x77, 0x7f, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x0a, 0x0c}, // NOR
    {0x22, 0x22, 0x77, 0x08, 0x77, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x0a, 0x0c}  // XNOR
};

class Logic : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Logic";
    }

    void Start() {
        selected = 0;
        operation[0] = 0;
        operation[1] = 2;
        const char * op_name_list[] = {"AND", "OR", "XOR", "NAND", "NOR", "XNOR", "-CV-"};
        LogicGateFunction logic_gate_list[] = {hem_AND, hem_OR, hem_XOR, hem_NAND, hem_NOR, hem_XNOR, hem_null};
        for(int i = 0; i < HEMISPHERE_NUMBER_OF_LOGIC; i++) 
        {
            op_name[i] = op_name_list[i];
            logic_gate[i] = logic_gate_list[i];    
        }
    }

    void Controller() {
        bool s1 = Gate(0); // Set logical states
        bool s2 = Gate(1);
        
        ForEachChannel(ch)
        {
            int idx = operation[ch];
            if (operation[ch] == HEMISPHERE_NUMBER_OF_LOGIC - 1) {
                // The last selection puts the index under CV control
                int cv = In(ch);
                if (cv < 0) cv = -cv; // So that CV input is bipolar (for use with LFOs, etc.)
                idx = constrain(ProportionCV(cv, 6), 0, 5);
            }
            result[ch] = logic_gate[idx](s1, s2);
            source[ch] = idx; // In case it comes from CV, need to display the right icon
            GateOut(ch, result[ch]);
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
        DrawIndicator();
    }

    void OnButtonPress() {
        selected = 1 - selected;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        operation[selected] += direction;
        if (operation[selected] == HEMISPHERE_NUMBER_OF_LOGIC) operation[selected] = 0;
        if (operation[selected] < 0) operation[selected] = HEMISPHERE_NUMBER_OF_LOGIC - 1;
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
        help[HEMISPHERE_HELP_DIGITALS] = "1=In1, 2=In2";
        help[HEMISPHERE_HELP_CVS] = "-CV-:1=Op1, 2=Op2";
        help[HEMISPHERE_HELP_OUTS] = "A=Result1 B=Res2";
        help[HEMISPHERE_HELP_ENCODER] = "Operation";
    }
    
private:
    const char* op_name[HEMISPHERE_NUMBER_OF_LOGIC];
    LogicGateFunction logic_gate[HEMISPHERE_NUMBER_OF_LOGIC];
    int operation[2];
    bool result[2];
    int source[2];
    int selected;
    
    void DrawSelector()
    {
        ForEachChannel(ch)
        {
            gfxPrint(0 + (31 * ch), 15, op_name[operation[ch]]);
            if (ch == selected) gfxCursor(0 + (31 * ch), 23, 30);
        }
    }    
    
    void DrawIndicator()
    {
        ForEachChannel(ch)
        {
            gfxBitmap(8 + (36 * ch), 45, 12, LOGIC_ICON[source[ch]]);
            if (result[ch]) gfxFrame(5 + (36 * ch), 42, 17, 13);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Logic,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Logic Logic_instance[2];

void Logic_Start(bool hemisphere) {
    Logic_instance[hemisphere].BaseStart(hemisphere);
}

void Logic_Controller(bool hemisphere, bool forwarding) {
    Logic_instance[hemisphere].BaseController(forwarding);
}

void Logic_View(bool hemisphere) {
    Logic_instance[hemisphere].BaseView();
}

void Logic_OnButtonPress(bool hemisphere) {
    Logic_instance[hemisphere].OnButtonPress();
}

void Logic_OnEncoderMove(bool hemisphere, int direction) {
    Logic_instance[hemisphere].OnEncoderMove(direction);
}

void Logic_ToggleHelpScreen(bool hemisphere) {
    Logic_instance[hemisphere].HelpScreen();
}

uint32_t Logic_OnDataRequest(bool hemisphere) {
    return Logic_instance[hemisphere].OnDataRequest();
}

void Logic_OnDataReceive(bool hemisphere, uint32_t data) {
    Logic_instance[hemisphere].OnDataReceive(data);
}
