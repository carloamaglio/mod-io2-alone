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
 *
 * Bathroom lights:
 *	Inputs:
 *		Sw - Switch. One switch to control two lights
 *	Outputs:
 *		R1 - Light 1 rele (for example washbasin)
 *      R2 - Light 2 rele (for example mirror)
 */

#include "system.h"
#include "plc.h"
#include "config.h"
#include "bathroomlights.h"

#if BATHROOMLIGHTS

#define OFFTIME (30*MINUTES)	// if someone forgot the light on, it turn off automatically after the time defined here

// switch input
#define SwPin						pinGPIO0
#define	Sw							(!_PORT(SwPin))
static Debounce swDebouncer[1];
#define	sw							debounceValue(swDebouncer)

// light 1 rele
#define pinLight1					pinR1
#define	setLight1(value)			(_LAT(pinLight1) = (value))
#define	offLight1()					(setLight1(0))
#define	onLight1()					(setLight1(1))

// light 2 rele
#define pinLight2					pinR2
#define	setLight2(value)			(_LAT(pinLight2) = (value))
#define	offLight2()					(setLight2(0))
#define	onLight2()					(setLight2(1))

#define	PWRONSTATE		100

// different version in order to compare different usage of memory (RAM and Flash) by the compiler
//		RAM, FLASH
// 0 --> 50, 1076
// 1 --> 84, 1415
// 2 --> 85, 1417
// 3 --> 85, 1385
#define	VERSION	3

#if VERSION==0

#define	FINALSTATE		(PWRONSTATE-1)

#endif

#if VERSION>=1

static SwitchCounter cntr[1];

#if VERSION>=2

// define the states of the outputs based on the number of switch toggles
// bit0 is the state of light 1
// bit1 is the state of light 2
static const unsigned char states_mode0[] = { 0b01, 0b11, 0b10 };
static const unsigned char states_mode1[] = { 0b11, 0b01, 0b10 };
//#define	STATESLEN	(sizeof(states_mode0) / sizeof(state_mode0[0]))
#define	STATESLEN	((int)sizeof(states_mode0))

#endif

#endif

void taskBathroomLights(void) {
	static unsigned char state = 0;
	static Timer timer[1];

	if (state==0) {				// hardware initializing...
		_ANSEL(SwPin) = 0;		// disable analog input
		_TRIS(SwPin) = 1;		// make pin an input
		_LAT(SwPin) = 0;			// store 0 in the LAT register
		_WPU(SwPin) = 1;			// turn on internal pull-up

		offLight1();
		//_ANSEL(pinRM) = 0;		// disable analog input
		_TRIS(pinLight1) = 0;		// make pin an output

		offLight2();
		_ANSEL(pinLight2) = 0;		// disable analog input
		_TRIS(pinLight2) = 0;		// make pin an output

#if VERSION>=1
		switchCounterInit(cntr, OFFTIME);
//		onDelayInit(ondelay, 1000);
#endif
		state = PWRONSTATE;
		state = 1;
	} else if (state >= PWRONSTATE) {					// power-on sequence (test the lights)
		switch (state) {
			case PWRONSTATE:							// 1. turn on the first light
				onLight1();
				timerStart(timer, 1000);
				state++;
				break;
			case PWRONSTATE+1:							// 2. after 1s turn on the second light
				if (timerExpired(timer)) {
					onLight2();
					timerStart(timer, 1000);
					state++;
				}
				break;
			case PWRONSTATE+2:							// 3. after 1s turn off the first light
				if (timerExpired(timer)) {
					offLight1();
					timerStart(timer, 1000);
					state++;
				}
				break;
			case PWRONSTATE+3:							// 4. after 1s turn off the second light
				if (timerExpired(timer)) {
					offLight2();
					state = 1;
				}
				break;
		}
	} else {											// after power-on sequence
#if VERSION>=1
		switchCounterCount(cntr, Sw);
//		onDelayUpdate(ondelay, Sw);

		if (switchCounterOutput(cntr)) {
#if VERSION==3
			// two different sequences using the jumper as configurator
//			const unsigned char *s = (module->jumper ? states_mode0 : states_mode1);
//			unsigned char c = s[(cntr->cntr-1) % STATESLEN];
			unsigned char c = (switchCounterOutput(cntr)-1) % STATESLEN;
			c = (module->jumper ? states_mode0 : states_mode1)[c];
			(c & 0b01) ? onLight1() : offLight1();
			(c & 0b10) ? onLight2() : offLight2();
#endif
#if VERSION==2
			if (module->jumper) {	// two different sequences using the jumper as configurator
				unsigned char c = states_mode0[(switchCounterCountValue(cntr)-1) % STATESLEN];
//				setLight1(c & 0b01);
//				setLight2(c & 0b10);
				(c & 0b01) ? onLight1() : offLight1();
				(c & 0b10) ? onLight2() : offLight2();
			} else {
				unsigned char c = states_mode1[(switchCounterCountValue(cntr)-1) % STATESLEN];
//				setLight1(c & 1);
//				setLight2(c & 2);
				(c & 0b01) ? onLight1() : offLight1();
				(c & 0b10) ? onLight2() : offLight2();
			}
#endif
#if VERSION==1
			if (module->jumper) {	// two different sequences using the jumper as configurator
				switch (switchCounterCountValue(cntr) % 3) {	// sequence 1
					case 1:						// switch on the first time
						onLight1();				// light 1 on
						onLight2();				// light 2 on
						break;
					case 2:						// switch 2nd time
						onLight1();				// light 1 on
						offLight2();			// light 2 off
						break;
					case 0:						// switch 3rd time
						offLight1();			// light 1 off
						onLight2();				// light 2 on
						break;
				}
			} else {
				switch (switchCounterCountValue(cntr) % 3) {	// sequence 2
					case 1:						// switch on the first time
						onLight1();				// light 1 on
						offLight2();			// light 2 off
						break;
					case 2:						// switch 2nd time
						onLight1();				// light 1 on
						onLight2();				// light 2 on
						break;
					case 0:						// switch 3rd time
						offLight1();			// light 1 off
						onLight2();				// light 2 on
						break;
				}
			}
#endif
		} else {
			offLight1();
			offLight2();
		}
#else
		debounceInput(swDebouncer, Sw);
		switch (state) {
			case INITIALSTATE:						// everithing is OFF
				if (sw==1) {						// if switch start the OFFTIME and go next state
					timerStart(timer, OFFTIME);
					state++;
				} else {
					offLight1();
					offLight2();
				}
				break;

			case INITIALSTATE+1:					// light 1 is ON
				if (timerExpired(timer)) {			// if light forgotten go to final state
					state = FINALSTATE;
				} else if (sw==0) {					// if switch released start 1s timer and go next state
					timerStart(timer, 1000);
					state++;
				} else {
					onLight1();
					offLight2();
				}
				break;
			case INITIALSTATE+2:					// light 1 is ON
				if (timerExpired(timer)) {			// if switch released for 1s go initial state
					state = INITIALSTATE;
				} else if (sw==1) {					// if switch pressed again in 1s start the OFFTIME and go next state
					timerStart(timer, OFFTIME);
					state++;
				} else {
					onLight1();
					offLight2();
				}
				break;

			case INITIALSTATE+3:					// lights 1 and 2 are ON
				if (timerExpired(timer)) {			// if light forgotten go to final state
					state = FINALSTATE;
				} else if (sw==0) {					// if switch released start 1s timer and go next state
					timerStart(timer, 1000);
					state++;
				} else {
					onLight1();
					onLight2();
				}
				break;
			case INITIALSTATE+4:					// lights 1 and 2 are ON
				if (timerExpired(timer)) {			// if switch released for 1s go initial state
					state = INITIALSTATE;
				} else if (sw==1) {					// if switch pressed again in 1s start the OFFTIME and go next state
					timerStart(timer, OFFTIME);
					state++;
				} else {
					onLight1();
					onLight2();
				}
				break;

			case INITIALSTATE+5:					// light 2 is ON
				if (timerExpired(timer)) {			// if light forgotten go to final state
					state = FINALSTATE;
				} else if (sw==0) {					// if switch released start 1s timer and go next state
					timerStart(timer, 1000);
					state++;
				} else {
					offLight1();
					onLight2();
				}
				break;
			case INITIALSTATE+6:					// light 2 is ON
				if (timerExpired(timer)|| sw==1) {	// if switch released for 1s or pressed again go initial state
					state = INITIALSTATE;
				} else {
					offLight1();
					onLight2();
				}
				break;

			case FINALSTATE:						// lights forgotten ON --> turn off automatically after a period
				if (sw==0) {						// if switch released go initial state
					state = INITIALSTATE;
				} else {
					offLight1();
					offLight2();
				}
				break;
		}
#endif
	}
}

#endif
