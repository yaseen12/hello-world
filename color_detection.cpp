#include "color_detection.h"

error_t find_hr(double * hr, double *detrended, int clip_duration, int fps, int nElements)
{
	//select filter
	const double *filter;
	switch(fps)
	{
	case 15:
		filter = filter_15;
		break;
	case 25:
		filter = filter_25;
		break;
	case 30:
		filter = filter_30;
		break;
	default:
		error_print(ERROR_BAD_FPS, __FILE__, __LINE__);
        return ERROR_BAD_FPS;
	}

	//push filter onto vector
	vector<double> coefs(filter, filter + TAPS);
	vector<double> unclean(detrended + int(clip_duration*fps), detrended + nElements);
	vector<double> filtered_signal;

	//perform filtering
	convfft(unclean, coefs, filtered_signal);

#ifdef _DEBUG_FILTERING_
	FILE* f = fopen("C:\\Imad\\Filtered.txt", "w");
	for(int i = 0; i < filtered_signal.size(); i++)
	{
		fprintf(f, "%f\n", filtered_signal[i]);
	}
	fclose(f);
#endif

	//compute fft to select peak frequency
	double *in;
	fftw_complex *fft;
	fftw_plan p;

	int N = filtered_signal.size();
	int nfft = N/2 + 1;

	//needs error checking
	in = (double*) fftw_malloc(sizeof(double) * N);
	fft = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
	p = fftw_plan_dft_r2c_1d(N, in, fft, FFTW_ESTIMATE);

	//initialize
	for(int i = 0; i < N; i++)
	{
		in[i] = filtered_signal[i];
	}

	//compute fft
	fftw_execute(p);

	//compute absolute value and store in place of real value
	for(int i = 0; i < nfft; i++)
	{
		fft[i][0] = sqrt(fft[i][0]*fft[i][0] + fft[i][1]*fft[i][1]);
	}

#ifdef _DEBUG_FILTERING_
	f = fopen("C:\\Imad\\FFT.txt", "w");
	for(int i = 0; i < nfft; i++)
	{
		fprintf(f, "%f\n", fft[i][0]);
	}
	fclose(f);
#endif

	//selecting peak frequency
	int loc = 0;
	double max_val = 0;
	for(int i = 0; i < nfft; i++)
	{
		if(fft[i][0] > max_val)
		{
			loc = i;
			max_val = fft[i][0];
		}
	}

	//pulse freq
	double fpulse = double(loc)/N*double(fps);

	//ma with five taps
	double *ma = (double*) malloc(sizeof(double) * N);
	double *time = (double*) malloc(sizeof(double) * N);
	double dt = 1.0/fps;
	time[0] = 0.0;
	for(int i = 1; i < N; i++)
	{
		time[i] = time[i - 1] + dt;
	}
	int i, k;
	double temp_sum;
	for(i = 0; i < 4; i++)
	{
		temp_sum = 0.0;
		for(k = i; k >= 0; k--)
		{
			temp_sum += in[k];
		}
		ma[i] = temp_sum * 0.2;
	}
	for(i = 4; i < N; i++)
	{
		ma[i] = 0.2 * (in[i] + in[i-1] + in[i-2] + in[i-3] + in[i-4]);
	}

#ifdef _DEBUG_FILTERING_
	f = fopen("C:\\Imad\\MA.txt", "w");
	for(int i = 0; i < N; i++)
	{
		fprintf(f, "%f\n", ma[i]);
	}
	fclose(f);
#endif

	//signal clipping
	double sd = 0, lim = 0;
	sd = stdev(ma, N);
	lim = sd/2;
	double m;
	int prev = 0;
	double * start_index = ma, * stop_index = ma + N;
#ifdef _DEBUG_FILTERING_
	FILE *window = fopen("C:\\Imad\\Windows.txt", "w");
	printf("%d\n", stop_index - start_index);
#endif
	for(int k = 0; k < N - WINDOW_LENGTH_CLIPPING; k+=WINDOW_LENGTH_CLIPPING)
	{
		m = 0;
#ifdef _DEBUG_FILTERING_
			fprintf(window, "%d: [", k);
#endif
		for(int count = 0, l = k; count < WINDOW_LENGTH_CLIPPING; count++, l++)
		{
#ifdef _DEBUG_FILTERING_
			fprintf(window, "%f ", ma[l]);
#endif
			if( m < fabs( ma[l] ) ) m = fabs( ma[l] );
		}
#ifdef _DEBUG_FILTERING_
			fprintf(window, "]\n%f\n\n", m);
#endif
		if( (prev == 0) && (m < lim) )
			start_index = ma + (k + (WINDOW_LENGTH_CLIPPING<<1));
		else if( (prev == 0) && (m >= lim) )
		{
			lim = sd/8;
#ifdef _DEBUG_FILTERING_
			fprintf(window, "Limit = %f\n\n", lim);
#endif
			prev = 1;
			start_index = ma + (k + WINDOW_LENGTH_CLIPPING);
		}
		else if( (prev == 1) && (m < lim) )
		{
			prev++;
		}
		else if( (prev >= 1) && (m >= lim) )
		{
			prev = 1;
		}
		else if( (prev == 2) && (k >= ( (N * 3) >> 2 ) ) )
		{
			stop_index = ma + (k - (WINDOW_LENGTH_CLIPPING<<2));
			break;
		}
	}
#ifdef _DEBUG_FILTERING_
	fclose(window);
#endif

#ifdef _DEBUG_FILTERING_
	f = fopen("C:\\Imad\\Clipped.txt", "w");
	double *temp = start_index;
	printf("%d\n", stop_index - start_index);
	while(temp < stop_index)
	{
		fprintf(f, "%f\n", *temp);
		temp++;
	}
	fclose(f);
#endif

	//updated standard dev
	sd = stdev(start_index, int(stop_index - start_index));
	int mpd = floor(fps/(FACTOR * fpulse));

	//peak detection
	char *peak_validity = 

	//ibi computation

	//hr computation

	//temporary solution
	*hr = double(loc)/N*double(fps);

	//free up internal mem alloc
	fftw_destroy_plan(p);
	fftw_free(in); fftw_free(fft);
	free(ma); free(time);

	printf("Found HR\n");
	return ERROR_NO_ERROR;
}

double stdev(double *sig, int N)
{
	double mean = 0, sd = 0, temp;
	for(int i = 0; i < N; i++)
	{
		temp = sig[i];
		mean += temp;
		sd += temp*temp;
	}
	return sqrt(sd/double(N) - (mean*mean)/double(N*N));
}
