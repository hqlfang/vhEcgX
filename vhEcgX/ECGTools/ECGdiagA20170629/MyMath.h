#ifndef  __MYMATH__H__
#define  __MYMATH__H__

#include <math.h>

class CMax
{
	long index;
public:
	CMax(void);
	~CMax(void);
public:
	short maximum(short *x,long length);
	long maximum(long *x,long length);
	float maximum(float *x,long length);
	double maximum(double *x,long length);
	long GetIndex() {return index;}
};

class CMin
{
	long index;
public:
	CMin(void);
	~CMin(void);
public:
	short minimum(short *x,long length);
	long minimum(long *x,long length);
	float minimum(float *x,long length);
	double minimum(double *x,long length);
	long GetIndex() {return index;}
};

class CAverage
{
public:
	CAverage(void);
	~CAverage(void);
public:
	short Average(short *x,long length);
	long Average(long *x,long length);
	float Average(float *x,long length);
	double Average(double *x,long length);
};


class CRootMeanAquare : public CAverage
{
public:
	CRootMeanAquare(void);
	~CRootMeanAquare(void);
public:
	double MeanAquare(short *x,long length);
	double MeanAquare(long *x,long length);
	double MeanAquare(float *x,long length);
	double MeanAquare(double *x,long length);
public:
	double RootMeanAquare(short *x,long length)		{return sqrt(MeanAquare(x,length));}
	double RootMeanAquare(long *x,long length)		{return sqrt(MeanAquare(x,length));}
	double RootMeanAquare(float *x,long length)		{return sqrt(MeanAquare(x,length));}
	double RootMeanAquare(double *x,long length)	{return sqrt(MeanAquare(x,length));}
};

class CLinearCorrelation : public CRootMeanAquare
{
public:
	CLinearCorrelation();
	~CLinearCorrelation();
public:
	double LinearCorrelation(short *x,short *y,long length);
	double LinearCorrelation(long *x,long *y,long length);
	double LinearCorrelation(float *x,float *y,long length);
	double LinearCorrelation(double *x,double *y,long length);
};

class CMaxLinearCorrelation : public CLinearCorrelation,public CMax,public CMin
{
	long	m_offset_range,m_offset;
public:
	CMaxLinearCorrelation(long offset_range=0);
	~CMaxLinearCorrelation();
public:
	double MaxLinearCorrelation(short *x,short *y,long length);
	double MaxLinearCorrelation(long *x,long *y,long length);
	double MaxLinearCorrelation(float *x,float *y,long length);
	double MaxLinearCorrelation(double *x,double *y,long length);
};

#endif
