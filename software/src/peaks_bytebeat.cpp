// Copyright 2015, 2013 Tim Churches, Émilie Gillet
//
// Author: Tim Churches (tim.churches@gmail.com)
// Loosely based on Peaks unbuffered processor code by: Émilie Gillet (Mutable Instruments)
// Byte beat equations: as indicated in the body of the code below.
// BitWiz equations used with permission of Jonatan Liljedahl <lijon@kymatica.com> (BitWiz author)
// Equation Composer equations from Microbe Modular Equation Composer - see https://github.com/clone45/EquationComposer
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
// Byte beats algorithms.

#include "peaks_bytebeat.h"

#include "extern/stmlib_utils_dsp.h"
// #include "util/stmlib_random.h"

namespace peaks {

using namespace stmlib;

const uint8_t kDownsample = 4;
const uint8_t kMaxEquationIndex = 1;

void ByteBeat::Init() {
  equation_ = 0;
  speed_ = 32678;
  pitch_ = 1;
  loopmode_ = false ;
  loop_start_ = 0 ;
  loop_end_ = 255 << 24 ;
  phase_ = 0 ;
  t_ = 0 ;
  p0_ = 127;
  p1_ = 127;
  p2_ = 127;
  stepmode_ = false ;
  last_sample_ = 13 ;

}

uint16_t ByteBeat::ProcessSingleSample(uint8_t control) {

  uint16_t sample = 0;
   
  if (control & CONTROL_GATE_RISING) {
    if (stepmode_) {
      ++t_ ;
    } else {
      phase_ = 0;
      if (loopmode_) {
        t_ = loop_start_ ;
      } else {
        t_ = 0 ;
      }
    }
  }

  if (!stepmode_) {
    ++phase_ ; 
  }    
  
  if (loopmode_ && (t_ < loop_start_ || t_ > loop_end_)) {
     t_ = loop_start_ ;
     phase_ = 0 ;
  }

  if (!stepmode_ && (phase_ % bytepitch_ == 0)) ++t_; 
// These equations push the boundaries of precedence comprehension.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
  uint8_t pitch = pitch_ ;
  uint8_t p0 = p0_ ;
  uint8_t p1 = p1_ ;
  uint8_t p2 = p2_ ;
    switch (equation_index_) {
        case 0: // hope - pitch OK
          // from http://royal-paw.com/2012/01/bytebeats-in-c-and-python-generative-symphonies-from-extremely-small-programs/
          // (atmospheric, hopeful)
          // sample = ( ( ((t_*3) & ((t_*pitch)>>10)) | ((t_*p0) & ((t_*pitch)>>10)) | ((t_*10) & (((t_*pitch)>>8)*p1) & p2) ) & 0xFF);
          // sample = ( ( ((t_*3) & ((t_*pitch)>>10)) | ((t_*p0) & ((t_*pitch)>>10)) | ((t_*10) & ((t_>>8)*p1) & p2) ) & 0xFF);
          sample = ( ( (((t_*pitch)*3) & (t_>>10)) | (((t_*pitch)*p0) & (t_>>10)) | ((t_*10) & ((t_>>8)*p1) & p2) ) & 0xFF);
          break;
        case 1: // love - pitch OK
          // equation by stephth via https://www.youtube.com/watch?v=tCRPUv8V22o at 3:38
          sample = (((((t_*pitch)*p0) & (t_>>4)) | ((t_*p2) & (t_>>7)) | ((t_*p1) & (t_>>10))) & 0xFF);
          break;
        case 2: // life - pitch OK
          // This one is the second one listed at from http://xifeng.weebly.com/bytebeats.html
          sample = ((( ((((((t_*pitch) >> p0) | (t_*pitch)) | ((t_*pitch) >> p0)) * p2) & ((5 * (t_*pitch)) | ((t_*pitch) >> p2)) ) | ((t_*pitch) ^ (t_ % p1)) ) & 0xFF));
          break;
       case 3:// age - pitch disabled
          // Arp rotator (equation 9 from Equation Composer Ptah bank)
          sample = (((t_)>>(p2>>4))&((t_)<<3)/((t_)*p1*((t_)>>11)%(3+(((t_)>>(16-(p0>>4)))%22))));
          break ;
        case 4: // clysm - pitch almost no effect
          //  BitWiz Transplant via Equation Composer Ptah bank 
          sample = ((t_*pitch)-(((t_*pitch)&p0)*p1-1668899)*(((t_*pitch)>>15)%15*(t_*pitch)))>>(((t_*pitch)>>12)%16)>>(p2%15);
          break ;
        case 5: // monk - pitch OK
          // Vocaliser from Equation Composer Khepri bank         
          sample = (((t_*pitch)%p0>>2)&p1)*(t_>>(p2>>5));
          break;
        case 6: // NERV - horrible!
          // Chewie from Equation Composer Khepri bank         
          sample = (p0-(((p2+1)/(t_*pitch))^p0|(t_*pitch)^922+p0))*(p2+1)/p0*(((t_*pitch)+p1)>>p1%19);
          break;
        case 7: // Trurl - pitch OK
          // Tinbot from Equation Composer Sobek bank   
          sample = ((t_*pitch)/(40+p0)*((t_*pitch)+(t_*pitch)|4-(p1+20)))+((t_*pitch)*(p2>>5));
          break;
        case 8: // Pirx  - pitch OK
          // My Loud Friend from Equation Composer Ptah bank   
          sample = ((((t_*pitch)>>((p0>>12)%12))%(t_>>((p1%12)+1))-(t_>>((t_>>(p2%10))%12)))/((t_>>((p0>>2)%15))%15))<<4;
          break;
        case 9: //Snaut
          // GGT2 from Equation Composer Ptah bank
          // sample = ((p0|(t_>>(t_>>13)%14))*((t_>>(p0%12))-p1&249))>>((t_>>13)%6)>>((p2>>4)%12);
          // "A bit high-frequency, but keeper anyhow" from Equation Composer Khepri bank.
          sample = ((t_*pitch)+last_sample_+p1/p0)%(p0|(t_*pitch)+p2);
           break;
        case 10: // Hari
          // The Signs, from Equation Composer Ptah bank
          sample = ((0&(251&((t_*pitch)/(100+p0))))|((last_sample_/(t_*pitch)|((t_*pitch)/(100*(p1+1))))*((t_*pitch)|p2)));
          break;
        case 11: // Kris - pitch OK
         // Light Reactor from Equation Composer Ptah bank
          sample = (((t_*pitch)>>3)*(p0-643|(325%t_|p1)&t_)-((t_>>6)*35/p2%t_))>>6;
          break;
        case 12: // Tichy
          sample = (t_*pitch_)>>7 & t_>>7 | t_>>8;
          // Alpha from Equation Composer Khepri bank
          // sample = ((((t_*pitch)^(p0>>3)-456)*(p1+1))/((((t_*pitch)>>(p2>>3))%14)+1))+((t_*pitch)*((182>>((t_*pitch)>>15)%16))&1) ;
          break;
        case 13: // Bregg - pitch OK
          // Hooks, from Equation Composer Khepri bank.
          sample = ((t_*pitch)&(p0+2))-(t_/p1)/last_sample_/p2;
          break;            
        case 14: // Avon - pitch OK
          // Widerange from Equation Composer Khepri bank
          sample = (((p0^((t_*pitch)>>(p1>>3)))-(t_>>(p2>>2))-t_%(t_&p1)));
          break;        
        case 15: // Orac
          // Abducted, from Equation Composer Ptah bank
          sample = (p0+(t_*pitch)>>p1%12)|((last_sample_%(p0+(t_*pitch)>>p0%4))+11+p2^t_)>>(p2>>12);
          break;
        default:
          sample = 0 ;
          break;          
  }
#pragma GCC diagnostic pop
  last_sample_ = sample ;
  return sample << 8 ;
}

// wrapper for use in QQ (Quantermain)
uint16_t ByteBeat::Clock() {
  stepmode_ = true;
  return ByteBeat::ProcessSingleSample(CONTROL_GATE_RISING) ;
}

}  // namespace peaks
