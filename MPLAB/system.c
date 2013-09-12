/**
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
 */

#include "system.h"

volatile unsigned char systick = 0;
Systime systime = 0;
char sysCanReset = 0;


/* Refer to the device datasheet for information about available
oscillator configurations. */
void ConfigureOscillator(void)
{
    /* TODO Add clock switching code if appropriate.  */

    /* Typical actions in this function are to tweak the oscillator tuning
    register, select new clock sources, and to wait until new clock sources
    are stable before resuming execution of the main project. */

	TMR0 = 0;
	//OSCCONbits.IRCF = 14;

	//OPTION_REG = 0b00001000;
	OPTION_REGbits.PS = 0;		// prescaler assignment 0=1:2; 1=1:4; 2=1:8; 3=1:16; 4=1:32; 5; 6; 7=1:256
	OPTION_REGbits.PSA = 1;		// prescaler assigned to: 0=timer0; 1=watchdog
	OPTION_REGbits.T0CS = 0;	// timer0 source select bit 0=internal Fosc/4; 1=signal on T0CKI
	OPTION_REGbits.T0SE = 0;	// timer0 edge selection 0=low to high; 1=high to low
	T0IE = 1;
	GIE = 1;
}


#define	Reset(me)		((me)->running = 0)

#ifndef	timerInit
void timerInit(Timer* me) {
	Reset(me);
}

void timerSingleShot(Timer* me) {
	me->multishot = 0;
}

#if 0
void TMR_MultipleShot(Timer* me) {
	me->multishot = 1;
}
#endif

void timerStart(Timer* me, unsigned long time) {
	me->time = time;
	systimeAdd(systime, me->timeout, time);
	me->running = 1;
}

void timerReset(Timer* me) {
	Reset(me);
}
#endif

bool timerExpired(Timer* me) {
	bool rv;
	rv = me->running;
	if (rv) {
		if (rv = (systimeSysCompare(me->timeout)<=0)) {
#if MULTISHOT
			if (me->multishot) {
				systimeInc(me->timeout, me->time);
			} else {
				Reset(me);
			}
#else
			Reset(me);
#endif
		}
		if (me->running) {
			sysCanReset = 0;
		}
	}
	return rv;
}

