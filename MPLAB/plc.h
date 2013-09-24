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
 * File:   plc.h
 * Author: Carlo
 *
 * Created on 5 settembre 2013, 10.09
 */

#ifndef PLC_H
#define	PLC_H

#include "system.h"

#ifdef	__cplusplus
extern "C" {
#endif


/*
 * Debounce - debounce filter on digital input
 *
 */
typedef struct Debounce Debounce;
struct Debounce {
	Timer timer[1];
	unsigned char state;
};

extern bool debounceUpdate(Debounce* me, bool i);
#define	debounceOutput(me)	((me)->state & 1)

/*
 * EdgeDetect - detect rising/falling edge of the input signal
 */
typedef struct EdgeDetect EdgeDetect;
struct EdgeDetect {
	unsigned char state;
};
extern void edgeDetectInit(EdgeDetect* me);
extern bool edgeDetectUpdate(EdgeDetect* me, bool i);
#define	edgeDetectOutput(me)	((me)->state==1)

/*
 * SwitchCounter -
 */
typedef struct SwitchCounter SwitchCounter;
struct SwitchCounter {
	unsigned char button:1;		// input type: 0=bistable switch, 1=monostable button
	unsigned long timeout;		// after the timeout cntr goes automatically to 0
	unsigned char state;
	unsigned char cntr;
	Debounce ireader[1];		// in order to debounce input signal
	Timer timer[1];
};
extern void switchCounterInit(SwitchCounter* me, unsigned int timeout);
extern void switchCounterSetInputType(SwitchCounter* me, char inputType);
extern unsigned char switchCounterCount(SwitchCounter* me, bool i);
#define	switchCounterOutput(me)		((me)->cntr)

/*
 * OnDelay - The output is only set after a configurable on-delay time has expired.
 *
 * The time Ta is triggered with a 0 to 1 transition at input Trg (Ta is the current time).
 * If the status of input Trg is 1 at least for the duration of the configured time T,
 * the output is set to 1 on expiration of this time (the output follows the input with on-delay).
 * The time is reset when the status at input Trg returns to 0 before the time T has expired.
 * The output is reset to 0 when the signal at input Trg is 0.
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
 * INPUT:
 *		Trg - You start the on-delay with a positive edge (0 to 1 transition) at input trg (Trigger).
 * PARAMETER:
 *		T - represents the time after which the output is switched on (0 to 1 transition of the
 *			output signal).
 * OUTPUT:
 *		Q - is switched on when the set time T has expired, provided Trg is still set.
 */
typedef struct OnDelay OnDelay;
struct OnDelay {
	unsigned long T;
	Timer timer[1];
	unsigned char state;
};

extern void onDelayInit(OnDelay* me, unsigned long T);
extern void onDelaySetT(OnDelay* me, unsigned long T);
extern bool onDelayUpdate(OnDelay* me, bool Trg);
#define	onDelayOutput(me)	((me)->state & 1)


/*
 * TimedOff -
 *
 *
 *				 +--------+
 *				 | +----+ |
 *				 | |    | |
 *			Trg -|-+    +-|
 *			R   -| +--+   |- Q
 *			     | |  |   |
 *			Par -|-+  +---|
 *				 |        |
 *			     +--------+
 *
 * INPUT:
 *		Trg -
 * PARAMETER:
 *		T -
 * OUTPUT:
 *		Q -
 */
#define TimedOff		OnDelay
#define	timedOffInit	onDelayInit
#define	timedOffSetT	onDelaySetT
#define	timedOffUpdate(_me, _Trg)	(onDelayUpdate((_me), (_Trg)) ^ (_Trg))

/*
 * OffDelay - When an off-delay is set, the output is reset when the configured time has expired.
 *
 * Output Q is set to hi immediately when the input Trg changes to hi.
 * The actual time Ta is retriggered at the 1 to 0 transition of Trg.
 * The output remains set. Output Q is reset to 0 with off-delay when Ta reaches the value configured at
 * T (Ta=T).
 * The time Ta is retriggered with a one-shot at input Trg.
 * You can set input R (Reset) to reset the time Ta and the output before Ta has expired.
 * 
 *				 +--------+
 *				 | +-+    |
 *				 | | |    |
 *			Trg -|-' '----|
 *			R   -| +----+ |- Q
 *			     | | :  | |
 *			Par -|-+ :  +-|
 *				 |        |
 *			     +--------+
 *
 * INPUT:
 *		Trg - You start the off-delay time with a negative edge (1 to 0 transition) at input Trg (Trigger).
 *		R - A signal at input R resets the off-delay time and the output.
 * PARAMETER:
 *		T - is the time that expires after which the output is switched off (1 to 0 transition of the output signal).
 * OUTPUT:
 *		Q - is set with a signal at input Trg. It holds this state until T has expired.
 */
typedef struct OffDelay OffDelay;
struct OffDelay {
	unsigned long T;
	Timer timer[1];
	unsigned char state;
};

extern void offDelayInit(OffDelay* me, unsigned long T);
extern void offDelaySetT(OffDelay* me, unsigned long T);
extern bool offDelayUpdate(OffDelay* me, bool Trg, bool R);
#define	offDelayOutput(me)	((me)->state & 1)


/*
 * OnOffDelay - The on-/off-delay function sets the output after the set ondelay time has expired, 
 * and resets it upon expiration of the off-delay time.
 *
 * The time TH is triggered with a 0 to 1 transition at input Trg.
 * If the status at input Trg is 1 at least for the duration of the
 * time TH, the output is set to 1 on expiration of the time TH
 * (the output follows the input with on-delay).
 * The time is reset when the signal at input Trg is reset to 0
 * before the time TH has expired..
 * A 1 to 0 transition at input Trg triggers the time TL.
 * If the status at input Trg is 0 at least for the duration of the
 * signal TL, the output is set to 0 on expiration of the time TL
 * (the output follows the input with off-delay).
 * The time is reset when the signal at input Trg changes to 1
 * again before the time TL has expired.
 * 
 *				 +--------+
 *				 | +---+  |
 *				 | | : |  |
 *			Trg -|-' : '--|
 *				 |   +--+ |- Q
 *			     |   |  | |
 *			Par -|---+  +-|
 *				 |        |
 *			     +--------+
 *
 * INPUT:
 *		Trg - A positive edge (0 to 1 transition) at input Trg (Trigger) triggers the on-delay time TH.
 *			  A negative edge (1 to 0 transition) at input Trg (Trigger) triggers the off-delay time TL.
 * PARAMETER:
 *		TH - TH is the time after which the output is set hi (output signal transition 0 to 1). 
 *		TL - TL is the time after which the output is reset (output signal transition 1 to 0).
 * OUTPUT:
 *		Q - Q is set when the configured time TH has expired and Trg is still set. It is reset
 *			on expiration of the time TL, if the trigger Trg has not been set again.
 */
typedef struct OnOffDelay OnOffDelay;
struct OnOffDelay {
	unsigned long TH;
	unsigned long TL;
	Timer timer[1];
	unsigned char state;
};

extern void onOffDelayInit(OnOffDelay* me, unsigned long TH, unsigned long TL);
extern void onOffDelaySetTH(OnOffDelay* me, unsigned long TH);
extern void onOffDelaySetTL(OnOffDelay* me, unsigned long TL);
extern bool onOffDelayUpdate(OnOffDelay* me, bool Trg);
#define	onOffDelayOutput(me)	((me)->state & 1)


typedef struct SetReset SetReset;
struct SetReset {
	unsigned char state;
};

extern void setResetInit(SetReset* me);
extern bool setResetUpdate(SetReset* me, bool S, bool R);
#define	setResetOutput(me)	((me)->state & 1)


#ifdef	__cplusplus
}
#endif

#endif	/* PLC_H */

