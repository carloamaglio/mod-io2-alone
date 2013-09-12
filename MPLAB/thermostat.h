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
 * File:   thermostat.h
 * Author: Carlo
 *
 * Created on 29 ottobre 2012, 12.29
 */

#ifndef THERMOSTAT_H
#define	THERMOSTAT_H

#include "config.h"

#ifdef	__cplusplus
extern "C" {
#endif

#if	THERMOSTAT
extern void taskThermostat(void);
#else
#define	taskThermostat()
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* THERMOSTAT_H */

