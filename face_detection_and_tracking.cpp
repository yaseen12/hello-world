#include "face_detection_and_tracking.h"

#ifdef _DEBUG_BOX_COORDS_
	FILE *matFile = fopen("C:\\Imad\\GeometricTx.txt", "w");
#endif


error_t face_detect(unsigned char * grayscale, const VideoProperties vp, BoxCoordinates** coordinatesList)
{
	int nFrames = vp.size[3];

	//create box coordinates array
	*coordinatesList = (BoxCoordinates * ) malloc(2*nFrames*sizeof(BoxCoordinates));//one box for each face section
	if(*coordinatesList == NULL)
	{
		error_print(ERROR_NOT_ENOUGH_MEMORY, __FILE__, __LINE__);
        return ERROR_NOT_ENOUGH_MEMORY;
	}

	//create the cascade classifier object used for the face detection
    CascadeClassifier face_cascade;
    face_cascade.load("C:\\OpenCV\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt.xml");

	//face detection in first frame
	Mat frame = Mat(vp.NROWS, vp.NCOLS, CV_8U, grayscale);
	grayscale += vp.NROWS * vp.NCOLS;//move to next image
	
	std::vector<Rect> faces;
	face_cascade.detectMultiScale(frame, faces, 1.1, 3, 0, Size(30,30));

	//check if face is detected in the video frame
	if(faces.size() == 0)
	{
		error_print(ERROR_FACE_NOT_FOUND, __FILE__, __LINE__);
        return ERROR_FACE_NOT_FOUND;
	}

	//break into two portions -> upper and lower
	Rect upper_face, lower_face;

	upper_face.x	  = (int) (faces[0].x + faces[0].width/2 - faces[0].width * WIDTH_REDUCTION_FACTOR / 2);
	upper_face.y	  =        faces[0].y;
	upper_face.width  = (int) (faces[0].width * WIDTH_REDUCTION_FACTOR);
	upper_face.height = (int) (faces[0].height * UPPER_HEIGHT_REDUCTION_FACTOR);
	
	lower_face.x	  = (int) (faces[0].x + faces[0].width/2 - faces[0].width * WIDTH_REDUCTION_FACTOR / 2);
	lower_face.y	  = (int) (faces[0].y + faces[0].height * LOWER_HEIGHT_REDUCTION_FACTOR);
	lower_face.width  = (int) (faces[0].width * WIDTH_REDUCTION_FACTOR);
	lower_face.height = (int) (faces[0].height - faces[0].height * LOWER_HEIGHT_REDUCTION_FACTOR);

	//fill first box coordinates
	fill_box_coords(BOX_LIST[0] , upper_face);
	fill_box_coords(BOX_LIST[1] , lower_face);

	//first detected face
	Mat upper_face_ROI = frame( upper_face );
	Mat lower_face_ROI = frame( lower_face );

	//feature extraction
	std::vector<Point2f> features_lower[2];
	std::vector<Point2f> features_upper[2];
	std::vector<Point2f> features[2];

	goodFeaturesToTrack(upper_face_ROI, features_upper[1], MAX_POINTS, QUALITY_LEVEL, MIN_FEATURE_DISTANCE);
	goodFeaturesToTrack(lower_face_ROI, features_lower[1], MAX_POINTS, QUALITY_LEVEL, MIN_FEATURE_DISTANCE);

	//refine extracted feature locations
	TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS, MAX_ITERATIONS, MIN_EPSILON);
    Size subPixWinSize(10,10), winSize(31,31);

	cornerSubPix(upper_face_ROI, features_upper[1], subPixWinSize, Size(-1,-1), termcrit);
	cornerSubPix(lower_face_ROI, features_lower[1], subPixWinSize, Size(-1,-1), termcrit);

	//coordinate system adjustment to match full image
	Point2f correctionU, correctionL;
	correctionU = Point2f(float (upper_face.x), float (upper_face.y));
	correctionL = Point2f(float (lower_face.x), float (lower_face.y));

	int nUpper = features_upper[1].size();
	int nLower = features_lower[1].size();

	for(int i = 0; i < nUpper; i++)
	{
		features_upper[1][i] += correctionU;
	}

	for(int i = 0; i < nLower; i++)
	{
		features_lower[1][i] += correctionL;
	}

	//join features into single list
	features[0].insert(features[0].end(), features_lower[1].begin(), features_lower[1].end());
	features[0].insert(features[0].end(), features_upper[1].begin(), features_upper[1].end());

	//preparations for feature tracking
	Mat prev = frame.clone();
	std::vector<unsigned char> status;//tracking reliability of features
	std::vector<float> err;

	Mat transform;//for storing affine projection between frames

	for(int i = 1; i < nFrames; i++)
	{
		frame = Mat(vp.NROWS, vp.NCOLS, CV_8U, grayscale);
		grayscale += vp.NROWS * vp.NCOLS;//advance to next image

		//compute flow -> Lucas Kanade Tomasi feature tracker
		calcOpticalFlowPyrLK(prev, frame, features[0], features[1], 
			                 status, err, winSize, 1, termcrit, 0, MIN_EIGENVALUE);
		
		//remove points which are not reliably tracked
		size_t j, k;
        for( j = k = 0; j < features[1].size(); j++ )
        {
			if( !status[j] ) { continue; }
			features[1][k  ] = features[1][j];
			features[0][k++] = features[0][j];
		}
		features[1].resize(k); features[0].resize(k);

		//need atleast two features to create affine projection matrix
		if(k <= 1)
		{
			error_print(ERROR_FEATURES_NOT_TRACKED, __FILE__, __LINE__);
			return ERROR_FEATURES_NOT_TRACKED;
		}

#ifdef _DEBUG_FEATURE_TRACKING_
		printf("Frame: %d\tFeatures: %d\n", i, k);
#endif

		//estimate affine projection matrix
		transform = estimateRigidTransform(features[0], features[1], true);

		//transform previous box
		fill_box_coords(BOX_LIST[i<<1]       , BOX_LIST[(i<<1) - 2]);//same height and width for now
		fill_box_coords(BOX_LIST[(i<<1) + 1] , BOX_LIST[(i<<1) - 1]);
		
		double trUpper[2] = {double(BOX_LIST[i<<1].x), double(BOX_LIST[i<<1].y)};
		double trLower[2] = {double(BOX_LIST[(i<<1) + 1].x), double(BOX_LIST[(i<<1) + 1].y)};
		
		double t11, t12, t13, t21, t22, t23;
		t11 = transform.at<double>(0,0);
		t12 = transform.at<double>(0,1);
		t13 = transform.at<double>(0,2);
		t21 = transform.at<double>(1,0);
		t22 = transform.at<double>(1,1);
		t23 = transform.at<double>(1,2);

		double uX = t11 * trUpper[0] + t12 * trUpper[1] + t13;
		double uY = t21 * trUpper[0] + t22 * trUpper[1] + t23;
		double lX = t11 * trLower[0] + t12 * trLower[1] + t13;
		double lY = t21 * trLower[0] + t22 * trLower[1] + t23;

		int ux = BOX_LIST[i<<1].x = int( floor( uX + 0.5 ) );
		int uy = BOX_LIST[i<<1].y = int( floor( uY + 0.5) );
		int lx = BOX_LIST[(i<<1)+1].x = int( floor( lX + 0.5 ) );
		int ly = BOX_LIST[(i<<1)+1].y = int( floor( lY + 0.5 ) );

#ifdef _DEBUG_BOX_COORDS_
		fprintf(matFile, "%d:\t[%.5f %.5f %.5f]\n\t[%.5f %.5f %.5f]\n", i, t11, t12, t13, t21, t22, t23);
		fprintf(matFile, "\n%d:\t(%.5f, %.5f)\t(%.5f, %.5f)\n\n", i, uX, uY, lX, lY);
		fprintf(matFile, "\n%d:\t(%d, %d)\t(%d, %d)\n\n", i, ux, uy, lx, ly);
#endif

		//box transformation complete

		//prepare for next frame
		std::swap(features[1], features[0]);
        cv::swap(prev, frame);
	}

#ifdef _DEBUG_FACE_DETECTING_
	Mat copyU = upper_face_ROI.clone();
	Mat copyL = lower_face_ROI.clone();
	RNG rng(12345);
	int r = 4;

	for( size_t i = 0; i < features_lower.size(); i++ )
    { 
		circle( copyL, features_lower[i], r, Scalar(rng.uniform(0,255), rng.uniform(0,255),
              rng.uniform(0,255)), -1, 8, 0 );
	}

	for( size_t i = 0; i < features_upper.size(); i++ )
    { 
		circle( copyU, features_upper[i], r, Scalar(rng.uniform(0,255), rng.uniform(0,255),
              rng.uniform(0,255)), -1, 8, 0 );
	}

	namedWindow("Reconstructed_Frame_Upper",WINDOW_AUTOSIZE); 
	imshow("Reconstructed_Frame_Upper", copyU);
	namedWindow("Reconstructed_Frame_Lower",WINDOW_AUTOSIZE); 
	imshow("Reconstructed_Frame_Lower", copyL);
	
	waitKey(30);
#endif

#ifdef _DEBUG_BOX_COORDS_
	fclose(matFile);
#endif

#ifdef _DEBUG_BOX_COORDS_
	namedWindow("Frame_Upper",WINDOW_AUTOSIZE);
	namedWindow("Frame_Lower",WINDOW_AUTOSIZE);
	Rect up, low;
	for(int i = nFrames-1; i >= 0; i--)
	{
		grayscale -= vp.NROWS * vp.NCOLS;
		fill_box_coords(up, BOX_LIST[i<<1]);
		fill_box_coords(low, BOX_LIST[(i<<1) + 1]);
		Mat temp = Mat(vp.NROWS, vp.NCOLS, CV_8U, grayscale);
		Mat upTemp = temp(up);
		Mat loTemp = temp(low);
		imshow("Frame_Upper", upTemp);
		imshow("Frame_Lower", loTemp);
		waitKey(30);
	}
#endif

	printf("Done tracking faces\n");

	return ERROR_NO_ERROR;
}
