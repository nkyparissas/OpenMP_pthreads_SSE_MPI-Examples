//Implementation: 1 task = parts of the 2 strings compared

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>


pthread_mutex_t lock;
pthread_mutex_t ID_lock;

// http://www.imdb.com/title/tt0357111/
typedef struct RRRrrrr {
	char ** A;
	char ** B;
	int ** Ham;
	int m;
	int n;
	int l;
	int threads;
} RRRrrrr;


void *ham_fun(void *param)
{
	static int counter = -1 ;
	int ID;
	
	RRRrrrr * another_object = (RRRrrrr *) param;
	int m = another_object->m;
	int n = another_object->n;
	int l = another_object->l;
	int cores= another_object->threads;
	

	pthread_mutex_lock(&ID_lock);
	
	if (counter==cores-1){
		counter = 0;
	}else{
		counter++;
	}
	ID=counter;
	
	pthread_mutex_unlock(&ID_lock);
		
	for (int i = 0; i < m; i++){
		for(int j = 0; j < n; j++){
			int sum = 0;
			for(int c = (ID*(l/(float)cores)); c < floor((ID+1)*(l/(float)cores)); c++){
			// floor(), float: in case ID*m / cores leaves a remainder
				if (another_object->A[c][i] != another_object->B[c][j])
					sum++;
			}
			pthread_mutex_lock(&lock);
			another_object->Ham[i][j] = another_object->Ham[i][j] + sum;
			pthread_mutex_unlock(&lock);
		}		
	}
	return 0;
}

double finky_freeky(int, int, int, int, int, int);

double gettime(void)
{
	struct timeval ttime;
	gettimeofday(&ttime, NULL);
	return ttime.tv_sec + ttime.tv_usec * 0.000001;
}

int main (int argc, char **argv){

	if (pthread_mutex_init(&lock, NULL) != 0){
		printf("\n mutex init failed\n");
	}
	
	if (pthread_mutex_init(&ID_lock, NULL) != 0){
		printf("\n mutex init failed\n");
	}

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
	printf("PTHREAD 1\nA size: %d, B size: %d, string length: %d, cores: %d\n", m, n, l, cores);
	if (m*n*l < 100*100*100){
		for (int i = 0; i < 100; i++){
			sum += finky_freeky(m, n, l, cores, seed, i);
		}
		avg = sum / 100;
		printf("Execution time: %lf sec\n", avg);
	}else
		printf("Execution time: %lf sec\n", finky_freeky(m, n, l, cores, seed, 99));
		
	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&ID_lock);
	
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
			B[i][j] = (rand()%2 == 0)?'0':'1' ;

	int** HamTable;

	assert(HamTable = (int **)malloc(m*sizeof(int*)));
	for (int i = 0; i < m; i++)	
		assert(HamTable[i] = (int *)malloc(n*sizeof(int)));

	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			HamTable[i][j] = 0;
	
	RRRrrrr * object = malloc(sizeof(RRRrrrr));
	object->A = A;
	object->B = B;
	object->Ham = HamTable;
	object->threads = cores;
	object->m = m;
	object->n = n;
	object->l = l;
	
	double time0 = gettime();

	pthread_t ham_thread[cores];

	for (int i = 0; i<cores; i++){
		if(pthread_create(&ham_thread[i], NULL, ham_fun, object)){
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
	}
	
	for (int i=0;i<cores; i++){
		pthread_join(ham_thread[i],NULL);
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
	
	for (int i = 0; i < l; i++){
		free(A[i]);
		free(B[i]);
	}

	free(A);
	free(B);

	free(object);

	return time1-time0;
}
