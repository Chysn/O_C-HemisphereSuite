/*
 * 7-segment numeric display for Hemisphere Suite. Why? Because it looks sweet, that's why.
 *
 * By Jason Justian (c)2018
 *
 * MIT License, see HemisphereSuite License info on GitHub
 */

#ifndef SEGMENTDISPLAY_H
#define SEGMENTDISPLAY_H

struct PixelOffset {
    uint8_t ox;
    uint8_t oy;

    void DrawAt(uint8_t x, uint8_t y) {
        graphics.setPixel(x + ox, y + oy);
    }
};

struct Segment {
    PixelOffset pixels[6];

    void DrawAt(uint8_t x, uint8_t y) {
        for (int i = 0; i < 6; i++)
        {
            pixels[i].DrawAt(x, y);
        }
    }
};

class SegmentDisplay {
public:
    void Init() {
        segment[0] = {PixelOffset{2,0}, PixelOffset{3,0}, PixelOffset{4,0}, PixelOffset{5,0}, PixelOffset{3,1}, PixelOffset{4,1}};
        segment[1] = {PixelOffset{6,2}, PixelOffset{6,3}, PixelOffset{7,1}, PixelOffset{7,2}, PixelOffset{7,3}, PixelOffset{7,4}};
        segment[2] = {PixelOffset{6,8}, PixelOffset{6,9}, PixelOffset{7,7}, PixelOffset{7,8}, PixelOffset{7,9}, PixelOffset{7,10}};
        segment[3] = {PixelOffset{3,10}, PixelOffset{4,10}, PixelOffset{2,11}, PixelOffset{3,11}, PixelOffset{4,11}, PixelOffset{5,11}};
        segment[4] = {PixelOffset{0,7}, PixelOffset{0,8}, PixelOffset{0,9}, PixelOffset{0,10}, PixelOffset{1,8}, PixelOffset{1,9}};
        segment[5] = {PixelOffset{0,1}, PixelOffset{0,2}, PixelOffset{0,3}, PixelOffset{0,4}, PixelOffset{1,2}, PixelOffset{1,3}};
        segment[6] = {PixelOffset{3,5}, PixelOffset{4,5}, PixelOffset{2,6}, PixelOffset{3,6}, PixelOffset{4,6}, PixelOffset{5,6}};

        uint8_t digits[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67};
        memcpy(&digit, &digits, sizeof(digits));
        decimal = 0;
    }

    void Print(uint8_t x, uint8_t y, int number) {
        x_pos = x;
        y_pos = y;

        uint8_t to_print[5];
        uint8_t q = 0;

        // Make a list of digits to print
        int pwrs[] = {10000, 1000, 100, 10, 1};
        int tmp = number;
        for (int r = 0; r < 5; r++)
        {
            if (number >= pwrs[r] || pwrs[r] < decimal) {
                to_print[q] = tmp / pwrs[r];
                tmp -= (to_print[q++] * pwrs[r]);
            } else if (!decimal) {
                // Padding for left of decimal
                x_pos += 10;
            }
        }

        // Are there any digits to print? Then print them!
        if (q) {
            for (int d = 0; d < q; d++)
            {
                PrintDigit(to_print[d]);
            }
        }
        decimal = 0;
    }

    void Print(int number) {
        Print(x_pos, y_pos, number);
    }

    void DecimalPoint(int decimal_) {
        for (int x = 0; x < 2; x++)
        {
            for (int y = 0; y < 3; y++)
            {
                graphics.setPixel(x_pos + x + 1, y_pos + y + 9);
            }
        }
        x_pos += 6;
        decimal = decimal_;
    }

private:
    Segment segment[7];
    uint8_t digit[10];
    uint8_t x_pos;
    uint8_t y_pos;
    int decimal;

    void PrintDigit(uint8_t d) {
        for (uint8_t b = 0; b < 7; b++)
        {
            if ((digit[d] >> b) & 0x01) segment[b].DrawAt(x_pos, y_pos);
        }
        x_pos += 10;
    }
};

#endif /* SEGMENTDISPLAY_H */
