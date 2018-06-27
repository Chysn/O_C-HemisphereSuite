////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Base Class
////////////////////////////////////////////////////////////////////////////////

const int LEFT_HEMISPHERE = 0;
const int RIGHT_HEMISPHERE = 1;
const int HEMISPHERE_MAX_CV = 7800;
const int HEMISPHERE_CLOCK_TICKS = 100; // 6ms
const int HEMISPHERE_CURSOR_TICKS = 12000;

// Codes for help system sections
const int HEMISPHERE_HELP_DIGITALS = 0;
const int HEMISPHERE_HELP_CVS = 1;
const int HEMISPHERE_HELP_OUTS = 2;
const int HEMISPHERE_HELP_ENCODER = 3;

// Simulated fixed floats by multiplying and dividing by powers of 2
#define int2simfloat(x) (x << 14)
#define simfloat2int(x) (x >> 14)
typedef int32_t simfloat;

// Hemisphere-specific macros
#define BottomAlign(h) (62 - h)
#define ForEachChannel(ch) for(int ch = 0; ch < 2; ch++)

class HemisphereApplet {
public:

    virtual const char* applet_name(); // Maximum of 10 characters
    virtual void View();
    virtual void Start();
    virtual void Controller();

    void BaseStart(int h) {
        hemisphere = h;
        gfx_offset = h * 65;
        io_offset = h * 2;

        // Initialize some things for startup
        ForEachChannel(ch)
        {
            clock_countdown[ch]  = 0;
            inputs[ch] = 0;
            outputs[ch] = 0;

        }
        help_active = 0;
        cursor_countdown = HEMISPHERE_CURSOR_TICKS;

        // Maintain previous app state by skipping Start
        if (!applet_started) {
            Start();
            applet_started = true;
        }
    }

    void BaseController(bool forwarding) {
        forwarding_on = (forwarding && hemisphere == RIGHT_HEMISPHERE);
        int fwd = forwarding_on ? io_offset : 0;
        ForEachChannel(ch)
        {
            // Set or forward CV inputs
            ADC_CHANNEL channel = (ADC_CHANNEL)(ch + io_offset - fwd);
            inputs[ch] = OC::ADC::raw_pitch_value(channel);

            // Handle clock timing
            if (clock_countdown[ch] > 0) {
                if (--clock_countdown[ch] == 0) Out(ch, 0);
            }
        }

        // Cursor countdown. See CursorBlink(), ResetCursor(), and gfxCursor()
        if (--cursor_countdown < -HEMISPHERE_CURSOR_TICKS) cursor_countdown = HEMISPHERE_CURSOR_TICKS;

        Controller();
    }

    void BaseView() {
        if (help_active) {
            // If help is active, draw the help screen instead of the application screen
            DrawHelpScreen();
        } else {
            View();
            DrawNotifications();
        }
    }

    /* Help Screen Toggle */
    void HelpScreen() {
        help_active = 1 - help_active;
    }

    bool CursorBlink() {
        return cursor_countdown > 0;
    }

    void ResetCursor() {
        cursor_countdown = HEMISPHERE_CURSOR_TICKS;
    }

    //////////////// Notifications from the base class regarding manager state(s)
    ////////////////////////////////////////////////////////////////////////////////
    void DrawNotifications() {
        // CV Forwarding Icon
        if (forwarding_on) {
            graphics.setPrintPos(61, 2);
            graphics.print(">");
            graphics.setPrintPos(59, 2);
            graphics.print(">");
        }
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

    void gfxBitmap(int x, int y, int w, const uint8_t *data) {
        graphics.drawBitmap8(x + gfx_offset, y, w, data);
    }

    //////////////// Hemisphere-specific graphics methods
    ////////////////////////////////////////////////////////////////////////////////
    /* Output bars are 12 pixels tall; there should be 15 pixels from the top of an
     * output bar to the top of the next vertical element.
     */
    void gfxOutputBar(int ch, int y) {
        int width = ProportionCV(outputs[ch], 60);
        if (width < 0) {width = 0;}
        int x = (hemisphere == 0) ? 64 - width : 0;
        gfxRect(x, y, width, 12);
    }

    /* Input bars are 6 pixels tall; there should be 10 pixels from the top of an
     * input bar to the top of the next vertical element.
     */
    void gfxInputBar(int ch, int y) {
        int width = ProportionCV(inputs[ch], 63);
        if (width < 0) {width = 0;}
        int x = (hemisphere == 0) ? 63 - width : 0;
        gfxFrame(x, y, width, 6);
    }

    /* Original butterfly: functional grouping (ins with outs) */
    void gfxButterfly() {
        ForEachChannel(ch)
        {
            gfxInputBar(ch, 15 + (ch * 10));
            gfxOutputBar(ch, 35 + (ch * 15));
        }
    }

    /* Alternate butterfly: Channel grouping (Ch1 with Ch2) */
    void gfxButterfly_Channel() {
        ForEachChannel(ch)
        {
            gfxInputBar(ch, 15 + (ch * 25));
            gfxOutputBar(ch, 25 + (ch * 25));
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

    void ClockOut(int ch, int ticks) {
        clock_countdown[ch] = ticks;
        Out(ch, 0, 5);
    }

    void ClockOut(int ch) {
        ClockOut(ch, HEMISPHERE_CLOCK_TICKS);
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
        Out(ch, 0, (high ? 5 : 0));
    }

    // Buffered I/O functions for use in Views
    int ViewIn(int ch) {return inputs[ch];}
    int ViewOut(int ch) {return outputs[ch];}

protected:
    const char* help[4];
    virtual void SetHelp();

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
        return Proportion(cv_value, HEMISPHERE_MAX_CV, max_pixels);
    }

private:
    int hemisphere; // Which hemisphere (0, 1) this applet uses
    int gfx_offset; // Graphics offset, based on the side
    int io_offset; // Input/Output offset, based on the side
    int inputs[2];
    int outputs[2];
    int clock_countdown[2];
    int cursor_countdown;
    bool forwarding_on; // Forwarding was on during the last ISR cycle
    bool applet_started; // Allow the app to maintain state during switching

    int help_active;
};
