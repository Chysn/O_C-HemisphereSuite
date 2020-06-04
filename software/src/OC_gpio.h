#ifndef OC_GPIO_H_
#define OC_GPIO_H_

#include "OC_options.h"

#ifdef FLIP_180
  #define CV4 19
  #define CV3 18
  #define CV2 20
  #define CV1 17
  
  #define TR4 0
  #define TR3 1
  #define TR2 2
  #define TR1 3
  
  #define but_top 4
  #define but_bot 5
#else
  #define CV1 19
  #define CV2 18
  #define CV3 20
  #define CV4 17
  
  #define TR1 0
  #define TR2 1
  #define TR3 2
  #define TR4 3
  
  #define but_top 5
  #define but_bot 4
#endif

#define OLED_DC 6
#define OLED_RST 7
#define OLED_CS 8

// OLED CS is active low
#define OLED_CS_ACTIVE LOW
#define OLED_CS_INACTIVE HIGH

#define DAC_RST 9
#define DAC_CS 10

// NOTE: encoder pins R1/R2 changed for rev >= 2c
#ifdef FLIP_180
  #define encL1 16
  #define encL2 15
  #define butL  14
  
  #define encR1 22
  #define encR2 21
  #define butR  23
#else
  #define encR1 16
  #define encR2 15
  #define butR  14
  
  #define encL1 22
  #define encL2 21
  #define butL  23
#endif

// NOTE: back side :(
#define OC_GPIO_DEBUG_PIN1 24
#define OC_GPIO_DEBUG_PIN2 25

#define OC_GPIO_BUTTON_PINMODE INPUT_PULLUP
#define OC_GPIO_TRx_PINMODE INPUT_PULLUP
#define OC_GPIO_ENC_PINMODE INPUT_PULLUP

#endif // OC_GPIO_H_
