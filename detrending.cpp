#include "detrending.h"

error_t detrending(double **detrended, double *sp_averaged, int nElements)
{
	//mem allocate to detrended
	*detrended = (double *) malloc(nElements * sizeof(double));
	if(*detrended == NULL)
	{
		error_print(ERROR_NOT_ENOUGH_MEMORY, __FILE__, __LINE__);
        return ERROR_NOT_ENOUGH_MEMORY;
	}

	//conversion to vector
	vector<double> sig;
	for(int i = 0; i < nElements; i++)
	{
		sig.push_back(sp_averaged[i]);
	}

	//other arguments for DWT
	vector<double> dwt_output, flag;

	// perform J-Level DWT
	vector<int> length;

	//wavelet decomposition of sp_averaged stream
	dwt_sym(sig, LEVELS, WAVELET, dwt_output, flag, length);

#ifdef _DEBUG_DETRENDING_
	FILE *out = fopen("C:\\Imad\\DWT.txt", "w");
	for(int i = 0; i < dwt_output.size(); i++)
	{
		fprintf(out, "%f\n", dwt_output[i]);
	}
	fclose(out);
#endif

	//setting details to zero
	int l_signal = dwt_output.size();
	int details_start = length[0];
	for(int i = details_start; i < l_signal; i++)
	{
		dwt_output[i] = 0.0;
	}

#ifdef _DEBUG_DETRENDING_
	out = fopen("C:\\Imad\\DWT_SMOOTHED.txt", "w");
	for(int i = 0; i < dwt_output.size(); i++)
	{
		fprintf(out, "%f\n", dwt_output[i]);
	}
	fclose(out);
#endif

	//reconstruction
	vector<double> baseline;
	idwt_sym(dwt_output, flag, WAVELET,	baseline, length);

#ifdef _DEBUG_DETRENDING_
	out = fopen("C:\\Imad\\Baseline.txt", "w");
	for(int i = 0; i < baseline.size(); i++)
	{
		fprintf(out, "%f\n", baseline[i]);
	}
	fclose(out);
#endif

	if(baseline.size() != nElements)
	{
		error_print(ERROR_DWT_SIZE_MISMATCH, __FILE__, __LINE__);
		return ERROR_DWT_SIZE_MISMATCH;
	}

	//subtraction
	for(int i = 0; i < nElements; i++)
	{
		(*detrended)[i] = sp_averaged[i] - baseline[i];
	}

#ifdef _DEBUG_DETRENDING_
	out = fopen("C:\\Imad\\Reconstructed.txt", "w");
	for(int i = 0; i < nElements; i++)
	{
		fprintf(out, "%f\n", (*detrended)[i]);
	}
	fclose(out);
#endif

	normalize(*detrended, nElements);

	printf("Done detrending\n");
	return ERROR_NO_ERROR;
}

/*
Normalize by subtracting the mean and dividing by the sd
*/
void normalize(double *signal, int nElements)
{
	double mean = 0, sd = 0;
	double temp;
	for(int i = 0; i < nElements; i++)
	{
		temp = signal[i];
		mean += temp;
		sd += temp*temp;
	}
	mean /= (double) nElements;
	sd = sqrt( (sd/(double) nElements) - mean*mean);

	for(int i = 0; i < nElements; i++)
	{
		signal[i] = (signal[i] - mean)/sd;
	}

}


