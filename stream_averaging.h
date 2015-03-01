/*
Implements the function for computing the spatial average of the ROI in the green channel
*/
#ifndef STREAM_AVERAGING_H
#define STREAM_AVERAGING_H

#include <opencv2\core\core.hpp>
#include "structs.h"
#include "errors.h"

using namespace cv;

#define NROWS size[0]
#define NCOLS size[1]
#define NCHANNELS size[2]
#define NFRAMES size[3]

error_t spatial_average(double ** average,\
	unsigned char * green,\
	VideoProperties vp,
	BoxCoordinates *);

#endif
