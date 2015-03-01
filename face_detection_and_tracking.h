#ifndef FACE_DETECT_H
#define FACE_DETECT_H

#include <opencv2\core\core.hpp>
#include <opencv2/objdetect/detection_based_tracker.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>

#ifdef _DEBUG_BOX_COORDS_
#include <stdio.h>
#endif

using namespace cv;

#include "structs.h"
#include "errors.h"

#define NROWS size[0]
#define NCOLS size[1]
#define NCHANNELS size[2]
#define NFRAMES size[3]

error_t face_detect(unsigned char * grayscale,\
				    VideoProperties vp,\
					BoxCoordinates** coordinatesList);

#define fill_box_coords(box, rect)\
{\
	box.x = rect.x;\
	box.y = rect.y;\
	box.width = rect.width;\
	box.height = rect.height;\
}

#define BOX_LIST (*coordinatesList)

#define WIDTH_REDUCTION_FACTOR 0.6
#define UPPER_HEIGHT_REDUCTION_FACTOR 0.2
#define LOWER_HEIGHT_REDUCTION_FACTOR 0.5
#define MAX_POINTS 200
#define QUALITY_LEVEL 0.01
#define MIN_FEATURE_DISTANCE 10
#define MAX_ITERATIONS 20
#define MIN_EPSILON 0.03
#define MIN_EIGENVALUE 0.001

#endif
