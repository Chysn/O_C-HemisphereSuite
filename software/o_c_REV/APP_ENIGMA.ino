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

#include "OC_strings.h"
#include "HSApplication.h"
#include "HSMIDI.h"
#include "enigma/TuringMachine.h"
#include "enigma/TuringMachineState.h"
#include "enigma/EnigmaStep.h"
#include "enigma/EnigmaOutput.h"
#include "enigma/EnigmaTrack.h"

// Modes
const byte ENIGMA_MODE_LIBRARY = 0; // Create, edit, save, favorite, sysex dump Turing Machines
const byte ENIGMA_MODE_ASSIGN = 1;  // Assign CV and MIDI output
const byte ENIGMA_MODE_SONG = 2;    // Assemble compositions by chaining Turing Machines
const byte ENIGMA_MODE_PLAY = 3;    // Play information and transport controls

// Parameters for Manaage mode (per Turing Machine)
const byte ENIGMA_TM_LENGTH = 0;
const byte ENIGMA_TM_PROBABILITY = 1;
const byte ENIGMA_TM_ROTATE = 2;

// Parameters for Assign mode (per output)
const byte ENIGMA_OUTPUT_TRACK = 0;
const byte ENIGMA_OUTPUT_TYPE = 1;
const byte ENIGMA_OUTPUT_SCALE = 2;
const byte ENIGMA_OUTPUT_ROOT = 3;
const byte ENIGMA_OUTPUT_MIDI_CH = 4;

// Parameters for Song mode (per step or track)
const byte ENIGMA_TRACK_DIVIDE = 0;
const byte ENIGMA_TRACK_LOOP = 1;
const byte ENIGMA_STEP_NUMBER = 2;
const byte ENIGMA_STEP_TM = 3;
const byte ENIGMA_STEP_P = 4;
const byte ENIGMA_STEP_REPEATS = 5;
const byte ENIGMA_STEP_TRANSPOSE = 6;

class EnigmaTMWS : public HSApplication, public SystemExclusiveHandler {
public:
	void Start() {
	    for (byte ix = 0; ix < HS::TURING_MACHINE_COUNT; ix++)
	    {
	        state_prob[ix] = 0;
	    }
	    tm_cursor = 0;
	    SwitchTuringMachine(0);

	    // Configure outputs and tracks
	    for (byte o = 0; o < 4; o++)
	    {
	        output[o].InitAs(o);
	        track[o].InitAs(o);
	        song_step[total_steps++].Init(o);
	    }

	    // Clear track list
	    BuildTrackStepList(0);
	}

	void Resume() {
	    SwitchTuringMachine(tm_cursor);
	}

    void Controller() {
        switch(mode) {
            case ENIGMA_MODE_LIBRARY : LibraryController();
                                       break;

            case ENIGMA_MODE_ASSIGN  : AssignController();
                                       break;

            default                  :
            case ENIGMA_MODE_SONG    :
            case ENIGMA_MODE_PLAY    : SongController();
                                       break;
        }
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
        if (++mode > ENIGMA_MODE_PLAY) mode = ENIGMA_MODE_LIBRARY;
        ResetCursor();
    }

    void OnLeftButtonLongPress() {
    }

    // Right button sets the parameter for the data type
    void OnRightButtonPress() {
        if (mode == ENIGMA_MODE_LIBRARY && !tm_state.IsFavorite()) {
            if (++tm_param > ENIGMA_TM_ROTATE) tm_param = 0;
        }
        if (mode == ENIGMA_MODE_ASSIGN) {
            if (++output_param > ENIGMA_OUTPUT_MIDI_CH) output_param = 0;
            if (output[output_cursor].type() > EnigmaOutputType::NOTE7) {
                // Skip scale parameters for trigger and gate outputs
                if (output_param == ENIGMA_OUTPUT_SCALE) output_param = ENIGMA_OUTPUT_MIDI_CH;
            }

        }
        if (mode == ENIGMA_MODE_SONG) {
            if (++step_param > ENIGMA_STEP_TRANSPOSE) step_param = ENIGMA_TRACK_DIVIDE;
        }
        ResetCursor();
    }

    void OnUpButtonPress() {
        if (mode == ENIGMA_MODE_LIBRARY) {
            tm_state.SetFavorite(1 - tm_state.IsFavorite());
            state_prob[tm_cursor] = 0;
            if (tm_state.IsFavorite()) tm_param = ENIGMA_TM_ROTATE;
            // TODO: Sysex out
        }
        if (mode == ENIGMA_MODE_ASSIGN) assign_audition = 0;
        if (mode == ENIGMA_MODE_SONG) {
            // Add a step to the end of the current track
            if (last_track_step_index < 99) {
                track_step[last_track_step_index++] = total_steps;
                song_step[total_steps++].Init(track_cursor);
            }
        }
    }

    void OnDownButtonPress() {
        if (mode == ENIGMA_MODE_LIBRARY) {
            tm_state.Reset();
        }
        if (mode == ENIGMA_MODE_ASSIGN) assign_audition = 1;
        if (mode == ENIGMA_MODE_SONG) {
            // TODO: Delete Step
        }
    }

    void OnDownButtonLongPress() {
    }

    // Left encoder selects the main object in the mode
    void OnLeftEncoderMove(int direction) {
        if (mode == ENIGMA_MODE_LIBRARY) {
            // For Library mode, that's the Turing Machine
            tm_cursor += direction;
            if (tm_cursor < 0) tm_cursor = HS::TURING_MACHINE_COUNT - 1;
            if (tm_cursor >= HS::TURING_MACHINE_COUNT) tm_cursor = 0;
            SwitchTuringMachine(tm_cursor);
        }
        if (mode == ENIGMA_MODE_ASSIGN) {
            output_cursor += direction;
            if (output_cursor < 0) output_cursor = 3;
            if (output_cursor > 3) output_cursor = 0;
        }
        if (mode == ENIGMA_MODE_SONG) {
            track_cursor += direction;
            if (track_cursor < 0) track_cursor = 3;
            if (track_cursor > 3) track_cursor = 0;
            BuildTrackStepList(track_cursor);
        }
        ResetCursor();
    }

    void OnRightEncoderMove(int direction) {
        switch(mode) {
            case ENIGMA_MODE_LIBRARY : DelegateTMParam(direction);
                                       break;

            case ENIGMA_MODE_ASSIGN  : DelegateOutputParam(direction);
                                       break;

            default:
            case ENIGMA_MODE_SONG    : DelegateStepParam(direction);
                                       break;
        }
    }

private:
    //////// OPERATING STATES
    TuringMachineState tm_state; // The currently-selected state in Library mode
    byte state_prob[HS::TURING_MACHINE_COUNT]; // Remember the last probability
    bool assign_audition = 0; // Which area does Assign monitor? 0=Library, 1=Song
    uint16_t track_step[100]; // List of steps in the current track
    uint16_t total_steps = 0; // Total number of song_step[] entries used; index of the next step
    byte last_track_step_index = 0; // For adding the next step

    //////// DATA
    EnigmaStep song_step[396]; // Max 99 steps per track
    EnigmaOutput output[4];
    EnigmaTrack track[4];

    //////// NAVIGATION
    byte mode = 0; // 0=Library 1=Assign 2=Song
    uint16_t edit_index = 0; // Current Song Mode step within track_step[]

    // Primary object cursors, one for each mode
    int8_t tm_cursor = 0; // For Library mode, choose the Turing Machine (0-39: A1-F8)
    int8_t output_cursor = 0; // For Output mode, choose the Output (0-3: A~D)
    int8_t track_cursor = 0; // For Song mode, choose the Track number (0-3: Tr1-Tr4)

    // Secondary object parameter cursors, one for each mode
    int8_t tm_param = 0; // 0=Length, 1=Probability
    int8_t output_param = 0; // 0=Track, 1=Type, 2=Scale, 3=MIDI Out Channel
    int8_t step_param = 0; // 0=Divide (track), 1=Loop (track), 2=Step#, 3=TM, 4=p, 5=Repeats, 6=Transpose

    void DrawHeader() {
        gfxHeader("Enigma - ");
        if (mode == ENIGMA_MODE_LIBRARY) gfxPrint("Library");
        if (mode == ENIGMA_MODE_ASSIGN) gfxPrint("Assign");
        if (mode == ENIGMA_MODE_SONG) gfxPrint("Song");
        if (mode == ENIGMA_MODE_PLAY) gfxPrint("Play");
    }

    // DrawInterface()'s job is to delegate to the various mode screens.
    void DrawInterface() {
        switch(mode) {
            case ENIGMA_MODE_LIBRARY : DrawLibraryInterface();
                                       break;

            case ENIGMA_MODE_ASSIGN  : DrawAssignInterface();
                                       break;

            case ENIGMA_MODE_SONG    : DrawSongInterface();
                                       break;

            default                  :
            case ENIGMA_MODE_PLAY    : DrawPlayInterface();
                                       break;
        }
    }

    void DrawLibraryInterface() {
        // Draw the left side, the selector
        for (byte line = 0; line < 4; line++)
        {
            byte y = 24 + (line * 10);
            byte ix = (tm_cursor + line) % HS::TURING_MACHINE_COUNT;
            char name[4];
            HS::TuringMachine::SetName(name, ix);
            gfxPrint(3, y, name);
            if (HS::user_turing_machines[ix].favorite) gfxBitmap(36, y, 8, FAVORITE_ICON);
        }
        DrawSelectorBox("Register");

        // The right side is for editing
        // Length
        byte length = tm_state.GetLength();
        gfxBitmap(64, 14, 8, NOTE_ICON);
        gfxPrint(76 + pad(10, length), 15, length);

        // Probability
        byte p = state_prob[tm_cursor];
        gfxPrint(96, 15, "p=");
        if (tm_state.IsFavorite()) gfxBitmap(116, 15, 8, FAVORITE_ICON);
        else gfxPrint(pad(100, p), p);

        // Cursors
        if (!tm_state.IsFavorite()) {
            if (tm_param == ENIGMA_TM_PROBABILITY) gfxCursor(109, 23, 18);
            if (tm_param == ENIGMA_TM_LENGTH) gfxCursor(77, 23, 12);
        }
        if (tm_param == ENIGMA_TM_ROTATE) {
            gfxBitmap(87, 27, 8, ROTATE_L_ICON);
            gfxBitmap(95, 27, 8, ROTATE_R_ICON);
        }

        // TM Graphic
        tm_state.DrawAt(64, 40);
    }

    void DrawAssignInterface() {
        // Draw the left side, the selector
        for (byte line = 0; line < 4; line++)
        {
            byte y = 24 + (line * 10);
            byte ix = (output_cursor + line) % 4;
            char out_name[2] = {static_cast<char>(ix + 'A'), '\0'};
            gfxPrint(3, y, out_name);
            gfxPrint(": Tk");
            gfxPrint(output[ix].track() + 1);
        }
        DrawSelectorBox("Output");

        // The right side is for editing
        // Track
        gfxPrint(56, 25, "Track ");
        gfxPrint(output[output_cursor].track() + 1);

        // Type
        gfxPrint(56, 35, enigma_type_names[output[output_cursor].type()]);

        // Scale/Root
        if (output[output_cursor].type() <= EnigmaOutputType::NOTE7) {
            gfxPrint(56, 45, OC::scale_names_short[output[output_cursor].scale()]);
            gfxPrint(114, 45, OC::Strings::note_names_unpadded[output[output_cursor].root()]);
        }

        // MIDI Channel
        gfxPrint(56, 55, "MIDI Ch ");
        gfxPrint(midi_channels[output[output_cursor].midi_channel()]);

        // Cursor
        if (output_param == ENIGMA_OUTPUT_TRACK) gfxCursor(57, 33, 42);
        if (output_param == ENIGMA_OUTPUT_TYPE) gfxCursor(57, 43, 70);
        if (output_param == ENIGMA_OUTPUT_SCALE) gfxCursor(57, 53, 30);
        if (output_param == ENIGMA_OUTPUT_ROOT) gfxCursor(115, 53, 12);
        if (output_param == ENIGMA_OUTPUT_MIDI_CH) gfxCursor(105, 63, 18);

        // Monitor
        gfxBitmap(56, 15, 8, AUDITION_ICON);
        gfxPrint(68, 15, assign_audition ? "Song" : "Library");
        gfxLine(48, 23, 123, 23);
    }

    void DrawSongInterface() {
        // Draw the left side, the selector
        for (byte line = 0; line < 4; line++)
        {
            byte y = 24 + (line * 10);
            byte t = (track_cursor + line) % 4;
            gfxPrint(3, y, "Track");
            gfxPrint(39, y, t + 1);
        }
        DrawSelectorBox("Track");

        // The right side is for editing
        // Track parameters
        gfxBitmap(56, 14, CLOCK_ICON);
        gfxPrint(68, 15, "/");
        gfxPrint(track[track_cursor].divide());

        gfxBitmap(98, 15, LOOP_ICON);
        gfxPrint(110, 15, track[track_cursor].loop() ? "On" : "Off");

        // Step parameters
        uint16_t ssi = track_step[edit_index]; // The song step index
        if (ssi < 0xffff) {
            gfxPrint(56 + pad(10, edit_index + 1), 25, edit_index + 1); // Step number (1-99)
            gfxPrint(": ");
            char name[4];
            HS::TuringMachine::SetName(name, song_step[ssi].tm());
            gfxPrint(name); // Turing machine name
            gfxPrint(" ");
            gfxPrint(pad(100, song_step[ssi].p()), song_step[ssi].p());
            gfxPrint("%");
            gfxPrint(80, 35, "x");
            gfxPrint(pad(10, song_step[ssi].repeats()), song_step[ssi].repeats()); // Number of times played
            gfxPrint("  ");
            gfxPrint(song_step[ssi].transpose() > -1 ? "+" : "");
            gfxPrint(song_step[ssi].transpose()); // Transpose
        }

        // Cursor
        if (step_param == ENIGMA_TRACK_DIVIDE) gfxCursor(74, 23, 12);
        if (step_param == ENIGMA_TRACK_LOOP) gfxCursor(110, 23, 18);
        if (step_param == ENIGMA_STEP_NUMBER) gfxCursor(56, 33, 12);
        if (step_param == ENIGMA_STEP_TM) gfxCursor(81, 33, 18);
        if (step_param == ENIGMA_STEP_P) gfxCursor(105, 33, 18);
        if (step_param == ENIGMA_STEP_REPEATS) gfxCursor(87, 43, 12);
        if (step_param == ENIGMA_STEP_TRANSPOSE) gfxCursor(117, 43, 12);

        // Draw the next two steps
        for (byte n = 0; n < 2; n++)
        {
            if (edit_index + n + 1 < 99) {
                uint16_t ssi = track_step[edit_index + n + 1];
                byte y = 45 + (10 * n);
                if (ssi < 0xffff) {
                    gfxPrint(56 + pad(10, edit_index + 2), y, edit_index + n + 2); // Step number (1-99)
                    gfxPrint(": ");
                    char name[4];
                    HS::TuringMachine::SetName(name, song_step[ssi].tm());
                    gfxPrint(name); // Turing machine name
                    gfxPrint(" x");
                    gfxPrint(song_step[ssi].repeats()); // Number of times played
                } else if (n == 0) {
                    gfxPrint(56 + pad(10, edit_index + 2), y, edit_index + n + 2); // Step number (1-99)
                    gfxPrint(": ");
                    gfxPrint(track[track_cursor].loop() ? "< Loop >" : "< Stop >");
                }
            }
        }
    }

    void DrawPlayInterface() {

    }

    /*
     *  Used by all Modes to allow selection of Primary Objects. This should go after the
     *  lines are drawn, because the first name on the list, as the selected object, is
     *  reversed
     */
    void DrawSelectorBox(const char* object) {
        gfxPrint(0, 15, object);
        gfxFrame(0, 23, 48, 40); // Selector window
        for (byte line = 0; line < 3; line++) gfxLine(0, 32 + (10 * line),  47, 32 + (10 * line));
        gfxInvert(1, 24, 46, 9);
    }

    // When a new TM is selected, load it here
    void SwitchTuringMachine(byte ix) {
        tm_state.Init(ix);
        tm_state.SetWriteMode(1); // This method is called from the Library, so set write access
        if (tm_state.IsFavorite()) tm_param = ENIGMA_TM_ROTATE;
    }

    //////// Parameter delegates
    void DelegateTMParam(int direction) {
        if (!tm_state.IsFavorite()) {
            if (tm_param == ENIGMA_TM_LENGTH) tm_state.ChangeLength(direction);
            if (tm_param == ENIGMA_TM_PROBABILITY)
                state_prob[tm_cursor] = constrain(state_prob[tm_cursor] + direction, 0, 100);
        }
        if (tm_param == ENIGMA_TM_ROTATE) tm_state.Rotate(direction);
    }

    void DelegateOutputParam(int direction) {
        if (output_param == ENIGMA_OUTPUT_TRACK) {
            if (output[output_cursor].track() > 0 || direction > 0)
                output[output_cursor].set_track(output[output_cursor].track() + direction);
        }
        if (output_param == ENIGMA_OUTPUT_TYPE) {
            if (output[output_cursor].type() > 0 || direction > 0)
                output[output_cursor].set_type(output[output_cursor].type() + direction);
        }
        if (output_param == ENIGMA_OUTPUT_SCALE) {
            if (output[output_cursor].scale() > 0 || direction > 0)
                output[output_cursor].set_scale(output[output_cursor].scale() + direction);
        }
        if (output_param == ENIGMA_OUTPUT_ROOT) {
            if (output[output_cursor].root() > 0 || direction > 0)
                output[output_cursor].set_root(output[output_cursor].root() + direction);
        }
        if (output_param == ENIGMA_OUTPUT_MIDI_CH) {
            if (output[output_cursor].midi_channel() > 0 || direction > 0)
                output[output_cursor].set_midi_channel(output[output_cursor].midi_channel() + direction);
        }
    }

    void DelegateStepParam(int direction) {
        // Track paramters
        if (step_param == ENIGMA_TRACK_DIVIDE) {
            if (track[track_cursor].divide() > 0 || direction > 0)
                track[track_cursor].set_divide(track[track_cursor].divide() + direction);
        }
        if (step_param == ENIGMA_TRACK_LOOP)
            track[track_cursor].set_loop(1 - track[track_cursor].loop());

        // Select a step
        if (step_param == ENIGMA_STEP_NUMBER) {
            if (edit_index > 0 || direction > 0) {
                // If there's a step to move into, move into it
                if (edit_index + direction < last_track_step_index)
                    if (track_step[edit_index + direction] != 0xffff) edit_index += direction;
            }
        }

        // Edit a step
        if (step_param > ENIGMA_STEP_NUMBER) {
            uint16_t ssi = track_step[edit_index]; // song step index
            if (step_param == ENIGMA_STEP_TM) {
                if (song_step[ssi].tm() > 0 || direction > 0)
                    song_step[ssi].set_tm(song_step[ssi].tm() + direction);
            }
            if (step_param == ENIGMA_STEP_P) {
                if (song_step[ssi].p() > 0 || direction > 0)
                    song_step[ssi].set_p(song_step[ssi].p() + direction);
            }
            if (step_param == ENIGMA_STEP_REPEATS) {
                if (song_step[ssi].repeats() > 0 || direction > 0)
                    song_step[ssi].set_repeats(song_step[ssi].repeats() + direction);
            }
            if (step_param == ENIGMA_STEP_TRANSPOSE) {
                if (song_step[ssi].transpose() > -48 || direction > 0)
                    song_step[ssi].set_transpose(song_step[ssi].transpose() + direction);
            }
        }
    }

    //////// Controllers
    void LibraryController() {
        // If a clock is present, advance the TuringMachineState
        if (Clock(0)) {
            uint16_t reg = tm_state.GetRegister();
            for (byte o = 0; o < 4; o++)
            {
                output[o].SendToDAC<EnigmaTMWS>(this, reg);
                //output[o].SentToMIDI<EnigmaTMWS>(this, reg);
            }
            tm_state.Advance(state_prob[tm_cursor]);
        }
    }

    void AssignController() {
        if (assign_audition) SongController();
        else LibraryController();
    }

    void SongController() {

    }

    //////// Data Collection
    void BuildTrackStepList(byte track) {
        byte ts_ix = 0;
        for (uint16_t ix = 0; ix < total_steps; ix++)
        {
            if (song_step[ix].track() == track) {
                // Found a step for the selected track; add it to the track list
                track_step[ts_ix++] = ix;
            }
        }
        last_track_step_index = ts_ix;

        // 0xffff indictates end of track
        while (ts_ix < 100) track_step[ts_ix++] = 0xffff;
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
    if (event ==  OC::APP_EVENT_RESUME) {
        EnigmaTMWS_instance.Resume();
    }
    if (event == OC::APP_EVENT_SUSPEND) {
        EnigmaTMWS_instance.OnSendSysEx();
    }
}

void EnigmaTMWS_loop() {} // Deprecated

void EnigmaTMWS_menu() {
    EnigmaTMWS_instance.BaseView();
}

void EnigmaTMWS_screensaver() {} // Deprecated

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
