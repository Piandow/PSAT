// Copyright Andy Singleton, 1993,1994
// This code is released for non-commercial use only
// For questions or upgrades contact:
// Andy Singleton, Creation Mechanics Inc.
// PO Box 248, Peterborough, NH 03458
// Internet: p00396@psilink.com
// Compuserve 73313,757
// Phone: (603) 563-7757

// QPQUICK
// Standard header files for compilers supporting Pre-Compiled Headers


#ifndef _PCH_H
#define _PCH_H

// Pick your compiler.  DOS/ANSI by default
//#define UNIX

#include <stdlib.h>
#ifdef UNIX
#include <stream.h>
#include <strstream.h>
#else
#include <conio.h>
#endif

#include <ctype.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <math.h>
#include <float.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <values.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif


//////////  Random number functions cause portability problems.  Resolve here.
// Define       rand_0to1()  (random float from 0 to 1)
//                      rnd(x) (random integer from 0 to x-1)
//                      rndomize() (initialize with a time based seed)
#ifdef UNIX
//////////////////// UNIX stuff
#define INT32 int
inline int kbhit() {return FALSE};              // Eliminate errors from DOS
// This "extern" may be necessary for your compiler, or it may not.  Comment out if not.
extern "C" double drand48();
extern "C" void srand48(long);
//
#define rand_0to1() drand48()
	inline int rnd(int __num)
		  { return(int)(drand48()*__num); }
	inline void rndomize(void) { srand48((unsigned) time(NULL)); }


#else
//////////////////// DOS/ANSI stuff
#define INT32 long
#define rand_0to1() (((float)rand())/RAND_MAX)
#ifndef __TURBOC__
	inline int rnd(int __num)
		 { return(int)(((long)rand()*__num)/(RAND_MAX+1)); }
	inline void rndomize(void) { srand((unsigned) time(NULL)); }


#else
//////////////////// Borland C++ stuff
#define rnd(x) random(x)
#define rndomize() randomize()
#endif

#endif

#endif
