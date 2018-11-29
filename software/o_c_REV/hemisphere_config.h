// Categories:
// 0x01 = Modulator
// 0x02 = Sequencer
// 0x04 = Clocking
// 0x08 = Quantizer
// 0x10 = Utility
// 0x20 = MIDI
// 0x40 = Logic
// 0x80 = Other

#define HEMISPHERE_AVAILABLE_APPLETS 51

//////////////////  id  cat   class name
#define HEMISPHERE_APPLETS { \
    DECLARE_APPLET(  8, 0x01, ADSREG), \
    DECLARE_APPLET( 34, 0x01, ADEG), \
    DECLARE_APPLET( 15, 0x02, AnnularFusion), \
    DECLARE_APPLET( 47, 0x09, ASR), \
    DECLARE_APPLET( 41, 0x41, Binary), \
    DECLARE_APPLET( 51, 0x80, BootsNCat), \
    DECLARE_APPLET( 38, 0x80, BowTieSeq), \
    DECLARE_APPLET(  4, 0x14, Brancher), \
    DECLARE_APPLET( 31, 0x04, Burst), \
    DECLARE_APPLET( 12, 0x10, Calculate),\
    DECLARE_APPLET( 32, 0x0a, Carpeggio), \
    DECLARE_APPLET(  6, 0x04, ClockDivider), \
    DECLARE_APPLET( 28, 0x04, ClockSkip), \
    DECLARE_APPLET( 30, 0x10, Compare), \
    DECLARE_APPLET( 24, 0x02, CVRecV2), \
    DECLARE_APPLET( 55, 0x80, DrCrusher), \
    DECLARE_APPLET(  9, 0x08, DualQuant), \
    DECLARE_APPLET( 45, 0x02, EnigmaJr), \
    DECLARE_APPLET( 42, 0x11, EnvFollow), \
    DECLARE_APPLET( 29, 0x04, GateDelay), \
    DECLARE_APPLET( 17, 0x50, GatedVCA), \
    DECLARE_APPLET( 16, 0x80, LoFiPCM), \
    DECLARE_APPLET( 10, 0x44, Logic), \
    DECLARE_APPLET( 21, 0x01, LowerRenz), \
    DECLARE_APPLET( 50, 0x04, Metronome), \
    DECLARE_APPLET(150, 0x20, hMIDIIn), \
    DECLARE_APPLET( 27, 0x20, hMIDIOut), \
    DECLARE_APPLET( 33, 0x10, MixerBal), \
    DECLARE_APPLET( 20, 0x02, Palimpsest), \
    DECLARE_APPLET( 44, 0x01, RunglBook), \
    DECLARE_APPLET( 26, 0x08, ScaleDuet), \
    DECLARE_APPLET( 40, 0x40, Schmitt), \
    DECLARE_APPLET( 23, 0x80, Scope), \
    DECLARE_APPLET( 14, 0x02, Sequence5), \
    DECLARE_APPLET( 48, 0x45, ShiftGate), \
    DECLARE_APPLET( 18, 0x02, TM), \
    DECLARE_APPLET( 36, 0x04, Shuffle), \
    DECLARE_APPLET(  7, 0x01, SkewedLFO), \
    DECLARE_APPLET( 19, 0x01, Slew), \
    DECLARE_APPLET( 46, 0x08, Squanch), \
    DECLARE_APPLET(  3, 0x10, Switch), \
    DECLARE_APPLET( 13, 0x40, TLNeuron), \
    DECLARE_APPLET( 37, 0x40, Trending), \
    DECLARE_APPLET( 11, 0x06, TrigSeq), \
    DECLARE_APPLET( 25, 0x06, TrigSeq16), \
    DECLARE_APPLET( 39, 0x80, Tuner), \
    DECLARE_APPLET( 52, 0x01, VectorEG), \
    DECLARE_APPLET( 49, 0x01, VectorLFO), \
    DECLARE_APPLET( 53, 0x01, VectorMod), \
    DECLARE_APPLET( 54, 0x01, VectorMorph), \
    DECLARE_APPLET( 43, 0x10, Voltage), \
}
/*    DECLARE_APPLET(127, 0x80, DIAGNOSTIC), \ */
