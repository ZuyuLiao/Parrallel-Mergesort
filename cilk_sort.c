#include <cilk/cilk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define THRESH 512

long binary_search( long *array, long size, long value );
void merge( long *res, long *array_b, long size_b, long *array_c, long size_c );
void p_merge( long *res, long *array_b, long size_b, long *array_c, long size_c );
void mergesort( long *res, long *in, long size );
void pMerge(long* array, long left, long mid, long right);
void mergeSort(long *array, long left, long right);

long binary_search( long *array, long size, long value ){
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

void mergeSort(long *array, long left, long right){
  if(left >= right) return;
  long mid = left + (right - left)/2;
  mergeSort(array, left, mid);
  mergeSort(array, mid + 1, right);
  pMerge(array, left, mid, right);  
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

void mergesort( long *res, long *in, long size ){
	if(size == 1){
		res[0] = in[0];
	}else if(size < THRESH){
		memcpy( res, in, sizeof(long) * size);
		mergeSort(res, 0, size - 1);
	}else{
		long *C = malloc(size * sizeof(long));
		if(C == 0){
			exit(-1);
			return;
		}
		cilk_spawn mergesort(C, in, size / 2);
		mergesort(C + (size / 2), in + (size / 2), size - (size / 2));
		cilk_sync;
		p_merge( res, C, (size / 2), C + (size / 2), size - (size / 2));
    }
}

void p_merge( long *res, long *array_b, long size_b, long *array_c, long size_c ){
    if(size_b < size_c){
        p_merge( res, array_c, size_c, array_b, size_b );
    }
    else if( size_b <= THRESH ){
		merge( res, array_b, size_b, array_c, size_c );
    }else{
		long mid = size_b / 2;
		long index = binary_search( array_c, size_c, array_b[mid] );
		res[mid + index] = array_b[mid];
		cilk_spawn p_merge( res, array_b, mid, array_c, index );
		p_merge( res + mid + index + 1, array_b + mid + 1, size_b - mid - 1, array_c + index, size_c - index);
		cilk_sync;
    }
}

void merge( long *res, long *array_b, long size_b, long *array_c, long size_c ){
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

long *cilk_sort(long *array, long size, unsigned int num_threads) {
    // NOTE: It is not required that you use the variable num_threads in
    //       your solution.
    long *res = malloc(sizeof(long) * size);
    if(res == 0){
		exit(-1);
	}
	mergesort( res, array, size );
	return res;
}