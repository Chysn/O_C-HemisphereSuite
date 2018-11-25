// Copyright (c) 2018, Jason Justian
//
// Bjorklund pattern filter, Copyright (c) 2016 Tim Churches
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

#include "bjorklund.h"
#define AF_DISPLAY_TIMEOUT 330000

struct AFStepCoord {
    uint8_t x;
    uint8_t y;
};

class AnnularFusion : public HemisphereApplet {
public:

    const char* applet_name() {
        return "AnnularFu";
    }

    void Start() {
        display_timeout = AF_DISPLAY_TIMEOUT;
        ForEachChannel(ch)
        {
            length[ch] = 16;
            beats[ch] = 4 + (ch * 4);
            pattern[ch] = EuclideanPattern(length[ch] - 1, beats[ch], 0);;
        }
        step = 0;
        SetDisplayPositions(0, 24);
        SetDisplayPositions(1, 16);
        last_clock = OC::CORE::ticks;
    }

    void Controller() {
        if (Clock(1)) step = 0; // Reset

        // Advance both rings
        if (Clock(0)) {
            last_clock = OC::CORE::ticks;
            ForEachChannel(ch)
            {
                int rotation = Proportion(DetentedIn(ch), HEMISPHERE_MAX_CV, length[ch]);

                // Store the pattern for display
                pattern[ch] = EuclideanPattern(length[ch] - 1, beats[ch], rotation);
                int sb = step % length[ch];
                if ((pattern[ch] >> sb) & 0x01) {
                    ClockOut(ch);
                }
            }

            // Plan for the thing to run forever and ever
            if (++step >= length[0] * length[1]) step = 0;
        }
        if (display_timeout > 0) --display_timeout;
    }

    void View() {
        gfxHeader(applet_name());
        DrawSteps();
        if (display_timeout > 0) DrawEditor();
    }

    void OnButtonPress() {
        display_timeout = AF_DISPLAY_TIMEOUT;
        if (++cursor > 3) cursor = 0;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        display_timeout = AF_DISPLAY_TIMEOUT;
        int ch = cursor < 2 ? 0 : 1;
        int f = cursor - (ch * 2); // Cursor function
        if (f == 0) {
            length[ch] = constrain(length[ch] + direction, 3, 32);
            if (beats[ch] > length[ch]) beats[ch] = length[ch];
            SetDisplayPositions(ch, 24 - (8 * ch));
        }
        if (f == 1) {
            beats[ch] = constrain(beats[ch] + direction, 1, length[ch]);
        }
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,4}, length[0] - 1);
        Pack(data, PackLocation {4,4}, beats[0] - 1);
        Pack(data, PackLocation {8,4}, length[1] - 1);
        Pack(data, PackLocation {12,4}, beats[1] - 1);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        length[0] = Unpack(data, PackLocation {0,4}) + 1;
        beats[0] = Unpack(data, PackLocation {4,4}) + 1;
        length[1] = Unpack(data, PackLocation {8,4}) + 1;
        beats[1] = Unpack(data, PackLocation {12,4}) + 1;
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "Rotate 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_OUTS]     = "Clock A=Ch1 B=Ch2";
        help[HEMISPHERE_HELP_ENCODER]  = "Length/Hits Ch1,2";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int step;
    int cursor = 0; // Ch1: 0=Length, 1=Hits; Ch2: 2=Length 3=Hits
    AFStepCoord disp_coord[2][32];
    uint32_t pattern[2];
    int last_clock;
    uint32_t display_timeout;
    
    // Settings
    int length[2];
    int beats[2];

    void DrawSteps() {
        ForEachChannel(ch)
        {
            DrawActiveSegment(ch);
            DrawPatternPoints(ch);
        }
    }

    void DrawActiveSegment(int ch) {
        if (last_clock && OC::CORE::ticks - last_clock < 166666) {
            int s1 = step % length[ch];
            int s2 = s1 + 1 == length[ch] ? 0 : s1 + 1;

            AFStepCoord s1_c = disp_coord[ch][s1];
            AFStepCoord s2_c = disp_coord[ch][s2];
            gfxLine(s1_c.x, s1_c.y, s2_c.x, s2_c.y);
        }
    }

    void DrawPatternPoints(int ch) {
        for (int p = 0; p < length[ch]; p++)
        {
            if ((pattern[ch] >> p) & 0x01) {
                gfxPixel(disp_coord[ch][p].x, disp_coord[ch][p].y);
                gfxPixel(disp_coord[ch][p].x + 1, disp_coord[ch][p].y);
            }
        }
    }

    void DrawEditor() {
        int ch = cursor < 2 ? 0 : 1; // Cursor channel
        int f = cursor - (ch * 2); // Cursor function

        // Length cursor
        gfxBitmap(1, 15, 8, LOOP_ICON);
        gfxPrint(12 + pad(10, length[ch]), 15, length[ch]);
        if (f == 0) gfxCursor(13, 23, 12);

        // Beats cursor
        gfxBitmap(1, 25, 8, X_NOTE_ICON);
        gfxPrint(12 + pad(10, beats[ch]), 25, beats[ch]);
        if (f == 1) gfxCursor(13, 33, 12);

        // Ring indicator
        gfxCircle(8, 52, 8);
        gfxCircle(8, 52, 4);

        if (ch == 0) gfxCircle(8, 52, 7);
        else gfxCircle(8, 52, 5);
    }

    /* Get coordinates of circle in two halves, from the top and from the bottom */
    void SetDisplayPositions(int ch, int r) {
        int cx = 31; // Center coordinates
        int cy = 39;
        int di = 0; // Display index (positions actually used in the display)
        int c_count = 0; // Count of pixels along the circumference
        int x_per_step = (r * 4) / 32;
        uint32_t pattern = EuclideanPattern(31, length[ch], 0);

        // Sweep across the top of the circle looking for positions within the
        // radius of the circle. Left to right:
        for (uint8_t x = 0; x < 63; x++)
        {
            // Top down
            for (uint8_t y = 0; y < 63; y++)
            {
                int rx = cx - x; // Positions relative to center
                int ry = cy - y;

                // Is this point within the radius?
                if (rx * rx + ry * ry < r * r + 1) {
                    if (c_count++ % x_per_step == 0) {
                        if (pattern & 0x01) disp_coord[ch][di++] = AFStepCoord {x, y};
                        pattern = pattern >> 0x01;
                    }
                    break; // Only use the first point
                }
            }
        }

        // Sweep across the top of the circle looking for positions within the
        // radius of the circle. Right to left:
        for (uint8_t x = 63; x > 0; x--)
        {
            // Bottom up
            for (uint8_t y = 63; y > 0; y--)
            {
                int rx = cx - x; // Positions relative to center
                int ry = cy - y;

                // Is this point within the radius?
                if (rx * rx + ry * ry < r * r + 1) {
                    if (c_count++ % x_per_step == 0) {
                        if (pattern & 0x01) disp_coord[ch][di++] = AFStepCoord {x, y};
                        pattern = pattern >> 0x01;
                    }
                    break; // Only use the first point
                }
            }
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to AnnularFusion,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
AnnularFusion AnnularFusion_instance[2];

void AnnularFusion_Start(bool hemisphere) {
    AnnularFusion_instance[hemisphere].BaseStart(hemisphere);
}

void AnnularFusion_Controller(bool hemisphere, bool forwarding) {
    AnnularFusion_instance[hemisphere].BaseController(forwarding);
}

void AnnularFusion_View(bool hemisphere) {
    AnnularFusion_instance[hemisphere].BaseView();
}

void AnnularFusion_OnButtonPress(bool hemisphere) {
    AnnularFusion_instance[hemisphere].OnButtonPress();
}

void AnnularFusion_OnEncoderMove(bool hemisphere, int direction) {
    AnnularFusion_instance[hemisphere].OnEncoderMove(direction);
}

void AnnularFusion_ToggleHelpScreen(bool hemisphere) {
    AnnularFusion_instance[hemisphere].HelpScreen();
}

uint32_t AnnularFusion_OnDataRequest(bool hemisphere) {
    return AnnularFusion_instance[hemisphere].OnDataRequest();
}

void AnnularFusion_OnDataReceive(bool hemisphere, uint32_t data) {
    AnnularFusion_instance[hemisphere].OnDataReceive(data);
}
