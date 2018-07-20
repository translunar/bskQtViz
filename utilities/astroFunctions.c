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
* astroFunctions.c
*
* University of Colorado, Autonomous Vehicle Systems (AVS) Lab
* Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
*/

#include "astroFunctions.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "projectMacros.h"
#include "SpiceUsr.h"
#include "shuntingYard.h"

/*
* Function: loadSpiceKernels
* Purpose: Attemps to load all necessary SPICE kernels
* Inputs:
*   dataPath = path to where SPICE data is located
* Outputs:
*   returns 0 if successful
*/
int loadSpiceKernels(const char *dataPath)
{
    if(loadSpiceKernel((char *)"naif0010.tls", dataPath)) {
        printf("Unable to load %s", "naif0010.tls");
        return 1;
    }
    if(loadSpiceKernel((char *)"pck00010.tpc", dataPath)) {
        printf("Unable to load %s", "pck00010.tpc");
        return 1;
    }
    if(loadSpiceKernel((char *)"de-403-masses.tpc", dataPath)) {
        printf("Unable to load %s", "de-403-masses.tpc");
        return 1;
    }
    if(loadSpiceKernel((char *)"de421.bsp", dataPath)) {
        printf("Unable to load %s", "de421.bsp");
        return 1;
    }
    if(loadSpiceKernel((char *)"MAR033_2000-2025.bsp", dataPath)) {
        printf("Unable to load %s", "MAR033_2000-2025.bsp");
        return 1;
    }
    return 0;
}

/*
* Function: unloadSpiceKernels
* Purpose: Unloads all necessary SPICE kernels
* Inputs:
* Outputs:
*/
void unloadSpiceKernels(void)
{
    unload_c("cspice/data/naif0010.tls");
    unload_c("cspice/data/pck00010.tpc");
    unload_c("cspice/data/de-403-masses.tpc");
    unload_c("cspice/data/de421.bsp");
    unload_c("cspice/data/MAR033_2000-2025.bsp");
}

/*
* Function: loadSpiceKernel
* Purpose: Attempts to load a SPICE kernel, returns 0 if successful
* Inputs:
*   kernelName = string name of kernel to load
*   dataPath = path to where SPICE data is located
* Outputs:
*   int = 0 if successful, 1 if failed
*/
int loadSpiceKernel(char *kernelName, const char *dataPath)
{
    if (strlen(dataPath) == 0) return 0;
    
    char fileName[CHAR_BUFFER_SIZE] = "";
    SpiceChar name[CHAR_BUFFER_SIZE] = "";
    
    strcpy(name, "REPORT");
    erract_c("SET", CHAR_BUFFER_SIZE, name);
    strcpy(fileName, dataPath);
    // Add trailing slash if the user didn't enter a path containing one
    int dataPathLength = strlen(dataPath);
    if (fileName[dataPathLength - 1] != '/') {
        fileName[dataPathLength] = '/';
    }
    strcat(fileName, kernelName);
    furnsh_c(fileName);

    strcpy(name, "DEFAULT");
    erract_c("SET", CHAR_BUFFER_SIZE, name);
    if(failed_c()) {
        return 1;
    }
    return 0;
}

/*
* Function: et2jd
* Purpose: Converts an input epoch represented in TDB seconds past J2000 to a
*   Julian Date.
* Inputs:
*   et = input epoch represented in TDB seconds past J2000
* Outputs:
*   julianDate = Julian Date
*/
double et2jd(double et)
{
    char str[CHAR_BUFFER_SIZE];
    et2utc_c(et, "J", 14, CHAR_BUFFER_SIZE - 1, str);
    return shuntingYard(&str[3]);
}

/*
* Function: jd2gps
* Purpose: Converts Julian date to GPS week number (since 1980.01.06) and
*   seconds of the week.
* Inputs:
*   julianDate = Julian Date
* Outputs:
*   gpsWeek = GPS week number
*   gpsSeconds = seconds of week since 0:00, Sun
*   gpsRollovers = number of GPS week rollovers (modulus 1024)
*/
void jd2gps(double julianDate, unsigned short *gpsWeek, unsigned long *gpsSeconds, int *gpsRollovers)
{
    /* Beginning of GPS week numbering 1980.01.06 */
    double julianDateGPS = 2444244.5;
    double temp;

    temp = (julianDate - julianDateGPS) / 7.0;
    if(temp < 0.0) {
        *gpsWeek = (unsigned short)ceil(temp);
    } else {
        *gpsWeek = (unsigned short)floor(temp);
    }
    *gpsSeconds = (unsigned long)((julianDate - (julianDateGPS + *gpsWeek * 7.0)) * 24.0 * 60.0 * 60.0);
    temp = *gpsWeek / 1024.0;
    if(temp < 0.0) {
        *gpsRollovers = (int)ceil(temp);
    } else {
        *gpsRollovers = (int)floor(temp);
    }
}

/*
* Function: getLocalSideralTime
* Purpose: finds the planet's polar rotation angle between body-fixed and inertial frames
* Inputs:
*   celestialObject = planet to calculate for
*   ET = ephemeris time
* Outputs:
*   gamma = local sideral time of the planets rotation
*/
double getLocalSideralTime(CelestialObject_t celestialObject, double ET)
{
    double EN[3][3];
    EN[0][1] = EN[0][0] = 0.;

    switch(celestialObject) {
        case CELESTIAL_EARTH:
            pxform_c("J2000", "IAU_EARTH", ET, EN);
            break;
        case CELESTIAL_MARS:
            pxform_c("MARSIAU", "IAU_MARS", ET, EN);
            break;

        default:
            break;
    }
    return atan2(EN[0][1], EN[0][0]);
}
