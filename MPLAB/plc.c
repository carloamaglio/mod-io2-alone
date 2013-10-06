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
 * PLC.C -	this module implements some function blocks that emulate some function blocks
 *			of Logo Siemens PLC
 *
 */

#include "system.h"
#include "plc.h"

#define	STATE(_s, _v)	(((_s)<<1) | (_v))



/*****************************************************************************
 *
 * Debounce -
 *
 */
#define	DEBOUNCE_TIME	50

#define	S0		STATE(0, 0)
#define	S1		STATE(1, 0)
#define	S2		STATE(2, 0)
#define	S3		STATE(3, 1)
#define	S4		STATE(4, 1)

bool debounceUpdate(Debounce* me, bool i) {

	switch (me->state) {
		case S0:		// init
			timerInit(me->timer);
			timerSingleShot(me->timer);
			me->state = S1;
			break;

		case S1:											// steady state
			if (i) {										// rising edge of input
				timerStart(me->timer, DEBOUNCE_TIME);
				me->state = S2;
			}
			break;

		case S2:											// input is high
			if (i) {
				if (timerExpired(me->timer)) {				// if it is high for the debounce time
					me->state = S3;							// it is considered high for the observer (output)
				}
			} else {
				me->state = S1;
			}
			break;

		case S3:											// input is steady high
			if (!i) {										// if it goes low next state
				timerStart(me->timer, DEBOUNCE_TIME);
				me->state = S4;
			}
			break;

		case S4:
			if (!i) {
				if (timerExpired(me->timer)) {
					me->state = S1;
				}
			} else {
				me->state = S3;
			}
			break;
	}
	return debounceOutput(me);
}
#undef	S0
#undef	S1
#undef	S2
#undef	S3
#undef	S4


/*****************************************************************************
 *
 * SwitchCounter -
 *
 * */

/**
 *
 * @param me
 * @param timeout - timeout for autoreset counter value
 */
void switchCounterInit(SwitchCounter* me, unsigned int timeout) {
	me->button = 0;
	me->timeout = timeout;
	me->cntr = 0;
	me->state = 0;
	timerInit(me->timer);
}

/**
 *
 * @param me
 * @param inputType - 0=bistable switch, 1=monostable button
 */
void switchCounterSetInputType(SwitchCounter* me, char inputType) {
	me->button = (inputType!=0);
}

/**
 * switchCounterCount - count the number of subsequent switch toggles
 *
 * @param me
 * @param i - value of the input signal
 * @return value of the counter. It can be retrieved calling
 */
unsigned char switchCounterCount(SwitchCounter* me, bool i) {
	i = debounceUpdate(me->ireader, i);

	if (me->button==0) {						// bistable switch
		switch (me->state) {
			case 0:											// steady state: input=0
				if (i) {									// if input goes high
					me->cntr++;									// counter is incremented
					timerStart(me->timer, me->timeout);			// timer is retriggered
					me->state++;
				} else if (timerExpired(me->timer)) {		// if input stay low for the time
					me->cntr = 0;								// counter goes to 0
				}
				break;

			case 1:
				if (i) {									// if input stay high
					if (timerExpired(me->timer)) {				// after the timeout
						me->cntr = 0;							// counter goes to 0
						me->state++;
					}
				} else {									// if input goes low
					timerStart(me->timer, 500);				// for 1 s
					me->state--;
				}
				break;

			case 2:
				if (!i) {									// if input goes low
					me->state = 0;
				}
				break;
		}
	} else {									// monostable button
		switch (me->state) {
			case 0:											// steady state: input=0
ZERO:
				if (i) {									// on button press
					me->cntr++;									// counter is incremented
					me->state++;								// go next state
				} else if (timerExpired(me->timer)) {		// if input stay low for the timeout
					me->cntr = 0;								// counter goes to 0
				}
				break;

			case 1:											// button is pressed: wait till release
				if (!i) {									// on button release
					timerStart(me->timer, 1500);				// 1 s timer
					me->state++;								// go next state
				}
				break;

			case 2:											// button just released
				if (i) {									// if button pressed again
					me->state = 0;
					goto ZERO;
				}
				if (timerExpired(me->timer)) {					// if 1 s elapsed
					timerStart(me->timer, me->timeout);			// timeout is retriggered
					me->state = 0;
				}
				break;
		}
	}

	return me->cntr;
}



/*****************************************************************************
 *
 * OnDelay -
 *
 *				 +--------+
 *				 | +----+ |
 *				 | |    | |
 *			Trg -|-'    '-|
 *				 | :  +-+ |- Q
 *			     | :  | | |
 *			Par -|-+--+ +-|
 *				 |        |
 *			     +--------+
 *
 * */
#define	S0		STATE(0, 0)
#define	S1		STATE(1, 0)
#define	S2		STATE(2, 1)

void onDelayInit(OnDelay* me, unsigned long T) {
	onDelaySetT(me, T);
	me->state = S0;
	timerInit(me->timer);
}

void onDelaySetT(OnDelay* me, unsigned long T) {
	me->T = T / TIMESCALE;
}

bool onDelayUpdate(OnDelay* me, bool Trg) {
	switch (me->state) {
		case S0:
			if (Trg) {
				me->state = S1;
				timerStart(me->timer, me->T);
			}
			break;
		case S1:
			if (Trg) {
				if (timerExpired(me->timer)) {
					me->state = S2;
				}
			} else {
				me->state = S0;
			}
			break;
		case S2:
			if (!Trg) {
				me->state = S0;
			}
			break;
	}
	return onDelayOutput(me);
}
#undef	S0
#undef	S1
#undef	S2


/*****************************************************************************
 *
 * OffDelay - 
 *
 * */
#define	S0				STATE(0, 0)
#define	S1				STATE(1, 1)
#define	S2				STATE(2, 1)

void offDelayInit(OffDelay* me, unsigned long T) {
	offDelaySetT(me, T);
	me->state = S0;
	timerInit(me->timer);
}

void offDelaySetT(OffDelay* me, unsigned long T) {
	me->T = T / TIMESCALE;
}

bool offDelayUpdate(OffDelay* me, bool Trg, bool R) {
	if (R) {
		me->state = S0;
	} else {
		switch (me->state) {
			case S0:
				if (Trg) {
					me->state = S1;
				}
				break;
			case S1:
				if (!Trg) {
					timerStart(me->timer, me->T);
					me->state = S2;
				}
				break;
			case S2:
				if (Trg) {
					me->state = S1;
				} else {
					if (timerExpired(me->timer)) {
						me->state = S0;
					}
				}
				break;
		}
	}
	return offDelayOutput(me);
}
#undef	S0
#undef	S1
#undef	S2


/*****************************************************************************
 *
 * OnOffDelay -
 *
 * */
#define	S0		STATE(0, 0)
#define	S1		STATE(1, 0)
#define	S2		STATE(2, 1)
#define	S3		STATE(3, 1)

void onOffDelayInit(OnOffDelay* me, unsigned long TH, unsigned long TL) {
	onOffDelaySetTH(me, TH);
	onOffDelaySetTL(me, TL);
	me->state = S0;
	timerInit(me->timer);
}

void onOffDelaySetTH(OnOffDelay* me, unsigned long TH) {
	me->TH = TH / TIMESCALE;
}

void onOffDelaySetTL(OnOffDelay* me, unsigned long TL) {
	me->TL = TL / TIMESCALE;
}

bool onOffDelayUpdate(OnOffDelay* me, bool Trg) {
	switch (me->state) {
		case S0:
			if (Trg) {
				me->state = S1;
				timerStart(me->timer, me->TH);
			}
			break;
		case S1:
			if (Trg) {
				if (timerExpired(me->timer)) {
					me->state = S2;
				}
			} else {
				me->state = S0;
			}
			break;
		case S2:
			if (!Trg) {
				me->state = S3;
				timerStart(me->timer, me->TL);
			}
			break;
		case S3:
			if (!Trg) {
				if (timerExpired(me->timer)) {
					me->state = S0;
				}
			} else {
				me->state = S0;
			}
			break;
	}
	return onOffDelayOutput(me);
}
#undef	S0
#undef	S1
#undef	S2
#undef	S3


/*****************************************************************************
 *
 * SetReset -
 *
 * */
#define	S0				STATE(0, 0)
#define	S1				STATE(1, 1)

void setResetInit(SetReset* me) {
	me->state = S0;
}

bool setResetUpdate(SetReset* me, bool S, bool R) {
	if (R) {
		me->state = S0;
	} else if (S) {
		me->state = S1;
	}
	return setResetOutput(me);
}
#undef	S0
#undef	S1
