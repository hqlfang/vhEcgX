#ifndef _CFILTER_H
#define _CFILTER_H


void ButterwirthFilter(int Freq,int SampleRate,double *data,int length,int maxp);
void PostProcess(int Channel,double *data[],int n,int NotchFreq,int SampleRate);
double NotchFreqDetect(double *data,int n,int NotchFreq,int SampleRate);
double NotchDiv(double f,double *data,int n,int SampleRate);


//2-order lowpass filter
class CLowpassFilter2 {
	double a1,a2,b0,b1,b2;
	double x1,x2,y1,y2;
	double fc;
	int fs;
public:
	CLowpassFilter2(double Freq,int SampleRate);
	void Init(double Freq,int SampleRate);
	void Init(double Freq);
	void Init();
	void InitialValue(int xt);
	void InitialValue(double xt);
	int  Filter(int xt);
	double Filter(double xt);
};

class CHighpassFilter {
	int fs;
	double fc;
	double a0,a1,a2,b1,b2;
	double x1,x2,y1,y2;
public:
	CHighpassFilter(double Freq,int SampleRate);
	void Init(double Freq,int SampleRate);
	void Init(double Freq)	{Init(Freq,fs);}
	void Init()	{Init(fc,fs);}
	void InitialValue(int xt);
	void InitialValue(double xt);
	int Filter(int xt);
	double Filter(double xt);
};

//2-order notch filter
class CNotchFilter {
	double Q,fc;
	int fs;
	double a1,a2,b0,b1,b2;
	double x1,x2,y1,y2;
public:
	CNotchFilter(double Freq,int SampleRate,double q=40);
	void Init(double Freq,int SampleRate,double q=40);
	void Init(double Freq) {Init(Freq,fs,Q);}
	void Init() {Init(fc,fs,Q);}
	int Filter(int xt);
	double Filter(double xt);
};

// Smooth
/*
class CSmoothFilter {
	int fs;
	int *m_pSpr, *m_pLowPass, *m_pNoise, *m_pDiffer;
	int Width,Pos,PhaseShift;
	int Count,CountRange[4];
	CLowpassFilter2 *LowPass;
public:
	CSmoothFilter(int SampleRate);
	~CSmoothFilter();
	void Init();
	int Smooth(int xt);
};
*/
class CSmoothFilter { //depends on power
	int *m_pSpr, *m_pLowPass, *m_pNoise;
	int Width,Pos,PhaseShift;
	int SamplingRate;
	int sw1,sw2,sw3,sw4,sw5,sw6,sw7,sw8,sw9;
	long Count;
	double TotalSum,WindowSum;
	CLowpassFilter2 *LowPass;
public:
	CSmoothFilter(int SampleRate);
	~CSmoothFilter();
	void Init();
	int Smooth(int xt);
};

//PaceMaker
class CPaceMaker {
	int fs,m_nLowPassFreq;
	BOOL m_bSmooth;

	int LowPassPhaseShift,SmoothPhaseShift,PhaseShift,ShiftCount;
	int ThresholdValue,ReserveValue,RestoreValue;
	int *m_Reserve,iRes,jRes;
	int SpikeWidth,WidthCount;
	BOOL PaceFound,Start,FirstTime;
	int MaxPace,MinPace,PaceCount;
	int *m_Source,*m_Differ,Width,Pos;
public:
	CPaceMaker(int LowPassFreq,BOOL Smooth,int SampleRate);
	~CPaceMaker();
	void Init();
	void Init(int LowPassFreq,BOOL Smooth);
	int Remove(int xt);
	int Recover(int xt);
	int GetSpikes();
    int Type();//0: no; 1: single; 2: double;
};

class CMultiChannelLowpass2 {  //for multi-channel
	double fc;
	int fs,ChlNum;
	CLowpassFilter2 **pass;
public:
	CMultiChannelLowpass2(double Freq,int SampleRate,int Channel);
	~CMultiChannelLowpass2();
	void Init(double Freq,int SampleRate);
	void Init(double Freq) {Init(Freq,fs);}
	void Init() {Init(fc,fs);}
	void Filter(short *data);
};

class CMultiChannelHighpass {  //for multi-channel
	double hfreq;
	int fs,ChlNum;
	CHighpassFilter **pass;
public:
	CMultiChannelHighpass(double Freq,int SampleRate,int Channel);
	~CMultiChannelHighpass();
	void Init(double Freq,int SampleRate);
	void Init(double Freq) {Init(Freq,fs);}
	void Init() {Init(hfreq,fs);}
	void Filter(short *data);
};

class CMultiChannelNotch {  //for multi-channel
	double Q,fc;
	int fs;
	int ChlNum;
	CNotchFilter **pass;
public:
	CMultiChannelNotch(double Freq,int SampleRate,int Channel,double q=40);
	~CMultiChannelNotch();
	void Init(double Freq,int SampleRate,double q=40);
	void Init(double Freq) {Init(Freq,fs,Q);}
	void Init() {Init(fc,fs,Q);}
	void Filter(short *data);
};

class CMultiChannelSmooth {
	int ChlNum;
	CSmoothFilter **smooth;
public:
	CMultiChannelSmooth(int SampleRate,int Channel);
	~CMultiChannelSmooth();
	void Init();
	void Smooth(short *data);
};

class CMultiChannelPaceMaker {
	int ChlNum;
	CPaceMaker **pace;
public:
	CMultiChannelPaceMaker(int LowPassFreq,BOOL Smooth,int SampleRate,int Channel);
	~CMultiChannelPaceMaker();
	void Init();
	void Init(int LowPassFreq,BOOL Smooth);
	void Remove(short *data);
	void Recover(short *data);
	void GetSpikes(short *Spikes);
	void Type(short *types);
};

class CLengthTransform {//输入的数据应不含工频干扰
	int wms,fs,w;
	int p,Li,pre[3];
	int	*L;
public:
	CLengthTransform(int SampleRate,int ms=65);
	~CLengthTransform();
	void Init(int SampleRate,int ms=65);
	void Init();
	int  Length(int xt);
	int  Length(int xt,int yt,int zt);
};

class CrtRRdetect {//采用CLengthTransform变换后的数据检测R波
	int fs,notch;
	int sec2,sec3,ms240;
	int count,rr;
	int minp,maxp,er;
	int	lw,w,q;//,*win;
	int pre,repeat;
	int status;
	long doing;
//	bool start,find,first;

	CLengthTransform *xyzLength;
	CLowpassFilter2 *xLow,*yLow,*zLow;
	CHighpassFilter *xHigh,*yHigh,*zHigh;
//	Difference	*xDiff,*yDiff,*zDiff;
	CNotchFilter	*xNotch,*yNotch,*zNotch;

	int RRdetect(int xt);	//返回RR间期ms

public:
	CrtRRdetect(int SampleRate,int freqNotch=50);
	~CrtRRdetect();
	void Init();
	void Init(int freqNotch) {notch=freqNotch;Init();}
	int QRSdetect(int xt);	//返回RR间期采样点数
	int QRSdetect(int xt,int yt,int zt);	//返回RR间期采样点数
};
/*
class RealtimeBaselineDrift {
	int  b, n, Rtimes;//,rt_hr;
	int *x,*hr;
	int p, q, p0, p1, p2, p3, p4;
	double y0, y1, y2, y3;
	double y, y1t, y2t, y3t;
	void calculatedrift();
public:
	RealtimeBaselineDrift(int SampleRate,int return_hr);
	~RealtimeBaselineDrift();
	void Init(int SampleRate,int return_hr);
	void Delete();
	void InitDrift();
	int  drift(int xt,int *isR);
};

//New, 2006-12-25
class RealtimeBaselineDrift {
	int  b, n;// Rtimes;//,rt_hr;
	int *x,*hr;
	int p, q, p0, p1, p2, p3, p4;
//	int Count;
	BOOL Ready;
	double y0, y1, y2, y3, y4;
	double y, y1t, y2t, y3t;
	void calculatedrift();
public:
	RealtimeBaselineDrift(int SampleRate,int return_hr);
	~RealtimeBaselineDrift();
	void Init(int SampleRate,int return_hr);
	void Delete();
	void InitDrift();
	int  drift(int xt,int *isR);
	BOOL  ReplayAvailable();
};
*/

////////////////////////////////////// 2013-5-15恢复使用  //2015-1-21 最后修改
class RealtimeBaselineDrift {
	int prPos, n, Rtimes;//,rt_hr;
	int rrCount,rrRange;
	int sRate;
	int *x,*hr;
	int *avrgX, aP;
	double averageV;
	int p, q, p0, p1, p2, p3, p4;  
	double y0, y1, y2, y3, y4;
	double y, y1t, y2t, y3t; 
	void calculatedrift();
//	int midValue(int pt);
	int prPosition();
public:
	RealtimeBaselineDrift(int SampleRate,int return_hr);
	~RealtimeBaselineDrift();
	void Init(int SampleRate,int return_hr);
	void Delete();
	void InitDrift();
	int  drift(int xt,int *isR,BOOL BL);
	bool  ReplayAvailable();
};

class MultiChannelBaseline {  //for multi-channel
	int ChlNum,fs;
	RealtimeBaselineDrift **baseline;
public:
	MultiChannelBaseline(int SampleRate,int Channel);
	~MultiChannelBaseline();
	void Init(int SampleRate,int Channel);
	void Init() {Init(fs,ChlNum);}
	int  Drift(short *data,int isR,BOOL BL);
	BOOL  ReplayAvailable();
};


#endif
