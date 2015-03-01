#include "errors.h"

void error_print(error_t error, const char * filename, const int line_number)
{
	fprintf(stderr, "ERROR: ");
	switch(error)
	{
	case ERROR_FILE_NOT_FOUND:
		fprintf(stderr, "File not found");
		break;
	case ERROR_NOT_ENOUGH_MEMORY:
		fprintf(stderr, "Not enough memory");
		break;
	case ERROR_FRAME_READ_ERROR:
		fprintf(stderr, "Frame could not be read");
		break;
	case ERROR_FRAME_DEPTH_MISMATCH_ERROR:
		fprintf(stderr, "Frame is not of type unsigned char");
		break;
	case ERROR_FACE_NOT_FOUND:
		fprintf(stderr, "Face could not be detected");
		break;
	case ERROR_FEATURES_NOT_TRACKED:
		fprintf(stderr, "Facial features could not be tracked");
		break;
	case ERROR_DWT_SIZE_MISMATCH:
		fprintf(stderr, "Mismatch in DWT size");
		break;
	case ERROR_BAD_FPS:
		fprintf(stderr, "Frame rate not supported");
		break;
	default:
		fprintf(stderr, "Code %d", error);
		break;
	}
	fprintf(stderr, "\nFile: %s, Line No. %d\n\n", filename, line_number);
}

void assert_no_error(error_t error)
{
	assert(error == ERROR_NO_ERROR);
}
