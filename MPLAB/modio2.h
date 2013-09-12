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
 * File:   modio2.h
 * Author: Carlo
 *
 * Created on 22 novembre 2012, 14.09
 */

#ifndef MODIO2_H
#define	MODIO2_H

#ifdef	__cplusplus
extern "C" {
#endif

// ingresso interruttore
#define pinJUMPER(f)		f(A, 5)
#define	JUMPER				(_PORT(pinJUMPER))

#define pinGPIO0(f)		f(A, 0)
#define	GPIO0			(_PORT(pinGPIO0))
#define	GPIO0set(value)	(_LAT(pinGPIO0) = (value))
#define	GPIO0off()		GPIO0set(0)
#define	GPIO0o()		GPIO0set(1)

#define pinGPIO1(f)		f(A, 1)
#define	GPIO1			(_PORT(pinGPIO1))
#define	GPIO1set(value)	(_LAT(pinGPIO1) = (value))
#define	GPIO1off()		GPIO1set(0)
#define	GPIO1o()		GPIO1set(1)

// rele' 1
#define pinR1(f)		f(C, 4)
#define	R1set(value)	(_LAT(pinR1) = (value))
#define	R1off()			R1set(0)
#define	R1on()			R1set(1)

// rele' 2
#define pinR2(f)		f(C, 2)
#define	R2set(value)	(_LAT(pinR2) = (value))
#define	R2off()			R2set(0)
#define	R2on()			R2set(1)

typedef struct _Module _Module, *Module;
struct _Module {
	unsigned int jumper:1;
};
extern _Module module[1];

extern void ModIO2Init(void);

#ifdef	__cplusplus
}
#endif

#endif	/* MODIO2_H */

