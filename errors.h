/*
	Defines the errors encountered in this program
	By convention, all custom writtern functions must return errors. Other things 
	are returned via pointers
*/

#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>
#include <assert.h>

#define error_t int

enum ERROR_LIST
{
	ERROR_NO_ERROR = 0,
	ERROR_FILE_NOT_FOUND,
	ERROR_NOT_ENOUGH_MEMORY,
	ERROR_FRAME_READ_ERROR,
	ERROR_FRAME_DEPTH_MISMATCH_ERROR,
	ERROR_FACE_NOT_FOUND,
	ERROR_FEATURES_NOT_TRACKED,
	ERROR_DWT_SIZE_MISMATCH,
	ERROR_BAD_FPS
};

void error_print(error_t error, const char * filename, const int line_number);
void assert_no_error(error_t error);

#endif
