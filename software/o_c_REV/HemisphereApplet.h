////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Base Class
////////////////////////////////////////////////////////////////////////////////

const int LEFT_HEMISPHERE = 0;
const int RIGHT_HEMISPHERE = 1;
const int HEMISPHERE_MAX_CV = 7800;
const int HEMISPHERE_CLOCK_TICKS = 100; // 6ms

// Simulated fixed floats by multiplying and dividing by a factor of ten
#define HEM_SIMFLOAT(x) 10000 * (x)
#define HEM_SIMFLOAT2INT(x) (x) / 10000
typedef int hem_simfloat;

class HemisphereApplet {
public:
    void IO(bool forwarding) {
        forwarding_on = (forwarding && hemisphere == RIGHT_HEMISPHERE);
        int fwd = forwarding_on ? io_offset : 0;
        for (int ch = 0; ch < 2; ch++)
        {
            // Set or forward CV inputs
            ADC_CHANNEL channel = (ADC_CHANNEL)(ch + io_offset - fwd);
            inputs[ch] = OC::ADC::raw_pitch_value(channel);

            // Handle clock timing
            if (clock_countdown[ch] > 0) {
                if (--clock_countdown[ch] == 0) Out(ch, 0);
            }
        }
    }

    /* Assign the child class instance to a left or right hemisphere */
    void SetHemisphere(int h) {
        hemisphere = h;
        gfx_offset = h * 65;
        io_offset = h * 2;
        forwarding_on = false;
        clock_countdown[0] = 0;
        clock_countdown[1] = 0;
    }

    /* Proportion CV values for display purposes */
    int ProportionCV(int value, int max) {
        int divisions = HEMISPHERE_MAX_CV / max; // Divide the CV into little pieces
        int proportion = value / divisions;
        if (proportion > max) proportion = max;
        return proportion;
    }

    /* System notifications from the base class regarding manager state(s) */
    void DrawNotifications() {
        // CV Forwarding Icon
        if (forwarding_on) {
            graphics.setPrintPos(61, 2);
            graphics.print(">");
            graphics.setPrintPos(59, 2);
            graphics.print(">");
        }
    }

    /* Offset Graphics Methods */
    void gfxPrint(int x, int y, const char *str) {
        graphics.setPrintPos(x + gfx_offset, y);
        graphics.print(str);
    }

    void gfxPrint(int x, int y, int num) {
        graphics.setPrintPos(x + gfx_offset, y);
        graphics.print(num);
    }

    void gfxPrint(const char *str) {
        graphics.print(str);
    }

    void gfxPrint(int num) {
        graphics.print(num);
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

    void gfxCircle(int x, int y, int r) {
        graphics.drawCircle(x + gfx_offset, y, r);
    }

    /* Hemisphere-specific graphics methods */
    void gfxOutputBar(int ch, bool screensaver) {
        int width = ProportionCV(outputs[ch], 60);
        if (width < 0) {width = 0;}
        int height = screensaver ? 2 : 12;
        int x = (hemisphere == 0) ? 64 - width : 0;
        gfxRect(x, 35 + (ch * 15), width, height);
    }

    void gfxInputBar(int ch, bool screensaver) {
        int width = ProportionCV(inputs[ch], 63);
        if (width < 0) {width = 0;}
        int height = screensaver ? 1 : 6;
        int x = (hemisphere == 0) ? 63 - width : 0;
        gfxFrame(x, 15 + (ch * 10), width, height);
    }

    void gfxButterfly(bool screensaver) {
        for (int ch = 0; ch < 2; ch++)
        {
            gfxOutputBar(ch, screensaver);
            gfxInputBar(ch, screensaver);
        }
    }

    void gfxHeader(const char *str) {
        gfxPrint(2, 2, str);
        gfxLine(0, 10, 62, 10);
        gfxLine(0, 12, 62, 12);
    }

    /* Offset I/O Methods */
    int In(int ch) {
        return inputs[ch];
    }

    void Out(int ch, int value, int octave) {
        DAC_CHANNEL channel = (DAC_CHANNEL)(ch + io_offset);
        OC::DAC::set_pitch(channel, value, octave);
        outputs[ch] = value;
    }

    void Out(int ch, int value) {
        Out(ch, value, 0);
    }

    bool Clock(int ch) {
        bool clocked = 0;
        if (hemisphere == 0) {
            if (ch == 0) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_1>();
            if (ch == 1) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_2>();
        }
        if (hemisphere == 1) {
            if (ch == 0) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_3>();
            if (ch == 1) clocked = OC::DigitalInputs::clocked<OC::DIGITAL_INPUT_4>();
        }
        return clocked;
    }

    int Gate(int ch) {
        bool gated = 0;
        if (hemisphere == 0) {
            if (ch == 0) gated = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_1>();
            if (ch == 1) gated = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_2>();
        }
        if (hemisphere == 1) {
            if (ch == 0) gated = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_3>();
            if (ch == 1) gated = OC::DigitalInputs::read_immediate<OC::DIGITAL_INPUT_4>();
        }
        return gated;
    }

    void ClockOut(int ch, int ticks) {
        clock_countdown[ch] = ticks;
        Out(ch, 0, 5);
    }

    void ClockOut(int ch) {
        ClockOut(ch, HEMISPHERE_CLOCK_TICKS);
    }

private:
    int hemisphere; // Which hemisphere (0, 1) this applet uses
    int gfx_offset; // Graphics offset, based on the side
    int io_offset; // Input/Output offset, based on the side
    int inputs[2];
    int outputs[2];
    int clock_countdown[2];
    bool forwarding_on; // Forwarding was on during the last ISR cycle
};
