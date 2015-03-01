#include "stream_averaging.h"

error_t spatial_average(double ** average, unsigned char * green, 
	                    VideoProperties vp, BoxCoordinates * boxList)
{
	int nFrames = vp.size[3];
	unsigned int sum;
	unsigned int nPixels;
	Rect up, lo;
	Mat frame, upM, loM;
	unsigned char * p;

	//create memory for spatially averaged signal
	*average = (double * ) malloc(nFrames*sizeof(double));
	if(*average == NULL)
	{
		error_print(ERROR_NOT_ENOUGH_MEMORY, __FILE__, __LINE__);
        return ERROR_NOT_ENOUGH_MEMORY;
	}

	for(int i = 0, ii = 0; i < nFrames; i++)
	{
		frame = Mat(vp.NROWS, vp.NCOLS, CV_8U, green);
		green += vp.NROWS * vp.NCOLS;

		ii = i << 1;

		up.x = boxList[ii].x;
		up.y = boxList[ii].y;
		up.height = boxList[ii].height;
		up.width = boxList[ii].width;

		upM = frame(up);
		
		lo.x = boxList[ii + 1].x;
		lo.y = boxList[ii + 1].y;
		lo.height = boxList[ii + 1].height;
		lo.width = boxList[ii + 1].width;

		loM = frame(lo);

		nPixels = up.height * up.width + lo.height * lo.width;
		sum = 0;

		for( int j = 0; j < up.height; j++)
		{
			p = upM.ptr<unsigned char>(j);
			for( int k = 0; k < up.width; k++)
			{
				sum += p[k];
			}
		}
		
		for( int j = 0; j < lo.height; j++)
		{
			p = loM.ptr<unsigned char>(j);
			for( int k = 0; k < lo.width; k++)
			{
				sum += p[k];
			}
		}

		(*average)[i] = double(sum)/double(nPixels);
	}

#ifdef _DEBUG_SP_AVERAGING_
	FILE *spFile = fopen("C:\\Imad\\sp_streams.txt", "w");
	for(int i = 0; i < vp.size[3]; i++)
	{
		fprintf(spFile, "%f\n", (*average)[i]);
	}
	fclose(spFile);
#endif

	printf("Done spatial averaging\n");
	return ERROR_NO_ERROR;
}
