#include "Stdafx.h"
#include "MyMath.h"


CMax::CMax(void)
{
	index=0;
}

CMax::~CMax(void)
{
}

short CMax::maximum(short *x,long length)
{
	index=0;
	for(long i=0;i<length;i++) {
		if(x[i]>x[index]) index=i;
	}
	return x[index];
}

long CMax::maximum(long *x,long length)
{
	index=0;
	for(long i=0;i<length;i++) {
		if(x[i]>x[index]) index=i;
	}
	return x[index];
}

float CMax::maximum(float *x,long length)
{
	index=0;
	for(long i=0;i<length;i++) {
		if(x[i]>x[index]) index=i;
	}
	return x[index];
}

double CMax::maximum(double *x,long length)
{
	index=0;
	for(long i=0;i<length;i++) {
		if(x[i]>x[index]) index=i;
	}
	return x[index];
}
///////////////////////////////////////////////////////////////

CMin::CMin(void)
{
	index=0;
}

CMin::~CMin(void)
{
}

short CMin::minimum(short *x,long length)
{
	index=0;
	for(long i=0;i<length;i++) {
		if(x[i]<x[index]) index=i;
	}
	return x[index];
}

long CMin::minimum(long *x,long length)
{
	index=0;
	for(long i=0;i<length;i++) {
		if(x[i]<x[index]) index=i;
	}
	return x[index];
}

float CMin::minimum(float *x,long length)
{
	index=0;
	for(long i=0;i<length;i++) {
		if(x[i]<x[index]) index=i;
	}
	return x[index];
}

double CMin::minimum(double *x,long length)
{
	index=0;
	for(long i=0;i<length;i++) {
		if(x[i]<x[index]) index=i;
	}
	return x[index];
}


///////////////////////////////////////////////////////////
CAverage::CAverage(void)
{
}

CAverage::~CAverage(void)
{
}

short CAverage::Average(short *x,long length)
{
	long ave=0;
	for(long i=0;i<length;i++) ave+=x[i];
	return short(ave/length);
}

long CAverage::Average(long *x,long length)
{
	long ave=0;
	for(long i=0;i<length;i++) ave+=x[i];
	return ave/length;
}

float CAverage::Average(float *x,long length)
{
	double ave=0;
	for(long i=0;i<length;i++) ave+=x[i];
	return float(ave/length);
}

double CAverage::Average(double *x,long length)
{
	double ave=0;
	for(long i=0;i<length;i++) ave+=x[i];
	return ave/length;
}
///////////////////////////////////////////////////
CRootMeanAquare::CRootMeanAquare(void) : CAverage()
{
}
CRootMeanAquare::~CRootMeanAquare(void)
{
}

double CRootMeanAquare::MeanAquare(short *x,long length)
{
	double s=0,z=Average(x,length);
	for(long i=0;i<length;i++) s+=((x[i]-z)*(x[i]-z));	
	return s/=length;
}
double CRootMeanAquare::MeanAquare(long *x,long length)
{
	double s=0,z=Average(x,length);
	for(long i=0;i<length;i++) s+=((x[i]-z)*(x[i]-z));	
	return s/=length;
}
double CRootMeanAquare::MeanAquare(float *x,long length)
{
	double s=0,z=Average(x,length);
	for(long i=0;i<length;i++) s+=((x[i]-z)*(x[i]-z));	
	return s/=length;
}
double CRootMeanAquare::MeanAquare(double *x,long length)
{
	double s=0,z=Average(x,length);
	for(long i=0;i<length;i++) s+=((x[i]-z)*(x[i]-z));	
	return s/=length;
}

//////////////////////////////////////////////////
CLinearCorrelation::CLinearCorrelation() : CRootMeanAquare()
{
}
CLinearCorrelation::~CLinearCorrelation()
{
}

double CLinearCorrelation::LinearCorrelation(short *x,short *y,long length)
{
	double zx=Average(x,length);
	double zy=Average(y,length);
	double sxx=0,syy=0,sxy=0;
	double xt,yt;
	for(long i=0;i<length;i++) {
		xt=x[i]-zx;	yt=y[i]-zy;
		sxx+=(xt*xt);	syy+=(yt*yt);	sxy+=(xt*yt);
	}
	if(sxx>0 && syy>0) return sxy*sxy/(sxx*syy);
	else return 0;
}
double CLinearCorrelation::LinearCorrelation(long *x,long *y,long length)
{
	double zx=Average(x,length);
	double zy=Average(y,length);
	double sxx=0,syy=0,sxy=0;
	double xt,yt;
	for(long i=0;i<length;i++) {
		xt=x[i]-zx;	yt=y[i]-zy;
		sxx+=(xt*xt);	syy+=(yt*yt);	sxy+=(xt*yt);
	}
	if(sxx>0 && syy>0) return sxy*sxy/(sxx*syy);
	else return 0;
}
double CLinearCorrelation::LinearCorrelation(float *x,float *y,long length)
{
	double zx=Average(x,length);
	double zy=Average(y,length);
	double sxx=0,syy=0,sxy=0;
	double xt,yt;
	for(long i=0;i<length;i++) {
		xt=x[i]-zx;	yt=y[i]-zy;
		sxx+=(xt*xt);	syy+=(yt*yt);	sxy+=(xt*yt);
	}
	if(sxx>0 && syy>0) return sxy*sxy/(sxx*syy);
	else return 0;
}
double CLinearCorrelation::LinearCorrelation(double *x,double *y,long length)
{
	double zx=Average(x,length);
	double zy=Average(y,length);
	double sxx=0,syy=0,sxy=0;
	double xt,yt;
	for(long i=0;i<length;i++) {
		xt=x[i]-zx;	yt=y[i]-zy;
		sxx+=(xt*xt);	syy+=(yt*yt);	sxy+=(xt*yt);
	}
	if(sxx>0 && syy>0) return sxy*sxy/(sxx*syy);
	else return 0;
}

//////////////////////////////////////////////////
CMaxLinearCorrelation::CMaxLinearCorrelation(long offset_range) : CLinearCorrelation()
{
	m_offset_range=offset_range;	m_offset=0;
}
CMaxLinearCorrelation::~CMaxLinearCorrelation()
{
}

double CMaxLinearCorrelation::MaxLinearCorrelation(short *x,short *y,long length)
{
/*	m_offset=0;
//	if(m_offset_range>0) {
	if(m_offset_range<=0) m_offset_range=length/2;
		if(length<=m_offset_range) return 0;

		long i,j;
		long start=m_offset_range,x0=start/2;
		long len=length-m_offset_range;
		double r=0,rmax=0;
		for(i=0;i<start;i++) {
			r=0;
			for(j=0;j<len;j++) {
				r+=(x[j+x0]*y[j+i]);
			}
			if(r>rmax) {rmax=r;m_offset=i;}
		}
//		m_offset-=x0;
		return LinearCorrelation(x+x0,y+m_offset,len);
//	}
//	else return LinearCorrelation(x,y,length);
*/
	m_offset=0;
	if(m_offset_range<=0) m_offset_range=length*2/5;
//		if(length<=m_offset_range) return 0;

		long start=m_offset_range;
		long len=length-m_offset_range;
		long i,j=start/2;
		double r=0,rmax=0;
		for(i=0;i<start;i++) {
			r=LinearCorrelation(x+j,y+i,len);
			if(rmax<r) {rmax=r;m_offset=i;}
		}
		m_offset-=j;
		return rmax;
//	}
//	else return LinearCorrelation(x,y,length);*/
}
double CMaxLinearCorrelation::MaxLinearCorrelation(long *x,long *y,long length)
{
	m_offset=0;
	if(m_offset_range<=0) m_offset_range=length*2/5;
//		if(length<=m_offset_range) return 0;

		long start=m_offset_range;
		long len=length-m_offset_range;
		long i,j=start/2;
		double r=0,rmax=0;
		for(i=0;i<start;i++) {
			r=LinearCorrelation(x+j,y+i,len);
			if(rmax<r) {rmax=r;m_offset=i;}
		}
		m_offset-=j;
		return rmax;
//	}
//	else return LinearCorrelation(x,y,length);
}
double CMaxLinearCorrelation::MaxLinearCorrelation(float *x,float *y,long length)
{
	m_offset=0;
	if(m_offset_range<=0) m_offset_range=length*2/5;
//		if(length<=m_offset_range) return 0;

		long start=m_offset_range;
		long len=length-m_offset_range;
		long i,j=start/2;
		double r=0,rmax=0;
		for(i=0;i<start;i++) {
			r=LinearCorrelation(x+j,y+i,len);
			if(rmax<r) {rmax=r;m_offset=i;}
		}
		m_offset-=j;
		return rmax;
//	}
//	else return LinearCorrelation(x,y,length);
}
double CMaxLinearCorrelation::MaxLinearCorrelation(double *x,double *y,long length)
{
	m_offset=0;
	if(m_offset_range<=0) m_offset_range=length*2/5;
//		if(length<=m_offset_range) return 0;

		long start=m_offset_range;
		long len=length-m_offset_range;
		long i,j=start/2;
		double r=0,rmax=0;
		for(i=0;i<start;i++) {
			r=LinearCorrelation(x+j,y+i,len);
			if(rmax<r) {rmax=r;m_offset=i;}
		}
		m_offset-=j;
		return rmax;
//	}
//	else return LinearCorrelation(x,y,length);
}
