// (c) 2018, Jason Justian (chysn), Beize Maze
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

// TuringMachineState is a class for containing the current state of a Turing Machine
// and advancing it to a different state by the following operations:
// 1) Bit shifting
// 2) Probability processing

#ifndef TURINGMACHINESTATE_H
#define TURINGMACHINESTATE_H

class TuringMachineState {
public:
    void Init(HS::TuringMachine TM) {
        original_reg = TM.reg;
        reg = TM.reg;
        len = TM.len;
    }

    void Init() {
        original_reg = random(0, 0xffff);
        reg = original_reg;
        len = 16;
    }

    /*
     * Get a TuringMachine based on the current state
     */
    HS::TuringMachine GetTuringMachine() {
        HS::TuringMachine TM = HS::TuringMachine {reg, len};
        return TM;
    }

    /*
     * Probability here is from 0 - 100
     */
    void ChangeProbability(int direction) {
        p = constrain(p + direction, 0, 100);
    }

    /*
     * Length here is from 1 - 16
     */
    void ChangeLength(int direction) {
        len = constrain(len + direction, 1, 16);
    }

    void Advance() {
        // Grab the bit that's about to be shifted away
        uint16_t last = (reg >> (len - 1)) & 0x01;

        // Does it change?
        if (random(0, 99) < p) last = 1 - last;

        // Shift left, then potentially add the bit from the other side
        reg = (reg << 1) + last;
    }

    /*
     * This might be a CV option in a single-TM applet
     */
    void Reset() {
        reg = original_reg;
    }

    // The following methods are for getting specific information from the TuringMachineState,
    // which is used in various ways by the application(s)

    /*
     * Get raw register (usually for generating a view of some kind, or saving a state)
     */
    uint16_t GetRegister() {return reg;}

    /*
     * Get length
     */
    byte GetLength() {return len;}

    /*
     * Note number is based on the low five bits of the register, for a range of about
     * 2.5 octaves
     */
    byte NoteNumber() {
        return static_cast<uint8_t>(reg & 0x1f) + 48;
    }

    /*
     * Modulation is based on the low byte of the register, and is the traditional
     * value used by the Whitwell Turing Machine
     */
    byte Modulation() {
        return static_cast<uint8_t>(reg & 0xff);
    }

    /*
     * Clock is based on the low bit of the register, for generating rhythms. There are two
     * ways to do this:
     *
     * -- Trigger: Send a trigger when Clock() is 1
     * -- Gate: Start a gate when Clock() is 1, and keep it on until there's a Clock() of 0
     */
    bool Clock() {
       return static_cast<bool>(reg & 0x01);
    }


private:
    uint16_t reg;
    byte len; // Length in steps
    uint16_t original_reg;
    byte p; // Probability
};

#endif // TURINGMACHINESTATE_H
