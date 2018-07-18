#pragma once
#include "stdafx.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////////
class CBaseNotchFilter {//ͬʱ���и�ͨ���ݲ����˲�����Ҫ�����Ƶ�ʲ�����500/600Hz��Ϊ50/60������������1.2Hz,��ͨ0.6Hz,delay 
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

class CBlNotchFilter {//ͬʱ���и�ͨ���ݲ����˲�����Ҫ�����Ƶ��Ϊ�ݲ�Ƶ�ʵ�������������1.2Hz,��ͨ0.6Hz,delay 
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
//ʵʱ������Ƶ�����Բ�ֵ��Ҫ��ԭʼƵ��<��ֵ��Ƶ�ʡ�
class FreqInterpretate {
	int m_fsrc,m_fdst;		//ԭʼƵ�ʣ���ֵ��Ƶ��
	int m_n,m_maxn;		//ʵ�ʲ�ֵ����������ֵ����
	double m_y0;
	double m_t0;
	double m_dt;	//����ϵ��
	short *m_y;
public:
	FreqInterpretate();
	FreqInterpretate(int fsrc,int fdst);
	virtual ~FreqInterpretate();
	void Init(int fsrc=0,int fdst=0);
    short CalculateReturnValues(short CurrentValue);	//���ز�ֵ�����
	short *GetData();//��ȡ��ֵ������
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
