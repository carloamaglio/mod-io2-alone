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

#define LIGHTOFFTIME	(40*MINUTES)		// turn off shower light automatically after the time defined here
#define FANONDELAY		(15*MINUTES)		// here define the delay between light switch on and fan on
#define FANOFFTIME		(30*MINUTES)		// turn off fan automatically after the time defined here

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

//#include "pt.h"
//
//static struct pt pt1;
//static bool l;
//static bool f;
//static Timer timer[1];
//
//static int protothread1(struct pt *pt)
//{
//	/* A protothread function must begin with PT_BEGIN() which takes a pointer to a struct pt. */
//	PT_BEGIN(pt);
//
//	/* We loop forever here. */
//	while (1) {
//		l = 0;
//
//		PT_WAIT_UNTIL(pt, SwL);
//		l = 1;
//		timerStart(timer, 1000);
//
//		PT_WAIT_UNTIL(pt, SwL==0 || timerExpired(timer));
//		if (SwL) {
//			f = 1;
//			timerStart(timer, 1000);
//			PT_WAIT_UNTIL(pt, SwL==0 || timerExpired(timer));
//			l = 0;
//			if (SwL) {
//				PT_WAIT_UNTIL(pt, timerExpired(timer));
//			}
//		}
//	}
//
//	/* All protothread functions must end with PT_END() which takes a pointer to a struct pt. */
//	PT_END(pt);
//}
//

/**
 * 
 */
void taskFan() {
	static TimedOff lighttimer[1];			// started when light switch toggle ON
	static OnOffDelay fanLightDelay[1];		//

	static TimedOff fantimer[1];			// started when fan switch toggle ON

	if (state==0) {				// hardware initializing...
		_HWINIT_DIGITAL_INPUT(SwFPin);
		_HWINIT_DIGITAL_INPUT(SwLPin);

		offLight();
		_HWINIT_DIGITAL_OUTPUT_A(pinLight);

		offFan();
		_HWINIT_DIGITAL_OUTPUT(pinFan);

		timedOffInit(lighttimer, LIGHTOFFTIME);
		onOffDelayInit(fanLightDelay, FANONDELAY, FANOFFTIME);

		timedOffInit(fantimer, FANOFFTIME);

		state = PWRONSTATE;

//		PT_INIT(&pt1);
//		timerInit(timer);
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
		debounceUpdate(swfDebouncer, SwF);	// read fan switch
		debounceUpdate(swlDebouncer, SwL);	// read light switch		__|----------------------------|____________

		bool lightQ = timedOffUpdate(lighttimer, swl);	//				__|---------------------|___________________
		setLight(lightQ);

		bool fanLightQ = onOffDelayUpdate(fanLightDelay, lightQ);	//	_________|-------------------------|________

		bool fanQ = timedOffUpdate(fantimer, swf);					//	____________________________________________

		// fan run if fan switch is pressed or light is on or shower just finished
		setFan(fanQ || fanLightQ);									//	_________|-------------------------|________

//		protothread1(&pt1);
	}
}

#endif
