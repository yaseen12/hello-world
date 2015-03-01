#include "file_reading.h"

#define GREEN_INDEX 1

error_t file_read(const char * filename, VideoProperties *properties, unsigned char ** green_channel, unsigned char ** grayscale)
{
	properties->fps = 0;
	properties->size[0] = properties->size[1] = properties->size[2] = properties->size[3] = 0;

	VideoCapture cap(filename); // open the video file for reading

    if ( !cap.isOpened() )  // if not success, exit program
    {
         error_print(ERROR_FILE_NOT_FOUND, __FILE__, __LINE__);
         return ERROR_FILE_NOT_FOUND;
    }

	//get properties of video
    properties->fps     = (float)cap.get(CAP_PROP_FPS)         ; //get the frames per seconds of the video
	properties->size[0] = (int)  cap.get(CAP_PROP_FRAME_HEIGHT);
	properties->size[1] = (int)  cap.get(CAP_PROP_FRAME_WIDTH) ;
	properties->size[2] =                                    1 ; // one channel only for this program
	properties->size[3] = (int)  cap.get(CAP_PROP_FRAME_COUNT) ;

	printf("Frame size %d x %d at %.1f fps\n", properties->size[1], properties->size[0], properties->fps);
	printf("Found %d frames in video\n\n", properties->size[3]);

	int nFrames = properties->size[3];

	*green_channel = (unsigned char *) malloc(properties->size[0] *\
											  properties->size[1] *\
											  properties->size[3] *\
											  sizeof(unsigned char));

	*grayscale     = (unsigned char *) malloc(properties->size[0] *\
											  properties->size[1] *\
											  properties->size[3] *\
											  sizeof(unsigned char));

	if(*green_channel == NULL || *grayscale == NULL)
	{
		error_print(ERROR_NOT_ENOUGH_MEMORY, __FILE__, __LINE__);
		return ERROR_NOT_ENOUGH_MEMORY;
	}

#ifdef _DEBUG_FILE_READING_
	namedWindow("Video_Preview",WINDOW_AUTOSIZE); //create a window called "MyVideo"
	namedWindow("Grayscale_Preview", WINDOW_AUTOSIZE);
#endif

	unsigned char * green_channel_temp = *green_channel;
	unsigned char * grayscale_temp = *grayscale;
	
	int nElementsGreen, nElementsGrayscale;

    for(int i = 0; i < nFrames; i++)
    {
		Mat frame, grayFrame;   

        bool bSuccess = cap.read(frame); // read a new frame from video

        if (!bSuccess) //if not success, break loop
        {
			error_print(ERROR_FRAME_READ_ERROR, __FILE__, __LINE__);
            return ERROR_FRAME_READ_ERROR;
        }
		
		//convert to grayscale
		cvtColor(frame, grayFrame, COLOR_BGR2GRAY);

#ifdef _DEBUG_FILE_READING_
		imshow("Video_Preview", frame); //show the frame in "MyVideo" window
		imshow("Grayscale_Preview", grayFrame);
		if(waitKey(30) == 27) //wait for 'esc' key press for 30 ms. If 'esc' key is pressed, break loop
		{
                printf("esc key is pressed by user\n"); 
                break; 
		}
#endif

		//copy green channel and grayscale image into array

		error_t eGreen = convert_mat_to_array(&frame, green_channel_temp, &nElementsGreen);
		error_t eGray = convert_mat_to_array(&grayFrame, grayscale_temp, &nElementsGrayscale);
		assert_no_error(eGreen | eGray);

#ifdef _DEBUG_VIDEO_TRANSFER_FILE_READING_
		Mat temp = Mat(480, 640, CV_8U, grayscale_temp);
		namedWindow("Frame_debug_row_conversion",WINDOW_AUTOSIZE); 
		imshow("Frame_debug_row_conversion", temp);
		waitKey(30);
#endif

		green_channel_temp += nElementsGreen;
		grayscale_temp += nElementsGrayscale;
		
    }

	cap.release();
	
	printf("Video read successfully\n");
    
	return ERROR_NO_ERROR;

}

error_t convert_mat_to_array(Mat * frame, unsigned char * lin_array, int * elements_copied)
{
	// accept only char type matrices
    if(frame->depth() != CV_8U)
	{
		error_print(ERROR_FRAME_DEPTH_MISMATCH_ERROR, __FILE__, __LINE__);
		return ERROR_FRAME_DEPTH_MISMATCH_ERROR;
	}

    int channels = frame->channels();

    int nRows = frame->rows;
    int nCols = frame->cols * channels;

    if (frame->isContinuous())
    {
        nCols *= nRows;
        nRows = 1;
    }

    int i,j;
    unsigned char* p;
	int nElements = 0;

	unsigned char *destination = lin_array;

	if(channels == 1)//extract grayscale
	{
		size_t size_col = nCols*sizeof(unsigned char);
		for( i = 0; i < nRows; ++i)
		{
			p = frame->ptr<unsigned char>(i);
			memcpy(destination, p, size_col);
			destination += size_col;
			nElements += size_col;
		}
#ifdef _DEBUG_ROW_CONVERSION_
		Mat temp = Mat(480, 640, CV_8U, destination-640*480);
		namedWindow("Frame_debug_row_conversion",WINDOW_AUTOSIZE); 
		imshow("Frame_debug_row_conversion", temp);
		waitKey(30);
#endif
	}
	else //extract green channel
	{
		for( i = 0; i < nRows; ++i)
		{
			p = frame->ptr<unsigned char>(i);
			for(j = GREEN_INDEX; j < nCols; j+=channels)
			{
				*destination = p[j];
				destination++;
				nElements++;
			}
		}
	}

	*elements_copied = nElements;
    return ERROR_NO_ERROR;
}
