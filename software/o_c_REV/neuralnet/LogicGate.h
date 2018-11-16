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

const char* const gate_name[11] = {
    "None", "NOT", "AND", "OR", "XOR", "NAND", "NOR", "XNOR", "FFlp", "Ltch", "TLNe"
};

const char* const source_name[14] = {
    "Dig1", "Dig2", "Dig3", "Dig4", "CV 1", "CV 2", "CV 3", "CV 4",
    "Neu1", "Neu2", "Neu3", "Neu4", "Neu5", "Neu6"
};

const uint8_t NN_LOGIC_ICON[11][12] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // None
    {0x08, 0x08, 0x7f, 0x41, 0x41, 0x22, 0x22, 0x14, 0x08, 0x08, 0x0a, 0x0c}, // Not
    {0x22, 0x22, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x3e, 0x08, 0x08, 0x08}, // AND
    {0x22, 0x22, 0x63, 0x77, 0x7f, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x08, 0x08}, // OR
    {0x22, 0x22, 0x77, 0x08, 0x77, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x08, 0x08}, // XOR
    {0x22, 0x22, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x3e, 0x08, 0x0a, 0x0c}, // NAND
    {0x22, 0x22, 0x63, 0x77, 0x7f, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x0a, 0x0c}, // NOR
    {0x22, 0x22, 0x77, 0x08, 0x77, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x0a, 0x0c}, // XNOR
    {0x22, 0x22, 0x7f, 0x51, 0x61, 0x41, 0x41, 0x41, 0x41, 0x7f, 0x08, 0x08}, // Flip Flop
    {0x22, 0x3e, 0x22, 0x14, 0x08, 0x14, 0x22, 0x22, 0x22, 0x3e, 0x02, 0x02}, // Latch
    {0xdb, 0xdb, 0x52, 0x52, 0x52, 0x3c, 0x10, 0x38, 0x38, 0x4c, 0x64, 0x38}, // TL Neuron
};



class LogicGate {
public:
    // State attribute
    bool state; // Current state of the gate
    uint16_t source_state; // Source bitfield, for display purposes

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
    bool Calculate(uint16_t source_state_) {
        source_state = source_state_;
        bool v1 = source_value(source1);
        bool v2 = source_value(source2);
        bool v3 = source_value(source3);

        switch(type) {
            case LogicGateType::NOT       : state = not_fn(v1); break;
            case LogicGateType::AND       : state = and_fn(v1, v2); break;
            case LogicGateType::OR        : state = or_fn(v1, v2); break;
            case LogicGateType::XOR       : state = xor_fn(v1, v2); break;
            case LogicGateType::NAND      : state = !and_fn(v1, v2); break;
            case LogicGateType::NOR       : state = !or_fn(v1, v2); break;
            case LogicGateType::XNOR      : state = !xor_fn(v1, v2); break;
            case LogicGateType::FLIP_FLOP : state = flip_flop_fn(v1, v2); break;
            case LogicGateType::LATCH     : state = latch_fn(v1, v2); break;
            case LogicGateType::TL_NEURON : state = tl_neuron_fn(v1, v2, v3); break;
            default                       : state = 0;
        }

        return state;
    }

    bool SourceValue(byte s) {
        bool v = 0;
        if (s == 0) v = source_value(source1);
        if (s == 1) v = source_value(source2);
        if (s == 2) v = source_value(source3);
        return v;
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
        if (cursor == 0) {
            type = constrain(type + direction, LogicGateType::NONE, LogicGateType::TL_NEURON);
        }
        if (cursor == 1 && type > LogicGateType::NONE) {
            source1 = constrain(source1 + direction, 0, LG_MAX_SOURCE);
        }
        if (cursor == 2 && type >= LogicGateType::AND) {
            source2 = constrain(source2 + direction, 0, LG_MAX_SOURCE);
        }
        if (cursor == 3 && type >= LogicGateType::AND) {
            source3 = constrain(source3 + direction, 0, LG_MAX_SOURCE);
        }
        if (type == LogicGateType::TL_NEURON) {
            if (cursor == 4) weight1 = constrain(weight1 + direction, -9, 9);
            if (cursor == 5) weight2 = constrain(weight2 + direction, -9, 9);
            if (cursor == 6) weight3 = constrain(weight3 + direction, -9, 9);
            if (cursor == 7) threshold = constrain(threshold + direction, -27, 27);
        }
    }

    void PrintParamNameAt(byte x, byte y, byte cursor) {
        if (cursor < 4) { // Other cursors are handled in the left-hand pane
            graphics.setPrintPos(x, y);
            if (cursor == 0) graphics.print("Type");
            if (cursor > 0 && cursor < 4) {
                if (type == LogicGateType::FLIP_FLOP) {
                    if (cursor == 1) graphics.print("Clock");
                    if (cursor == 2) graphics.print("Flip");
                } else if (type == LogicGateType::LATCH) {
                    if (cursor == 1) graphics.print("Set");
                    if (cursor == 2) graphics.print("Reset");
                } else if (type == LogicGateType::TL_NEURON) {
                    graphics.print("Den ");
                    graphics.print(cursor);
                } else {
                    graphics.print("Src");
                }
            }
        }
    }

    void PrintValueAt(byte x, byte y, byte cursor) {
        if (cursor < 4) {
            graphics.setPrintPos(x, y);
            if (cursor == 0) graphics.print(gate_name[type]);
            if (cursor == 1) graphics.print(source_name[source1]);
            if (cursor == 2) graphics.print(source_name[source2]);
            if (cursor == 3) graphics.print(source_name[source3]);
        }
    }

    void DrawSmallAt(byte x, byte y, bool show_name) {
        if (show_name) {
            graphics.setPrintPos(x, y);
            graphics.print(gate_name[type]);
        }
        graphics.drawBitmap8(x + 4, y + 10, 12, NN_LOGIC_ICON[type]);
    }

    void DrawInputs(byte n) {
        if (type > LogicGateType::NONE) draw_line_from(source1, n);
        if (type >= LogicGateType::AND) draw_line_from(source2, n);
        if (type == LogicGateType::TL_NEURON) draw_line_from(source3, n);
    }

private:
    bool source_value(uint16_t source) {
        uint16_t v = source_state & (0x01 << source);
        return static_cast<bool>(v);
    }

    //////// LOGIC GATE EVALUATION
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

    void draw_line_from(byte source, byte n) {
        byte fx;
        byte fy;
        byte tx = ((n / 2) * 32) + 27;
        byte ty = ((n % 2) * 24) + 30;
        if (source < 8) {
            // Physical input sources
            fx = ((source / 4) * 6) + 3;
            fy = ((source % 4) * 10) + 27;
        } else {
            // Logical output sources
            source -= 8;
            fx = ((source / 2) * 32) + 39;
            fy = ((source % 2) * 24) + 30;
        }
        graphics.drawLine(fx, fy, tx, ty);
    }
};


#endif // LOGICGATE_H
