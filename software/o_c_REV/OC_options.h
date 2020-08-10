/*
 *
 * compile options.
 *
 */

#ifndef OC_OPTIONS_H_
#define OC_OPTIONS_H_

/* ------------ uncomment for Buchla/non-octaval/quartertone support: -------------------------------  */
//#define BUCHLA_SUPPORT
/* ------------ uncomment for use with Northernlight cOC program card: ------------------------------  */
//#define BUCHLA_cOC
/* ------------ uncomment for use with Northernlight 4U / cOC or 2OC: -------------------------------  */
//#define BUCHLA_4U
/* ------------ uncomment for use with Northernlight 2OC on the left side: --------------------------  */
//#define NORTHERNLIGHT_2OC_LEFTSIDE
/* ------------ print debug messages to USB serial --------------------------------------------------  */
//#define PRINT_DEBUG
/* ------------ flip screen / IO mapping ------------------------------------------------------------  */
//#define FLIP_180
/* ------------ invert screen pixels ----------------------------------------------------------------  */
//#define INVERT_DISPLAY
/* ------------ use DAC8564 -------------------------------------------------------------------------  */
//#define DAC8564

// This is the branch for the 10V OC Plus and VOR support, so these default to set

/* ------------ uncomment for use with Plum Audio VOR anabled versions (OCP, 1uO_c v2, 4Robots) ---------------------------------------------------------  */
#define VOR
/* ------------ uncomment for use with Plum Audio 1uO_c 4Robots (To use Up button long press to change VOR instead activate screensaver) ----------------  */
#define VOR_NO_RANGE_BUTTON

#if defined(VOR)
  #define IO_10V
#endif

#endif

