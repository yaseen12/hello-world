#include "global_header.h"

void help(char* progName)
{
    printf("\n");
    printf("This program takes a video file as input and attempts\n");
	printf("to estimate the heart rate\n");
	printf("Usage -- %s video_name\n\n", progName);
}

int main(int argc, char ** argv)
{

	help(argv[0]);

	if(argc != 2) 
		return -1;

	unsigned char * green_channel, * grayscale;
	VideoProperties properties;
	BoxCoordinates *face_coordinates;
	double * averaged_stream;
	double * detrended;
	double hr;

	//file reading
	error_t file_read_error = file_read(argv[1], &properties, &green_channel, &grayscale);
	assert_no_error(file_read_error);

	//face detection and tracking
	error_t face_detect_error = face_detect(grayscale, properties, &face_coordinates);
	assert_no_error(face_detect_error);

	//averaging
	error_t sp_averaging_error = spatial_average(&averaged_stream, green_channel,
		                                         properties, face_coordinates);
	assert_no_error(sp_averaging_error);

	//detrending
	error_t detrending_error = detrending(&detrended, averaged_stream, properties.size[3]);
	assert_no_error(detrending_error);

	//filtering and processing
	error_t hr_error = find_hr(&hr, detrended, CLIP_DURATION, (int)properties.fps, properties.size[3]);
	assert_no_error(hr_error);

	printf("\n\tHeart rate = %.1f bpm\n\n", hr*60.0);

	//memory deallocation
	free(green_channel);
	free(grayscale);
	free(face_coordinates);
	free(averaged_stream);
	free(detrended);

	return 0;
}
