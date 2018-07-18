#include "stdafx.h"
#include "Filters.h"
#include <math.h>
#include <stdio.h>
#include "EcgDiagDefines.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//Lowpass filter
LowpassFilter::LowpassFilter(double Freq,int SampleRate)
{
	Init(Freq,SampleRate);
}

void LowpassFilter::Init(double Freq,int SampleRate)
{
	int i;
  double fts,c,c1[2],c2[2];
  fts=(double)SampleRate/Freq;
  c1[0]=0.5883;
  c2[0]=0.101309;
  c1[1]=0.243507;
  c2[1]=0.101268;
  for(i=0;i<2;i++) {
     c=1.+c1[i]*fts+c2[i]*fts*fts;
     a0[i]=1./c;
     a1[i]=2./c;
     a2[i]=a0[i];
     b1[i]=2.*(1.-c2[i]*fts*fts)/c;
     b2[i]=(1.-c1[i]*fts+c2[i]*fts*fts)/c;
  }
  x1=x2=0.;
  y1=y2=z1=z2=0.;
}

int LowpassFilter::Filter(int xt)
{
  double yt=0,zt=0;
  yt=a0[0]*xt+a1[0]*x1+a2[0]*x2-b1[0]*y1-b2[0]*y2;
  zt=a0[1]*yt+a1[1]*y1+a2[1]*y2-b1[1]*z1-b2[1]*z2;
  if(fabs(yt)<1.0e-100) yt=0.0;
  if(fabs(zt)<1.0e-100) zt=0.0;
  x2=x1;  x1=xt;
  y2=y1;  y1=yt;
  z2=z1;  z1=zt;
  return (int)zt;
}

//冒泡排序（整数）从小到大
void BubbleSort(int *data,int n)
{
	int i,temp;
	BOOL bubble;
	for(;;) {
		bubble=FALSE;
		for(i=0;i<n-1;i++) {
			if(data[i]>data[i+1]) {
				temp=data[i];
				data[i]=data[i+1];
				data[i+1]=temp;
				bubble=TRUE;
			}
		}
		if(!bubble) break;
	}
}

//冒泡排序（整数）从小到大 Array排序
void BubbleSort(short *data,short *SortArray,int n)
{
	int i,temp;
	char chtemp;
	BOOL bubble;
	for(;;) {
		bubble=FALSE;
		for(i=0;i<n-1;i++) {
			if(data[i]>data[i+1]) {
				temp=data[i];        chtemp=SortArray[i];
				data[i]=data[i+1];   SortArray[i]=SortArray[i+1];
				data[i+1]=temp;      SortArray[i+1]=chtemp;
				bubble=TRUE;
			}
		}
		if(!bubble) break;
	}
}///////////////////////////////////////////////////////////
//Highpass filter
HighpassFilter::HighpassFilter(double Freq, int SampleRate)
{
	if(Freq<=0.01) Freq=0.01;
	Init(Freq,SampleRate);
}

void HighpassFilter::Init(double Freq, int SampleRate)
{
  double a,c,fsfc,tscp;
  fsfc=SampleRate/Freq;
  c=0.5883003;
  tscp=fsfc/M_PI;
  a=1.0+c*fsfc+tscp*tscp;
  a0=tscp*tscp/a;
  a1=-2.0*a0;
  a2=a0;
  b1=(2.0-2.0*tscp*tscp)/a;
  b2=(1.0-c*fsfc+tscp*tscp)/a;
  x1=x2=y1=y2=0;
}

void HighpassFilter::InitialValue(int xt)
{
  x1=x2=y1=y2=xt;
}

void HighpassFilter::InitialValue(double xt)
{
  x1=x2=y1=y2=xt;
}

int HighpassFilter::Filter(int xt)
{
  double yt;
  yt=a0*xt+a1*x1+a2*x2-b1*y1-b2*y2;
if(fabs(yt)<1.0e-100) yt=0.;
  x2=x1;
  x1=xt;
  y2=y1;
  y1=yt;
  return int(yt);
}

double HighpassFilter::Filter(double xt)
{
  double yt;
  yt=a0*xt+a1*x1+a2*x2-b1*y1-b2*y2;
if(fabs(yt)<1.0e-100) yt=0.;
  x2=x1;
  x1=xt;
  y2=y1;
  y1=yt;
  return yt;
}

