#ifndef ENIGMASTEP_H
#define ENIGMASTEP_H

class EnigmaStep {
private:
    byte tk; // High 2 bits=Track, low 6 bits=TM
    byte pr; // Probability 0-100
    byte re; // Repeats 1-255
    byte tr; // Transpose -48 ~ 48 (0-96)

public:
    // Getters
    byte track() {return (tk >> 6) & 0x03;} // High two bits
    byte tm() {return (tk & 0x3f);} // Low six bits
    byte p() {return pr;}
    byte repeats() {return re;}
    int8_t transpose() {return static_cast<int8_t>(tr - 48);}

    // Setters
    void set_track(byte track_) {
        if (track_ > 3) track_ = 3;
        tk |= (track_ << 6);
    }
    void set_tm(byte tm_) {
        if (tm_ > 63) tm_ = 63;
        tk |= tm_;
    }
    void set_p(byte p_) {
        if (p_ > 100) p_ = 100;
        pr = p_;
    }
    void set_repeats(byte repeats_) {
        re = repeats_;
    }
    void set_transpose(int8_t transpose_) {
        tr = static_cast<byte>(transpose_ + 48);
    }
};

#endif // ENIGMASTEP_H
