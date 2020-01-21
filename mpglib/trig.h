//
//  trig.h : Basic trigonometry routines.
//
//! \file trig.h
//! \author J Vijn
//! \date 20080130 - 20080210
//
/* === NOTES ===
*/

#ifndef __TRIG_H__
#define __TRIG_H__

// These things may already be present in other headers.
typedef unsigned int uint;

#ifndef countof
#define countof(array)	( sizeof(array)/sizeof(array[0]) )
#endif

#ifndef ALIGN
#define ALIGN(n)	__attribute__((aligned(n)))
#endif

// --------------------------------------------------------------------
// CONSTANTS
// --------------------------------------------------------------------

static const uint BRAD_PI_SHIFT=14,   BRAD_PI = 1<<BRAD_PI_SHIFT;
static const uint BRAD_HPI= BRAD_PI/2, BRAD_2PI= BRAD_PI*2; 

// --------------------------------------------------------------------
// PROTOTYPES 
// --------------------------------------------------------------------

int isin(int x);
int icos(int x);
int itan(int x);

uint atan2Null(int x, int y);
uint atan2Oct(int x, int y);
uint atan2OctDiv(int x, int y);
uint atan2Lookup(int x, int y);
uint atan2Lerp(int x, int y);
uint atan2InvLerp(int x, int y);
uint atan2Taylor(int x, int y);
uint atan2Gba(int x, int y);
uint atan2Tonc(int x, int y);
uint atan2Sin(int x, int y);
uint atan2Cordic(int x, int y);


#endif // __TRIG_H__

// EOF
