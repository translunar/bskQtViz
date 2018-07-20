/*
 ISC License

 Copyright (c) 2016-2017, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */
/*
* astroFunctions.h
*
* University of Colorado, Autonomous Vehicle Systems (AVS) Lab
* Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
*/

#include "orbitalMotion.h"

int  loadSpiceKernels(const char *dataPath);
void unloadSpiceKernels();
int  loadSpiceKernel(char *kernelName, const char *dataPath);

double et2jd(double et);
void jd2gps(double julianDate, unsigned short *gpsWeek, unsigned long *gpsSeconds, int *gpsRollovers);
double getLocalSideralTime(CelestialObject_t celestialObject, double ET);
void unloadSpiceKernels(void);
