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

#define int2signal(x) (x << 9)
#define signal2int(x) (x >> 9)
typedef int32_t vosignal_t;

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

    uint16_t pack() {
        return static_cast<uint16_t>((time << 16) | level);
    }

    void unpack(uint16_t p) {
        level = p & 0x00ff;
        time = (p >> 8) & 0x00ff;
    }
};

#define VO_MAX_SEGMENTS 12

class VectorOscillator {
public:
    int Diagnostic() {
        return diag;
    }

    /* Add a new segment to the end */
    void SetSegment(VOSegment segment) {
        if (segment_count < VO_MAX_SEGMENTS) {
            memcpy(&segments[segment_count], &segment, sizeof(segments[segment_count]));
            total_time += segments[segment_count].time;
            segment_count++;
        }
    }

    /* Update an existing segment */
    void SetSegment(byte ix, VOSegment segment) {
        ix = constrain(ix, 0, segment_count - 1);
        total_time -= segments[ix].time;
        memcpy(&segments[ix], &segment, sizeof(segments[ix]));
        total_time += segments[ix].time;
        if (ix == segment_count) segment_count++;
    }

    VOSegment GetSegment(byte ix) {
        ix = constrain(ix, 0, segment_count - 1);
        return segments[ix];
    }

    void SetScale(uint16_t scale_) {scale = scale_;}

    void SetFrequency(uint32_t frequency_) {
        frequency = frequency_;
    }

    bool GetEOC() {return eoc;}

    void Reset() {
        segment_index = 0;
        signal = scale_level(segments[segment_count - 1].level);
        compute_rise(segment_index);
        eoc = 0;
    }

    int32_t Next() {
        if (validate()) {
            if (eoc) Reset();
            signal += rise;
            if (countdown-- <= 0) advance_segment();
            diag = countdown;
        }
        return signal2int(signal);
    }

private:
    VOSegment segments[12]; // Array of segments in this Oscillator
    byte segment_count = 0; // Number of segments
    int total_time = 0; // Sum of time values for all segments
    vosignal_t signal = 0; // Current scaled signal << 10 for more precision
    bool eoc = 1; // The most recent tick's next() read was the end of a cycle
    byte segment_index = 0; // Which segment the Oscillator is currently traversing
    vosignal_t rise; // The amount (per tick) the signal must rise to reach the target
    int32_t countdown; // The number of ticks until the next segment
    uint32_t frequency; // In centiHertz
    uint16_t scale; // The maximum (and minimum negative) output for this Oscillator

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
     * 0-63, and this is converted to a bipolar value by subtracting 128.
     */
    vosignal_t scale_level(byte level) {
        int b_level = constrain(level, 0, 255) - 128;
        int scaled = Proportion(b_level, 255, scale);
        vosignal_t scaled_level = int2signal(scaled);
        return scaled_level;
    }

    void advance_segment() {
        ++segment_index;
        if (segment_index >= segment_count) {
            eoc = 1;
        } else {
            compute_rise(segment_index);
        }
    }

    void compute_rise(byte ix) {
        byte level = segments[ix].level;
        int time = static_cast<uint32_t>(segments[ix].time);
        vosignal_t target = scale_level(level);

        // How many ticks should a complete cycle last? cycle_ticks is 10 times that number.
        int32_t cycle_ticks = 16666667 / frequency;

        // How many ticks should the current segment last?
        int32_t segment_ticks = Proportion(time, total_time, cycle_ticks);

        // The total difference between the target and the current signal, divided by how many ticks
        // it should take to get there, is the rise. The / 10 is to cancel the extra precision
        // from the previous two calculations.
        if (segment_ticks > 0) {
            rise = ((target - signal) * 10) / segment_ticks;
            countdown = static_cast<uint16_t>(segment_ticks / 10);
        }
        else {
            signal = target;
            countdown = 0;
            rise = 0;
        }
        diag = rise;
    }
};

#endif // HS_VECTOR_OSCILLATOR
