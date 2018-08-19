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
//
// The Darkest Timeline
// CV Recorder/Sequencer

#include "util/util_settings.h"
#include "OC_DAC.h"
#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"
#include "SystemExclusiveHandler.h"

#define DT_LENGTH 64
#define DT_INDEX 65
#define DT_MIDI_CHANNEL 66
#define DT_SCALE 67
#define DARKEST_TIMELINE_SETTING_LAST 68

const int DT_SEQ_STEPS = 64;  // Total number of steps (CV + Probability timelines)
const int DT_TIMELINE_SIZE = 32; // Total size of each timeline
const int DT_TIMELINE_CV = 0; // For specifying a CV timeline
const int DT_TIMELINE_PROBABILITY = 1; // For specifying a Probability timeline
const int DT_VIEW_HEIGHT = 25; // Height, in pixels, of each view (top for CV, bottom for Probability)
const int DT_DATA_MAX = (12 << 7); // Highest value from ADC method used
const int DT_CURSOR_TICKS = 8000; // How many ticks that the cursor blinks on and off when recording
const int DT_TRIGGER_TICKS = 100; // How many ticks a trigger lasts (about 6ms)

class DarkestTimeline : public SystemExclusiveHandler,
    public settings::SettingsBase<DarkestTimeline, DARKEST_TIMELINE_SETTING_LAST> {
public:
    void Init() {
    		apply_value(DT_LENGTH, 16);
    		apply_value(DT_INDEX, 0);
    		apply_value(DT_MIDI_CHANNEL, 0); // MIDI Off
    		apply_value(DT_SCALE, 5); // semitones
    	    cursor = 0; // The sequence's record/play point
    	    clocked  = 0; // If 1, the sequencer needs to handle a clock input
    	    cv_record_enabled = 0; // When 1, CV will be recorded as the cursor moves
    	    prob_record_enabled = 0; // When 1, probability will be recorded as the cursor moves
    	    index_edit_enabled = 0; //  When 1, the right encoder changes the index instead of the length
    	    blink_countdown = DT_CURSOR_TICKS; // For record modes to blink the cursor
    	    trigger1_countdown = 0; // ISR cycles remaining for trigger 1
    	    trigger2_countdown = 0; // ISR cycles remaining for trigger 2
    	    respond_to_clock = 1; // If 1, allow the cursor to be moved with the digital ins
    	    last_cv_index = 0; // Allows a manual change to index stick around until changed by CV
    	    setup_cursor = 0; // Which parameter is being edited on the setup screen
    	    last_note[0] = -1;
    	    last_note[1] = -1;

    	    for (int i = 0; i < DT_SEQ_STEPS; i++) values_[i] = 0;

        quantizer.Init();
        quantizer.Configure(OC::Scales::GetScale(5), 0xffff);
    }

    void ISR() {
        ListenForSysEx();
    		UpdateInputState();
    		UpdateOutputState();
    		if (--blink_countdown < -DT_CURSOR_TICKS) blink_countdown = DT_CURSOR_TICKS;
    		if (--trigger1_countdown < 0) trigger1_countdown = 0;
    		if (--trigger2_countdown < 0) trigger2_countdown = 0;
    }

    void Resume() {
    		cursor = 0;
    		clocked = 0;
    		cv_record_enabled = 0;
    		prob_record_enabled = 0;
    		index_edit_enabled = 0;
    }

    void View() {
        if (setup_cursor) {
            DrawSetupScreen();
        } else {
            DrawCVView();
            DrawProbabilityView();
        }
    }

    void UpdateInputState() {
		int idx = data_index_at_position(0);
		int32_t adc_value;

    		// Update CV value if CV Record is enabled
		if (cv_record_enabled) {
			adc_value = OC::ADC::raw_pitch_value(ADC_CHANNEL_1);
			if (adc_value < 0) adc_value = 0; // Constrain CV to positive
			if (adc_value > DT_DATA_MAX) adc_value = DT_DATA_MAX;
			values_[idx] = static_cast<int>(adc_value);
		}

		// Update Probability value if Probability Record is enabled
		if (prob_record_enabled) {
			adc_value = OC::ADC::raw_pitch_value(ADC_CHANNEL_2);
			if (adc_value < 0) adc_value = 0; // Constrain probability to positive
            if (adc_value > DT_DATA_MAX) adc_value = DT_DATA_MAX;
			values_[idx + DT_TIMELINE_SIZE] = static_cast<int>(adc_value);
		}

		// Update Index value
		if (!index_edit_enabled) {
			adc_value = OC::ADC::raw_pitch_value(ADC_CHANNEL_3);
			if (adc_value < 180) adc_value = 0;
			int cv_index = adc_value / (DT_DATA_MAX / DT_TIMELINE_SIZE);
			if (cv_index != last_cv_index) {
				apply_value(DT_INDEX, cv_index);
				last_cv_index = cv_index;
			}
		}

		// Step forward on digital input 1
		if (OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_1>() && respond_to_clock) {
			move_cursor(1);
		}

		// Step backward on digital input 2
		if (OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_2>() && respond_to_clock) {
			move_cursor(-1);
		}

		// Reset on digital input 3
		if (OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_3>() && respond_to_clock) {
			cursor = 0;
			clocked = 1;
		}
    }

    void UpdateOutputState() {
    		int idx = data_index_at_position(0);

    		// Update normal CV value
        int32_t pitch = quantizer.Process(values_[idx], 0, 0);
    	    OC::DAC::set_pitch(DAC_CHANNEL_A, pitch, 0);

    	    // Update alternate universe timeline CV value
    	    int alternate_universe = (idx + length()) % DT_TIMELINE_SIZE;
        pitch = quantizer.Process(values_[alternate_universe], 0, 0);
    	    OC::DAC::set_pitch(DAC_CHANNEL_B, pitch, 0);

    	    if (clocked) { // Only do this once for each clock trigger
			// Check for probability-based normal trigger
			int prob = DT_DATA_MAX - values_[idx + DT_TIMELINE_SIZE];
			int comp = random(300, DT_DATA_MAX - 300);
			if (comp > prob) {
			    trigger1_countdown = DT_TRIGGER_TICKS;
			    if (last_note[0] > -1) usbMIDI.sendNoteOff(last_note[0], 0, last_channel[0]);
			    if (midi_channel()) {
                    last_channel[0] = midi_channel();
                    last_note[0] = MIDIQuantizer::NoteNumber(values_[idx]);
                    usbMIDI.sendNoteOn(last_note[0], 100, last_channel[0]);
                    last_length[0] = OC::CORE::ticks - last_clock[0];
                    last_clock[0] = OC::CORE::ticks;
			    }
			}

			// Check for probability-based alternate trigger (the probability is the complement of the first one)
			prob = values_[idx + DT_TIMELINE_SIZE];
			comp = random(300, DT_DATA_MAX - 300);
			if (comp > prob) {
			    trigger2_countdown = DT_TRIGGER_TICKS;
                if (last_note[1] > -1) usbMIDI.sendNoteOff(last_note[1], 0, last_channel[1]);
                if (midi_channel()) {
                    last_channel[1] = midi_channel() + 1;
                    last_note[1] = MIDIQuantizer::NoteNumber(values_[alternate_universe]);
                    usbMIDI.sendNoteOn(last_note[1], 100, last_channel[1]);
                    last_length[1] = OC::CORE::ticks - last_clock[1];
                    last_clock[1] = OC::CORE::ticks;
                }
			}

    	    		clocked = 0; // Reset and wait for next clock
    	    }

    	    // Update trigger CV values
		OC::DAC::set_pitch(DAC_CHANNEL_C, 1, (trigger1_countdown > 0) ? 5 : 0);
		OC::DAC::set_pitch(DAC_CHANNEL_D, 1, (trigger2_countdown > 0) ? 5 : 0);

		// Turn off notes that have been on for too long
		for (uint8_t ch = 0; ch < 2; ch++)
		{
		    if (last_note[ch] > -1 && (OC::CORE::ticks - last_clock[ch]) > (last_length[ch]) * 2) {
		        usbMIDI.sendNoteOff(last_note[ch], 0, last_channel[ch]);
		        last_note[ch] = -1;
		    }
		}
    }

    void ChangeValue(int direction) {
        if (setup_cursor == 0) change_value(DT_LENGTH, -direction);
        if (setup_cursor == 1) change_value(DT_MIDI_CHANNEL, direction);
        if (setup_cursor == 2) {
            change_value(DT_SCALE, direction);
            quantizer.Configure(OC::Scales::GetScale(scale()), 0xffff);
        }
    }

    void Navigate(int direction) {
    		if (index_edit_enabled) {
    			move_index(direction);
    		} else {
    			move_cursor(direction);
    		}
    }

    void ToggleRecord(int data_type) {
    		if (data_type == DT_TIMELINE_CV) cv_record_enabled = (1 - cv_record_enabled);
    		if (data_type == DT_TIMELINE_PROBABILITY) prob_record_enabled = (1 - prob_record_enabled);
    		if (data_type == DT_INDEX) index_edit_enabled = (1 - index_edit_enabled);
    }

	void TogglePlay() {
		respond_to_clock = (1 - respond_to_clock);
	}

    void ToggleSetupScreen() {
        if (++setup_cursor == 3) setup_cursor = 0;
    }

    void ClearTimeline(int timeline_type) {
    		int offset = (timeline_type == DT_TIMELINE_CV) ? 0 : DT_TIMELINE_SIZE;
    		for (int i = 0; i < DT_TIMELINE_SIZE; i++)
    		{
    			values_[i + offset] = 0;
    		}
    }

    void RandomizeTimeline() {
    		for (int i = 0; i < DT_SEQ_STEPS; i++)
    		{
    			values_[i] = random(0, DT_DATA_MAX);
    		}
    		apply_value(DT_INDEX, 0);
    		cursor = 0;
    }

    void DrawHeader() {
		graphics.setPrintPos(0, 0);
		graphics.print("Darkest Timeline   ");
		graphics.print(length());
        graphics.drawLine(0, 10, 127, 10);
    }

    void DrawView(int timeline_type) {
		int y_offset = (timeline_type == DT_TIMELINE_CV) ? 10 : 37;
		int data_offset = (timeline_type == DT_TIMELINE_CV) ? 0 : DT_TIMELINE_SIZE;
		for (int position = 0; position < length(); position++)
		{
			int idx = data_index_at_position(position);
			int data = values_[idx + data_offset]; // CV data will be proportioned for display
			bool is_recording = (timeline_type == DT_TIMELINE_CV) ? cv_record_enabled : prob_record_enabled;
			bool is_index = (idx == index()) ? 1 : 0;
			DrawColumn(position, y_offset, data, is_recording, is_index);
		}
    }

    void DrawCVView() {
    		DrawView(DT_TIMELINE_CV);
    }

    void DrawProbabilityView() {
		DrawView(DT_TIMELINE_PROBABILITY);
    }

    void DrawSetupScreen() {
        graphics.setPrintPos(0, 20);
        graphics.print("MIDI Out Channel:");
        graphics.print(midi_channels[values_[DT_MIDI_CHANNEL]]);
        if (setup_cursor == 1) graphics.drawFrame(0, 18, 127, 11);

        graphics.setPrintPos(0, 35);
        graphics.print("Scale:");
        graphics.print(OC::scale_names[values_[DT_SCALE]]);
        if (setup_cursor == 2) graphics.drawFrame(0, 33, 127, 11);
    }

    void DrawColumn(int position, int y_offset, int data, bool is_recording, bool is_index) {
    		int units_per_pixel = DT_DATA_MAX / DT_VIEW_HEIGHT;
    		int height = data / units_per_pixel;
    		if (height > 23) height = 23;
    		int width = (128 / length()) / 2;
    		int x_pos = (128 / length()) * position;
    		int x_offset = width / 2;
    		int y_pos = DT_VIEW_HEIGHT - height + y_offset; // Find the top
    		if (position == 0) { // The cursor position is at the leftmost column
    			graphics.drawRect(x_pos + x_offset, y_pos, width, height);
    			if (is_recording && blink_countdown > 0) {
    				// If in record mode, draw a full-sized blinking frame around the cursor
    				graphics.invertRect(x_pos, y_offset, width * 2, DT_VIEW_HEIGHT);
    			}
    		} else {
    			graphics.drawFrame(x_pos + x_offset, y_pos, width, height);
    		}

    		if (is_index) {
    			int start_y = 10;

    			if (index_edit_enabled) {
    				// Draw a solid blinking line if the index edit is enabled
    				if (blink_countdown < 0) graphics.drawLine(x_pos, start_y, x_pos, 63);
    			} else {
				for (int y = start_y; y < 63; y += 4)
				{
					// Check raw y for safety, as I keep playing with the numbers above.
					if (y + 2 < 63) graphics.drawLine(x_pos, y, x_pos, y + 1);
				}
    			}
		}
    }

    int length() {return values_[DT_LENGTH];}

    int index() {return values_[DT_INDEX];}

    int midi_channel() {return values_[DT_MIDI_CHANNEL];}

    int scale() {return values_[DT_SCALE];}

    void NotesOff() {
        for (uint8_t ch = 0; ch < 2; ch++)
        {
            if (last_note[ch] > -1) {
                usbMIDI.sendNoteOff(last_note[ch], 0, last_channel[ch]);
                last_note[ch] = -1;
            }
        }
    }

    /* When the app is suspended, it sends out a system exclusive dump, generated here */
    void OnSendSysEx() {
        uint8_t V[35];
        // Limit is 48 packed bytes, and there are 130 bytes that need packing. So wrap this
        // app's data up into four smaller sysex "pages" of 32 bytes each.
        for (int page = 0; page < 4; page++)
        {
            int ix = 0;
            V[ix++] = (char)page;
            V[ix++] = (uint8_t)values_[DT_LENGTH]; // Store length and index with each page
            V[ix++] = (uint8_t)values_[DT_INDEX];
            for (int b = 0; b < 16; b++)
            {
                V[ix++] = values_[(page * 16) + b] & 0xff; // Low byte
                V[ix++] = (values_[(page * 16) + b] >> 8) & 0xff; // High byte
            }
            UnpackedData unpacked;
            unpacked.set_data(ix, V);
            PackedData packed = unpacked.pack();
            SendSysEx(packed, 'D');
        }
    }

    void OnReceiveSysEx() {
        uint8_t V[35];
        if (ExtractSysExData(V, 'D')) {
            int ix = 0;
            int page = V[ix++];
            values_[DT_LENGTH] = V[ix++];
            values_[DT_INDEX] = V[ix++];
            for (int b = 0; b < 16; b++)
            {
                uint8_t low = V[ix++];
                uint8_t high = V[ix++];
                values_[(page * 16) + b] = (uint16_t)(high << 8) | low;
            }
        }
    }

private:
    int cursor;
    bool clocked;
    bool cv_record_enabled;
    bool prob_record_enabled;
    bool respond_to_clock;
    bool index_edit_enabled;
    int last_cv_index;
    uint8_t setup_cursor;

    // Keep track of MIDI events
    int last_note[2];
    int last_channel[2];
    uint32_t last_clock[2];
    uint32_t last_length[2];

    int blink_countdown;
    int trigger1_countdown;
    int trigger2_countdown;

    braids::Quantizer quantizer;

    /* Geez, don't even ask me to explain this. It's 11PM. (position + cursor) % length constrains the
	 * pattern to length steps. Then the index is added. Then the % DT_TIMELINE_SIZE constrains
	 * the pattern to the entire indexed pattern.
	 *
	 * Note that this is the data index for the CV timeline. For the probability timeline, add
	 * DT_TIMELINE_SIZE to the returned value.
	 */
    int data_index_at_position(int position) {
    		int idx = (((position + cursor) % length()) + index()) % DT_TIMELINE_SIZE;
    		return idx;
    }

    int move_cursor(int direction) {
    		cursor += direction;
    		if (cursor < 0) cursor = length() - 1;
    		if (cursor >= length()) cursor = 0;
    		clocked = 1;
    		return cursor;
    }

    int move_index(int direction) {
    		values_[DT_INDEX] += direction;
    		if (index() < 0) values_[DT_INDEX] = DT_TIMELINE_SIZE - 1;
    		if (index() >= DT_TIMELINE_SIZE) values_[DT_INDEX] = 0;
    		return values_[DT_INDEX];
    }
};

#define DARKEST_TIMELINE_VALUE_ATTRIBUTE {0, 0, 8192, "step", NULL, settings::STORAGE_TYPE_U16},
#define DO_SIXTEEN_TIMES(A) A A A A A A A A A A A A A A A A
SETTINGS_DECLARE(DarkestTimeline, DARKEST_TIMELINE_SETTING_LAST) {
	DO_SIXTEEN_TIMES(DARKEST_TIMELINE_VALUE_ATTRIBUTE)
	DO_SIXTEEN_TIMES(DARKEST_TIMELINE_VALUE_ATTRIBUTE)
	DO_SIXTEEN_TIMES(DARKEST_TIMELINE_VALUE_ATTRIBUTE)
	DO_SIXTEEN_TIMES(DARKEST_TIMELINE_VALUE_ATTRIBUTE)
	{16, 1, DT_TIMELINE_SIZE, "length", NULL, settings::STORAGE_TYPE_U8},
	{0, 0, DT_TIMELINE_SIZE - 1, "index", NULL, settings::STORAGE_TYPE_U8},
    {0, 0, 16, "midi channel", NULL, settings::STORAGE_TYPE_U8},
    {5, 0, OC::Scales::NUM_SCALES - 1, "scale", NULL, settings::STORAGE_TYPE_U8}
};

DarkestTimeline timeline_instance;

// App stubs
void DARKESTTIMELINE_init() {
  timeline_instance.Init();
}

size_t DARKESTTIMELINE_storageSize() {
	return DarkestTimeline::storageSize();
}

size_t DARKESTTIMELINE_save(void *storage) {
	return timeline_instance.Save(storage);
}

size_t DARKESTTIMELINE_restore(const void *storage) {
	return timeline_instance.Restore(storage);
}

void DARKESTTIMELINE_isr() {
	return timeline_instance.ISR();
}

void DARKESTTIMELINE_handleAppEvent(OC::AppEvent event) {
    if (event ==  OC::APP_EVENT_RESUME) {
    		timeline_instance.Resume();
    }
    if (event == OC::APP_EVENT_SUSPEND) {
        timeline_instance.NotesOff();
        timeline_instance.OnSendSysEx();
    }
}

void DARKESTTIMELINE_loop() {
}

void DARKESTTIMELINE_menu() {
	timeline_instance.DrawHeader();
	timeline_instance.View();
}

void DARKESTTIMELINE_screensaver() {
    timeline_instance.DrawCVView();
    timeline_instance.DrawProbabilityView();
}

void DARKESTTIMELINE_handleButtonEvent(const UI::Event &event) {

	if (event.control == OC::CONTROL_BUTTON_UP) {
		if (event.type == UI::EVENT_BUTTON_PRESS) timeline_instance.ToggleRecord(DT_TIMELINE_CV);
	}

	if (event.control == OC::CONTROL_BUTTON_DOWN) {
		if (event.type == UI::EVENT_BUTTON_PRESS) timeline_instance.ToggleRecord(DT_TIMELINE_PROBABILITY);
		if (event.type == UI::EVENT_BUTTON_LONG_PRESS) {
			timeline_instance.ClearTimeline(DT_TIMELINE_CV);
			timeline_instance.ClearTimeline(DT_TIMELINE_PROBABILITY);
		}
	}

	if (event.control == OC::CONTROL_BUTTON_R) {
		timeline_instance.ToggleRecord(DT_INDEX);
	}

	if (event.control == OC::CONTROL_BUTTON_L) {
		if (event.type == UI::EVENT_BUTTON_LONG_PRESS) {
			timeline_instance.RandomizeTimeline();
		} else {
		    timeline_instance.ToggleSetupScreen();
		}
	}
}

void DARKESTTIMELINE_handleEncoderEvent(const UI::Event &event) {
	if (event.control == OC::CONTROL_ENCODER_L) {
		timeline_instance.ChangeValue(event.value);
	}

	if (event.control == OC::CONTROL_ENCODER_R) {
		timeline_instance.Navigate(event.value);
	}
}

void FillValueAttributes(settings::value_attr attributes[]) {
	for (int i = 0; i < DT_SEQ_STEPS; i++)
	{
		settings::value_attr tmp = {0, 0, 65536, "seq", NULL, settings::STORAGE_TYPE_U16};
		attributes[i] = tmp;
	}
}
