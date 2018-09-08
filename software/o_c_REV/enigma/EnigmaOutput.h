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

#ifndef ENIGMAOUTPUT_H
#define ENIGMAOUTPUT_H

#include "../braids_quantizer.h"
#include "../braids_quantizer_scales.h"
#include "../OC_scales.h"

enum EnigmaOutputType {
    NOTE3,
    NOTE4,
    NOTE5,
    NOTE6,
    NOTE7,
    MODULATION,
    TRIGGER,
    GATE,
};
const char* enigma_type_names[] = {"Note 3-Bit", "Note 4-Bit", "Note 5-Bit", "Note 6-Bit", "Note 7-Bit", "Modulation", "Trigger", "Gate"};

class EnigmaOutput {
private:
    // Saved data
    byte tk; // Track 0-4
    byte ty; // Type (see enum above)
    byte sc; // Scale
    byte sr; // Scale root
    byte mc; // MIDI channel (1-16, 0=off)

    // Internal data
    byte out; // Output 0-4
    braids::Quantizer quantizer;

public:
    void InitAs(byte o) {
        out = o;
        tk = o;
        sc = 5;
        sr = 0;
        mc = 0;

        // Initialize quantizer
        quantizer.Init();
        quantizer.Configure(OC::Scales::GetScale(sc), 0xffff);

        // Set type based on output initialization
        if (o == 0) set_type(EnigmaOutputType::NOTE5);
        if (o == 1) set_type(EnigmaOutputType::MODULATION);
        if (o == 2) set_type(EnigmaOutputType::TRIGGER);
        if (o == 3) set_type(EnigmaOutputType::GATE);
    }

    // Getters
    byte output() {return out;}
    byte track() {return tk;}
    byte type() {return ty;}
    byte scale() {return sc;}
    byte root() {return sr;}
    byte midi_channel() {return mc;}

    // Setters
    void set_output(byte output_) {out = constrain(output_, 0, 3);}
    void set_track(byte track_) {tk = constrain(track_, 0, 3);}
    void set_type(byte type_) {ty = constrain(type_, 0, EnigmaOutputType::GATE);}
    void set_scale(byte scale_) {
        sc = constrain(scale_, 0, OC::Scales::NUM_SCALES - 1);
        quantizer.Init();
        quantizer.Configure(OC::Scales::GetScale(sc), 0xffff);
    }
    void set_root(byte root_) {sr = constrain(root_, 0, 11);}
    void set_midi_channel(byte midi_channel_) {mc = constrain(midi_channel_, 0, 16);}

    /* Sends data to an output based on the current output type. The I/O methods
     * are public methods of the app, so make sure to send a type that supports
     * Out, ClockOut, and GateOut (i.e. HSApplication, or HemisphereApplet)
     */
    template <class C>
    void SendToDAC(C *app, uint16_t reg) {

        // Quantize a note based on how many bits
        if (ty <= EnigmaOutputType::NOTE7) {
            byte bits = ty + 3; // Number of bits
            uint8_t mask = 0;
            for (byte s = 0; s < bits; s++) mask |= (0x01 << s);
            int note_shift = 0;
            if (ty == EnigmaOutputType::NOTE6) note_shift = 36;
            if (ty == EnigmaOutputType::NOTE5) note_shift = 48;
            if (ty <= EnigmaOutputType::NOTE4) note_shift = 60;
            byte note_number = (reg & mask) + note_shift;
            app->Out(out, quantizer.Lookup(note_number));
        }

        // Modulation based on low 8 bits
        if (ty == EnigmaOutputType::MODULATION) {
            app->Out(out, (reg & 0x00ff) * 6);
        }

        // Trigger sends a clock when low bit is high
        bool clock = reg & 0x0001;
        if (ty == EnigmaOutputType::TRIGGER && clock) {
            app->ClockOut(out);
        }

        // Gate goes high or low based on the low bit, and stays there until changed
        if (ty == EnigmaOutputType::GATE) {
            app->GateOut(out, clock);
        }
    }
};

#endif // ENIGMAOUTPUT_H
