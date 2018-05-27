#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>

#include "xmmintrin.h"
#include "pmmintrin.h"

#include <malloc.h>

double gettime(void)
{
	struct timeval ttime;
	gettimeofday(&ttime , NULL);
	return ttime.tv_sec + ttime.tv_usec * 0.000001;
}

float randpval ()
{
	int vr = rand();
	int vm = rand()%vr;
	float r = ((float)vm)/(float)vr;
	assert(r>=0.0 && r<=1.00001);
	return r;
}

float horizontal_max_Vec4(__m128 x) {	
	__attribute__((aligned(16))) float result[4];
	_mm_store_ps(result, x);
	
	float max = result[0];
	
	if (result[1] > max) max = result[1];
	if (result[2] > max) max = result[2];
	if (result[3] > max) max = result[3];
	
	return max;

} 

int main(int argc, char ** argv)
{
	int N = atoi(argv[1]);
	int iters = 1000;
	
	srand(1);
	
	// Allocate aligned memory blocks
	float * mVec = (float*)_mm_malloc(sizeof(float)*N, 16);
	assert(mVec!=NULL);
	float * nVec = (float*)_mm_malloc(sizeof(float)*N, 16);
	assert(nVec!=NULL);
	float * LVec = (float*)_mm_malloc(sizeof(float)*N, 16);
	assert(LVec!=NULL);
	float * RVec = (float*)_mm_malloc(sizeof(float)*N, 16);
	assert(RVec!=NULL);
	float * CVec = (float*)_mm_malloc(sizeof(float)*N, 16);
	assert(CVec!=NULL);
	float * FVec = (float*)_mm_malloc(sizeof(float)*N, 16);
	assert(FVec!=NULL);
	
	for(int i=0;i<N;i++)
	{
		mVec[i] = (float)(2+rand()%10);
		nVec[i] = (float)(2+rand()%10);
		
		LVec[i] = 0.0;
		for(int j=0;j<mVec[i];j++)
		{
			LVec[i] += randpval();
		}
		
		RVec[i] = 0.0;
		for(int j=0;j<nVec[i];j++)
		{
			RVec[i] += randpval();
		}
		
		CVec[i] = 0.0;
		for(int j=0;j<mVec[i]*nVec[i];j++)
		{
			CVec[i] += randpval();
		}
		
		FVec[i] = 0.0;
		assert(mVec[i]>=2.0 && mVec[i]<=12.0);
		assert(nVec[i]>=2.0 && nVec[i]<=12.0);
		assert(LVec[i]>0.0 && LVec[i]<=1.0*mVec[i]);
		assert(RVec[i]>0.0 && RVec[i]<=1.0*nVec[i]);
		assert(CVec[i]>0.0 && CVec[i]<=1.0*mVec[i]*nVec[i]);
	}
	
	__attribute__((aligned(16))) __m128 num, num_0, num_1, num_2, 
	den, den_0, den_1, mmVec, mnVec, mLVec, mRVec, mCVec, mFVec, maxF4;
			
	float maxF = 0;
	
	double timeTotal = 0.0f;
	
	maxF4 = _mm_setzero_ps();
	
	for(int j=0;j<iters;j++)
	{
		double time0=gettime();  
		
		//N/4: SSE processes 4 float numbers simultaneously
		for(int i=0; i<N/4; i++){
			//float num_0 = LVec[i]+RVec[i];
			mLVec = _mm_load_ps(LVec+(i*4));
			mRVec = _mm_load_ps(RVec+(i*4));
			num_0 = _mm_add_ps(mLVec, mRVec);
			//float num_1 = mVec[i]*(mVec[i]-1.0)/2.0;
			mmVec = _mm_load_ps(mVec+(i*4));	
			num_1 = _mm_mul_ps(_mm_div_ps(_mm_sub_ps(mmVec, _mm_set_ps(1, 1, 1, 1)), _mm_set_ps(2, 2, 2, 2)), mmVec);
			//float num_2 = nVec[i]*(nVec[i]-1.0)/2.0;
			mnVec = _mm_load_ps(nVec+(i*4));
			num_2 = _mm_mul_ps(_mm_div_ps(_mm_sub_ps(mnVec, _mm_set_ps(1, 1, 1, 1)), _mm_set_ps(2, 2, 2, 2)), mnVec);
			//float num = num_0/(num_1+num_2);
			num = _mm_div_ps(num_0, _mm_add_ps(num_1, num_2));
			//float den_0 = CVec[i]-LVec[i]-RVec[i];
			mCVec = _mm_load_ps(CVec+(i*4));
			den_0 = _mm_sub_ps(_mm_sub_ps(mCVec, mLVec), mRVec);
			//float den_1 = mVec[i]*nVec[i];
			den_1 = _mm_mul_ps(_mm_load_ps(mVec+(i*4)), _mm_load_ps(nVec+(i*4)));
			//float den = den_0/den_1;
			den = _mm_div_ps(den_0, den_1);
			//FVec[i] = num/(den+0.01);
			mFVec = _mm_div_ps(num, _mm_add_ps(den, _mm_set_ps(0.01, 0.01, 0.01, 0.01)));
			//maxF = FVec[i]>maxF?FVec[i]:maxF;
			maxF4 = _mm_max_ps(maxF4, mFVec);	
		}
		
		maxF = horizontal_max_Vec4(maxF4); //returns max float from m128
		
		// Process the residue data in case N/4 leaves a remainder  
		for (int i = N-(N%4); i<N; i++){
			float num_0 = LVec[i]+RVec[i];
			float num_1 = mVec[i]*(mVec[i]-1.0)/2.0;
			float num_2 = nVec[i]*(nVec[i]-1.0)/2.0;
			float num = num_0/(num_1+num_2);
			float den_0 = CVec[i]-LVec[i]-RVec[i];
			float den_1 = mVec[i]*nVec[i];
			float den = den_0/den_1;
			FVec[i] = num/(den+0.01);
			maxF = FVec[i]>maxF?FVec[i]:maxF;
		}

		double time1 = gettime();
		timeTotal += time1-time0;
	}
	
	printf("Time %f Max %f\n", timeTotal/iters, maxF);
	
	free(mVec);
	free(nVec);
	free(LVec);
	free(RVec);
	free(CVec);
	free(FVec);
}