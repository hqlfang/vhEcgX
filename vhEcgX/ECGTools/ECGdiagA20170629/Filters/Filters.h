#ifndef _FILTER_H
#define _FILTER_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class LowpassFilter {
	double a0[2],a1[2],a2[2],b1[2],b2[2];
	double x1,x2,y1,y2,z1,z2;
public:
	LowpassFilter(double Freq,int SampleRate);
	void Init(double Freq,int SampleRate);
	int Filter(int xt);
};

class HighpassFilter {
	int fs;
	double fc;
	double a0,a1,a2,b1,b2;
	double x1,x2,y1,y2;
public:
	HighpassFilter(double Freq,int SampleRate);
	void Init(double Freq,int SampleRate);
	void Init(double Freq)	{Init(Freq,fs);}
	void Init()	{Init(fc,fs);}
	void InitialValue(int xt);
	void InitialValue(double xt);
	int Filter(int xt);
	double Filter(double xt);
};

void BubbleSort(int *data,int n);
void BubbleSort(short *data,short *SortArray,int n);

#endif