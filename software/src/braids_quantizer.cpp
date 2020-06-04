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

#include "braids_quantizer.h"
#include "OC_options.h"

#include <algorithm>
#include <cstdlib>

namespace braids {

void SortScale(Scale &scale) {
  std::sort(scale.notes, scale.notes + scale.num_notes);
}


void Quantizer::Init() {
  enabled_ = true;
  codeword_ = 0;
  transpose_ = 0;
  previous_boundary_ = 0;
  next_boundary_ = 0;
  for (int16_t i = 0; i < 128; ++i) {
    codebook_[i] = (i - 64) << 7;
  }
}

int32_t Quantizer::Process(int32_t pitch, int32_t root, int32_t transpose) {
  if (!enabled_) {
    return pitch;
  }

  pitch -= root;
  #ifdef BUCHLA_4U
    pitch -= ((12 << 7) << 2);
  #else
    pitch -= ((12 << 7) << 1);
  #endif
  if (!requantize_ && (pitch >= previous_boundary_ && pitch <= next_boundary_ && transpose == transpose_)) {
    // We're still in the voronoi cell for the active codeword.
    pitch = codeword_;
  } else {
    // Search for the nearest neighbour in the codebook.
    int16_t upper_bound_index = std::upper_bound(
        &codebook_[3],
        &codebook_[126],
        static_cast<int16_t>(pitch)) - &codebook_[0];
    int16_t lower_bound_index = upper_bound_index - 2;

    int16_t best_distance = 16384;
    int16_t q = -1;
    for (int16_t i = lower_bound_index; i <= upper_bound_index; ++i) {
      int16_t distance = abs(pitch - codebook_[i]);
      if (distance < best_distance) {
        best_distance = distance;
        q = i;
      }
    }

    // Enlarge the current voronoi cell a bit for hysteresis.
    previous_boundary_ = (9 * codebook_[q - 1] + 7 * codebook_[q]) >> 4;
    next_boundary_ = (9 * codebook_[q + 1] + 7 * codebook_[q]) >> 4;

    // Apply transpose after setting up boundaries
    q += transpose;
    if (q < 1) q = 1;
    else if (q > 126) q = 126;
    note_number_ = q;
    codeword_ = codebook_[q];
    transpose_ = transpose;
    pitch = codeword_;
  }
  pitch += root;
  #ifdef BUCHLA_4U
    pitch += ((12 << 7) << 2);
  #else
    pitch += ((12 << 7) << 1);
  #endif
  return pitch;
}

int32_t Quantizer::Lookup(int32_t index) const {
  if (index < 0)
    return codebook_[0];
  else if (index > 127)
    return codebook_[127];
  else
    return codebook_[index];
}

void Quantizer::Requantize() {
    requantize_ = 1;
}

}  // namespace braids
