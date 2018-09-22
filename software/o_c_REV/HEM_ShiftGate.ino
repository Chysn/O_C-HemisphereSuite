class ShiftGate : public HemisphereApplet {
public:

    const char* applet_name() {
        return "ShiftGate";
    }

    void Start() {
        ForEachChannel(ch)
        {
            length[ch] = 4;
            trigger[ch] = ch;
            reg[ch] = random(0, 0xffff);
        }
    }

    void Controller() {
        if (Clock(0)) StartADCLag();

        if (EndOfADCLag()) {
            ForEachChannel(ch)
            {
                // Grab the bit that's about to be shifted away
                int last = (reg[ch] >> (length[ch] - 1)) & 0x01;

                if (!Clock(1)) { // Digital 2 freezes the buffer
                    // XOR the incoming one-bit data with the high bit to get a new value
                    bool data = In(ch) > HEMISPHERE_3V_CV ? 0x01 : 0x00;
                    last = (data != last);
                }

                // Shift left, then potentially add the bit from the other side
                reg[ch] = (reg[ch] << 1) + last;

                bool clock = reg[ch] & 0x01;
                if (trigger[ch]) {
                    if (clock) ClockOut(ch);
                }
                else GateOut(ch, clock);
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void OnButtonPress() {
        if (++cursor > 3) cursor = 0;
    }

    void OnEncoderMove(int direction) {
        byte ch = cursor > 1 ? 1 : 0;
        byte c = cursor > 1 ? cursor - 2 : cursor;
        if (c == 0) length[ch] = constrain(length[ch] + direction, 1, 16);
        if (c == 1) trigger[ch] = 1 - trigger[ch];
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,4}, length[0] - 1);
        Pack(data, PackLocation {4,4}, length[1] - 1);
        Pack(data, PackLocation {8,1}, trigger[0]);
        Pack(data, PackLocation {9,1}, trigger[1]);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        length[0] = Unpack(data, PackLocation {0,4}) + 1;
        length[1] = Unpack(data, PackLocation {4,4}) + 1;
        trigger[0] = Unpack(data, PackLocation {8,1});
        trigger[1] = Unpack(data, PackLocation {9,1});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Freeze";
        help[HEMISPHERE_HELP_CVS]      = "1,2 Gate=Flip Bit0";
        help[HEMISPHERE_HELP_OUTS]     = "A,B Gate/Trigger";
        help[HEMISPHERE_HELP_ENCODER]  = "Length/Type";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor; // 0=Length, 1=Trigger/Gate
    
    // Settings
    int8_t length[2]; // 1-16
    uint16_t reg[2]; // Registers
    bool trigger[2]; // 0=Gate, 1=Trigger

    void DrawInterface() {
        gfxPrint(1, 15, "Length:");
        gfxPrint(1 + pad(10, length[0]), 25, length[0]);
        gfxPrint(33 + pad(10, length[1]), 25, length[1]);

        gfxPrint(1, 37, "Type:");
        gfxPrint(1, 47, trigger[0] ? "Trig" : "Gate");
        gfxPrint(33, 47, trigger[1] ? "Trig" : "Gate");

        byte x = cursor > 1 ? cursor - 2 : cursor;
        byte y = cursor > 1 ? 1 : 0;
        gfxCursor(1 + (33 * y), 33 + (22 * x), 24);

        // Register display
        ForEachChannel(ch)
        {
            byte x = 1 + (32 * ch);
            for (int b = 0; b < 16; b++)
            {
                if ((reg[ch] >> b) & 0x01) gfxPixel(x + (b * 2), 35);
            }
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to ShiftGate,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
ShiftGate ShiftGate_instance[2];

void ShiftGate_Start(bool hemisphere) {ShiftGate_instance[hemisphere].BaseStart(hemisphere);}
void ShiftGate_Controller(bool hemisphere, bool forwarding) {ShiftGate_instance[hemisphere].BaseController(forwarding);}
void ShiftGate_View(bool hemisphere) {ShiftGate_instance[hemisphere].BaseView();}
void ShiftGate_Screensaver(bool hemisphere) {ShiftGate_instance[hemisphere].BaseScreensaverView();}
void ShiftGate_OnButtonPress(bool hemisphere) {ShiftGate_instance[hemisphere].OnButtonPress();}
void ShiftGate_OnEncoderMove(bool hemisphere, int direction) {ShiftGate_instance[hemisphere].OnEncoderMove(direction);}
void ShiftGate_ToggleHelpScreen(bool hemisphere) {ShiftGate_instance[hemisphere].HelpScreen();}
uint32_t ShiftGate_OnDataRequest(bool hemisphere) {return ShiftGate_instance[hemisphere].OnDataRequest();}
void ShiftGate_OnDataReceive(bool hemisphere, uint32_t data) {ShiftGate_instance[hemisphere].OnDataReceive(data);}
