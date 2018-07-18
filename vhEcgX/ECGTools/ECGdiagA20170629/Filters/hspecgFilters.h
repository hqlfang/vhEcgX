#pragma once
#include "stdafx.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////////
class CBaseNotchFilter {//同时具有高通和陷波的滤波器，要求采样频率不高于500/600Hz且为50/60整倍数。带宽1.2Hz,高通0.6Hz,delay 
	int m_f;
	int m_fs;
	int	L,M,K;
	int d1,d2,d3;
	int *x,*y;
	int xi,yi;
public:
	CBaseNotchFilter();
	CBaseNotchFilter(int NotFreq,int SampleRate);
	virtual ~CBaseNotchFilter();
	void Init(int Freq=0,int SampleRate=0);
	int Filter(int xt);
};

class CBlNotchFilter {//同时具有高通和陷波的滤波器，要求采样频率为陷波频率的整数倍。带宽1.2Hz,高通0.6Hz,delay 
	int m_f;
	int m_fs;
	int m_n;
	int m_index;
	CBaseNotchFilter **m_filters;
	void Destroy();
public:
	CBlNotchFilter();
	CBlNotchFilter(int NotFreq,int SampleRate);
	virtual ~CBlNotchFilter();
	void Init(int Freq=0,int SampleRate=0);
	int Filter(int xt);
};

class CMutiBlNotchFilter{
	int m_chnum;
	CBlNotchFilter **blac;
public:
	CMutiBlNotchFilter(int Freq,int SampleRate,int ChlNumber);
	virtual ~CMutiBlNotchFilter();
	void Init(int Freq=0,int SampleRate=0);
	void Filter(int *xt);
	void Filter(short *xt);
};

//////////////////////////////////////////////////////////////////////////
//实时任意整频率线性插值，要求，原始频率<插值后频率。
class FreqInterpretate {
	int m_fsrc,m_fdst;		//原始频率，插值后频率
	int m_n,m_maxn;		//实际插值点数，最大插值点数
	double m_y0;
	double m_t0;
	double m_dt;	//比例系数
	short *m_y;
public:
	FreqInterpretate();
	FreqInterpretate(int fsrc,int fdst);
	virtual ~FreqInterpretate();
	void Init(int fsrc=0,int fdst=0);
    short CalculateReturnValues(short CurrentValue);	//返回插值后个数
	short *GetData();//获取插值后数据
};

class CHspecgNotchFilter {
	int m_f;
	int m_fs;
	bool m_bFint;
	CBlNotchFilter m_blac;
	FreqInterpretate m_fint;
public:
	CHspecgNotchFilter();
	CHspecgNotchFilter(int NotFreq,int SampleRate);
	virtual ~CHspecgNotchFilter();
	void Init(int Freq=0,int SampleRate=0);
	int Filter(int xt);
};


class CHspecgMultiNotchFilter {
	int m_chnum;
	CHspecgNotchFilter **m_pNotchFilter;
public:
	CHspecgMultiNotchFilter();
	CHspecgMultiNotchFilter(int NotFreq,int SampleRate,int ChlNum);
	virtual ~CHspecgMultiNotchFilter();
	void Init(int Freq=0,int SampleRate=0);
	void Filter(int *xt);
	void Filter(short *xt);
};
