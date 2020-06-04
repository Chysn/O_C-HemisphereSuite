// Copyright 2015 Émilie Gillet.
//
// Author: Émilie Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Note quantizer

#ifndef BRAIDS_QUANTIZER_H_
#define BRAIDS_QUANTIZER_H_

//#include "stmlib/stmlib.h"
#include "util/util_macros.h"

namespace braids {
  
struct Scale {
  int16_t span;
  size_t num_notes;
  int16_t notes[16];
};

void SortScale(Scale &);

class Quantizer {
 public:
  Quantizer() { }
  ~Quantizer() { }
  
  void Init();
  
  int32_t Process(int32_t pitch) {
    return Process(pitch, 0, 0);
  }
  
  int32_t Process(int32_t pitch, int32_t root, int32_t transpose);
  
  void Configure(const Scale& scale, uint16_t mask = 0xffff) {
    Configure(scale.notes, scale.span, scale.num_notes, mask);
  }

  bool enabled() const {
    return enabled_;
  }

  // HACK for TM
  int32_t Lookup(int32_t index) const;

  // Force Process to process again
  void Requantize();

 private:
  bool enabled_;
  int16_t enabled_notes_[16];
  int16_t codebook_[128];
  int32_t codeword_;
  int32_t transpose_;
  int32_t previous_boundary_;
  int32_t next_boundary_;
  uint16_t note_number_;
  bool requantize_;

  inline void Configure(const int16_t* notes, int16_t scale_span, size_t num_notes, uint16_t mask)
  {  
    enabled_ = notes != NULL && num_notes != 0 && scale_span != 0 && (mask & ~(0xffff<<num_notes));
    if (enabled_) {
  
      // Build up array that contains only the enabled notes, and use that to
      // generate the codebook. This avoids a bunch of issues and checks in the
      // main generating loop.
      size_t num_enabled_notes = 0;
      for (size_t i = 0; i < num_notes; ++i) {
        if (mask & 1)
          enabled_notes_[num_enabled_notes++] = notes[i];
        mask >>= 1;
      }
      notes = enabled_notes_;
     
      int32_t octave = 0;
      size_t note = 0;
      int16_t span = scale_span;
      int16_t *codebook;
      
      codebook = &codebook_[0];
    
      for (int32_t i = 0; i < 64; ++i) {
        int32_t up = notes[note] + span * octave;
        int32_t down = notes[num_enabled_notes - 1 - note] + (-octave - 1) * span;
        CLIP(up)
        CLIP(down)
        *(codebook + 64 + i) = up;
        *(codebook + 63 - i) = down;
        ++note;
        if (note >= num_enabled_notes) {
          note = 0;
          ++octave;
        }
      }
    }
  }

  DISALLOW_COPY_AND_ASSIGN(Quantizer);
};

}  // namespace braids

#endif // BRAIDS_QUANTIZER_H_
