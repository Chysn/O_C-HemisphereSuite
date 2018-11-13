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

#ifndef LOGICGATE_H
#define LOGICGATE_H

// 0-7 are inputs, 8-13 are neuron outputs
#define LG_MAX_SOURCE 13

enum LogicGateType {
    NONE,

    // Unary
    NOT,

    // Binary
    AND,
    OR,
    XOR,
    NAND,
    NOR,
    XNOR,
    FLIP_FLOP,
    LATCH,

    // Ternary
    TL_NEURON
};

const char* const gate_name[10] = {
    "None", "NOT", "AND", "OR", "XOR", "NAND", "NOR", "XNOR", "FLIP", "LATCH"
};

const char* const source_name[14] = {
    "Dig 1", "Dig 2", "Dig 3", "Dig 4", "CV 1", "CV 2", "CV 3", "CV 4",
    "Neu 1", "Neu 2", "Neu 3", "Neu 4", "Neu 5", "Neu 6"
};

class LogicGate {
public:
    // State attribute
    bool state;
    uint16_t last_source_state;

    // General attributes
    int type;
    int source1;
    int source2;
    int source3;

    // Threshold Logic Neuron attributes
    int weight1;
    int weight2;
    int weight3;
    int threshold;

    /* Set the state based on gate type and source valutes */
    bool Calculate(uint16_t state) {
        bool v1 = source_value(state, source1);
        bool v2 = source_value(state, source2);
        bool v3 = source_value(state, source3);
        bool result = 0;

        switch(type) {
            case LogicGateType::NOT       : result = not_fn(v1); break;
            case LogicGateType::AND       : result = and_fn(v1, v2); break;
            case LogicGateType::OR        : result = or_fn(v1, v2); break;
            case LogicGateType::XOR       : result = xor_fn(v1, v2); break;
            case LogicGateType::NAND      : result = !and_fn(v1, v2); break;
            case LogicGateType::NOR       : result = !or_fn(v1, v2); break;
            case LogicGateType::XNOR      : result = !xor_fn(v1, v2); break;
            case LogicGateType::FLIP_FLOP : result = flip_flop_fn(v1, v2); break;
            case LogicGateType::LATCH     : result = latch_fn(v1, v2); break;
            case LogicGateType::TL_NEURON : result = tl_neuron_fn(v1, v2, v3); break;
            default                       : result = 0;
        }

        return result;
    }


    /* How many cursor positions does this LogicGate use? */
    byte NumParam() {
        byte max;
        switch (type) {
            case LogicGateType::NONE:
                max = 1; // When there's no type, the only parameter is the type
                break;

            case LogicGateType::NOT:
                max = 2; // Type and 1 source
                break;

            case LogicGateType::TL_NEURON:
                max = 8; // Type, three sources, three weights, and threshold
                break;

            default:
                max = 3; // Most types (binary) have a type and two sources
        }
        return max;
    }

    void UpdateValue(byte cursor, int direction) {
        if (cursor == 0) { // 0 is always type
            type += direction;
            type = constrain(type, LogicGateType::NONE, LogicGateType::TL_NEURON);
        }
        if (cursor == 1 && type > LogicGateType::NONE) {
            source1 += direction;
            source1 = constrain(source1, 0, LG_MAX_SOURCE);
        }
        if (cursor == 2 && type >= LogicGateType::AND) {
            source2 += direction;
            source2 = constrain(source2, 0, LG_MAX_SOURCE);
        }
        if (cursor == 3 && type >= LogicGateType::AND) {
            source3 += direction;
            source3 = constrain(source3, 0, LG_MAX_SOURCE);
        }
        if (type == LogicGateType::TL_NEURON) {
            if (cursor == 4) weight1 = constrain(weight1 + direction, -9, 9);
            if (cursor == 5) weight2 = constrain(weight2 + direction, -9, 9);
            if (cursor == 6) weight3 = constrain(weight3 + direction, -9, 9);
            if (cursor == 7) threshold = constrain(threshold + direction, -27, 27);
        }
    }

    void PrintParamNameAt(byte x, byte y, byte cursor) {
        graphics.setPrintPos(x, y);
        if (cursor == 0) graphics.print("Type");
        if (cursor > 0 && cursor < 4) graphics.print("Src");
    }

    void PrintValueAt(byte x, byte y, byte cursor) {
        graphics.setPrintPos(x, y);
        if (cursor == 0) graphics.print(gate_name[type]);
        if (cursor == 1) graphics.print(source_name[source1]);
        if (cursor == 2) graphics.print(source_name[source2]);
        if (cursor == 3) graphics.print(source_name[source3]);
    }

    void DrawSmallAt(byte x, byte y) {
        graphics.setPrintPos(x,y);
        graphics.print(type);
    }

private:
    bool source_value(uint16_t state, byte source) {
        return static_cast<bool>(state & (0x01 >> source));
    }

    bool not_fn(bool a) {return !a;}

    bool and_fn(bool a, bool b) {return a & b;}

    bool or_fn(bool a, bool b) {return a | b;}

    bool xor_fn(bool a, bool b) {return a != b;}

    bool flip_flop_fn(bool clock, bool flip) {
        if (clock && flip) state = !state;
        return state;
    }

    bool latch_fn(bool set, bool reset) {
        if (reset) state = 0;
        if (set) state = 1;
        return state;
    }

    bool tl_neuron_fn(bool d1, bool d2, bool d3) {
        int v = (d1 * weight1) + (d2 * weight2) + (d3 * weight3);
        return (v > threshold);
    }
};


#endif // LOGICGATE_H
