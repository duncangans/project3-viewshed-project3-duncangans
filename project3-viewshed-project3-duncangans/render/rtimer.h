#ifndef RTIMER_H
#define RTIMER_H

#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

typedef struct {
  struct rusage rut1, rut2;
  struct timeval tv1, tv2;
} Rtimer;

#define rt_start(rt)	                                \
  if((getrusage(RUSAGE_SELF, &rt.rut1) < 0)		\
	 || (gettimeofday(&(rt.tv1), NULL) < 0)) {	\
	perror("rusage/gettimeofday");	                \
	exit(1);					\
  }


/* doesn't really stop, just updates endtimes */
#define rt_stop(rt)					\
  if((getrusage(RUSAGE_SELF, &rt.rut2) < 0)		\
	 || (gettimeofday(&(rt.tv2), NULL) < 0)) {	\
        perror("rusage/gettimeofday");			\
        exit(1);					\
  }

/* not required to be called, but makes values print as 0. 
   obviously a hack */
#define rt_zero(rt) bzero(&(rt),sizeof(Rtimer));
	

#define rt_u_useconds(rt)				        \
	(((double)rt.rut2.ru_utime.tv_usec +			\
	  (double)rt.rut2.ru_utime.tv_sec*1000000)		\
	 - ((double)rt.rut1.ru_utime.tv_usec +			\
		(double)rt.rut1.ru_utime.tv_sec*1000000))

#define rt_s_useconds(rt)					\
	 (((double)rt.rut2.ru_stime.tv_usec +			\
	   (double)rt.rut2.ru_stime.tv_sec*1000000)		\
	  - ((double)rt.rut1.ru_stime.tv_usec +			\
		 (double)rt.rut1.ru_stime.tv_sec*1000000))

#define rt_w_useconds(rt)				\
	 (((double)rt.tv2.tv_usec +			\
	   (double)rt.tv2.tv_sec*1000000)		\
	  - ((double)rt.tv1.tv_usec +			\
		 (double)rt.tv1.tv_sec*1000000))

#define rt_seconds(rt) (rt_w_useconds(rt)/1000000)

#define rt_sprint(buf, rt) rt_sprint_safe(buf,rt)

char * rt_sprint_safe(char *buf, Rtimer rt);

#endif /* RTIMER_H */
