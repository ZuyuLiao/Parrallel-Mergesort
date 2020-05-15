#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <stdbool.h>
#define THRESH 1024
pthread_mutex_t mutex;
static unsigned int thread_nr = 1;
static unsigned int thread_max = 0;
long binary_search2( long *array, long size, long value );
void merge2(long *res, long *array_b, long size_b, long *array_c, long size_c);
void p_merge2(long *res, long *array_b, long size_b, long *array_c, long size_c, long rec_level);
void* p_merge3(void *arg);
void mergesort2(long* res, long* in, long size, long rec_level);
void* p_mergesort2(void* args);
bool canIncrease();
void nom_mergesort(long *res, long *in, long size, long rec_level);
void printArr(long *res, long size) {
	for(int i = 0; i < size; i++) {
		printf("%ld ", res[i]);
	}
	printf("\n");
}

typedef struct mergesort2Params {
	long* res;
	long* in;
	long size;
	long level;

} mergesort2Params;
typedef struct p_merge2Params {
	long* res;
	long* array_b;
	long size_b;
	long* array_c;
	long size_c;
	long level;
} p_merge2Params;
long binary_search2( long *array, long size, long value ){
	long left = 0;
	long right = size - 1;
	if(array[left] >= value){
		return 0;
	}
	if(array[right] < value){
		return size;
	}
	while(left < right){
		long mid = (left + right) / 2;
		if(array[mid] < value){
			left = mid + 1;
		}else{
			right = mid;
		}
	}

	return left;
}
void nom_mergesort(long* res, long *in, long size, long rec_level) {
	if(size == 1) {
		res[0] = in[0];
	} else {
		long *C = malloc(size * sizeof(long));
		if(C == 0) {
			exit(-1);
			return;
		}
		nom_mergesort(C, in, size / 2, rec_level - 1);
		nom_mergesort(C + (size / 2), in + (size / 2), size - (size / 2), rec_level - 1);
		p_merge2(res, C, (size / 2), C + (size / 2), size - (size / 2), rec_level);
	}
}

void pMerge(long* array, long left, long mid, long right){
  long i, j, k;
  long l1 = mid - left + 1;
  long l2 = right - mid;
  long a1[l1], a2[l2];
  for (i = 0; i < l1; i++) a1[i] = array[left+i];
  for (j = 0; j < l2; j++) a2[j] = array[mid+1+j];
  i = 0;
  j = 0;
  k = left;
  while(i < l1 && j < l2){
    if(a1[i] <= a2[j]){
      array[k] = a1[i];
      i++;
    }else{
      array[k] = a2[j];
      j++;   
    }
    k++;
  }
  while(i < l1){
    array[k] = a1[i];
    i++;
    k++;
  }
  while(j < l2){
    array[k] = a2[j];
    j++;    
    k++;
  }
  //free(a1);
  //  //free(a2);
  }

void mergeSort(long *array, long left, long right){
  if(left >= right) return;
  long mid = left + (right - left)/2;
  mergeSort(array, left, mid);
  mergeSort(array, mid + 1, right);
  pMerge(array, left, mid, right);  
}
 

void* p_mergesort2(void* args) {
	mergesort2Params* arg = (mergesort2Params*) args;
	mergesort2(arg->res, arg->in, arg->size, arg->level);
	return 0;
}
bool canIncrease() {
	pthread_mutex_lock(&mutex);
	bool res = true;
	if(thread_nr < thread_max) {
		res = true;
		thread_nr++;
	} else {
		res = false;
	}
	pthread_mutex_unlock(&mutex);
	return res;
}
void mergesort2(long* res, long* in, long size, long rec_level){
	//printf("num of threads:%u\n", thread_nr);
	//pthread_mutex_lock(&mutex);
	//thread_nr++;
	//pthread_mutex_unlock(&mutex); 
	if(size == 1){
		res[0] = in[0];
	}
	else{
		if(size < THRESH) {
			memcpy( res, in, sizeof(long) * size);
			mergeSort(res, 0, size - 1);
			return;
		}
		long *C = malloc(size * sizeof(long));
		if(C == 0){
			exit(-1);
		}
		mergesort2Params mergesortArgs[2];
		mergesortArgs[0].res = C;
		mergesortArgs[0].in = in;
		mergesortArgs[0].size = size / 2;
		mergesortArgs[0].level = rec_level - 1;
		mergesortArgs[1].res = C + (size / 2);
		mergesortArgs[1].in = in + (size / 2);
		mergesortArgs[1].size = size - (size / 2);
		mergesortArgs[1].level = rec_level - 1;
		bool thread_crt = false;
		pthread_t left;
		if(!canIncrease()) {
			mergesort2(C, in, size / 2, rec_level - 1);
		} else {
			//pthread_t left, right;
			//pthread_create(&left, NULL, p_mergesort2, (void*)&mergesortArgs[0]);
			pthread_create(&left, NULL, p_mergesort2, (void*)&mergesortArgs[0]);
			thread_crt = true;
			//	pthread_join(left, NULL);
		}
		mergesort2(C + (size / 2), in + (size / 2), size - (size / 2), rec_level - 1);
		if(thread_crt){
			pthread_join(left, NULL);
			pthread_mutex_lock(&mutex);
			thread_nr -= 1;
			pthread_mutex_unlock(&mutex);
		}
		p_merge2(res, C, size / 2, C + (size / 2), size - (size / 2), rec_level);

	}
}
void* p_merge3(void* arg) {
	p_merge2Params* args = (p_merge2Params*) arg;
	//   pthread_mutex_lock(&mutex);
	// thread_nr++;
	// pthread_mutex_unlock(&mutex);
	p_merge2(args->res, args->array_b, args->size_b, args->array_c, args->size_c, args->level);
	return 0;
}
void p_merge2(long *res, long *array_b, long size_b, long *array_c, long size_c, long rec_level) {
	//	pthread_mutex_lock(&mutex);
	//  thread_nr++;
	// pthread_mutex_unlock(&mutex); 
	if(size_b < size_c){
		p_merge2(res, array_c, size_c, array_b, size_b, rec_level);
	}
	if(size_b < THRESH) {
		merge2(res, array_b, size_b, array_c, size_c);
		return;
	}
	long mid = size_b / 2;
	long index = binary_search2( array_c, size_c, array_b[mid] );
	res[mid + index] = array_b[mid];
	pthread_t left;
	bool thread_crt = false;
	if(!canIncrease()){
		p_merge2( res, array_b, mid, array_c, index, rec_level);
	}else{
		//printf("hello");
		p_merge2Params p_mergeArgs[2];
		p_mergeArgs[0].res = res;
		p_mergeArgs[0].array_b  = array_b;
		p_mergeArgs[0].size_b = mid;
		p_mergeArgs[0].array_c = array_c;
		p_mergeArgs[0].size_c = index;
		p_mergeArgs[0].level = rec_level - 1;
		p_mergeArgs[1].res = res + mid + index + 1;
		p_mergeArgs[1].array_b  = array_b + mid + 1;
		p_mergeArgs[1].size_b = size_b - mid - 1;
		p_mergeArgs[1].array_c = array_c + index;
		p_mergeArgs[1].size_c = size_c - index;
		p_mergeArgs[1].level = rec_level - 1;
		//pthread_t left, right;
		//pthread_create(&left, NULL, p_merge3, &p_mergeArgs[0]);

		pthread_create(&left, NULL, p_merge3, &p_mergeArgs[0]);
		//p_merge2(res + mid + index + 1, array_b + mid + 1, size_b - mid - 1, array_c + index, size_c - index, rec_level - 1);
		thread_crt = true;
		//pthread_join(left, NULL);

	}
	p_merge2(res + mid + index + 1, array_b + mid + 1, size_b - mid - 1, array_c + index, size_c - index, rec_level - 1);
	if(thread_crt) {
		pthread_join(left, NULL);
		pthread_mutex_lock(&mutex);
		thread_nr -= 1;
		pthread_mutex_unlock(&mutex);
	}
}

void merge2( long *res, long *array_b, long size_b, long *array_c, long size_c ){
	while(size_b > 0 && size_c > 0){
		if(*array_b <= *array_c){
			*res++ = *array_b++;
			size_b--;
		}else{
			*res++ = *array_c++;
			size_c--;
		}
	}

	while(size_b > 0){
		*res++ = *array_b++;
		size_b--;
	}


	while(size_c > 0){
		*res++ = *array_c++;
		size_c--;
	}
}
long *pthread_sort(long *array, long size, unsigned int num_threads) {
	long *res = malloc(sizeof(long) * size);
	if(res == 0){
		exit(-1);
	}
	int level = (int)(log(num_threads) / log(2));
	pthread_mutex_init(&mutex, NULL);
	thread_max = num_threads;
	mergesort2(res, array, size, level);
	pthread_mutex_destroy(&mutex);
	thread_nr = 1;
	return res;
}