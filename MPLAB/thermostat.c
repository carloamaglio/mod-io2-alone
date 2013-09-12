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
 * Thermostat:
 *	Inputs:
 *		Thermostat						______---------------------------_________________
 *	Outputs:
 *		Boiler							______---------------------------_________________
 *		Convector						_________________--------------------_____________
 *											  |          |               |   |
 *											  |          |               |   |
 *											  |<-------->|               |<->|
 *										       RTtON                      RTtOFF
 */

#include "system.h"
#include "plc.h"
#include "config.h"
#include "thermostat.h"

#if	THERMOSTAT

static char state = 0;

#define	RTtON	(7*MINUTES)
#define	RTtOFF	(1*MINUTES)
static Timer rtTime[1];

// thermostat input
#define	thermostatPin			pinGPIO0
#define	thermostat				(!_PORT(thermostatPin))

// boiler rele'
#define boilerPin				pinR1
#define	setBoiler(value)		(_LAT(boilerPin) = (value))
#define	offBoiler()				(setBoiler(0))
#define	onBoiler()				(setBoiler(1))

// convector rele'
#define convectorPin			pinR2
#define	setConvector(value)		(_LAT(convectorPin) = (value))
#define	offConvector()			(setConvector(0))
#define	onConvector()			(setConvector(1))

static Debounce ireader[1];

void taskThermostat() {

	bool t = debounceUpdate(ireader, thermostat);

	setBoiler(t);	// boiler signal follow thermostat immediatly

	switch (state) {
		case 0:		// init
			_ANSEL(thermostatPin) = 0;		// disable analog input
			_TRIS(thermostatPin) = 1;		// make pin an input
			_LAT(thermostatPin) = 0;		// store 0 in the LAT register
			_WPU(thermostatPin) = 1;		// turn on internal pull-up

			offBoiler();
			//_ANSEL(pinRM) = 0;		// disable analog input
			_TRIS(boilerPin) = 0;		// make pin an output

			offConvector();
			_ANSEL(convectorPin) = 0;		// disable analog input
			_TRIS(convectorPin) = 0;		// make pin an output

			timerSingleShot(rtTime);
			state++;

		case 1:		// all is turned off
			if (t) {
				timerStart(rtTime, RTtON);
				offConvector();
				state++;
			}
			break;

		case 2:		// thermostat active but convector is off
			if (t) {
				if (timerExpired(rtTime)) {
					onConvector();
					state++;
				}
			} else {
				state--;
			}
			break;

		case 3:		// thermostat active and all is turned on
			if (t) {
			} else {
				timerStart(rtTime, RTtOFF);
				state++;
			}
			break;

		case 4:		// thermostat is inactive but convector is on yet
			if (t) {
				state--;
			} else {
				if (timerExpired(rtTime)) {
					offConvector();
					state = 1;
				}
			}
			break;
	}
}

#endif
