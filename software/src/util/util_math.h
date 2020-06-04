// Copyright (c) 2016 Patrick Dowling
//
// Author: Patrick Dowling (pld@gurkenkiste.com)
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

#ifndef UTIL_MATH_H_
#define UTIL_MATH_H_

// Woo. Funky macro magic to avoid dividing by non-power-of-two.
// Essentially a quick fixed-point calculation, but only valid up to 2^exp

#define FAST_FP_DIV(n, div, exp) \
  (((n) * (((1 << exp) + 1) / div)) >> exp)

#define FAST_FP_MOD(n, div, exp) \
  ((n) - FAST_FP_DIV(n, div, exp) * div)

#define DIV_8(n, div) \
  FAST_FP_DIV(n, div, 8)

#define MOD_8(n, div) \
  FAST_FP_MOD(n, div, 8)

inline uint32_t USAT16(uint32_t value) __attribute__((always_inline));
inline uint32_t USAT16(uint32_t value) {
  uint32_t result;
  __asm("usat %0, %1, %2" : "=r" (result) : "I" (16), "r" (value));
  return result;
}

inline uint32_t USAT16(int32_t value) __attribute__((always_inline));
inline uint32_t USAT16(int32_t value) {
  uint32_t result;
  __asm("usat %0, %1, %2" : "=r" (result) : "I" (16), "r" (value));
  return result;
}

static inline uint32_t multiply_u32xu32_rshift24(uint32_t a, uint32_t b) __attribute__((always_inline));
static inline uint32_t multiply_u32xu32_rshift24(uint32_t a, uint32_t b)
{
  register uint32_t lo, hi;
  asm volatile("umull %0, %1, %2, %3" : "=r" (lo), "=r" (hi) : "r" (a), "r" (b));
  return (lo >> 24) | (hi << 8);
}

static inline uint32_t multiply_u32xu32_rshift(uint32_t a, uint32_t b, uint32_t shift) __attribute__((always_inline));
static inline uint32_t multiply_u32xu32_rshift(uint32_t a, uint32_t b, uint32_t shift)
{
  register uint32_t lo, hi;
  asm volatile("umull %0, %1, %2, %3" : "=r" (lo), "=r" (hi) : "r" (a), "r" (b));
  return (lo >> shift) | (hi << (32 - shift));
}

template <typename T, T smoothing>
struct SmoothedValue {
  SmoothedValue() : value_(0) { }

  T value_;

  T value() const {
    return value_;
  }

  void push(T value) {
    value_ = (value_ * (smoothing - 1) + value) / smoothing;
  }

  void set(T value) {
    value_ = value;
  }
};

#define SCALE8_16(x) ((((x + 1) << 16) >> 8) - 1)

#endif // UTIL_MATH_H_
