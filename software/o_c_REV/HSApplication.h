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

/*
 * HSAppIO.h
 *
 * HSAppIO is a base class for full O_C apps that are designed to work (or act) like Hemisphere apps,
 * for consistency in development, or ease of porting apps or applets in either direction.
 */

#ifndef int2simfloat
#define int2simfloat(x) (x << 14)
#define simfloat2int(x) (x >> 14)
typedef int32_t simfloat;
#endif

#include "HSicons.h"

#ifndef HSAPPLICATION_H_
#define HSAPPLICATION_H_

#define HSAPPLICATION_CURSOR_TICKS 12000
#define HSAPPLICATION_5V 7680
#define HSAPPLICATION_3V 4608
#define HSAPPLICATION_CHANGE_THRESHOLD 32

#ifdef BUCHLA_4U
#define PULSE_VOLTAGE 8
#else
#define PULSE_VOLTAGE 5
#endif

class HSApplication {
public:
    virtual void Start();
    virtual void Controller();
    virtual void View();
    virtual void Resume();

    void BaseController() {
        for (uint8_t ch = 0; ch < 4; ch++)
        {
            // Set ADC input values
            inputs[ch] = OC::ADC::raw_pitch_value((ADC_CHANNEL)ch);
            if (abs(inputs[ch] - last_cv[ch]) > HSAPPLICATION_CHANGE_THRESHOLD) {
                changed_cv[ch] = 1;
                last_cv[ch] = inputs[ch];
            } else changed_cv[ch] = 0;

            if (clock_countdown[ch] > 0) {
                if (--clock_countdown[ch] == 0) Out(ch, 0);
            }
        }

        // Cursor countdowns. See CursorBlink(), ResetCursor(), gfxCursor()
        if (--cursor_countdown < -HSAPPLICATION_CURSOR_TICKS) cursor_countdown = HSAPPLICATION_CURSOR_TICKS;

        Controller();
    }

    void BaseStart() {
        // Initialize some things for startup
        for (uint8_t ch = 0; ch < 4; ch++)
        {
            clock_countdown[ch]  = 0;
            adc_lag_countdown[ch] = 0;
        }
        cursor_countdown = HSAPPLICATION_CURSOR_TICKS;

        Start();
    }

    void BaseView() {
        View();
        last_view_tick = OC::CORE::ticks;
    }

    int Proportion(int numerator, int denominator, int max_value) {
        simfloat proportion = int2simfloat((int32_t)numerator) / (int32_t)denominator;
        int scaled = simfloat2int(proportion * max_value);
        return scaled;
    }

    //////////////// Hemisphere-like IO methods
    ////////////////////////////////////////////////////////////////////////////////
    void Out(int ch, int value, int octave = 0) {
        OC::DAC::set_pitch((DAC_CHANNEL)ch, value, octave);
        outputs[ch] = value + (octave * (12 << 7));
    }

    int In(int ch) {
        return inputs[ch];
    }

    // Apply small center detent to input, so it reads zero before a threshold
    int DetentedIn(int ch) {
        return (In(ch) > 64 || In(ch) < -64) ? In(ch) : 0;
    }

    bool Changed(int ch) {
        return changed_cv[ch];
    }

    bool Gate(int ch) {
        bool high = 0;
        if (ch == 0) high = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_1>();
        if (ch == 1) high = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_2>();
        if (ch == 2) high = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_3>();
        if (ch == 3) high = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_4>();
        return high;
    }

    void GateOut(int ch, bool high) {
        Out(ch, 0, (high ? PULSE_VOLTAGE : 0));
    }

    bool Clock(int ch) {
        bool clocked = 0;
        if (ch == 0) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_1>();
        if (ch == 1) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_2>();
        if (ch == 2) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_3>();
        if (ch == 3) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_4>();
        if (clocked) {
        		cycle_ticks[ch] = OC::CORE::ticks - last_clock[ch];
        		last_clock[ch] = OC::CORE::ticks;
        }
        return clocked;
    }

    void ClockOut(int ch, int ticks = 100) {
        clock_countdown[ch] = ticks;
        Out(ch, 0, PULSE_VOLTAGE);
    }

    // Buffered I/O functions for use in Views
    int ViewIn(int ch) {return inputs[ch];}
    int ViewOut(int ch) {return outputs[ch];}
    int ClockCycleTicks(int ch) {return cycle_ticks[ch];}

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
    void StartADCLag(int ch) {adc_lag_countdown[ch] = 96;}
    bool EndOfADCLag(int ch) {return (--adc_lag_countdown[ch] == 0);}

    //////////////// Hemisphere-like graphics methods for easy porting
    ////////////////////////////////////////////////////////////////////////////////
    void gfxCursor(int x, int y, int w) {
        if (CursorBlink()) gfxLine(x, y, x + w - 1, y);
    }

    void gfxPos(int x, int y) {
        graphics.setPrintPos(x, y);
    }

    void gfxPrint(int x, int y, const char *str) {
        graphics.setPrintPos(x, y);
        graphics.print(str);
    }

    void gfxPrint(int x, int y, int num) {
        graphics.setPrintPos(x, y);
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

    void gfxPixel(int x, int y) {
        graphics.setPixel(x, y);
    }

    void gfxFrame(int x, int y, int w, int h) {
        graphics.drawFrame(x, y, w, h);
    }

    void gfxRect(int x, int y, int w, int h) {
        graphics.drawRect(x, y, w, h);
    }

    void gfxInvert(int x, int y, int w, int h) {
        graphics.invertRect(x, y, w, h);
    }

    void gfxLine(int x, int y, int x2, int y2) {
        graphics.drawLine(x, y, x2, y2);
    }

    void gfxDottedLine(int x, int y, int x2, int y2, uint8_t p = 2) {
#ifdef HS_GFX_MOD
        graphics.drawLine(x, y, x2, y2, p);
#else
        graphics.drawLine(x, y, x2, y2);
#endif
    }

    void gfxCircle(int x, int y, int r) {
        graphics.drawCircle(x, y, r);
    }

    void gfxBitmap(int x, int y, int w, const uint8_t *data) {
        graphics.drawBitmap8(x, y, w, data);
    }

    // Like gfxBitmap, but always 8x8
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
        if (number < 0 && padding > 0) padding -= 6; // Compensate for minus sign
        return padding;
    }

    void gfxHeader(const char *str) {
         gfxPrint(1, 2, str);
         gfxLine(0, 10, 127, 10);
         gfxLine(0, 12, 127, 12);
    }

protected:
    // Check cursor blink cycle
    bool CursorBlink() {
        return (cursor_countdown > 0);
    }

    void ResetCursor() {
        cursor_countdown = HSAPPLICATION_CURSOR_TICKS;
    }

private:
    int clock_countdown[4]; // For clock output timing
    int adc_lag_countdown[4]; // Lag countdown for each input channel
    int cursor_countdown; // Timer for cursor blinkin'
    uint32_t last_view_tick; // Time since the last view, for activating screen blanking
    int inputs[4]; // Last ADC values
    int outputs[4]; // Last DAC values; inputs[] and outputs[] are used to allow access to values in Views
    bool changed_cv[4]; // Has the input changed by more than 1/8 semitone since the last read?
    int last_cv[4]; // For change detection
    uint32_t last_clock[4]; // Tick number of the last clock observed by the child class
    uint32_t cycle_ticks[4]; // Number of ticks between last two clocks
};

#endif /* HSAPPLICATION_H_ */
