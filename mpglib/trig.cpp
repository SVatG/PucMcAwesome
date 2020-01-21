//
//  trig.cpp : Basic trigonometry routines.
//
//! \file trig.cpp
//! \author J Vijn
//! \date 20080130 - 20080210
//
/* === NOTES ===
	Multiple atan2's present. Select the one you want.
	Best performance tends to be with atan2Lerp or possibly atan2Tonc.
	If your division really sucks, use atan2Cordic.
	
	This module still needs a header for some system-specific basics 
	before you can use it. Specifically, ALIGN and QDIV will needs some 
	extra effort.
*/

#include "trig.h"

// --------------------------------------------------------------------
// MACROS / INLINES
// --------------------------------------------------------------------

// Get the octant a coordinate pair is in. 
#define OCTANTIFY(_x, _y, _o)	do {							\
	int _t;	_o= 0;												\
	if(_y<  0)	{			 _x= -_x;   _y= -_y; _o += 4; }		\
	if(_x<= 0)	{ _t= _x;    _x=  _y;   _y= -_t; _o += 2; }		\
	if(_x<=_y)	{ _t= _y-_x; _x= _x+_y; _y=  _t; _o += 1; }		\
} while(0);


#if defined(__SYS_GBA__)		
// GBA specific

static inline int QDIV(int num, int den, int bits)
{
	extern int Div(int, int);
	return Div(num<<bits), den);
}

#else if defined(__SYS_NDS__)
// NDS specific

// Special-case division because I need a little more control 
// than divf32 offers
static inline int QDIV(int num, int den, const int bits)
{
	while(REG_DIVCNT & DIV_BUSY);
	REG_DIVCNT = DIV_64_32;

	REG_DIV_NUMER = ((int64)num)<<bits;
	REG_DIV_DENOM_L = den;

	while(REG_DIVCNT & DIV_BUSY);

	return (REG_DIV_RESULT_L);
}
#else

static inline int QDIV(int num, int den, int bits)
{
	return (num<<bits)/den;
}

#endif


// --------------------------------------------------------------------
// CONSTANTS
// --------------------------------------------------------------------

static const uint SINLUT_ONE= 0x8000;
static const uint SINLUT_PI_SHIFT= 8, SINLUT_PI= 1<<SINLUT_PI_SHIFT;
static const uint SINLUT_HPI= SINLUT_PI/2, SINLUT_2PI= SINLUT_PI*2;
static const uint SINLUT_STRIDE= BRAD_PI/SINLUT_PI, SINLUT_STRIDE_SHIFT= 6;

static const uint TANLUT_ONE= 0x10000;
static const uint TANLUT_PI_SHIFT= 8, TANLUT_PI= 0x100;
static const uint TANLUT_HPI= TANLUT_PI/2, TANLUT_2PI= TANLUT_PI*2;
static const uint TANLUT_STRIDE= BRAD_PI/TANLUT_PI, TANLUT_STRIDE_SHIFT= 6;

static const uint ATAN_ONE = 0x1000, ATAN_FP= 12, ATAN_PI = BRAD_PI;
static const uint ATANLUT_STRIDE = ATAN_ONE / 0x80, ATANLUT_STRIDE_SHIFT= 5;


// --------------------------------------------------------------------
// LUTS
// --------------------------------------------------------------------

//{{SINLUT
#define SINLUT_SIZE		130
#define SINLUT_FP		15
extern const unsigned short sinLUT[130];

// Sine LUT. Interval: [0, PI/2]; PI= 0x100, Q15 values.
const unsigned short sinLUT[130] ALIGN(4)=
{
	0x0000,0x0192,0x0324,0x04B6,0x0648,0x07D9,0x096B,0x0AFB,
	0x0C8C,0x0E1C,0x0FAB,0x113A,0x12C8,0x1455,0x15E2,0x176E,
	0x18F9,0x1A83,0x1C0C,0x1D93,0x1F1A,0x209F,0x2224,0x23A7,
	0x2528,0x26A8,0x2827,0x29A4,0x2B1F,0x2C99,0x2E11,0x2F87,
	0x30FC,0x326E,0x33DF,0x354E,0x36BA,0x3825,0x398D,0x3AF3,
	0x3C57,0x3DB8,0x3F17,0x4074,0x41CE,0x4326,0x447B,0x45CD,
	0x471D,0x486A,0x49B4,0x4AFB,0x4C40,0x4D81,0x4EC0,0x4FFB,
	0x5134,0x5269,0x539B,0x54CA,0x55F6,0x571E,0x5843,0x5964,
// 64
	0x5A82,0x5B9D,0x5CB4,0x5DC8,0x5ED7,0x5FE4,0x60EC,0x61F1,
	0x62F2,0x63EF,0x64E9,0x65DE,0x66D0,0x67BD,0x68A7,0x698C,
	0x6A6E,0x6B4B,0x6C24,0x6CF9,0x6DCA,0x6E97,0x6F5F,0x7023,
	0x70E3,0x719E,0x7255,0x7308,0x73B6,0x7460,0x7505,0x75A6,
	0x7642,0x76D9,0x776C,0x77FB,0x7885,0x790A,0x798A,0x7A06,
	0x7A7D,0x7AEF,0x7B5D,0x7BC6,0x7C2A,0x7C89,0x7CE4,0x7D3A,
	0x7D8A,0x7DD6,0x7E1E,0x7E60,0x7E9D,0x7ED6,0x7F0A,0x7F38,
	0x7F62,0x7F87,0x7FA7,0x7FC2,0x7FD9,0x7FEA,0x7FF6,0x7FFE,
// 128
	0x8000,0x7FFE
};
//}}SINLUT


//{{TANLUT
#define TANLUT_SIZE		129
#define TANLUT_FP		16
extern const unsigned int tanLUT[129];

// Tangens LUT, domain: [0, PI/2]; PI= 0x100, Q16 values.
// tan(PI/2) set to 400.
const unsigned int tanLUT[129]=
{
	0x00000000,0x00000324,0x00000649,0x0000096E,0x00000C94,0x00000FBA,0x000012E2,0x0000160C,
	0x00001937,0x00001C64,0x00001F93,0x000022C5,0x000025F9,0x00002931,0x00002C6C,0x00002FAA,
	0x000032EC,0x00003632,0x0000397D,0x00003CCC,0x00004020,0x00004379,0x000046D8,0x00004A3D,
	0x00004DA8,0x0000511A,0x00005492,0x00005812,0x00005B99,0x00005F28,0x000062C0,0x00006660,
	0x00006A0A,0x00006DBD,0x0000717A,0x00007542,0x00007914,0x00007CF2,0x000080DC,0x000084D2,
	0x000088D6,0x00008CE7,0x00009106,0x00009534,0x00009971,0x00009DBE,0x0000A21C,0x0000A68C,
	0x0000AB0E,0x0000AFA3,0x0000B44C,0x0000B909,0x0000BDDD,0x0000C2C7,0x0000C7C9,0x0000CCE3,
	0x0000D218,0x0000D768,0x0000DCD4,0x0000E25E,0x0000E806,0x0000EDD0,0x0000F3BB,0x0000F9CB,
// 64
	0x00010000,0x0001065D,0x00010CE3,0x00011394,0x00011A74,0x00012184,0x000128C6,0x0001303F,
	0x000137F0,0x00013FDD,0x00014809,0x00015077,0x0001592D,0x0001622E,0x00016B7E,0x00017523,
	0x00017F22,0x00018980,0x00019445,0x00019F76,0x0001AB1C,0x0001B73F,0x0001C3E7,0x0001D11F,
	0x0001DEF1,0x0001ED6A,0x0001FC96,0x00020C84,0x00021D44,0x00022EE9,0x00024187,0x00025534,
	0x00026A0A,0x00028026,0x000297A8,0x0002B0B5,0x0002CB79,0x0002E823,0x000306EC,0x00032816,
	0x00034BEB,0x000372C6,0x00039D11,0x0003CB48,0x0003FE02,0x000435F7,0x00047405,0x0004B940,
	0x00050700,0x00055EF9,0x0005C35D,0x00063709,0x0006BDD0,0x00075CE6,0x00081B98,0x0009046E,
	0x000A2736,0x000B9CC6,0x000D8E82,0x001046EA,0x00145B00,0x001B2672,0x0028BC49,0x00517BB6,
// 128
	0x01900000
};
//}}TANLUT


//{{ATANLUT
#define ATANLUT_SIZE	130
#define ATANLUT_FP		15
extern const unsigned short atanLUT[130];

// Arctangens LUT. Interval: [0, 1] (one=128); PI=0x20000
const unsigned short atanLUT[130] ALIGN(4)=
{
	0x0000,0x0146,0x028C,0x03D2,0x0517,0x065D,0x07A2,0x08E7,
	0x0A2C,0x0B71,0x0CB5,0x0DF9,0x0F3C,0x107F,0x11C1,0x1303,
	0x1444,0x1585,0x16C5,0x1804,0x1943,0x1A80,0x1BBD,0x1CFA,
	0x1E35,0x1F6F,0x20A9,0x21E1,0x2319,0x2450,0x2585,0x26BA,
	0x27ED,0x291F,0x2A50,0x2B80,0x2CAF,0x2DDC,0x2F08,0x3033,
	0x315D,0x3285,0x33AC,0x34D2,0x35F6,0x3719,0x383A,0x395A,
	0x3A78,0x3B95,0x3CB1,0x3DCB,0x3EE4,0x3FFB,0x4110,0x4224,
	0x4336,0x4447,0x4556,0x4664,0x4770,0x487A,0x4983,0x4A8B,
// 64
	0x4B90,0x4C94,0x4D96,0x4E97,0x4F96,0x5093,0x518F,0x5289,
	0x5382,0x5478,0x556E,0x5661,0x5753,0x5843,0x5932,0x5A1E,
	0x5B0A,0x5BF3,0x5CDB,0x5DC1,0x5EA6,0x5F89,0x606A,0x614A,
	0x6228,0x6305,0x63E0,0x64B9,0x6591,0x6667,0x673B,0x680E,
	0x68E0,0x69B0,0x6A7E,0x6B4B,0x6C16,0x6CDF,0x6DA8,0x6E6E,
	0x6F33,0x6FF7,0x70B9,0x717A,0x7239,0x72F6,0x73B3,0x746D,
	0x7527,0x75DF,0x7695,0x774A,0x77FE,0x78B0,0x7961,0x7A10,
	0x7ABF,0x7B6B,0x7C17,0x7CC1,0x7D6A,0x7E11,0x7EB7,0x7F5C,
// 128
	0x8000,0x80A2
};
//}}ATANLUT


// --------------------------------------------------------------------
// FUNCTIONS
// --------------------------------------------------------------------

//! Function to bracket a value in LUT. 
/*!	Consider a monotonously increasing LUT (or sorted array), with  
	two adjacent value form a bin. This function will find the 
	bin in which \a key can be found.
	\param key		Value to bracket.
	\param array	Array to search.
	\param size		Size of the array.
	\return	The low bracket of the bin.
*/
template<class T>
static inline uint lutBracket(const T &key, const T array[], uint size)
{
	int i, low=0, high= size-1;

	while(low+1<high)
	{
		i= (low+high)/2;
		if(key < array[i])
			high= i;
		else
			low= i;
	}

	return i;
}

//! Get a sine value as a Q12 fixed-point number.
/*! Uses linear interpolation between surrounding LUT entries for accuracy.
	\param x	Angle, with 0x8000 for a full circle.
	\return		Sine in with 12 bits of precision.
*/
int isin(int x)
{
	int h, ya, yb, y;
	uint ux, quad;

	ux= (uint)x%BRAD_2PI;
	h= ux % SINLUT_STRIDE;
	quad= ux/SINLUT_STRIDE / SINLUT_HPI;
	ux= ux/SINLUT_STRIDE % SINLUT_HPI;
	
	if(quad & 1)
	{
		ya= sinLUT[SINLUT_HPI-ux  ];
		yb= sinLUT[SINLUT_HPI-ux-1];
	}
	else
	{
		ya= sinLUT[ux  ];
		yb= sinLUT[ux+1];
	}

	y=(ya + ((yb-ya)*h >> SINLUT_STRIDE_SHIFT))>>3;
	return (quad & 2) ? -y : +y;
}


//! Get a sine value as a Q12 fixed-point number.
/*! Uses linear interpolation between surrounding LUT entries for accuracy.
	\param x	Angle, with 0x8000 for a full circle.
	\return		Sine in with 12 bits of precision.
*/
int icos(int x)
{
	int h, ya, yb, y;
	uint ux, quad;

	x += BRAD_HPI;
	ux= (uint)x % BRAD_2PI;
	h= ux % SINLUT_STRIDE;
	quad= ux/SINLUT_STRIDE / SINLUT_HPI;
	ux= ux/SINLUT_STRIDE % SINLUT_HPI;
	

	if(quad & 1)
	{
		ya= sinLUT[SINLUT_HPI-ux  ];
		yb= sinLUT[SINLUT_HPI-ux-1];
	}
	else
	{
		ya= sinLUT[ux  ];
		yb= sinLUT[ux+1];
	}

	y=(ya + ((yb-ya)*h >> SINLUT_STRIDE_SHIFT))>>3;
	return (quad & 2) ? -y : +y;
}

//! Get a tangent value as a Q12 fixed-point number.
/*! Uses linear interpolation between surrounding LUT entries for accuracy.
	\param x	Angle, with 0x8000 for a full circle.
	\return		Tangent in with 12 bits of precision.
*/
int itan(int x)
{
	uint ux= (uint)x % BRAD_PI;
	uint xa= ux / TANLUT_STRIDE;
	int ya, yb, h= ux % TANLUT_STRIDE;

	if(ux <= BRAD_HPI)
	{
		ya= tanLUT[xa  ];
		yb= tanLUT[xa+1];
	}
	else
	{
		ya= -tanLUT[TANLUT_PI-xa  ];
		yb= -tanLUT[TANLUT_PI-xa-1];
	}

	return (ya + ((yb-ya)*h >> TANLUT_STRIDE_SHIFT))>>4;
}


// --------------------------------------------------------------------
// ARCTANS
// --------------------------------------------------------------------

// Just the form
uint atan2Null(int x, int y)
{
	return x;
}

// With octants
uint atan2Oct(int x, int y)
{
	if(y==0)	return (x>=0 ? 0 : BRAD_PI);

	int phi;
	uint t;

	OCTANTIFY(x, y, phi);
	phi *= BRAD_PI/4;

	return phi;
}

// With octants+div
uint atan2OctDiv(int x, int y)
{
	if(y==0)	return (x>=0 ? 0 : BRAD_PI);

	int phi;
	uint t;

	OCTANTIFY(x, y, phi);
	phi *= BRAD_PI/4;
	t= QDIV(y, x, ATAN_FP);

	return phi+t;
}

// atan2 via simple lookup. No interpolation.
uint atan2Lookup(int x, int y)
{
	if(y==0)	return (x>=0 ? 0 : BRAD_PI);

	int phi;
	uint t;

	OCTANTIFY(x, y, phi);
	phi *= BRAD_PI/4;
	t= QDIV(y, x, ATAN_FP);

	return phi + atanLUT[t/ATANLUT_STRIDE]/8;
}


// Basic lookup+linear interpolation for atan2. 
// Returns [0,2pi), where pi ~ 0x4000.
uint atan2Lerp(int x, int y)
{
	if(y==0)	return (x>=0 ? 0 : BRAD_PI);

	int phi, fa, fb, h;
	uint t;

	OCTANTIFY(x, y, phi);
	phi *= BRAD_PI/4;

	t= QDIV(y, x, ATAN_FP);
	h= t % ATANLUT_STRIDE;
	fa= atanLUT[t/ATANLUT_STRIDE  ];
	fb= atanLUT[t/ATANLUT_STRIDE+1];

	return phi + ((fa + ((fb-fa)*h >> ATANLUT_STRIDE_SHIFT))>>3);
}

// atan2 using inverse lookup and linear interpolation. 
// Returns [0,2pi), where pi ~ 0x4000.
uint atan2InvLerp(int x, int y)
{
	if(y==0)	return (x>=0 ? 0 : BRAD_PI);

	int phi, fa, dphi, ta, tb;
	uint t;

	OCTANTIFY(x, y, phi);
	phi *= BRAD_PI/4;

	t= QDIV(y, x, TANLUT_FP);
	fa= lutBracket(t, tanLUT, TANLUT_SIZE);
	ta= tanLUT[fa  ];
	tb= tanLUT[fa+1];

	dphi= fa*TANLUT_STRIDE + QDIV((t-ta)*TANLUT_STRIDE, tb-ta, 0);
	return phi + dphi;
}

// Basic Taylor series (15th order) for atan2. 
// Returns [0,2pi), where pi ~ 0x4000.
uint atan2Taylor(int x, int y)
{
	if(y==0)	return (x>=0 ? 0 : BRAD_PI);

	static const int fixShift= 15, base=0xA2F9;
	int i, phi, t, t2, dphi;

	OCTANTIFY(x, y, phi);
	phi *= BRAD_PI/4;

	t= QDIV(y, x, fixShift);
	t2= -t*t>>fixShift;

	dphi= 0;
	for(i=15; i>=1; i -= 2)
	{
		dphi  = dphi*t2>>fixShift;
		dphi += QDIV(base, i, 0);
	}

	return phi + (dphi*t >> (fixShift+3));
}

// Approximate Taylor series for atan2, GBA implementation. 
// Returns [0,2pi), where pi ~ 0x4000.
uint atan2Gba(int x, int y)
{
	if(y==0)	return (x>=0 ? 0 : BRAD_PI);

	static const int fixShift= 15;
	int phi, t, t2, dphi;

	OCTANTIFY(x, y, phi);
	phi *= BRAD_PI/4;

	t= QDIV(y, x, fixShift);
	t2= -t*t>>fixShift;

	dphi= 0x00A9;
	dphi= 0x0390 + (t2*dphi>>fixShift);
	dphi= 0x091C + (t2*dphi>>fixShift);
	dphi= 0x0FB6 + (t2*dphi>>fixShift);
	dphi= 0x16AA + (t2*dphi>>fixShift);
	dphi= 0x2081 + (t2*dphi>>fixShift);
	dphi= 0x3651 + (t2*dphi>>fixShift);
	dphi= 0xA2F9 + (t2*dphi>>fixShift);

	return phi + (dphi*t >> (fixShift+3));
}

// Approximate Taylor series for atan2, home grown implementation. 
// Returns [0,2pi), where pi ~ 0x4000.
uint atan2Tonc(int x, int y)
{
	if(y==0)	return (x>=0 ? 0 : BRAD_PI);

	static const int fixShift= 15;
	int  phi, t, t2, dphi;

	OCTANTIFY(x, y, phi);
	phi *= BRAD_PI/4;

	t= QDIV(y, x, fixShift);
	t2= -t*t>>fixShift;

	dphi= 0x0470;
	dphi= 0x1029 + (t2*dphi>>fixShift);
	dphi= 0x1F0B + (t2*dphi>>fixShift);
	dphi= 0x364C + (t2*dphi>>fixShift);
	dphi= 0xA2FC + (t2*dphi>>fixShift);
	dphi= dphi*t>>fixShift;

	return phi + ((dphi+4)>>3);
}

// atan2 via added sines.
// Returns [0,2pi), where pi ~ 0x4000.
uint atan2Sin(int x, int y)
{
	if(y==0)	return (x>=0 ? 0 : BRAD_PI);

	int phi, dphi;
	uint t;

	OCTANTIFY(x, y, phi);
	phi *= BRAD_PI/4;

	t= QDIV(y, x, ATAN_FP);
	dphi= 0x14FF*isin(9*t/8) + 0x7D*isin(37*t/8);

	return phi + (dphi>>12);
}

// atan via CORDIC (coordinate rotations).
// Returns [0,2pi), where pi ~ 0x4000.
uint atan2Cordic(int x, int y)
{
	if(y==0)	return (x>=0 ? 0 : BRAD_PI);

	int phi;

	OCTANTIFY(x, y, phi);
	phi *= BRAD_PI/4;

	// Scale up a bit for greater accuracy.
	if(x < 0x10000)
	{ 
		x *= 0x1000;
		y *= 0x1000;
	}

	// atan(2^-i) terms using PI=0x10000
	const u16 list[]=
	{ 
		0x4000, 0x25C8, 0x13F6, 0x0A22, 0x0516, 0x028C, 0x0146, 0x00A3, 
		0x0051, 0x0029, 0x0014, 0x000A, 0x0005, 0x0003, 0x0001, 0x0001
	};

	int i, tmp, dphi=0;
	for(i=1; i<12; i++)
	{
		if(y>=0)
		{
			tmp= x + (y>>i);
			y  = y - (x>>i);
			x  = tmp;
			dphi += list[i];
		}
		else
		{
			tmp= x - (y>>i);
			y  = y + (x>>i);
			x  = tmp;
			dphi -= list[i];
		}
	}
	return phi + (dphi>>2);
}

// EOF
