/*
 * HSAppIO.h
 *
 *  Created on: Aug 9, 2018
 *      Author: Jason Justian
 */

#ifndef int2simfloat
#define int2simfloat(x) (x << 14)
#define simfloat2int(x) (x >> 14)
typedef int32_t simfloat;
#endif

#ifndef HSAPPIO_H_
#define HSAPPIO_H_

class HSAppIO {
public:
    int Proportion(int numerator, int denominator, int max_value) {
        simfloat proportion = int2simfloat((int32_t)numerator) / (int32_t)denominator;
        int scaled = simfloat2int(proportion * max_value);
        return scaled;
    }

protected:
    void HSIOController() {
        for (uint8_t ch = 0; ch < 4; ch++)
        {
            if (clock_countdown[ch] > 0) {
                if (--clock_countdown[ch] == 0) Out(ch, 0);
            }
        }
    }

    void Out(int ch, int value, int octave = 0) {
        OC::DAC::set_pitch((DAC_CHANNEL)ch, value, octave);
    }

    int In(int ch) {
        return OC::ADC::raw_pitch_value((ADC_CHANNEL)ch);
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
        Out(ch, 0, (high ? 5 : 0));
    }

    void ClockOut(int ch, int ticks = 100) {
        clock_countdown[ch] = ticks;
        Out(ch, 0, 5);
    }

    void StartADCLag(int ch) {
        adc_lag_countdown[ch] = 96;
    }

    bool EndOfADCLag(int ch) {
        return (--adc_lag_countdown[ch] == 0);
    }

private:
    int clock_countdown[4]; // For clock output timing
    int adc_lag_countdown[4]; // Lag countdown for each input channel
};

#endif /* SOFTWARE_O_C_REV_HSAPPIO_H_ */
