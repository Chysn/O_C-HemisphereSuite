// Categories:
// 0x01 = Modulator
// 0x02 = Sequencer
// 0x04 = Clocking
// 0x08 = Quantizer
// 0x10 = Utility
// 0x20 = MIDI
// 0x40 = Audio
// 0x80 = Other

#define HEMISPHERE_AVAILABLE_APPLETS 28
#define HEMISPHERE_APPLETS { \
    DECLARE_APPLET(  8, 0x01, ADSREG), \
    DECLARE_APPLET( 15, 0x02, AnnularFusion), \
    DECLARE_APPLET(  4, 0x14, Brancher), \
    DECLARE_APPLET( 12, 0x10, Calculate),\
    DECLARE_APPLET(  6, 0x04, ClockDivider), \
    DECLARE_APPLET( 28, 0x04, ClockSkip), \
    DECLARE_APPLET( 30, 0x10, Compare), \
    DECLARE_APPLET( 24, 0x02, CVRec), \
    DECLARE_APPLET(  9, 0x08, DualQuant), \
    DECLARE_APPLET( 19, 0x01, GameOfLife), \
    DECLARE_APPLET( 29, 0x04, GateDelay), \
    DECLARE_APPLET( 17, 0x50, GatedVCA), \
    DECLARE_APPLET( 16, 0x40, LoFiPCM), \
    DECLARE_APPLET( 10, 0x14, Logic), \
    DECLARE_APPLET( 21, 0x01, LowerRenz), \
    DECLARE_APPLET(150, 0x20, hMIDIIn), \
    DECLARE_APPLET( 27, 0x20, hMIDIOut), \
    DECLARE_APPLET( 20, 0x02, Palimpsest), \
    DECLARE_APPLET( 26, 0x08, ScaleDuet), \
    DECLARE_APPLET( 23, 0x80, Scope), \
    DECLARE_APPLET( 14, 0x02, Sequence5), \
    DECLARE_APPLET(  7, 0x01, SkewedLFO), \
    DECLARE_APPLET(  3, 0x10, Switch), \
    DECLARE_APPLET( 19, 0x01, Slew), \
    DECLARE_APPLET( 13, 0x14, TLNeuron), \
    DECLARE_APPLET( 18, 0x02, TM), \
    DECLARE_APPLET( 11, 0x0c, TrigSeq), \
    DECLARE_APPLET( 25, 0x0c, TrigSeq16), \
}
