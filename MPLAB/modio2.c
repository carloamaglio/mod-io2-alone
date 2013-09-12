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
 */

#include	"system.h"
#include	"modio2.h"

_Module module[1];

void ModIO2Init(void)
{
	OPTION_REGbits.nWPUEN = 0;	// enable weak pull-up

	ANSELC = 0;
	ANSELA = 0;
	//_ANSEL(pinJUMPER) = 0;		// disable analog input
	_TRIS(pinJUMPER) = 1;		// make pin an input
	_LAT(pinJUMPER) = 0;		// make pin an input
	_WPU(pinJUMPER) = 1;		// turn on internal pull-up

	_ANSEL(pinGPIO0) = 0;

	module->jumper = JUMPER;
}
