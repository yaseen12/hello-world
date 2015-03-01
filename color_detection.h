#ifndef COLOR_DETECTION_H
#define COLOR_DETECTION_H

#include <vector>

#include "wavelet2d.h"
#include "structs.h"
#include "debug.h"
#include "errors.h"

#include "fftw3.h"

#define TAPS 129
#define WINDOW_LENGTH_CLIPPING 8
#define FACTOR 1.75

extern const double filter_15[];
extern const double filter_25[];
extern const double filter_30[];

error_t find_hr(double * hr, double *detrended, int clip_duration, int fps, int nElements);

double stdev(double *sig, int N);

#endif
