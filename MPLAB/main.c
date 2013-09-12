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
 * File:   main.c
 * Author: Carlo
 *
 * Created on 26 ottobre 2012, 11.37
 */

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "config.h"
#include "fan.h"
#include "thermostat.h"
#include "bathroomlights.h"
#include "scalelights.h"

#pragma config  WDTE=OFF, MCLRE=ON, CP=OFF, FOSC=INTOSC

#if 0
#define pinI(f)			f(A, 0)
#define	I				(_PORT(pinI))

#define pinR(f)			f(C, 4)		//Led, OUTPUT , pin RC4
#define	Rset(value)		(_PORT(pinR) = (value))
#define	Roff()			(Rset(0))
#define	Ron()			(Rset(1))

#define pinR2(f)		f(C, 2)		//Led, OUTPUT , pin RC4
#define	R2set(value)	(_PORT(pinR2) = (value))
#define	R2off()			(R2set(0))
#define	R2on()			(R2set(1))

static unsigned char state = 0;
static Timer tmr[1];

#define	PERIOD	2500

void taskForTest() {
	static int cntr;

	if (!state) {
			_TRIS(pinI) = 1;		// make pin an input
			ANSELAbits.ANSA0 = 0;
			_ANSEL(pinI) = 0;		// disable analog input
			_WPU(pinI) = 1;			// turn on internal pull-up
			_TRIS(pinR) = 0;		// make pin an output
			_TRIS(pinR2) = 0;		// make pin an output
			timerInit(tmr);
			timerStart(tmr, PERIOD);
			cntr = 0;
			state = 1;
	} else {
		if (!I) {
			Ron();
			R2on();
			timerReset(tmr);
			state = 2;
		} else {
			if (0 && state!=0) return;

			switch (state) {
				case 0:	// init
					break;

				case 1:
					if (timerExpired(tmr)) {
						state++;
					}
					break;

				case 2:
					Ron();
					R2on();
					//PORTAbits.RA1 = (i&1);
					timerStart(tmr, PERIOD);
					state++;
					break;

				case 3:
					if (timerExpired(tmr)) {
						state++;
					}
					break;

				case 4:
					Roff();
					R2off();
					//PORTAbits.RA1 = (i&1);
					timerStart(tmr, PERIOD);
					state = 1;
					break;
			}
		}
	}
}
#else

#define	taskForTest()

#endif

void main(void) {

	ConfigureOscillator();
	ModIO2Init();

	while (1) {
		sysCanReset = 1;

		taskForTest();
		taskFan();
		taskThermostat();
		taskBathroomLights();
		taskScaleLights();

		if (sysCanReset) {
			systime = 0;
			systick = 0;
		} else {
			di();
			systimeInc(systime, systick);
			systick = 0;
			ei();
		}
	}
}
