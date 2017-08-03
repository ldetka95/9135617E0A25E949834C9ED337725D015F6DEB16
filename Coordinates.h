#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <glut/glut.h>

#include <math.h>

#define PI 3.1416

void cts( double x, double y, double z, double* r, double* i, double* a ) {
    // x, y, z, radius - units
    // inclination, azimuth - radians
    double rT = sqrt( x * x + y * y + z * z );
    double iT = acos( y / rT );
    double aT = atan2( z, x );
    if ( r ) {
        ( *r ) = rT;
    }
    if ( i ) {
        ( *i ) = iT;
    }
    if ( a ) {
        ( *a ) = aT;
    }
}

void stc( double r, double i, double a, double* x, double* y, double* z ) {
    // x, y, z, radius - units
    // inclination, azimuth - radians
    double sT = sin( i );
    double xT = r * sT * cos( a );
    double yT = r * cos( i );
    double zT = r * sT * sin( a );
    if ( x ) {
        ( *x ) = xT;
    }
    if ( y ) {
        ( *y ) = yT;
    }
    if ( z ) {
        ( *z ) = zT;
    }
}

void ptc( double r, double a, double* x, double* y ) {
    // x, y, radius - units
    // angle - radians
    double xT = sin( a ) * r;
    double yT = cos( a ) * r;
    if ( x ) {
        ( *x ) = xT;
    }
    if ( y ) {
        ( *y ) = yT;
    }
}

const double DEGR90 = ( 90.0 * PI / 180.0 );

void ctp( double x, double y, double* r, double* a ) {
    // x, y, radius - units
    // angle - radians
    double rT = sqrt( x * x + y * y );
    double aT = ( DEGR90 - atan2( y, x ) );
    if ( aT < 0 ) {
        aT = aT + 2 * PI;
    }
    if ( r ) {
        ( *r ) = rT;
    }
    if ( a ) {
        ( *a ) = aT;
    }
}

// maybe it will help sometimes

const double ___CONST_RAD_MULTIPLIER = PI / 180.0;

double radn( double angle_degress ) {
    return angle_degress * ___CONST_RAD_MULTIPLIER;
}

double degr( double angle_radians ) {
    return angle_radians / ___CONST_RAD_MULTIPLIER;
}



