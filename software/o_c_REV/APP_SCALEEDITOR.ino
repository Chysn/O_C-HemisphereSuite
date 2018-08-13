#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"
#include "HSApplication.h"
#include "SystemExclusiveHandler.h"
#include "SegmentDisplay.h"

class ScaleEditor : public HSApplication, public SystemExclusiveHandler {
public:
	void Start() {
	    current_scale = 0;
	    current_note = 0;
        quantizer.Configure(OC::Scales::GetScale(current_scale), 0xffff);
        current_import_scale = 5;
        undo_value = OC::user_scales[current_scale].notes[0];
        octave = 1;
        QuantizeCurrent();
        segment.Init();
	}

    void Controller() {
        ListenForSysEx();

        // Scale monitor
        int32_t pitch = In(0);
        int32_t quantized = quantizer.Process(pitch, 0, 0);
        Out(0, quantized);

        // Current note monitor
        Out(1, current_quantized);
    }

    void View() {
        gfxHeader("Scale Editor    ");
        gfxPrint(OC::scale_names_short[current_scale]);
        if (import_mode) DrawImportScreen();
        else DrawInterface();
    }

    void ScreensaverView() {
        View();
    }

    /* Send SysEx on app suspend and when the left encoder is pressed */
    void OnSendSysEx() { // Left Enc Push
        uint8_t V[35];
        int ix = 0;

        // Encode span
        uint16_t span = static_cast<uint16_t>(OC::user_scales[current_scale].span);
        V[ix++] = span & 0xff; // Low byte
        V[ix++] = (span >> 8) & 0xff; // High byte

        // Encode length
        V[ix++] = static_cast<uint8_t>(OC::user_scales[current_scale].num_notes);

        // Encode values
        for (int i = 0; i < 16; i++)
        {
            uint16_t note = static_cast<uint16_t>(OC::user_scales[current_scale].notes[i]);
            V[ix++] = note & 0xff; // Low
            V[ix++] = (note >> 8) & 0xff; // High
        }

        UnpackedData unpacked;
        unpacked.set_data(ix, V);
        PackedData packed = unpacked.pack();
        SendSysEx(packed, 'E');
    }

    /* Send SysEx on app suspend and when the left encoder is pressed */
    void OnReceiveSysEx() {
        uint8_t V[35];
        if (ExtractSysExData(V, 'E')) {
            int ix = 0;

            // Decode span
            uint8_t low = V[ix++];
            uint8_t high = V[ix++];
            OC::user_scales[current_scale].span = (int16_t)(high << 8) | low;

            // Decode length
            OC::user_scales[current_scale].num_notes = V[ix++];

            // Decode values
            for (int i = 0; i < 16; i++)
            {
                uint8_t low = V[ix++];
                uint8_t high = V[ix++];
                OC::user_scales[current_scale].notes[i] = (uint16_t)(high << 8) | low;
            }
        }
    }

    /////////////////////////////////////////////////////////////////
    // Control handlers
    /////////////////////////////////////////////////////////////////
    void OnLeftButtonPress() {
        Undo();
    }

    void OnLeftButtonLongPress() {
        if (import_mode) ToggleImportMode();
        else OnSendSysEx();
    }

    void OnRightButtonPress() {
        if (import_mode) ImportScale();
        else ToggleLengthSet();
    }

    void OnUpButtonPress() {
        SwitchScale(1);
    }

    void OnDownButtonPress() {
        SwitchScale(-1);
    }

    void OnDownButtonLongPress() {
        ToggleImportMode();
    }

    void OnLeftEncoderMove(int direction) {
        if (!import_mode) ChangeValue(direction);
    }

    void OnRightEncoderMove(int direction) {
        if (import_mode) ChangeImport(direction);
        else if (length_set_mode) ChangeLength(direction);
        else ChangeNote(direction);
    }


private:
    int8_t cursor;
    int8_t current_scale;
    int8_t current_note;
    int8_t current_import_scale;
    int undo_value;
    bool length_set_mode;
    bool import_mode;
    braids::Quantizer quantizer;
    int current_quantized;
    int octave;
    SegmentDisplay segment;

    void DrawInterface() {
        // Start by drawing rectangles representing the scale notes
        // Between 4-16 steps for each user scale. I'll make them 6 pixels wide each with
        // a one pixel gutter on each side. It'll be centered, so the x offset will be
        uint8_t offset_x = (128 - (8 * OC::user_scales[current_scale].num_notes)) / 2;
        const uint8_t note_height = 12;
        uint8_t join_x = 0; // The currently-selected note will be joined to the editor window at this X position
        for (uint8_t n = 0; n < OC::user_scales[current_scale].num_notes; n++)
        {
            if (length_set_mode) {
                if (n < OC::user_scales[current_scale].num_notes - 1 || CursorBlink()) gfxFrame(1 + offset_x + (8 * n), 15, 6, note_height);
            } else {
                if (n == current_note) {
                    join_x = 3 + offset_x + (8 * n);
                    gfxRect(1 + offset_x + (8 * n), 15, 6, note_height);
                } else gfxFrame(1 + offset_x + (8 * n), 15, 6, note_height);
            }
        }

        // Draw the joiner line from the current note to the value editor
        if (join_x > 0) {
            // Vertical line from the note box down
            gfxLine(join_x, 15 + note_height, join_x, 15 + note_height + 8);
            gfxLine(join_x + 1, 15 + note_height, join_x + 1, 15 + note_height + 8);

            // Horizontal line to the center
            gfxLine(join_x, 15 + note_height + 8, 64, 15 + note_height + 8);

            // Center line down to the value editor
            gfxLine(63, 15 + note_height + 8, 63, 15 + note_height + 16);
            gfxLine(64, 15 + note_height + 8, 64, 15 + note_height + 16);
            uint8_t value_editor_y = 38 + note_height;

            // Show the value
            uint32_t note_value = OC::user_scales[current_scale].notes[current_note];
            uint32_t cents = (note_value * 100) >> 7;
            uint32_t frac_cents = ((note_value * 100000) >> 7) - cents * 1000;

            segment.Print(12, value_editor_y, cents);
            segment.DecimalPoint(1000); // Go to 3 places after the decimal
            segment.Print(frac_cents);
        }

        // Show the length if it's length set mode
        if (length_set_mode) {
            gfxPrint(40, 40 + note_height, OC::user_scales[current_scale].num_notes);
            gfxPrint(" Notes");
        }
    }

    void DrawImportScreen() {
        gfxPrint(0, 15, "Import");
        gfxPrint(0, 35, OC::scale_names[current_import_scale]);
        gfxCursor(0, 43, 127);

        gfxPrint(0, 55, "[CANCEL]");
        gfxPrint(78, 55, "[IMPORT]");
    }

    /////////////////////////////////////////////////////////////////
    // Control handler activities
    /////////////////////////////////////////////////////////////////
    void SwitchScale(int direction) { // Up/Down buttons
        current_scale = constrain(current_scale + direction, 0, OC::Scales::SCALE_USER_LAST - 1);

        // Configure and force requantize for real-time monitoring purposes
        quantizer.Configure(OC::Scales::GetScale(current_scale), 0xffff);
        QuantizeCurrent();

        uint8_t length = static_cast<uint8_t>(OC::user_scales[current_scale].num_notes);
        if (current_note > length - 1) current_note = OC::user_scales[current_scale].num_notes - 1;
        undo_value = OC::user_scales[current_scale].notes[current_note];
    }

    /* Switch length set mode on and off when the right encoder is pressed */
    void ToggleLengthSet() { // Right Push
        length_set_mode = 1 - length_set_mode;
        ResetCursor();
    }

    void ToggleImportMode() {
        import_mode = 1 - import_mode;
    }

    void ChangeNote(int direction) {
        int length = OC::user_scales[current_scale].num_notes;
        current_note += direction;
        if (current_note < 0) {
            if (octave > -1) current_note = length - 1;
            else current_note += 1;
            octave = constrain(octave - 1, -1, 4);
        }
        if (current_note == length) {
            if (octave < 4) current_note = 0;
            else current_note -= 1;
            octave = constrain(octave + 1, -1, 4);
        }
        undo_value = OC::user_scales[current_scale].notes[current_note];
        QuantizeCurrent();
    }

    void ChangeImport(int direction) {
        current_import_scale += direction;
        if (current_import_scale == OC::Scales::NUM_SCALES) current_import_scale = 0;
        if (current_import_scale < 0) current_import_scale = OC::Scales::NUM_SCALES - 1;
    }

    void ChangeLength(int direction) {
        int length = OC::user_scales[current_scale].num_notes + direction;
        length = constrain(length, 4, 16);
        OC::user_scales[current_scale].num_notes = length;
        if (current_note > (length - 1)) current_note = length - 1;

        // Configure and force requantize for real-time monitoring purposes
        quantizer.Configure(OC::Scales::GetScale(current_scale), 0xffff);
        QuantizeCurrent();
    }

    void ChangeValue(int direction) {
        int16_t new_value = OC::user_scales[current_scale].notes[current_note] + direction;

        // Is this constraint necessary? What if you want the scale to go in reverse, or want it to be
        // otherwise out-of-order? Heck with this, I say, let the user decide.
        //
        // But, yeah, this is necessary, it turns out. The note stops working right if the one above it is
        // at a higher pitch. I haven't looked into why, but it's probably in the Braids quantizer code somewhere
        int length = OC::user_scales[current_scale].num_notes;
        const int32_t min = current_note > 0 ? OC::user_scales[current_scale].notes[current_note - 1] : 0;
        const int32_t max = current_note < length - 1 ? OC::user_scales[current_scale].notes[current_note + 1] : OC::user_scales[current_scale].span;
        if (new_value <= min) new_value = current_note > 0 ? min + 1 : 0;
        if (new_value >= max) new_value = max - 1;
        OC::user_scales[current_scale].notes[current_note] = new_value;

        // Configure and force requantize for real-time monitoring purposes
        quantizer.Configure(OC::Scales::GetScale(current_scale), 0xffff);
        QuantizeCurrent();
    }

    void ImportScale() {
        OC::Scale source = OC::Scales::GetScale(current_import_scale);
        memcpy(&OC::user_scales[current_scale], &source, sizeof(source));
        quantizer.Configure(OC::Scales::GetScale(current_scale), 0xffff);
        QuantizeCurrent();
        import_mode = 0;
        undo_value = OC::user_scales[current_scale].notes[current_note];
    }

    void Undo() {
        OC::user_scales[current_scale].notes[current_note] = static_cast<int16_t>(undo_value);
        QuantizeCurrent();
    }

    void QuantizeCurrent() {
        int transpose = OC::user_scales[current_scale].span * octave;
        quantizer.Requantize();
        current_quantized = quantizer.Process(OC::user_scales[current_scale].notes[current_note] + transpose, 0, 0);
        quantizer.Requantize(); // This is for the next one in the Controller
    }
};

ScaleEditor scale_editor_instance;

// App stubs
void SCALEEDITOR_init() {
    scale_editor_instance.BaseStart();
}

// Not using O_C Storage
size_t SCALEEDITOR_storageSize() {return 0;}
size_t SCALEEDITOR_save(void *storage) {return 0;}
size_t SCALEEDITOR_restore(const void *storage) {return 0;}

void SCALEEDITOR_isr() {
	return scale_editor_instance.BaseController();
}

void SCALEEDITOR_handleAppEvent(OC::AppEvent event) {
    if (event == OC::APP_EVENT_SUSPEND) {
        scale_editor_instance.OnSendSysEx();
    }
}

void SCALEEDITOR_loop() {}

void SCALEEDITOR_menu() {
    scale_editor_instance.BaseView();
}

void SCALEEDITOR_screensaver() {
    scale_editor_instance.BaseScreensaverView();
}

void SCALEEDITOR_handleButtonEvent(const UI::Event &event) {
    // For left encoder, handle press and long press
    if (event.control == OC::CONTROL_BUTTON_L) {
        if (event.type == UI::EVENT_BUTTON_LONG_PRESS) scale_editor_instance.OnLeftButtonLongPress();
        else scale_editor_instance.OnLeftButtonPress();
    }

    // For right encoder, only handle press (long press is reserved)
    if (event.control == OC::CONTROL_BUTTON_R && event.type == UI::EVENT_BUTTON_PRESS) scale_editor_instance.OnRightButtonPress();

    // For up button, handle only press (long press is reserved)
    if (event.control == OC::CONTROL_BUTTON_UP) scale_editor_instance.OnUpButtonPress();

    // For down button, handle press and long press
    if (event.control == OC::CONTROL_BUTTON_DOWN) {
        if (event.type == UI::EVENT_BUTTON_PRESS) scale_editor_instance.OnDownButtonPress();
        if (event.type == UI::EVENT_BUTTON_LONG_PRESS) scale_editor_instance.OnDownButtonLongPress();
    }
}

void SCALEEDITOR_handleEncoderEvent(const UI::Event &event) {
    // Left encoder turned
    if (event.control == OC::CONTROL_ENCODER_L) scale_editor_instance.OnLeftEncoderMove(event.value);

    // Right encoder turned
    if (event.control == OC::CONTROL_ENCODER_R) scale_editor_instance.OnRightEncoderMove(event.value);
}
