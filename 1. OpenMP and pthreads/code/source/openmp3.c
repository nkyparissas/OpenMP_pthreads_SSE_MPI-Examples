//Implementation: 1 task = 1 string from set A compared with all the strings from set B

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>

double finky_freeky(int, int, int, int, int, int);

double gettime(void)
{
	struct timeval ttime;
	gettimeofday(&ttime, NULL);
	return ttime.tv_sec + ttime.tv_usec * 0.000001;
}

int main (int argc, char **argv){
	
	double sum = 0;
	double avg = 0;
	int m, l, n, cores, seed;
	
	if(argc == 6){
		m = atoi(argv[1]);
		n = atoi(argv[2]);
		l = atoi(argv[3]);
		cores = atoi(argv[4]);
		seed = atoi(argv[5]);
	}else{
		printf("Wrong number of arguments\n");
		return 0;
	}
	
	/* 
	Average of 100 runs:
	In case the problem consists of 1M cells or less, the processing time
	is small and has a significant variance.
	*/
	printf("OPENMP 3\nA size: %d, B size: %d, string length: %d, cores: %d\n", m, n, l, cores);
	if (m*n*l <= 100*100*100){
		for (int i = 0; i < 100; i++){
			sum += finky_freeky(m, n, l, cores, seed, i);
		}
		avg = sum / 100;
		printf("Execution time: %lf sec\n", avg);
	}else
		printf("Execution time: %lf sec\n", finky_freeky(m, n, l, cores, seed, 99));
		
	return 1;
}

double finky_freeky(int m, int n, int l, int cores, int seed, int loop){

	char** A;
	char** B;
	
	srand(seed);

	assert(A = (char **)malloc(l*sizeof(char*)));
	for (int i = 0; i < l; i++)	
		assert(A[i] = (char *)malloc(m*sizeof(char)));

	assert(B = (char **)malloc(l*sizeof(char*)));
	for (int i = 0; i < l; i++)	
		assert(B[i] = (char *)malloc(n*sizeof(char)));

	for (int i = 0; i < l; i++ )
		for(int j = 0; j < m; j++)
			A[i][j] = (rand()%2 == 0)?'0':'1' ;

	for (int i = 0; i < l; i++ )
		for(int j = 0; j < n; j++)
			B[i][j] =(rand()%2 == 0)?'0':'1' ;

	int** HamTable;

	assert(HamTable = (int **)malloc(m*sizeof(int*)));
	for (int i = 0; i < m; i++)	
		assert(HamTable[i] = (int *)malloc(n*sizeof(int)));

	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			HamTable[i][j] = 0;
	
	double time0 = gettime();
	
	#pragma omp parallel num_threads(cores)
	{
		int ID=omp_get_thread_num();

		for(int i = (ID*(m/(float)cores)); i < floor((ID+1)*(m/(float)cores)); i++){
		// floor(), float: in case ID*m / cores leaves a remainder	
			for(int j = 0; j < n; j++){
				for(int c = 0; c < l; c++){
					if (A[c][i] != B[c][j])
						HamTable[i][j]++;
				}
			}		
		}
	/* 
	The parallel construct implies a barrier 
	in the end of the parallel region. No need 
	for an explicit barrier.
	*/
	}

	double time1 = gettime();
	
	/* 
	unsigned long long int because if m=10000 n=10000 l=1000, 
	then there can be sum > unsigned int
	*/
	unsigned long long int sum = 0;
	for (int i = 0; i < m; i++)
			for(int j = 0; j < n; j++)
				sum = HamTable[i][j] + sum;
	
	/*
	Print the results if this is the last loop executed.
	*/
	if (loop == 99)
		printf("Hamming distance for %d, %d, %d: %llu\n", m, n, l, sum );

	for (int i = 0; i < m; i++) 
		free (HamTable[i]);
	free (HamTable);
	
	for (int i = 0; i < l; i++)	{
		free(A[i]);
		free(B[i]);
	}

	free(A);
	free(B);

	return time1-time0;
}
