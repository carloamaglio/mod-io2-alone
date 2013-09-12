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
 * Fan and Shower light
 * When you have a shower it's good to have the fan run for a time after shower end
 *
 *	Inputs:
 *		SwF - Switch to run fan for a time indipendently from shower
 *		SwL - Switch to turn on shower light and consequently the fan
 *	Outputs:
 *		RF - Rele fan (rele 1)
 *		RL - Rele light (rele 2)
 */

#include "system.h"
#include "plc.h"
#include "config.h"
#include "fan.h"

#if FAN

#define FANOFFTIME		(40*MINUTES)		// turn off fan automatically after the time defined here
#define LIGHTOFFTIME	(40*MINUTES)		// turn off shower light automatically after the time defined here
#define DLIGHT			(15*MINUTES)		// here define the delay between light switch on and fan on

// switch fan input
#define SwFPin						pinGPIO0
#define	SwF							(!_PORT(SwFPin))
static Debounce swfDebouncer[1];
#define	swf							debounceOutput(swfDebouncer)

// switch light input
#define SwLPin						pinGPIO1
#define	SwL							(!_PORT(SwLPin))
static Debounce swlDebouncer[1];
#define	swl							debounceOutput(swlDebouncer)

// fan rele
#define pinFan						pinR1
#define	setFan(value)				(_LAT(pinFan) = ((value)?1:0))
#define	offFan()					(setFan(0))
#define	onFan()						(setFan(1))

// light rele
#define pinLight					pinR2
#define	setLight(value)				(_LAT(pinLight) = (value))
#define	offLight()					(setLight(0))
#define	onLight()					(setLight(1))

#define	PWRONSTATE		100

static unsigned char state = 0;

/**
 * 
 */
void taskFan() {
	static TimedOff fantimer[1];			// started when fan switch toggle ON
	static TimedOff lighttimer[1];			// started when light switch toggle ON
	static TimedOff lightofftimer[1];		// started when light switch toggle OFF
	static OnDelay dlight[1];				// 

	if (state==0) {				// hardware initializing...
		_HWINIT_DIGITAL_INPUT(SwFPin);
		_HWINIT_DIGITAL_INPUT(SwLPin);

		offLight();
		_HWINIT_DIGITAL_OUTPUT_A(pinLight);

		offFan();
		_HWINIT_DIGITAL_OUTPUT(pinFan);

		timedOffInit(fantimer, FANOFFTIME);
		timedOffInit(lighttimer, LIGHTOFFTIME);
		timedOffInit(lightofftimer, FANOFFTIME);
		onDelayInit(dlight, DLIGHT);
		state = PWRONSTATE;
	} else if (state >= PWRONSTATE) {					// power-on sequence (test the lights)
		switch (state) {
			case PWRONSTATE:							// 1. turn on the light
				onLight();
				timerStart(fantimer->timer, 1000);
				state++;
				break;
			case 1+PWRONSTATE:							// 2. after 1s turn on the fan
				if (timerExpired(fantimer->timer)) {
					onFan();
					timerStart(fantimer->timer, 1000);
					state++;
				}
				break;
			case 2+PWRONSTATE:							// 3. after 1s turn off the light
				if (timerExpired(fantimer->timer)) {
					offLight();
					timerStart(fantimer->timer, 10000);
					state++;
				}
				break;
			case 3+PWRONSTATE:							// 4. after 10s turn off the fan
				if (timerExpired(fantimer->timer)) {
					offFan();
					state = 1;
				}
				break;
		}
	} else {
		debounceUpdate(swfDebouncer, SwF);			// read fan switch with debounce
		debounceUpdate(swlDebouncer, SwL);			// read light switch with debounce

		bool ql = timedOffUpdate(lighttimer, swl);
		setLight(ql);

		bool qdlight = onDelayUpdate(dlight, ql);

		bool qoffl = timedOffUpdate(lightofftimer, swl);

		bool qf;
		qf = timedOffUpdate(fantimer, swf);

		setFan(qf || qdlight || qoffl);					// fan run if fan switch is pressed or light is on or shower just finished
	}
}

#endif
