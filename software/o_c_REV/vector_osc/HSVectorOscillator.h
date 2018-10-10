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

#ifndef HS_VECTOR_OSCILLATOR
#define HS_VECTOR_OSCILLATOR

namespace HS {

const byte VO_SEGMENT_COUNT = 64; // The total number of segments in user memory
const byte VO_MAX_SEGMENTS = 12; // The maximum number of segments in a waveform

/*
 * The VOSegment is a single segment of the VectorOscillator that specifies a target
 * level and relative time.
 *
 * A waveform is constructed of two or more VOSegments.
 *
 */
struct VOSegment {
    byte level;
    byte time;

    bool IsTOC() {return (time == 0xff && level > 0);}
    void SetTOC(byte segments) {
        time = 0xff;
        level = segments;
    }

    /* If this is a TOC segment, Segments() will return how many segments are in the waveform */
    byte Segments() {return level;}

};

VOSegment user_waveforms[VO_SEGMENT_COUNT];

}; // namespace HS

#define int2signal(x) (x << 10)
#define signal2int(x) (x >> 10)
typedef int32_t vosignal_t;

enum {
    VO_TRIANGLE,
    VO_SAWTOOTH,
    VO_SQUARE,
    VO_NUMBER_OF_WAVEFORMS
};

typedef HS::VOSegment VOSegment;

class VectorOscillator {
public:
    int Diagnostic() {
        return diag;
    }

    /* Oscillator defaults to cycling. Turn off cycling for EGs, etc */
    void Cycle(bool cycle_ = 1) {cycle = cycle_;}

    /* Oscillator defaults to non-sustaining. Turing on for EGs, etc. */
    void Sustain(bool sustain_ = 1) {sustain = sustain_;}

    /* Move to the release stage after sustain */
    void Release() {
        segment_index = segment_count - 1;
        rise = calculate_rise(segment_index);
        sustained = 0;
    }

    /* The offset amount will be added to each voltage output */
    void Offset(int32_t offset_) {offset = offset_;}

    /* Add a new segment to the end */
    void SetSegment(HS::VOSegment segment) {
        if (segment_count < HS::VO_MAX_SEGMENTS) {
            memcpy(&segments[segment_count], &segment, sizeof(segments[segment_count]));
            total_time += segments[segment_count].time;
            segment_count++;
        }
    }

    /* Update an existing segment */
    void SetSegment(byte ix, HS::VOSegment segment) {
        ix = constrain(ix, 0, segment_count - 1);
        total_time -= segments[ix].time;
        memcpy(&segments[ix], &segment, sizeof(segments[ix]));
        total_time += segments[ix].time;
        if (ix == segment_count) segment_count++;
    }

    HS::VOSegment GetSegment(byte ix) {
        ix = constrain(ix, 0, segment_count - 1);
        return segments[ix];
    }

    void SetScale(uint16_t scale_) {scale = scale_;}

    /* frequency is centihertz (e.g., 440 Hz is 44000) */
    void SetFrequency(uint32_t frequency_) {
        frequency = frequency_;
        rise = calculate_rise(segment_index);
    }

    bool GetEOC() {return eoc;}

    byte TotalTime() {return total_time;}

    byte SegmentCount() {return segment_count;}

    void Start() {
        eoc = 0;
        Reset();
    }

    void Reset() {
        segment_index = 0;
        signal = scale_level(segments[segment_count - 1].level);
        rise = calculate_rise(segment_index);
        sustained = 0;
    }

    int32_t Next() {
        if (!sustained) { // Observe sustain state
            if (!eoc || cycle) { // Observe cycle setting
                eoc = 0;
                if (validate()) {
                    if (rise) {
                        signal += rise;
                        if (rise >= 0 && signal >= target) advance_segment();
                        if (rise < 0 && signal <= target) advance_segment();
                    } else {
                        if (countdown) {
                            --countdown;
                            if (countdown == 0) advance_segment();
                        }
                    }
                }
            }
        }
        return signal2int(signal) + offset;
    }

    /* Some simple waveform presets */
    void SetWaveform(byte waveform) {
        switch (waveform) {
            case VO_TRIANGLE:
                SetSegment(VOSegment {255,1});
                SetSegment(VOSegment {0,1});
                break;

            case VO_SAWTOOTH:
                SetSegment(VOSegment {255, 0});
                SetSegment(VOSegment {0,1});
                break;

            case VO_SQUARE:
                SetSegment(VOSegment {255,0});
                SetSegment(VOSegment {255,1});
                SetSegment(VOSegment {0,0});
                SetSegment(VOSegment {0,1});
                break;
        }
    }

private:
    VOSegment segments[12]; // Array of segments in this Oscillator
    byte segment_count = 0; // Number of segments
    int total_time = 0; // Sum of time values for all segments
    vosignal_t signal = 0; // Current scaled signal << 10 for more precision
    vosignal_t target = 0; // Target scaled signal. When the target is reached, the Oscillator moves to the next segment.
    bool eoc = 1; // The most recent tick's next() read was the end of a cycle
    byte segment_index = 0; // Which segment the Oscillator is currently traversing
    vosignal_t rise; // The amount (per tick) the signal must rise to reach the target
    uint32_t frequency; // In centihertz
    uint16_t scale; // The maximum (and minimum negative) output for this Oscillator
    uint32_t countdown; // Ticks left for a segment with a rise of 0
    bool cycle = 1; // Waveform will cycle
    int32_t offset = 0; // Amount added to each voltage output (e.g., to make it unipolar)
    bool sustain = 0; // Waveform stops when it reaches the end of the penultimate stage
    bool sustained = 0; // Current state of sustain. Only active when sustain = 1

    int diag = 0; // Diagnostic

    /*
     * The Oscillator can only oscillate if the following conditions are true:
     *     (1) The frequency must be greater than 0
     *     (2) There must be more than one steps
     *     (3) The total time must be greater than 0
     *     (4) The scale is greater than 0
     */
    bool validate() {
        bool valid = 1;
        if (frequency == 0) valid = 0;
        if (segment_count < 2) valid = 0;
        if (total_time == 0) valid = 0;
        if (scale == 0) valid = 0;
        return valid;
    }

    int32_t Proportion(int numerator, int denominator, int max_value) {
        vosignal_t proportion = int2signal((int32_t)numerator) / (int32_t)denominator;
        int32_t scaled = signal2int(proportion * max_value);
        return scaled;
    }

    /*
     * Provide a signal value based on a segment level. The segment level is internally
     * 0-255, and this is converted to a bipolar value by subtracting 128.
     */
    vosignal_t scale_level(byte level) {
        int b_level = constrain(level, 0, 255) - 128;
        int scaled = Proportion(b_level, 127, scale);
        vosignal_t scaled_level = int2signal(scaled);
        return scaled_level;
    }

    void advance_segment() {
        if (sustain && segment_index == segment_count - 2) {
            sustained = 1;
        } else {
            if (++segment_index >= segment_count) {
                if (cycle) Reset();
                eoc = 1;
            } else rise = calculate_rise(segment_index);
            sustained = 0;
        }
    }

    vosignal_t calculate_rise(byte ix) {
        // Determine the target level for this segment
        byte level = segments[ix].level;
        int time = static_cast<uint32_t>(segments[ix].time);
        target = scale_level(level);

        // Determine the starting level of this segment to get the total segment rise
        /* TODO: Probably remove this after testing
        if (ix > 0) ix--;
        else ix = segment_count - 1;
        level = segments[ix].level;
        vosignal_t starting = scale_level(level);
        */

        // How many ticks should a complete cycle last? cycle_ticks is 10 times that number.
        int32_t cycle_ticks = 16666667 / frequency;

        // How many ticks should the current segment last?
        int32_t segment_ticks = Proportion(time, total_time, cycle_ticks);
        diag = segment_ticks;

        // The total difference between the target and the current signal, divided by how many ticks
        // it should take to get there, is the rise. The / 10 is to cancel the extra precision
        // from the previous two calculations.
        vosignal_t new_rise = 0;
        if (segment_ticks > 0) {
            new_rise = ((target - signal) * 10) / segment_ticks;
            if (new_rise == 0) countdown = segment_ticks / 10;
        } else {
            signal = target;
            countdown = 1;
        }

        return new_rise;
    }
};

#endif // HS_VECTOR_OSCILLATOR
