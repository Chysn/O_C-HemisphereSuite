namespace HS {

#define VO_TOC 255
const byte WAVEFORM_LIBRARY_COUNT = 16;

// Waveform Library Names
enum {
    Triangle = 32,
    Sawtooth,
    Ramp,
    Sine,
    Square,
    Pulse2,
    Pulse3,
    Pulse4,
    Trapezoid,
    Logarithmic,
    Exponential,
};

VOSegment library_waveforms[] = {
    // Triangle
    VOSegment {2, VO_TOC}, // 255=TOC marker, and 2=Number of segments in this waveform
    VOSegment {255, 1},
    VOSegment {0, 1},

    // Sawtooth
    VOSegment {2, VO_TOC},
    VOSegment {255, 0},
    VOSegment {0, 1},

    // Ramp
    VOSegment {2, VO_TOC},
    VOSegment {0, 0},
    VOSegment {255, 1},

    // Sine
    VOSegment {12, VO_TOC},
    VOSegment {192, 1},
    VOSegment {238, 1},
    VOSegment {255, 1},
    VOSegment {238, 1},
    VOSegment {191, 1},
    VOSegment {128, 1},
    VOSegment {64, 1},
    VOSegment {18, 1},
    VOSegment {1, 1},
    VOSegment {18, 1},
    VOSegment {65, 1},
    VOSegment {128, 1},

    // Square
    VOSegment {4, VO_TOC},
    VOSegment {255, 0},
    VOSegment {255, 1},
    VOSegment {0, 0},
    VOSegment {0, 1},

    // Pulse 2:1
    VOSegment {4, VO_TOC},
    VOSegment {255, 0},
    VOSegment {255, 2},
    VOSegment {0, 0},
    VOSegment {0, 1},

    // Pulse 3:1
    VOSegment {4, VO_TOC},
    VOSegment {255, 0},
    VOSegment {255, 3},
    VOSegment {0, 0},
    VOSegment {0, 1},

    // Pulse 4:1
    VOSegment {4, VO_TOC},
    VOSegment {255, 0},
    VOSegment {255, 4},
    VOSegment {0, 0},
    VOSegment {0, 1},

    // Trapezoid
    VOSegment {3, VO_TOC},
    VOSegment {255, 1},
    VOSegment {255, 3},
    VOSegment {0, 1},

    // Log
    VOSegment {12, VO_TOC},
    VOSegment {255, 0},
    VOSegment {254, 1},
    VOSegment {251, 1},
    VOSegment {243, 1},
    VOSegment {237, 1},
    VOSegment {226, 1},
    VOSegment {207, 1},
    VOSegment {179, 1},
    VOSegment {141, 1},
    VOSegment {84, 1},
    VOSegment {0, 1},
    VOSegment {0, 0},

    // Exp
    VOSegment {12, VO_TOC},
    VOSegment {255, 0},
    VOSegment {186, 1},
    VOSegment {139, 1},
    VOSegment {101, 1},
    VOSegment {75, 1},
    VOSegment {53, 1},
    VOSegment {35, 1},
    VOSegment {18, 1},
    VOSegment {7, 1},
    VOSegment {4, 1},
    VOSegment {0, 1},
    VOSegment {0, 0},

    // Diminishing Saw
    VOSegment {8, VO_TOC},
    VOSegment {255, 1},
    VOSegment {0, 0},
    VOSegment {192, 1},
    VOSegment {0, 0},
    VOSegment {128, 1},
    VOSegment {0, 0},
    VOSegment {64, 1},
    VOSegment {0, 0},

    // Step
    VOSegment {9, VO_TOC},
    VOSegment {255, 0},
    VOSegment {255, 1},
    VOSegment {192, 0},
    VOSegment {192, 1},
    VOSegment {128, 0},
    VOSegment {128, 1},
    VOSegment {64, 0},
    VOSegment {64, 1},
    VOSegment {0, 0},

    // EG1
    VOSegment {4, VO_TOC},
    VOSegment {255, 0},
    VOSegment {128, 2},
    VOSegment {128, 1},
    VOSegment {0, 2},

    // EG2
    VOSegment {4, VO_TOC},
    VOSegment {255, 1},
    VOSegment {128, 2},
    VOSegment {128, 1},
    VOSegment {0, 2},

    // EG3
    VOSegment {5, VO_TOC},
    VOSegment {218, 0},
    VOSegment {255, 1},
    VOSegment {128, 1},
    VOSegment {192, 1},
    VOSegment {0, 1},

};

}; // namespace HS
