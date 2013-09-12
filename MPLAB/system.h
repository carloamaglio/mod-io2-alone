/* 
 * Copyright 2013 Dott. Ing. Carlo Amaglio - Via Emigli, 10 - 25081 Bedizzole (BS) - Italy
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * File:   system.h
 * Author: Carlo
 *
 * Created on 2 ottobre 2012, 17.04
 */

#ifndef SYSTEM_H
#define	SYSTEM_H

/* Microcontroller MIPs (FCY) */
#define _XTAL_FREQ		500000L
#define SYS_FREQ		_XTAL_FREQ
#define FCY				(SYS_FREQ/4)
//#define __PICC__

#include <xc.h>

#define	bool	char

/*
 * from http://www.starlino.com/port_macro.html
 * MACROS FOR EASY PIN HANDLING USAGE
 * #define	pinI(f)		f(A, 2)		//Switch, INPUT , pin RA2
 * #define	pinR(f)		f(C, 4)		//Led, OUTPUT , pin RC4
 *
 * #define	I				_PORT(pinI)
 * #define	R				_PORT(pinR)
 *
 * _TRIS(pinI) = 1;		//make pin an input
 * _WPU(pinI) = 1;			// turn on internal pull-up
 * _TRIS(pinR) = 0;		// make pin an output
 */
#define _ANSEL(pin)				pin(_ANSEL_F)
#define _ANSEL_F(alpha, b)		(ANSEL ## alpha ## bits.ANS ## alpha ## b)
#define _TRIS(pin)				pin(_TRIS_F)
#define _TRIS_F(alpha, b)		(TRIS ## alpha ## bits.TRIS ## alpha ## b)
#define _PORT(pin)				pin(_PORT_F)
#define _PORT_F(alpha, b)		(PORT ## alpha ## bits.R ## alpha ## b)
#define _LAT(pin)				pin(_LAT_F)
#define _LAT_F(alpha, b)		(LAT ## alpha ## bits.LAT ## alpha ## b)
#define _WPU(pin)				pin(_WPU_F)
#define _WPU_F(alpha, b)		(WPU ## alpha ## bits.WPU ## alpha ## b)

#define	_HWINIT_DIGITAL_INPUT(p)	{							\
		_ANSEL(p) = 0;		/* disable analog input */			\
		_TRIS(p) = 1;		/* make pin an input */				\
		_LAT(p) = 0;		/* store 0 in the LAT register */	\
		_WPU(p) = 1;		/* turn on internal pull-up */		\
}

#define	_HWINIT_DIGITAL_OUTPUT(p)	{							\
		_TRIS(p) = 0;			/* make pin an output */		\
}

#define	_HWINIT_DIGITAL_OUTPUT_A(p)	{							\
		_ANSEL(p) = 0;			/* disable analog input */		\
		_TRIS(p) = 0;			/* make pin an output */		\
}

#define	Systime	unsigned long
extern volatile unsigned char systick;
extern Systime systime;
extern char sysCanReset;
#define	systimeCompare(_ta, _tb)		((_ta)>(_tb) ? 1 : -1)
#define	systimeSysCompare(_t)			systimeCompare(_t, systime)
#define	systimeInc(_t, _time)			(_t += (_time))
#define	systimeAdd(_src, _dst, _time)	(_dst = (_src) + (_time))

/* Definition of some useful constants */
#define	SECONDS	1000L
#define	MINUTES	(60*SECONDS)
#define	HOURS	(60*MINUTES)
#define	DAYS	(24*HOURS)

/* Custom oscillator configuration funtions, reset source evaluation
functions, and other non-peripheral microcontroller initialization functions
go here. */
extern void ConfigureOscillator(void); /* Handles clock switching/osc initialization */

#define MULTISHOT	0

/**
 * Timer - 
 */
typedef struct Timer Timer;
struct Timer {
	Systime timeout;
#if MULTISHOT
	unsigned long time;
	unsigned int multishot : 1;
#endif
	unsigned int running : 1;
};

#if 1
#if MULTISHOT
#define	timerInit(_me)				((_me)->time=0, (_me)->running=0, (_me)->multishot=0)
#define	timerSingleShot(_me)		((_me)->multishot=0)
//#define	TMR_MultipleShot(_me)		((_me)->multishot=1)
#define	timerStart(_me, _t)			((_me)->time=(_t), systimeAdd(systime, (_me)->timeout, (_t)), (_me)->running=1, sysCanReset=0)
#else
#define	timerInit(_me)				((_me)->running=0)
#define	timerSingleShot(_me)
#define	timerStart(_me, _t)			(systimeAdd(systime, (_me)->timeout, (_t)), (_me)->running=1, sysCanReset=0)
#endif
#define	timerReset(_me)				((_me)->running=0)
#else
extern void timerInit(Timer me);
extern void timerSingleShot(Timer me);
//extern void TMR_MultipleShot(TMR me);
extern void timerStart(Timer me, unsigned long time);
extern void timerReset(Timer me);
#endif

extern bool timerExpired(Timer* me);

#define	TIMESCALE	1


#endif	/* SYSTEM_H */
