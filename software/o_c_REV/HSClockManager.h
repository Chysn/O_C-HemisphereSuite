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

// A "tick" is one ISR cycle, which happens 16666.667 times per second, or a million
// times per minute. A "tock" is a metronome beat.

#ifndef CLOCK_MANAGER_H
#define CLOCK_MANAGER_H

const uint16_t CLOCK_TEMPO_MIN = 10;
const uint16_t CLOCK_TEMPO_MAX = 300;

class ClockManager {
    static ClockManager *instance;
    uint32_t ticks_per_tock; // Based on the selected tempo in BPM
    uint32_t last_tock_tick; // The tick of the most recent tock
    uint32_t last_tock_check; // To avoid checking the tock more than once per tick
    bool tock; // The most recent tock value
    uint16_t tempo; // The set tempo, for display somewhere else
    bool running; // Specifies whether the clock is running for interprocess communication
    bool paused; // Specifies whethr the clock is paused
    int8_t tocks_per_beat; // Multiplier
    bool cycle; // Alternates for each tock, for display purposes
    byte count; // Multiple counter
    bool forwarded; // Master clock forwarding is enabled when true

    ClockManager() {
        SetTempoBPM(120);
        SetMultiply(1);
        running = 0;
        paused = 0;
        cycle = 0;
        last_tock_tick = 0;
        last_tock_check = 0;
        count = 0;
        tock = 0;
        forwarded = 0;
    }

public:
    static ClockManager *get() {
        if (!instance) instance = new ClockManager;
        return instance;
    }

    void SetMultiply(int8_t multiply) {
        multiply = constrain(multiply, 1, 24);
        tocks_per_beat = multiply;
    }

    /* Set ticks per tock, based on one million ticks per minute divided by beats per minute.
     * This is approximate, because the arithmetical value is likely to be fractional, and we
     * need to live with a certain amount of imprecision here. So I'm not even rounding up.
     */
    void SetTempoBPM(uint16_t bpm) {
        bpm = constrain(bpm, CLOCK_TEMPO_MIN, CLOCK_TEMPO_MAX);
        ticks_per_tock = 1000000 / bpm;
        tempo = bpm;
    }

    int8_t GetMultiply() {return tocks_per_beat;}

    /* Gets the current tempo. This can be used between client processes, like two different
     * hemispheres.
     */
    uint16_t GetTempo() {return tempo;}

    void Reset() {
        last_tock_tick = OC::CORE::ticks;
        count = 0;
        cycle = 1 - cycle;
    }

    void Start() {
        forwarded = 0;
        running = 1;
        Unpause();
    }

    void Stop() {
        running = 0;
        Unpause();
    }

    void Pause() {paused = 1;}

    void Unpause() {paused = 0;}

    void ToggleForwarding() {forwarded = 1 - forwarded;}

    bool IsRunning() {return (running && !paused);}

    bool IsPaused() {return paused;}

    bool IsForwarded() {return forwarded;}

    /* Returns true if the clock should fire on this tick, based on the current tempo */
    bool Tock() {
        uint32_t now = OC::CORE::ticks;
        if (now != last_tock_check) {
            last_tock_check = now;
            if (now >= (last_tock_tick + (ticks_per_tock / static_cast<uint32_t>(tocks_per_beat)))) {
                tock = 1;
                last_tock_tick = now;
                if (++count >= tocks_per_beat) Reset();
            } else tock = 0;
        }
        return tock;
    }

    bool EndOfBeat() {return count == 0;}

    bool Cycle() {return cycle;}
};

ClockManager *ClockManager::instance = 0;

#endif // CLOCK_MANAGER_H
