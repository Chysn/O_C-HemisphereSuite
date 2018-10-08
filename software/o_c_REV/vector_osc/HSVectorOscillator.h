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
    void Cycle(bool cycle_) {cycle = cycle_;}

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
    }

    int32_t Next() {
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
        return signal2int(signal);
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
        if (++segment_index >= segment_count) {
            if (cycle) Reset();
            eoc = 1;
        } else rise = calculate_rise(segment_index);
    }

    vosignal_t calculate_rise(byte ix) {
        // Determine the target level for this segment
        byte level = segments[ix].level;
        int time = static_cast<uint32_t>(segments[ix].time);
        target = scale_level(level);

        // Determine the starting level of this segment to get the total segment rise
        if (ix > 0) ix--;
        else ix = segment_count - 1;
        level = segments[ix].level;
        vosignal_t starting = scale_level(level);

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
            new_rise = ((target - starting) * 10) / segment_ticks;
            if (new_rise == 0) countdown = segment_ticks / 10;
        } else {
            signal = target;
            countdown = 1;
        }

        return new_rise;
    }
};

class WaveformManager {
public:
    /*
     * The segment at user_waveforms[0] should have a level byte of 0xfc, and the time
     * byte should have a value of 0xe2. This indicates that the memory is set up for
     * segment storage. If Validate() is false, then Setup() should be executed.
     */
    bool static Validate() {
        return (HS::user_waveforms[0].level == 0xfc && HS::user_waveforms[0].time == 0xe2);
    }

    /* Add a triangle and sawtooth waveform */
    void static Setup() {
        HS::user_waveforms[0] = VOSegment {0xfc, 0xe2};
        HS::user_waveforms[1] = VOSegment {0x02, 0xff}; // TOC entry: 2 steps
        HS::user_waveforms[2] = VOSegment {0xff, 0x01}; // First segment of triangle
        HS::user_waveforms[3] = VOSegment {0x00, 0x01}; // Second segment of triangle
        HS::user_waveforms[4] = VOSegment {0x02, 0xff}; // TOC entry: 2 steps
        HS::user_waveforms[5] = VOSegment {0xff, 0x00}; // First segment of sawtooth
        HS::user_waveforms[6] = VOSegment {0x00, 0x01}; // Second segment of sawtooth
        for (byte i = 7; i < 64; i++) HS::user_waveforms[i] = VOSegment {0x00, 0xff};
    }

    byte static WaveformCount() {
        byte count = 0;
        for (byte i = 0; i < HS::VO_SEGMENT_COUNT; i++)
        {
            if (HS::user_waveforms[i].IsTOC()) count++;
        }
        return count;
    }

    byte static SegmentsRemaining() {
        byte segment_count = 1; // Include validation segment
        for (byte i = 0; i < HS::VO_SEGMENT_COUNT; i++)
        {
            if (HS::user_waveforms[i].IsTOC()) {
                segment_count += HS::user_waveforms[i].Segments();
            }
        }
        return (64 - segment_count);
    }

    VectorOscillator static VectorOscillatorFromWaveform(byte waveform_number) {
        VectorOscillator osc;
        byte count = 0;
        for (byte i = 0; i < HS::VO_SEGMENT_COUNT; i++)
        {
            if (HS::user_waveforms[i].IsTOC()) {
                if (count == waveform_number) {
                    for (int s = 0; s < HS::user_waveforms[i].Segments(); s++)
                    {
                        osc.SetSegment(HS::user_waveforms[i + s + 1]);
                    }
                    break;
                }
                count++;
            }
        }
        return osc;
    }

    byte static GetSegmentIndex(byte waveform_number, byte segment_number, int8_t direction = 0) {
        byte count = 0;
        byte segment_index = 0; // Index from which to copy

        // Find the waveform that's the target of the add operation
        for (byte i = 0; i < HS::VO_SEGMENT_COUNT; i++)
        {
            if (HS::user_waveforms[i].IsTOC() && count++ == waveform_number) {
                segment_index = i + segment_number + 1;
                HS::user_waveforms[i].SetTOC(HS::user_waveforms[i].Segments() + direction);
                break;
            }
        }

        return segment_index;
    }

    void static AddSegmentToWaveformAtSegmentIndex(byte waveform_number, byte segment_number) {
        byte insert_point = GetSegmentIndex(waveform_number, segment_number, 1);

        // If the waveform was found, move the remaining steps to insert a new segment. The
        // newly-inserted step should be a copy of the insert point.
        if (insert_point) {
            for (int i = (HS::VO_SEGMENT_COUNT - 1); i > insert_point ; i--)
            {
                memcpy(&HS::user_waveforms[i], &HS::user_waveforms[i - 1], sizeof(HS::user_waveforms[i - 1]));
            }
        }
    }

    void static DeleteSegmentFromWaveformAtSegmentIndex(byte waveform_number, byte segment_number) {
        byte delete_point = GetSegmentIndex(waveform_number, segment_number, -1);

        // If the waveform was found, move the remaining steps to overwrite the deleted segment.
        if (delete_point) {
            for (int i = delete_point; i < (HS::VO_SEGMENT_COUNT - 1); i++)
            {
                memcpy(&HS::user_waveforms[i], &HS::user_waveforms[i + 1], sizeof(HS::user_waveforms[i + 1]));
            }
        }
    }

    void static Update(byte waveform_number, byte segment_number, VOSegment *segment) {
        byte ix = GetSegmentIndex(waveform_number, segment_number);
        if (ix) {
            HS::user_waveforms[ix].level = segment->level;
            HS::user_waveforms[ix].time = segment->time;
        }
    }
};

#endif // HS_VECTOR_OSCILLATOR
