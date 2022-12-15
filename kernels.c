/********************************************************
 * Kernels to be optimized for the  Performance Project
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include <sys/time.h>
#include <sys/resource.h>

/* Below are statements to set up the performance measurement utilities */
/* we use rdtsc, clock, and getusage utilities to measure performance */

//#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))
#if defined(__i386__)

static __inline__ unsigned long long rdtsc(void)
{
  unsigned long long int x;
     __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
     return x;
}
#elif defined(__x86_64__)


static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
#endif

/* end of definitions to set up measurement utilities */


/* 
 * Please fill in the following team struct 
 */
team_t team = {
    "ozzy",              /* Team name */

    "Ozzy Simpson",     /* First member full name */
    "ozzy@gwu.edu",  /* First member email address */

    "",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* the getUserTime function is used for measurement, you should not change the code for this function */

long int getUserTime()
{
	int who= RUSAGE_SELF;
	int ret;
	struct rusage usage;
	struct rusage *p=&usage;
	//long int current_time;

	ret=getrusage(who,p);
	if(ret == -1)
	{
		printf("Could not get GETRUSAGE to work in function %s at line %d in file %s\n",
				__PRETTY_FUNCTION__, __LINE__, __FILE__);
		exit(1);
	}
	return (p->ru_utime.tv_sec * 1000000 + p->ru_utime.tv_usec);
}

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
 /* The parameters, pointers, rusage_time, rdtsc_time, and cpu_time_used are used to measure performance and return values to caller. */
 /* You should not change the code that uses these parameters and variables. */
 
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst, int *rusage_time, unsigned long long *rdtsc_time) 
{
	int i, j;
	/* the variables below are used for performance measurement and not for computing the results of the algorithm */
	long int rusage_start_time, rusage_end_time = 0;
	unsigned long long rdtsc_start_time, rdtsc_end_time = 0;
	/* call system functions to start measuring performance. you should not bother to change these. */
	
	rusage_start_time = getUserTime();
	rdtsc_start_time = rdtsc();

/* below is the main computations for the rotate function */

	for (j = 0; j < dim; j++)
		for (i = 0; i < dim; i++)
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];

/* the remaining lines in this function stop the measurement and set the values before returning. */

	rusage_end_time = getUserTime();
	rdtsc_end_time = rdtsc();

	*rusage_time = rusage_end_time - rusage_start_time;
	*rdtsc_time = rdtsc_end_time - rdtsc_start_time;
}

 /* The parameters, pointers, rusage_time, rdtsc_time, and cpu_time_used are used to measure performance and return values to caller. */
 /* You should not change the code that uses these parameters and variables. */
char my_rotate_descr[] = "my_rotate: Naive baseline implementation";
void my_rotate(int dim, pixel *src, pixel *dst, int *rusage_time, unsigned long long *rdtsc_time) 
{
	int i, j;
		/* the variables below are used for performance measurement and not for computing the results of the algorithm */
	long int rusage_start_time, rusage_end_time = 0;
        unsigned long long rdtsc_start_time, rdtsc_end_time = 0;
	/* call system functions to start measuring performance. you should not bother to change these. */
        rusage_start_time = getUserTime();
	rdtsc_start_time = rdtsc();

/* ANY CHANGES ARE MADE HERE */
/* below are the main computations for your implementation of the rotate. Any changes in implementation will go here or the other functions it may call */
	int dim2, k;
	dim2 = dim * dim;
	dst += dim2 - dim; // go to the start of last row of dst
	for (j = 0; j < dim; j += 16) {
		for (i = 0; i < dim; i++) {
			for (k = 0; k < 15; k++) {
				dst[k] = *src;
				src += dim;
			}
			dst[k] = *src; // last one
			dst -= dim; // go up a row
			src -= dim*15 - 1; // go to start of next column
		}
		// go to start of next block
		dst += 16 + dim2;
		src += dim * 15;
	}


/* end of computation for rotate function. any changes you make should be made above this line. */
/* END OF CHANGES in this function */

/* the remaining lines in this function stop the measurement and set the values before returning. */
	rusage_end_time = getUserTime();
        rdtsc_end_time = rdtsc();

	*rusage_time = rusage_end_time - rusage_start_time;
	*rdtsc_time = rdtsc_end_time - rdtsc_start_time;
}





/***************
 * SMOOTH KERNEL
 **************/

/***************************************************************
 * Various typedefs and helper functions for the smooth function
 * You may modify these any way you like.
 **************************************************************/

/* A struct used to compute averaged pixel value */
typedef struct {
	int red;
	int green;
	int blue;
	int num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int minimum(int a, int b) 
{ return (a < b ? a : b); }
static int maximum(int a, int b) 
{ return (a > b ? a : b); }

/* 
 * initialize_pixel_sum - Initializes all fields of sum to 0 
 */
static void initialize_pixel_sum(pixel_sum *sum) 
{
	sum->red = sum->green = sum->blue = 0;
	sum->num = 0;
	return;
}

/* 
 * accumulate_sum - Accumulates field values of p in corresponding 
 * fields of sum 
 */
static void accumulate_sum(pixel_sum *sum, pixel p) 
{
	sum->red += (int) p.red;
	sum->green += (int) p.green;
	sum->blue += (int) p.blue;
	sum->num++;
	return;
}

/* 
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel 
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) 
{
	current_pixel->red = (unsigned short) (sum.red/sum.num);
	current_pixel->green = (unsigned short) (sum.green/sum.num);
	current_pixel->blue = (unsigned short) (sum.blue/sum.num);
	return;
}

/* 
 * avg - Returns averaged pixel value at (i,j) 
 */
static pixel avg(int dim, int i, int j, pixel *src) 
{
	int ii, jj;
	pixel_sum sum;
	pixel current_pixel;

	initialize_pixel_sum(&sum);
	for(ii = maximum(i-1, 0); ii <= minimum(i+1, dim-1); ii++) 
		for(jj = maximum(j-1, 0); jj <= minimum(j+1, dim-1); jj++) 
			accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

	assign_sum_to_pixel(&current_pixel, sum);
	return current_pixel;
}

/******************************************************
 * Your different versions of the smooth kernel go here
 ******************************************************/

/*
 * naive_smooth - The naive baseline version of smooth 
 */
  /* The parameters, pointers, rusage_time, rdtsc_time, and cpu_time_used are used to measure performance and return values to caller. */
 /* You should not change the code that uses these parameters and variables. */
 
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst, int *rusage_time, unsigned long long *rdtsc_time) 
{
	int i, j;
	
	/* the variables below are used for performance measurement and not for computing the results of the algorithm */
	long int rusage_start_time, rusage_end_time = 0;
        unsigned long long rdtsc_start_time, rdtsc_end_time = 0;

	/* call system functions to start measuring performance. you should not bother to change these. */
        rusage_start_time = getUserTime();
	rdtsc_start_time = rdtsc();

/* below are the main computations for the smooth function */
	for (j = 0; j < dim; j++)
		for (i = 0; i < dim; i++)
			dst[RIDX(i, j, dim)] = avg(dim, i, j, src);

/* the remaining lines in this function stop the measurement and set the values before returning. */
	rusage_end_time = getUserTime();
        rdtsc_end_time = rdtsc();

	*rusage_time = rusage_end_time - rusage_start_time;
	*rdtsc_time = rdtsc_end_time - rdtsc_start_time;
}

 /* The parameters, pointers, rusage_time, rdtsc_time, and cpu_time_used are used to measure performance and return values to caller. */
 /* You should not change the code that uses these parameters and variables. */
 
char my_smooth_descr[] = "my_smooth: Naive baseline implementation";
void my_smooth(int dim, pixel *src, pixel *dst, int *rusage_time, unsigned long long *rdtsc_time) 
{
	int i, j;
	
	/* the variables below are used for performance measurement and not for computing the results of the algorithm */
	long int rusage_start_time, rusage_end_time = 0;
        unsigned long long rdtsc_start_time, rdtsc_end_time = 0;
	/* call system functions to start measuring performance. you should not bother to change these. */
        rusage_start_time = getUserTime();
	rdtsc_start_time = rdtsc();
	
/* ANY CHANGES TO BE MADE SHOULD BE BELOW HERE */
/* below are the main computations for your implementation of the smooth function. Any changes in implementation will go here or the other functiosn it calls */

	int dim2, dimMinusOne, dimPlusOne, iMinusOne, iPlusOne, k, l, m, n, o, p, q, r, s;
	dim2 = dim * dim;
	dimMinusOne = dim - 1;
	dimPlusOne = dim + 1;
	pixel current_pixel;

	// top left corner
	current_pixel.red = (src[0].red + src[1].red + src[dim].red + src[dimPlusOne].red) >> 2;
	current_pixel.green = (src[0].green + src[1].green + src[dim].green + src[dimPlusOne].green) >> 2;
	current_pixel.blue = (src[0].blue + src[1].blue + src[dim].blue + src[dimPlusOne].blue) >> 2;
	dst[0] = current_pixel;

	// top side
	iMinusOne = -1;
	iPlusOne = 1;
	l = dim;
	k = dim - 1;
	j = dim + 1;
	for (i = 1; i < dimMinusOne; i++) {
		iMinusOne++;
		iPlusOne++;
		l++;
		k++;
		j++;
		current_pixel.red = (src[iMinusOne].red + src[i].red + src[iPlusOne].red + src[k].red + src[l].red + src[j].red) / 6;
		current_pixel.green = (src[iMinusOne].green + src[i].green + src[iPlusOne].green + src[k].green + src[l].green + src[j].green) / 6;
		current_pixel.blue = (src[iMinusOne].blue + src[i].blue + src[iPlusOne].blue + src[k].blue + src[l].blue + src[j].blue) / 6;
		dst[i] = current_pixel;
	}

	// top right corner
	j = dimMinusOne + dim;
	k = j - 1;
	iMinusOne++;
	current_pixel.red = (src[iMinusOne].red + src[dimMinusOne].red + src[k].red + src[j].red) >> 2;
	current_pixel.green = (src[iMinusOne].green + src[dimMinusOne].green + src[k].green + src[j].green) >> 2;
	current_pixel.blue = (src[iMinusOne].blue + src[dimMinusOne].blue + src[k].blue + src[j].blue) >> 2;
	dst[dimMinusOne] = current_pixel;

	// right side
	k = dim2 - 1;
	n = j - 1;
	l = n - dim;
	m = j - dim;
	p = j + dim;
	o = p - 1;
	for (i = j; i < k; i += dim) {
		current_pixel.red = (src[l].red + src[m].red + src[n].red + src[i].red + src[o].red + src[p].red) / 6;
		current_pixel.green = (src[l].green + src[m].green + src[n].green + src[i].green + src[o].green + src[p].green) / 6;
		current_pixel.blue = (src[l].blue + src[m].blue + src[n].blue + src[i].blue + src[o].blue + src[p].blue) / 6;
		dst[i] = current_pixel;
		l += dim;
		n += dim;
		m += dim;
		o += dim;
		p += dim;
	}

	// bottom right corner
	l = k - 1;
	m = k - dim;
	n = m - 1;
	current_pixel.red = (src[n].red + src[m].red + src[l].red + src[k].red) >> 2;
	current_pixel.green = (src[n].green + src[m].green + src[l].green + src[k].green) >> 2;
	current_pixel.blue = (src[n].blue + src[m].blue + src[l].blue + src[k].blue) >> 2;
	dst[k] = current_pixel;

	// bottom side, from right
	j = m + 1;
	m = l - dim;
	k = m - 1;
	n = m + 1;
	o = l - 1;
	p = l + 1;
	for (i = l; i > j; i--) {
		current_pixel.red = (src[k].red + src[m].red + src[n].red + src[o].red + src[i].red + src[p].red) / 6;
		current_pixel.green = (src[k].green + src[m].green + src[n].green + src[o].green + src[i].green + src[p].green) / 6;
		current_pixel.blue = (src[k].blue + src[m].blue + src[n].blue + src[o].blue + src[i].blue + src[p].blue) / 6;
		dst[i] = current_pixel;
		m--;
		k--;
		n--;
		o--;
		p--;
	}

	// bottom left corner
	j = i - dim;
	k = j + 1;
	l = i + 1;
	current_pixel.red = (src[j].red + src[k].red + src[i].red + src[l].red) >> 2;
	current_pixel.green = (src[j].green + src[k].green + src[i].green + src[l].green) >> 2;
	current_pixel.blue = (src[j].blue + src[k].blue + src[i].blue + src[l].blue) >> 2;
	dst[i] = current_pixel;

	// left side, from bottom
	k = j - dim;
	l = k + 1;
	m = j + 1;
	n = j + dim;
	o = n + 1;
	for (i = j; i > 0; i -= dim) {
		current_pixel.red = (src[k].red + src[l].red + src[i].red + src[m].red + src[n].red + src[o].red) / 6;
		current_pixel.green = (src[k].green + src[l].green + src[i].green + src[m].green + src[n].green + src[o].green) / 6;
		current_pixel.blue = (src[k].blue + src[l].blue + src[i].blue + src[m].blue + src[n].blue + src[o].blue) / 6;
		dst[i] = current_pixel;
		k -= dim;
		l -= dim;
		m -= dim;
		n -= dim;
		o -= dim;
	}

	// inside
	k = dim;
	l = k - 1;
	m = k + 1;
	n = l - dim;
	o = k - dim;
	p = o + 1;
	q = l + dim;
	r = q + 1;
	s = r + 1;
	for (i = 1; i < dimMinusOne; i++) {
		for (j = 1; j < dimMinusOne; j++) {
			k++;
			l++;
			m++;
			n++;
			o++;
			p++;
			q++;
			r++;
			s++;
			current_pixel.red = (src[l].red + src[k].red + src[m].red + src[n].red + src[o].red + src[p].red + src[q].red + src[r].red + src[s].red) / 9;
			current_pixel.green = (src[l].green + src[k].green + src[m].green + src[n].green + src[o].green + src[p].green + src[q].green + src[r].green + src[s].green) / 9;
			current_pixel.blue = (src[l].blue + src[k].blue + src[m].blue + src[n].blue + src[o].blue + src[p].blue + src[q].blue + src[r].blue + src[s].blue) / 9;

			dst[k] = current_pixel;
		}
		k += 2;
		l += 2;
		m += 2;
		n += 2;
		o += 2;
		p += 2;
		q += 2;
		r += 2;
		s += 2;
	}

	/* end of computation for smooth function. so don't change anything after this in this function. */
	/* END OF CHANGES */

	/* the remaining lines in this function stop the measurement and set the values before returning. */
	rusage_end_time = getUserTime();
        rdtsc_end_time = rdtsc();

	*rusage_time = rusage_end_time - rusage_start_time;
	*rdtsc_time = rdtsc_end_time - rdtsc_start_time;
}
