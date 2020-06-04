// Copyright (c) 2015 Patrick Dowling
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

#ifndef EEPROMSTORAGE_H_
#define EEPROMSTORAGE_H_

#include <EEPROM.h>

/* Define a storage implemenation using teensy EEPROM */
struct EEPROMStorage {
  static const size_t LENGTH = 2048;

  static void update(size_t addr, const void *data, size_t length) {
    EEPtr e = addr;
    const uint8_t *src = (const uint8_t *)data;
    while (length--)
      (*e++).update(*src++);
  }

  static void write(size_t addr, const void *data, size_t length) {
    EEPtr e = addr;
    const uint8_t *src = (const uint8_t*)data;
    while (length--)
      (*e++) = (*src++);
  }

  static void read(size_t addr, void *data, size_t length) {
    EEPtr e = addr;
    uint8_t *dst = (uint8_t*)data;
    while (length--)
      *dst++ = *e++;
  }
};

#endif // EEPROMSTORAGE_H_
