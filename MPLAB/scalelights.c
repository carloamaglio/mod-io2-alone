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
 * scalelights.c:
 *	Inputs:
 *		Button - Wall push button
 *		Sensor - PIR movement sensor (timed)
 *	Outputs:
 *		R1 - Rele'
 *
 *	Description:
 *		With the wall switch you can turn ON and OFF the lights as usual (toggle mode). If you turn on the light and
 *		forgot the light turn off automatically after a period (OFFTIME).
 *		When the PIR sensor see a movement it close the contacts (S input is activated) for a time selected on the
 *		sensor itself (for example 2 minutes). If in the meantime the sensor sees another movement the time is
 *		retriggered. If during this time you push the wall switch you turn off the light immediatly.
 */

#include "system.h"
#include "plc.h"
#include "config.h"
#include "scalelights.h"

#if SCALELIGHTS

#define OFFTIME (2*HOURS)	// if someone forgot the light on, it turn off automatically after the time defined here

// switch input
#define ButtonPin				pinGPIO0
#define	ButtonInput				(!_PORT(ButtonPin))
static Debounce buttonDebouncer[1];
#define	button					debounceOutput(buttonDebouncer)

// sensor input
#define SensorPin				pinGPIO1
#define	Sensor					(!_PORT(SensorPin))

// light rele'
#define pinLight				pinR1
#define	setLight(value)			(_LAT(pinLight) = (value))
#define	offLight()				(setLight(0))
#define	onLight()				(setLight(1))

static SwitchCounter cntr[1];

static const unsigned char states[] = { 1, 3, 2 };
#define	STATESLEN	((int)sizeof(states))

static unsigned char state = 0;
static Timer timer[1];

static void init(void) {
	_ANSEL(ButtonPin) = 0;			// disable analog input
	_TRIS(ButtonPin) = 1;			// make pin an input
	_LAT(ButtonPin) = 0;			// store 0 in the LAT register
	_WPU(ButtonPin) = 1;			// turn on internal pull-up

	offLight();
	//_ANSEL(pinRM) = 0;			// disable analog input
	_TRIS(pinLight) = 0;			// make pin an output

	switchCounterInit(cntr, OFFTIME);
	state = 1;
}

static void startup(void) {
	switch (state) {
		case 1:
			onLight();
			timerStart(timer, 2500);
			state++;
			break;
		case 2:
			if (timerExpired(timer)) {
				offLight();
				state = 4;
			}
			break;
	}
}

void taskScaleLights(void) {

	if (state==0) {					// initializing
		init();
	} else if (state<=4) {			// startup
		startup();
	} else {
		debounceUpdate(buttonDebouncer, ButtonInput);
		switchCounterCount(cntr, button);

		if (cntr->cntr) {
			if (module->jumper) {	// two different sequences using the jumper as configurator
				unsigned char c = states[cntr->cntr % STATESLEN];
//				setMirrorLight(c & 1);
//				setWashbasinLight(c & 2);
				(c & 1) ? onLight() : offLight();
				(c & 2) ? onLight() : offLight();
//				switch (cntr->cntr % 3) {	// bathroom 1 sequence
//					case 1:			// switch turn on the first time
//						onWashbasinLight();			// washbasin light on
//						offMirrorLight();		// mirror light off
//						break;
//					case 2:			// 2nd time
//						onWashbasinLight();			// washbasin light on
//						onMirrorLight();			// mirror light on
//						break;
//					case 0:			// 3rd time
//						offWashbasinLight();		// washbasin light off
//						onMirrorLight();			// mirror light on
//						break;
//				}
			} else {
				switch (cntr->cntr % 3) {	// bathroom 2 sequence
					case 1:			// switch turn on the first time
						onLight();			// washbasin light on
						break;
					case 2:			// 2nd time
						offLight();		// mirror light off
						break;
					case 0:			// 3rd time
						offLight();		// washbasin light off
						break;
				}
			}
		} else {
			offLight();
		}
	}
}

#endif
