namespace HS {

const byte WAVEFORM_LIBRARY_COUNT = 8;

// Waveform Library Names
enum {
    Triangle = 32,
    Sawtooth,
    Ramp,
    Sine,
    Square,
    Pulse2,
    Pulse3,
    Pulse4
};

VOSegment library_waveforms[] = {
    // Triangle
    VOSegment {2, 255}, // 255=TOC marker, and 2=Number of segments in this waveform
    VOSegment {255, 1},
    VOSegment {0, 1},

    // Sawtooth
    VOSegment {2, 255},
    VOSegment {255, 0},
    VOSegment {0, 1},

    // Ramp
    VOSegment {2, 255},
    VOSegment {0, 0},
    VOSegment {255, 1},

    // Sine
    VOSegment {12, 255},
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
    VOSegment {4, 255},
    VOSegment {255, 0},
    VOSegment {255, 1},
    VOSegment {0, 0},
    VOSegment {0, 1},

    // Pulse 2:1
    VOSegment {4, 255},
    VOSegment {255, 0},
    VOSegment {255, 2},
    VOSegment {0, 0},
    VOSegment {0, 1},

    // Pulse 3:1
    VOSegment {4, 255},
    VOSegment {255, 0},
    VOSegment {255, 3},
    VOSegment {0, 0},
    VOSegment {0, 1},

    // Pulse 4:1
    VOSegment {4, 255},
    VOSegment {255, 0},
    VOSegment {255, 4},
    VOSegment {0, 0},
    VOSegment {0, 1},

    //
};

}; // namespace HS
