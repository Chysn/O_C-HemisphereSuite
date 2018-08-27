// Copyright (c) 2018, Jason Justian
//
// Based on Braids Quantizer, Copyright 2015 Olivier Gillet.
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

#include "HSApplication.h"
#include "HSMIDI.h"
#include "enigma/TuringMachine.h"
#include "enigma/TuringMachineState.h"
#include "enigma/EnigmaStep.h"

// Modes
const byte ENIGMA_MODE_MANAGE = 0; // Create, edit, save, favorite, sysex dump Turing Machines
const byte ENIGMA_MODE_ASSIGN = 1; // Assign CV and MIDI output
const byte ENIGMA_MODE_SONG = 2;   // Assemble compositions by chaining Turing Machines

// Parameters for Manaage mode (per Turing Machine)
const byte ENIGMA_TM_LENGTH = 0;
const byte ENIGMA_TM_PROBABILITY = 1;

// Parameters for Assign mode (per output)
const byte ENIGMA_OUTPUT_TRACK = 0;
const byte ENIGMA_OUTPUT_TYPE = 1;
const byte ENIGMA_OUTPUT_SCALE = 2;
const byte ENIGMA_OUTPUT_MIDI_CH = 3;

// Parameters for Song mode (per step)
const byte ENIGMA_STEP_TM = 0;
const byte ENIGMA_STEP_REPEATS = 1;
const byte ENIGMA_STEP_TRANSPOSE = 2;

class EnigmaTMWS : public HSApplication, public SystemExclusiveHandler {
public:
	void Start() {
	}

    void Controller() {
    }

    void View() {
        DrawHeader();
        DrawInterface();
    }

    void ScreensaverView() {
        DrawInterface();
    }

    void OnSendSysEx() {
    }

    void OnReceiveSysEx() {
    }

    /////////////////////////////////////////////////////////////////
    // Control handlers
    /////////////////////////////////////////////////////////////////
    // Left button press changes the Mode
    void OnLeftButtonPress() {
        if (++mode > ENIGMA_MODE_SONG) mode = ENIGMA_MODE_MANAGE;
    }

    void OnLeftButtonLongPress() {
    }

    // Right button sets the parameter for the data type
    void OnRightButtonPress() {
        if (mode == ENIGMA_MODE_MANAGE) {
            if (++tm_param > ENIGMA_TM_LENGTH) tm_param = 0;
        }
        if (mode == ENIGMA_MODE_ASSIGN) {
            if (++output_param > ENIGMA_OUTPUT_MIDI_CH) output_param = 0;
        }
        if (mode == ENIGMA_MODE_SONG) {
            if(++step_param > ENIGMA_STEP_TRANSPOSE) step_param = 0;
        }
    }

    void OnUpButtonPress() {
    }

    void OnDownButtonPress() {
    }

    void OnDownButtonLongPress() {
    }

    // Left encoder selects the main object in the mode
    void OnLeftEncoderMove(int direction) {
        if (mode == ENIGMA_MODE_MANAGE) {
            // For Manage mode, that's the Turing Machine
            tm_cursor += direction;
            if (tm_cursor < 0) tm_cursor = HS::TURING_MACHINE_COUNT - 1;
            if (tm_cursor >= HS::TURING_MACHINE_COUNT) tm_cursor = 0;
            LoadNextTM(tm_cursor);
        }
    }

    void OnRightEncoderMove(int direction) {
        switch(mode) {
            case ENIGMA_MODE_MANAGE : DelegateTMParam(direction);
                                      break;

            case ENIGMA_MODE_ASSIGN : DelegateOutputParam(direction);
                                      break;

            default:
            case ENIGMA_MODE_SONG   : DelegateStepParam(direction);
                                      break;
        }
    }

private:
    //////// OPERATING STATES
    TuringMachineState tm_state; // The currently-selected state in Manager mode

    //////// DATA
    EnigmaStep song[512];

    //////// NAVIGATION
    char mode; // 0=Manage 1=Assign 2=Song

    // Primary object cursors, one for each mode
    int8_t tm_cursor; // For Manage mode, choose the Turing Machine (0-39: A1-F8)
    int8_t output_cursor; // For Output mode, choose the Output (0-3: A~D)
    int8_t track_cursor; // For Song mode, choose the Track number (0-3: Tr1-Tr4)

    // Secondary object parameter cursors, one for each mode
    int8_t tm_param; // 0=Length, 1=Probability
    int8_t output_param; // 0=Track, 1=Type, 2=Scale, 3=MIDI Out Channel
    int8_t step_param; // 0=Track, 1=TM, 2=Probability,

    void DrawHeader() {
        gfxHeader("Enigma - ");
        if (mode == ENIGMA_MODE_MANAGE) gfxPrint("Manage");
        if (mode == ENIGMA_MODE_ASSIGN) gfxPrint("Assign");
        if (mode == ENIGMA_MODE_SONG) gfxPrint("Song");
    }

    // DrawInterface()'s job is to delegate to the various mode screens.
    void DrawInterface() {
        switch(mode) {
            case ENIGMA_MODE_MANAGE : DrawManageInterface();
                                      break;

            case ENIGMA_MODE_ASSIGN : DrawAssignInterface();
                                      break;

            default:
            case ENIGMA_MODE_SONG   : DrawSongInterface();
                                      break;
        }
    }

    void DrawManageInterface() {
        // Draw the left side, the selector

    }

    void DrawAssignInterface() {

    }

    void DrawSongInterface() {

    }

    // When a new TM is selected, load it here
    void LoadNextTM(byte ix) {
        if (HS::TMAccess::len(ix)) tm_state.Init(HS::user_turing_machines[ix]);
        else tm_state.Init();
    }

    //////// Parameter delegates
    void DelegateTMParam(int direction) {
        if (tm_param == ENIGMA_TM_LENGTH) tm_state.ChangeLength(direction);
        if (tm_param == ENIGMA_TM_PROBABILITY) tm_state.ChangeProbability(direction);
    }

    void DelegateOutputParam(int direction) {

    }

    void DelegateStepParam(int direction) {

    }
};

EnigmaTMWS EnigmaTMWS_instance;

// App stubs
void EnigmaTMWS_init() {
    EnigmaTMWS_instance.BaseStart();
}

// Not using O_C Storage
size_t EnigmaTMWS_storageSize() {return 0;}
size_t EnigmaTMWS_save(void *storage) {return 0;}
size_t EnigmaTMWS_restore(const void *storage) {return 0;}

void EnigmaTMWS_isr() {
	return EnigmaTMWS_instance.BaseController();
}

void EnigmaTMWS_handleAppEvent(OC::AppEvent event) {
    if (event == OC::APP_EVENT_SUSPEND) {
        EnigmaTMWS_instance.OnSendSysEx();
    }
}

void EnigmaTMWS_loop() {} // Deprecated

void EnigmaTMWS_menu() {
    EnigmaTMWS_instance.BaseView();
}

void EnigmaTMWS_screensaver() {
    EnigmaTMWS_instance.BaseScreensaverView();
}

void EnigmaTMWS_handleButtonEvent(const UI::Event &event) {
    // For left encoder, handle press and long press
    if (event.control == OC::CONTROL_BUTTON_L) {
        if (event.type == UI::EVENT_BUTTON_LONG_PRESS) EnigmaTMWS_instance.OnLeftButtonLongPress();
        else EnigmaTMWS_instance.OnLeftButtonPress();
    }

    // For right encoder, only handle press (long press is reserved)
    if (event.control == OC::CONTROL_BUTTON_R && event.type == UI::EVENT_BUTTON_PRESS) EnigmaTMWS_instance.OnRightButtonPress();

    // For up button, handle only press (long press is reserved)
    if (event.control == OC::CONTROL_BUTTON_UP) EnigmaTMWS_instance.OnUpButtonPress();

    // For down button, handle press and long press
    if (event.control == OC::CONTROL_BUTTON_DOWN) {
        if (event.type == UI::EVENT_BUTTON_PRESS) EnigmaTMWS_instance.OnDownButtonPress();
        if (event.type == UI::EVENT_BUTTON_LONG_PRESS) EnigmaTMWS_instance.OnDownButtonLongPress();
    }
}

void EnigmaTMWS_handleEncoderEvent(const UI::Event &event) {
    // Left encoder turned
    if (event.control == OC::CONTROL_ENCODER_L) EnigmaTMWS_instance.OnLeftEncoderMove(event.value);

    // Right encoder turned
    if (event.control == OC::CONTROL_ENCODER_R) EnigmaTMWS_instance.OnRightEncoderMove(event.value);
}
