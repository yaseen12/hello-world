#ifndef DETRENDING_H
#define DETRENDING_H

#include <math.h>

#include "wavelet2d.h"
#include "errors.h"
#include "debug.h"

#ifdef _DEBUG_DETRENDING_
#include <stdio.h>
#endif

#define LEVELS 4
#define WAVELET "db8"

error_t detrending(double **detrended, double *sp_averaged, int nElements);

void normalize(double *signal, int nElements);

#endif
