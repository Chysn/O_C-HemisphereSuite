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
//
// Thanks to Mike Thomas, for tons of help with the Buchla stuff
//

////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Base Class
////////////////////////////////////////////////////////////////////////////////

#include "HSicons.h"
#include "HSClockManager.h"

#define LEFT_HEMISPHERE 0
#define RIGHT_HEMISPHERE 1
#ifdef BUCHLA_4U
#define HEMISPHERE_MAX_CV 15360
#define HEMISPHERE_CENTER_CV 7680
#else
#define HEMISPHERE_MAX_CV 7680
#define HEMISPHERE_CENTER_CV 0
#endif
#define HEMISPHERE_3V_CV 4608
#define HEMISPHERE_CLOCK_TICKS 100
#define HEMISPHERE_CURSOR_TICKS 12000
#define HEMISPHERE_ADC_LAG 33
#define HEMISPHERE_CHANGE_THRESHOLD 32

#ifdef BUCHLA_4U
#define PULSE_VOLTAGE 8
#else
#define PULSE_VOLTAGE 5
#endif

// Codes for help system sections
#define HEMISPHERE_HELP_DIGITALS 0
#define HEMISPHERE_HELP_CVS 1
#define HEMISPHERE_HELP_OUTS 2
#define HEMISPHERE_HELP_ENCODER 3

// Simulated fixed floats by multiplying and dividing by powers of 2
#ifndef int2simfloat
#define int2simfloat(x) (x << 14)
#define simfloat2int(x) (x >> 14)
typedef int32_t simfloat;
#endif

// Hemisphere-specific macros
#define BottomAlign(h) (62 - h)
#define ForEachChannel(ch) for(int ch = 0; ch < 2; ch++)

// Specifies where data goes in flash storage for each selcted applet, and how big it is
typedef struct PackLocation {
    int location;
    int size;
} PackLocation;

class HemisphereApplet {
public:

    virtual const char* applet_name(); // Maximum of 9 characters
    virtual void Start();
    virtual void Controller();
    virtual void View();

    void BaseStart(bool hemisphere_) {
        hemisphere = hemisphere_;
        gfx_offset = hemisphere * 64;
        io_offset = hemisphere * 2;

        // Initialize some things for startup
        ForEachChannel(ch)
        {
            clock_countdown[ch]  = 0;
            inputs[ch] = 0;
            outputs[ch] = 0;
            adc_lag_countdown[ch] = 0;
        }
        help_active = 0;
        cursor_countdown = HEMISPHERE_CURSOR_TICKS;

        // Shutdown FTM capture on Digital 4, used by Tuner
#ifdef FLIP_180
        if (hemisphere == 0)
#else
        if (hemisphere == 1)
#endif
        {
            FreqMeasure.end();
            OC::DigitalInputs::reInit();
        }

        // Maintain previous app state by skipping Start
        if (!applet_started) {
            applet_started = true;
            Start();
        }
    }

    void BaseController(bool master_clock_on) {
        master_clock_bus = (master_clock_on && hemisphere == RIGHT_HEMISPHERE);
        ForEachChannel(ch)
        {
            // Set CV inputs
            ADC_CHANNEL channel = (ADC_CHANNEL)(ch + io_offset);
            inputs[ch] = OC::ADC::raw_pitch_value(channel);
            if (abs(inputs[ch] - last_cv[ch]) > HEMISPHERE_CHANGE_THRESHOLD) {
                changed_cv[ch] = 1;
                last_cv[ch] = inputs[ch];
            } else changed_cv[ch] = 0;

            // Handle clock timing
            if (clock_countdown[ch] > 0) {
                if (--clock_countdown[ch] == 0) Out(ch, 0);
            }
        }

        // Cursor countdowns. See CursorBlink(), ResetCursor(), gfxCursor()
        if (--cursor_countdown < -HEMISPHERE_CURSOR_TICKS) cursor_countdown = HEMISPHERE_CURSOR_TICKS;

        Controller();
    }

    void BaseView() {
        // If help is active, draw the help screen instead of the application screen
        if (help_active) DrawHelpScreen();
        else View();
        last_view_tick = OC::CORE::ticks;
    }

    // Screensavers are deprecated in favor of screen blanking, but the BaseScreensaverView() remains
    // to avoid breaking applets based on the old boilerplate
    void BaseScreensaverView() {}

    /* Help Screen Toggle */
    void HelpScreen() {
        help_active = 1 - help_active;
    }

    /* Check cursor blink cycle. */
    bool CursorBlink() {
        return (cursor_countdown > 0);
    }

    void ResetCursor() {
        cursor_countdown = HEMISPHERE_CURSOR_TICKS;
    }

    void DrawHelpScreen() {
        gfxHeader(applet_name());
        SetHelp();
        for (int section = 0; section < 4; section++)
        {
            int y = section * 12 + 16;
            graphics.setPrintPos(0, y);
            if (section == HEMISPHERE_HELP_DIGITALS) graphics.print("Dig");
            if (section == HEMISPHERE_HELP_CVS) graphics.print("CV");
            if (section == HEMISPHERE_HELP_OUTS) graphics.print("Out");
            if (section == HEMISPHERE_HELP_ENCODER) graphics.print("Enc");
            graphics.invertRect(0, y - 1, 19, 9);

            graphics.setPrintPos(20, y);
            graphics.print(help[section]);
        }
    }

    //////////////// Offset graphics methods
    ////////////////////////////////////////////////////////////////////////////////
    void gfxCursor(int x, int y, int w) {
        if (CursorBlink()) gfxLine(x, y, x + w - 1, y);
    }

    void gfxPos(int x, int y) {
        graphics.setPrintPos(x + gfx_offset, y);
    }

    void gfxPrint(int x, int y, const char *str) {
        graphics.setPrintPos(x + gfx_offset, y);
        graphics.print(str);
    }

    void gfxPrint(int x, int y, int num) {
        graphics.setPrintPos(x + gfx_offset, y);
        graphics.print(num);
    }

    void gfxPrint(int x_adv, int num) { // Print number with character padding
        for (int c = 0; c < (x_adv / 6); c++) gfxPrint(" ");
        gfxPrint(num);
    }

    void gfxPrint(const char *str) {
        graphics.print(str);
    }

    void gfxPrint(int num) {
        graphics.print(num);
    }

    /* Convert CV value to voltage level and print  to two decimal places */
    void gfxPrintVoltage(int cv) {
        int v = (cv * 100) / (12 << 7);
        bool neg = v < 0 ? 1 : 0;
        if (v < 0) v = -v;
        int wv = v / 100; // whole volts
        int dv = v - (wv * 100); // decimal
        gfxPrint(neg ? "-" : "+");
        gfxPrint(wv);
        gfxPrint(".");
        if (dv < 10) gfxPrint("0");
        gfxPrint(dv);
        gfxPrint("V");
    }

    void gfxPixel(int x, int y) {
        graphics.setPixel(x + gfx_offset, y);
    }

    void gfxFrame(int x, int y, int w, int h) {
        graphics.drawFrame(x + gfx_offset, y, w, h);
    }

    void gfxRect(int x, int y, int w, int h) {
        graphics.drawRect(x + gfx_offset, y, w, h);
    }

    void gfxInvert(int x, int y, int w, int h) {
        graphics.invertRect(x + gfx_offset, y, w, h);
    }

    void gfxLine(int x, int y, int x2, int y2) {
        graphics.drawLine(x + gfx_offset, y, x2 + gfx_offset, y2);
    }

    void gfxLine(int x, int y, int x2, int y2, bool dotted) {
        graphics.drawLine(x + gfx_offset, y, x2 + gfx_offset, y2, dotted ? 2 : 1);
    }

    void gfxDottedLine(int x, int y, int x2, int y2, uint8_t p = 2) {
        graphics.drawLine(x + gfx_offset, y, x2 + gfx_offset, y2, p);
    }

    void gfxCircle(int x, int y, int r) {
        graphics.drawCircle(x + gfx_offset, y, r);
    }

    void gfxBitmap(int x, int y, int w, const uint8_t *data) {
        graphics.drawBitmap8(x + gfx_offset, y, w, data);
    }

    void gfxIcon(int x, int y, const uint8_t *data) {
        gfxBitmap(x, y, 8, data);
    }

    uint8_t pad(int range, int number) {
        uint8_t padding = 0;
        while (range > 1)
        {
            if (abs(number) < range) padding += 6;
            range = range / 10;
        }
        return padding;
    }

    //////////////// Hemisphere-specific graphics methods
    ////////////////////////////////////////////////////////////////////////////////

    /* Show channel-grouped bi-lateral display */
    void gfxSkyline() {
        ForEachChannel(ch)
        {
            int height = ProportionCV(ViewIn(ch), 32);
            gfxFrame(23 + (10 * ch), BottomAlign(height), 6, 63);

            height = ProportionCV(ViewOut(ch), 32);
            gfxInvert(3 + (46 * ch), BottomAlign(height), 12, 63);
        }
    }

    void gfxHeader(const char *str) {
        gfxPrint(1, 2, str);
        gfxLine(0, 10, 62, 10);
        gfxLine(0, 12, 62, 12);
    }

    //////////////// Offset I/O methods
    ////////////////////////////////////////////////////////////////////////////////
    int In(int ch) {
        return inputs[ch];
    }

    // Apply small center detent to input, so it reads zero before a threshold
    int DetentedIn(int ch) {
        return (In(ch) > (HEMISPHERE_CENTER_CV + 64) || In(ch) < (HEMISPHERE_CENTER_CV - 64)) ? In(ch) : HEMISPHERE_CENTER_CV;
    }

    void Out(int ch, int value, int octave = 0) {
        DAC_CHANNEL channel = (DAC_CHANNEL)(ch + io_offset);
        OC::DAC::set_pitch(channel, value, octave);
        outputs[ch] = value + (octave * (12 << 7));
    }

    /*
     * Has the specified Digital input been clocked this cycle?
     *
     * If physical is true, then logical clock types (master clock forwarding and metronome) will
     * not be used.
     */
    bool Clock(int ch, bool physical = 0) {
        bool clocked = 0;
        if (hemisphere == 0) {
            if (ch == 0) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_1>();
            if (ch == 1) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_2>();
        } else if (hemisphere == 1) {
            if (ch == 0) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_3>();
            if (ch == 1) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_4>();
        }

        if (ch == 0 && !physical) {
            ClockManager *clock_m = clock_m->get();
            if (clock_m->IsRunning()) clocked = clock_m->Tock();
            else if (master_clock_bus) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_1>();
        }

        if (clocked) {
        		cycle_ticks[ch] = OC::CORE::ticks - last_clock[ch];
        		last_clock[ch] = OC::CORE::ticks;
        }
        return clocked;
    }

    void ClockOut(int ch, int ticks = HEMISPHERE_CLOCK_TICKS) {
        clock_countdown[ch] = ticks;
        Out(ch, 0, PULSE_VOLTAGE);
    }

    bool Gate(int ch) {
        bool high = 0;
        if (hemisphere == 0) {
            if (ch == 0) high = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_1>();
            if (ch == 1) high = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_2>();
        }
        if (hemisphere == 1) {
            if (ch == 0) high = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_3>();
            if (ch == 1) high = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_4>();
        }
        return high;
    }

    void GateOut(int ch, bool high) {
        Out(ch, 0, (high ? PULSE_VOLTAGE : 0));
    }

    // Buffered I/O functions
    int ViewIn(int ch) {return inputs[ch];}
    int ViewOut(int ch) {return outputs[ch];}
    int ClockCycleTicks(int ch) {return cycle_ticks[ch];}
    bool Changed(int ch) {return changed_cv[ch];}

protected:
    bool hemisphere; // Which hemisphere (0, 1) this applet uses
    const char* help[4];
    virtual void SetHelp();

    /* Forces applet's Start() method to run the next time the applet is selected. This
     * allows an applet to start up the same way every time, regardless of previous state.
     */
    void AllowRestart() {
        applet_started = 0;
    }

    //////////////// Calculation methods
    ////////////////////////////////////////////////////////////////////////////////

    /* Proportion method using simfloat, useful for calculating scaled values given
     * a fractional value.
     *
     * Solves this:  numerator        ???
     *              ----------- = -----------
     *              denominator       max
     *
     * For example, to convert a parameter with a range of 1 to 100 into value scaled
     * to HEMISPHERE_MAX_CV, to be sent to the DAC:
     *
     * Out(ch, Proportion(value, 100, HEMISPHERE_MAX_CV));
     *
     */
    int Proportion(int numerator, int denominator, int max_value) {
        simfloat proportion = int2simfloat((int32_t)numerator) / (int32_t)denominator;
        int scaled = simfloat2int(proportion * max_value);
        return scaled;
    }

    /* Proportion CV values into pixels for display purposes.
     *
     * Solves this:     cv_value           ???
     *              ----------------- = ----------
     *              HEMISPHERE_MAX_CV   max_pixels
     */
    int ProportionCV(int cv_value, int max_pixels) {
        int prop = constrain(Proportion(cv_value, HEMISPHERE_MAX_CV, max_pixels), 0, max_pixels);
        return prop;
    }

    /* Add value to a 32-bit storage unit at the specified location */
    void Pack(uint32_t &data, PackLocation p, uint32_t value) {
        data |= (value << p.location);
    }

    /* Retrieve value from a 32-bit storage unit at the specified location and of the specified bit size */
    int Unpack(uint32_t data, PackLocation p) {
        uint32_t mask = 1;
        for (int i = 1; i < p.size; i++) mask |= (0x01 << i);
        return (data >> p.location) & mask;
    }

    /* ADC Lag: There is a small delay between when a digital input can be read and when an ADC can be
     * read. The ADC value lags behind a bit in time. So StartADCLag() and EndADCLag() are used to
     * determine when an ADC can be read. The pattern goes like this
     *
     * if (Clock(ch)) StartADCLag(ch);
     *
     * if (EndOfADCLog(ch)) {
     *     int cv = In(ch);
     *     // etc...
     * }
     */
    void StartADCLag(int ch = 0) {
        adc_lag_countdown[ch] = HEMISPHERE_ADC_LAG;
    }

    bool EndOfADCLag(int ch = 0) {
        return (--adc_lag_countdown[ch] == 0);
    }

    /* Master Clock Forwarding is activated. This is updated with each ISR cycle by the Hemisphere Manager */
    bool MasterClockForwarded() {return master_clock_bus;}

private:
    int gfx_offset; // Graphics offset, based on the side
    int io_offset; // Input/Output offset, based on the side
    int inputs[2];
    int outputs[2];
    uint32_t last_clock[2]; // Tick number of the last clock observed by the child class
    uint32_t cycle_ticks[2]; // Number of ticks between last two clocks
    int clock_countdown[2];
    int cursor_countdown;
    int adc_lag_countdown[2]; // Time between a clock event and an ADC read event
    bool master_clock_bus; // Clock forwarding was on during the last ISR cycle
    bool applet_started; // Allow the app to maintain state during switching
    int last_view_tick; // Tick number of the most recent view
    int help_active;
    bool changed_cv[2]; // Has the input changed by more than 1/8 semitone since the last read?
    int last_cv[2]; // For change detection
};
