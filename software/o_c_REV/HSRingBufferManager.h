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

class RingBufferManager {
    static RingBufferManager *instance;
    int buffer[256];
    byte position = 0;
    byte index = 0;
    bool ready = 0; // Allows a second instance to know that a first instance has been clocked
    uint32_t registration[2]; // Keep track of which hemispheres are using ASR

    uint32_t last_advance_tick = 0; // To prevent double-advancing
    uint32_t last_write_tick = 0; // To prevent double-writing

    RingBufferManager() {
        for (byte i = 0; i < 255; i++) buffer[i] = 0;
        position = 0;
    }

public:
    static RingBufferManager *get() {
        if (!instance) instance = new RingBufferManager;
        return instance;
    }

    void Register(bool hemisphere) {
        registration[hemisphere] = OC::CORE::ticks;
    }

    byte CountInstances() {
        byte count = 0;
        for (byte i = 0; i < 2; i++) if (OC::CORE::ticks - registration[i] < 8) count++;
        return count;
    }

    // Allows a second instance to know that a first instance has been clocked
    bool Ready(bool hemisphere) {
        bool r = 0;
        if (hemisphere == RIGHT_HEMISPHERE && CountInstances() == 2) {
            r = ready;
            ready = 0;
        }
        return r;
    }

    void SetIndex(byte ix) {index = ix;}

    byte GetIndex() {return index;}

    byte GetPosition() {return position;}

    void WriteValueToBuffer(int cv) {
        if (OC::CORE::ticks > last_write_tick) buffer[position] = cv;
        last_write_tick = OC::CORE::ticks;
    }

    int ReadNextValue(byte output, bool hemisphere, int index_mod = 0) {
        if (hemisphere == RIGHT_HEMISPHERE && CountInstances() == 2) output += 2;
        byte ix = position - ((index + 1) * output) + index_mod;
        int cv = buffer[ix];
        return cv;
    }

    void Advance() {
        if (OC::CORE::ticks > last_advance_tick) {
            ++position; // No need to check range; 256 positions and an 8-bit counter
            last_advance_tick = OC::CORE::ticks;
            ready = 1;
        }
    }

    int GetY(bool hemisphere, byte x) {
        // If there are two instances, and this is the right hemisphere, show the
        // second 128 bytes. Otherwise, just show the first 128.
        byte offset = (hemisphere == RIGHT_HEMISPHERE && CountInstances() == 2) ? 64 : 0;
        byte ix = (x + offset + position + 196);
        int cv = buffer[ix] + HEMISPHERE_MAX_CV; // Force this positive
        int y = ((cv << 7) / (HEMISPHERE_MAX_CV * 2)) * 23;
        return 23 - (y >> 7);
    }
};

RingBufferManager *RingBufferManager::instance = 0;
