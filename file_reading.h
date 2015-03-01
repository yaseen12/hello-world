#ifndef FILE_READING_H
#define FILE_READING_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "structs.h"
#include "errors.h"
#include "debug.h"

using namespace cv;

#define NROWS size[0]
#define NCOLS size[1]
#define NCHANNELS size[2]
#define NFRAMES size[3]


int file_read(const char * filename,\
	             VideoProperties * properties,\
				 unsigned char ** green_channel,\
				 unsigned char ** grayscale);

int convert_mat_to_array(Mat * frame,\
							 unsigned char * lin_array,\
							 int * elements_copied);


#endif
