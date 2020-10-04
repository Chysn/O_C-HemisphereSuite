#include "HSApplication.h"
#include "HSMIDI.h"

#define HEMISPHERE_MAX_CV 7680
#define HEMISPHERE_CENTER_CV 0
#define SCOPE_WIDTH 128

class BigScope : public HSApplication, public SystemExclusiveHandler {
public:
    void Start() {
        last_bpm_tick = OC::CORE::ticks;
        bpm = 0;
        sample_ticks = 320;
        freeze = 0;
        last_scope_tick = 0;
    }

    void Resume() {
    }

    void Controller() {
        if (Clock(0)) {
            int this_tick = OC::CORE::ticks;
            int time = this_tick - last_bpm_tick;
            last_bpm_tick = this_tick;
            bpm = 1000000 / time;
            if (bpm > 9999) bpm = 9999;

            if (last_scope_tick) {
                int cycle_ticks = OC::CORE::ticks - last_scope_tick;
                sample_ticks = cycle_ticks / 64;
                sample_ticks = constrain(sample_ticks, 2, 64000);
            }
            last_scope_tick = OC::CORE::ticks;
        }

        if (!freeze) {
            last_cv = In(0);

            if (--sample_countdown < 1) {
                sample_countdown = sample_ticks;
                if (++sample_num > 63) sample_num = 0;
                int sample = Proportion(In(0), HEMISPHERE_MAX_CV, 128);
                sample = constrain(sample, -128, 127) + 127;
                snapshot[sample_num] = (uint8_t) sample;
            }

            Out(0, In(0));
        }
    }

    void View() {
        gfxHeader("Scope");
        //gfxPrint(1, 2, "Scope");
        //DrawTicks();
        DrawVoltage();
        DrawBPM();
        DrawInput1();
        if (freeze) {
            gfxInvert(0, 24, 64, 40);
        }
    }

    void OnSendSysEx() {
    }

    void OnReceiveSysEx() {
    }

    /////////////////////////////////////////////////////////////////
    // Control handlers
    /////////////////////////////////////////////////////////////////
    void OnLeftButtonPress() {
        freeze = 1 - freeze;
    }

    void OnLeftButtonLongPress() {

    }

    void OnRightButtonPress() {
    }

    void OnUpButtonPress() {
    }

    void OnDownButtonPress() {
    }

    void OnDownButtonLongPress() {
    }

    void OnLeftEncoderMove(int direction) {
        if (sample_ticks < 32) sample_ticks += direction;
        else sample_ticks += direction * 10;
        sample_ticks = constrain(sample_ticks, 2, 64000);
        last_encoder_move = OC::CORE::ticks;
    }

    void OnRightEncoderMove(int direction) {
    }

private:
    // BPM Calcultion
    int last_bpm_tick;
    int bpm;

    // CV monitor
    int last_cv;
    bool freeze;

    // Scope
    uint8_t snapshot[64];
    int sample_ticks; // Ticks between samples
    int sample_countdown; // Last time a sample was taken
    int sample_num; // Current sample number at the start
    int last_encoder_move; // The last the the sample_ticks value was changed
    int last_scope_tick; // Used to auto-calculate sample countdown

    void DrawBPM() {
        gfxPrint(110, 1, bpm / 4);
        gfxBitmap(102, 1, 8, CLOCK_ICON);
    }

//    void DrawTicks() {
//        gfxPrint(40, 1, sample_ticks);
//    }


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

    void DrawVoltage() {
        gfxBitmap(45, 3, 8, CV_ICON);
        gfxPos(55, 1);
        gfxPrintVoltage(last_cv);
    }

    void DrawInput1() {
        for (int s = 0; s < 64; s++)
        {
            int x = s + sample_num;
            if (x > 63) x -= 64;
            int l = Proportion(snapshot[x], 255, 28);
            gfxPixel(x, (28 - l) + 24);
        }

    }

};

BigScope BigScope_instance;

// App stubs
void BigScope_init() {
    BigScope_instance.BaseStart();
}

// Not using O_C Storage
size_t BigScope_storageSize() { return 0; }

size_t BigScope_save(void *storage) { return 0; }

size_t BigScope_restore(const void *storage) { return 0; }

void BigScope_isr() {
    return BigScope_instance.BaseController();
}

void BigScope_handleAppEvent(OC::AppEvent event) {
    if (event == OC::APP_EVENT_RESUME) {
        BigScope_instance.Resume();
    }
    if (event == OC::APP_EVENT_SUSPEND) {
        BigScope_instance.OnSendSysEx();
    }
}

void BigScope_loop() {} // Deprecated

void BigScope_menu() {
    BigScope_instance.BaseView();
}

void BigScope_screensaver() {} // Deprecated

void BigScope_handleButtonEvent(const UI::Event &event) {
    // For left encoder, handle press and long press
    if (event.control == OC::CONTROL_BUTTON_L) {
        if (event.type == UI::EVENT_BUTTON_LONG_PRESS) BigScope_instance.OnLeftButtonLongPress();
        else BigScope_instance.OnLeftButtonPress();
    }

    // For right encoder, only handle press (long press is reserved)
    if (event.control == OC::CONTROL_BUTTON_R && event.type == UI::EVENT_BUTTON_PRESS)
        BigScope_instance.OnRightButtonPress();

    // For up button, handle only press (long press is reserved)
    if (event.control == OC::CONTROL_BUTTON_UP) BigScope_instance.OnUpButtonPress();

    // For down button, handle press and long press
    if (event.control == OC::CONTROL_BUTTON_DOWN) {
        if (event.type == UI::EVENT_BUTTON_PRESS) BigScope_instance.OnDownButtonPress();
        if (event.type == UI::EVENT_BUTTON_LONG_PRESS) BigScope_instance.OnDownButtonLongPress();
    }
}

void BigScope_handleEncoderEvent(const UI::Event &event) {
    // Left encoder turned
    if (event.control == OC::CONTROL_ENCODER_L) BigScope_instance.OnLeftEncoderMove(event.value);

    // Right encoder turned
    if (event.control == OC::CONTROL_ENCODER_R) BigScope_instance.OnRightEncoderMove(event.value);
}
