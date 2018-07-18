#include "stdafx.h"
#include "CFilters.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RR_SHIFTWINDOW 30

/////////////////////////////////////////////////////
//2阶低通
CLowpassFilter2::CLowpassFilter2(double Freq,int SampleRate)
{
	Init(Freq,SampleRate);
}
void CLowpassFilter2::Init(double Freq,int SampleRate)
{
	fc=Freq;	fs=SampleRate;
	double c=1.0/tan(M_PI*Freq/SampleRate);
	double n0=1,n1=2,n2=1;
	double cc=c*c,s2=sqrt(2.0);
	double d0=cc+s2*c+1;
	double d1=-2*(cc-1);
	double d2=cc-s2*c+1;
	b0=n0/d0;	b1=n1/d0;	b2=n2/d0;	a1=d1/d0;	a2=d2/d0;
	x1=x2=y1=y2=0;
}
void CLowpassFilter2::Init(double Freq)
{
	Init(Freq,fs);
}
void CLowpassFilter2::Init()
{
	Init(fc,fs);
}
void CLowpassFilter2::InitialValue(int xt)
{
  x1=x2=y1=y2=xt;
}

void CLowpassFilter2::InitialValue(double xt)
{
  x1=x2=y1=y2=xt;
}

int CLowpassFilter2::Filter(int xt)
{
	double yt=b0*xt+b1*x1+b2*x2-a1*y1-a2*y2;
	x2=x1;	x1=xt;	y2=y1;	y1=yt;
	return int(yt);
}
double CLowpassFilter2::Filter(double xt)
{
	double yt=b0*xt+b1*x1+b2*x2-a1*y1-a2*y2;
	x2=x1;	x1=xt;	y2=y1;	y1=yt;
	return yt;
}

///////////////////////////////////////////////////////////
//Highpass filter
CHighpassFilter::CHighpassFilter(double Freq,int SampleRate)
{
	if(Freq<=0.01) Freq=0.01;
	Init(Freq,SampleRate);
}

void CHighpassFilter::Init(double Freq,int SampleRate)
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

void CHighpassFilter::InitialValue(int xt)
{
  x1=x2=y1=y2=xt;
}

void CHighpassFilter::InitialValue(double xt)
{
  x1=x2=y1=y2=xt;
}

int CHighpassFilter::Filter(int xt)
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

double CHighpassFilter::Filter(double xt)
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

/////////////////////////////////////////////////////////////////
//2-order notch filter
CNotchFilter::CNotchFilter(double Freq,int SampleRate,double q)
{
	Init(Freq,SampleRate,q);
}
void CNotchFilter::Init(double Freq,int SampleRate,double q)
{
	Q=q;	fc=Freq;	fs=SampleRate;
	double c=1.0/tan(M_PI*Freq/SampleRate);
	double n0=q*(c*c+1);
	double n1=-2*q*(c*c-1);
	double n2=n0;
	double d0=n0+c;
	double d1=n1;
	double d2=n0-c;
	b0=n0/d0;	b1=n1/d0;	b2=n2/d0;	a1=d1/d0;	a2=d2/d0;
	x1=x2=y1=y2=0;
}
int CNotchFilter::Filter(int xt)
{
	double yt=b0*xt+b1*x1+b2*x2-a1*y1-a2*y2;
	x2=x1;	x1=xt;	y2=y1;	y1=yt;
	return int(yt);
}
double CNotchFilter::Filter(double xt)
{
	double yt=b0*xt+b1*x1+b2*x2-a1*y1-a2*y2;
	x2=x1;	x1=xt;	y2=y1;	y1=yt;
	return yt;
}

/////////////////////////////////////////////////////////
//Smooth Filter
/*
CSmoothFilter::CSmoothFilter(int SampleRate)
{
	fs=SampleRate;
	Width=50*SampleRate/1000;
	m_pSpr=new int[Width];
	m_pLowPass=new int[Width];
	m_pNoise=new int[Width];
	m_pDiffer=new int[Width];
	CountRange[0]=4*SampleRate/1000;  
	CountRange[0]=5*SampleRate/1000;
	CountRange[0]=7*SampleRate/1000;
	CountRange[0]=9*SampleRate/1000;
	Pos=0;
	PhaseShift=11*SampleRate/1000;
	LowPass=new CLowpassFilter2(20,SampleRate);
	for(int i=0;i<Width;i++) {
		m_pSpr[i]=0;
		m_pLowPass[i]=0;
		m_pNoise[i]=0;
		m_pDiffer[i]=1;
	}
	Count=2;
}
CSmoothFilter::~CSmoothFilter()
{
	delete []m_pSpr;
	delete []m_pLowPass;
	delete []m_pNoise;
	delete []m_pDiffer;
	delete LowPass;
}
void CSmoothFilter::Init()
{
	CountRange[0]=4*fs/1000;  
	CountRange[0]=5*fs/1000;
	CountRange[0]=7*fs/1000;
	CountRange[0]=9*fs/1000;
	Pos=0;
	PhaseShift=11*fs/1000;
	for(int i=0;i<Width;i++) {
		m_pSpr[i]=0;
		m_pLowPass[i]=0;
		m_pNoise[i]=0;
		m_pDiffer[i]=1;
	}
	Count=2;
}
int CSmoothFilter::Smooth(int xt)
{
	int i,sw,sum,CrectValue;
	m_pSpr[Pos]=xt;
	m_pLowPass[Pos]=LowPass->Filter(xt);
	m_pNoise[Pos]=m_pSpr[(Pos-PhaseShift+Width)%Width]-m_pLowPass[Pos];
	m_pDiffer[Pos]=m_pNoise[Pos]-m_pNoise[(Pos-1+Width)%Width];
	if(m_pDiffer[Pos]*m_pDiffer[(Pos-1+Width)%Width]<=0) Count++;
	if(Count<3) sw=1;
	else if(Count<5) sw=3;
	else if(Count<CountRange[0]) sw=Width/11;
	else if(Count<CountRange[1]) sw=Width/9;
	else if(Count<CountRange[2]) sw=Width/7;
	else if(Count<CountRange[3]) sw=Width/5;
	else sw=Width/4;
	if(sw==1) CrectValue=m_pNoise[(Pos-Width/2+Width)%Width];
	else if(sw==3) {
		CrectValue=(2*m_pNoise[(Pos-Width/2+Width)%Width]
			+m_pNoise[(Pos-Width/2-1+Width)%Width]
			+m_pNoise[(Pos-Width/2+1+Width)%Width])/4;
	}
	else {
		sum=0;
		for(i=0;i<sw;i++) {
			sum+=m_pNoise[(Pos-Width/2-sw/2+i+Width)%Width];
		}
		CrectValue=sum/sw;
	}
	CrectValue+=m_pLowPass[(Pos-Width/2-1+Width)%Width];
	Pos=(Pos+1)%Width;
	if(Count>0&&m_pDiffer[Pos]*m_pDiffer[(Pos+1)%Width]<=0) Count--;
	return CrectValue;
}
*/
CSmoothFilter::CSmoothFilter(int SampleRate)  
{
	SamplingRate=SampleRate;
	Width=30*SamplingRate/1000;
	m_pSpr=new int[Width];
	m_pLowPass=new int[Width];
	m_pNoise=new int[Width];
	LowPass=new CLowpassFilter2(25,SampleRate);
	Init();
}
void CSmoothFilter::Init()  
{
	Pos=0;
	PhaseShift=9*SamplingRate/1000;
	for(int i=0;i<Width;i++) {
		m_pSpr[i]=0;
		m_pLowPass[i]=0;
		m_pNoise[i]=0;
	}
	Count=0;
	TotalSum=0;
	WindowSum=0;
	sw1=1*SamplingRate/1000;
	sw2=2*SamplingRate/1000;
	sw3=3*SamplingRate/1000;
	sw4=4*SamplingRate/1000;
	sw5=5*SamplingRate/1000;
	sw6=6*SamplingRate/1000;
	sw7=7*SamplingRate/1000;
	sw8=8*SamplingRate/1000;
	sw9=9*SamplingRate/1000;
}
CSmoothFilter::~CSmoothFilter()
{
	delete []m_pSpr;
	delete []m_pLowPass;
	delete []m_pNoise;
	delete LowPass;
}
int CSmoothFilter::Smooth(int xt)
{
	int i,sw,CrectValue;
	m_pSpr[Pos]=xt;
	m_pLowPass[Pos]=LowPass->Filter(xt);
	CrectValue=m_pNoise[Pos];
	m_pNoise[Pos]=m_pSpr[(Pos-PhaseShift+Width)%Width]-m_pLowPass[Pos];
	Count++;
	TotalSum+=abs(m_pNoise[Pos]);
	WindowSum+=(abs(m_pNoise[Pos])-abs(CrectValue));
	if(WindowSum>4.0*Width*TotalSum/Count) sw=0;
	else if(WindowSum>3.5*Width*TotalSum/Count) sw=sw1;
	else if(WindowSum>3.0*Width*TotalSum/Count) sw=sw2;
	else if(WindowSum>2.6*Width*TotalSum/Count) sw=sw3;
	else if(WindowSum>2.2*Width*TotalSum/Count) sw=sw4;
	else if(WindowSum>1.8*Width*TotalSum/Count) sw=sw5;
	else if(WindowSum>1.5*Width*TotalSum/Count) sw=sw6;
	else if(WindowSum>1.2*Width*TotalSum/Count) sw=sw7;
	else if(WindowSum>1.0*Width*TotalSum/Count) sw=sw8;
	else sw=sw9;
	if(sw==0) {
		CrectValue=(2*m_pNoise[(Pos+Width/2)%Width]+m_pNoise[(Pos+Width/2-1)%Width]+m_pNoise[(Pos+Width/2+1)%Width])/4;
	}
	else {
		CrectValue=m_pNoise[(Pos+Width/2+sw)%Width];
		for(i=-sw;i<sw;i++) {
			CrectValue+=m_pNoise[(Pos+Width/2+i)%Width];
		}
		CrectValue/=(sw*2+1);
	}
	CrectValue+=m_pLowPass[(Pos+Width/2)%Width];
	Pos=(Pos+1)%Width;
	return CrectValue;
}

//PaceMaker
CPaceMaker::CPaceMaker(int LowPassFreq,BOOL Smooth,int SampleRate)
{
	fs=SampleRate;
	m_nLowPassFreq=LowPassFreq;
	m_bSmooth=Smooth;

	SpikeWidth=20*SampleRate/1000;
	WidthCount=SpikeWidth;
	PaceFound=0;
	ThresholdValue=750;             //not make sure of uV
	if(Smooth) SmoothPhaseShift=24*SampleRate/1000;
	else SmoothPhaseShift=0;
	if(LowPassFreq==0) LowPassPhaseShift=0;
	else if(LowPassFreq<30) LowPassPhaseShift=10*SampleRate/1000;
	else if(LowPassFreq<40) LowPassPhaseShift=8*SampleRate/1000;
	else if(LowPassFreq<50) LowPassPhaseShift=7*SampleRate/1000;
	else if(LowPassFreq<60) LowPassPhaseShift=6*SampleRate/1000;
	else if(LowPassFreq<70) LowPassPhaseShift=5*SampleRate/1000;
	else if(LowPassFreq<80) LowPassPhaseShift=4*SampleRate/1000;
	else if(LowPassFreq<100) LowPassPhaseShift=3*SampleRate/1000;
	else if(LowPassFreq<120) LowPassPhaseShift=2*SampleRate/1000;
	else if(LowPassFreq<=150) LowPassPhaseShift=1*SampleRate/1000;
	else LowPassPhaseShift=0;
	PhaseShift=SmoothPhaseShift+LowPassPhaseShift+1;
	ShiftCount=PhaseShift;
	m_Reserve=new int[SpikeWidth+1];
	for(iRes=0;iRes<=SpikeWidth;iRes++) m_Reserve[iRes]=0;
	iRes=jRes=0;
	Start=0;
	MaxPace=0;
	MinPace=4*SampleRate/1000;
	PaceCount=0;
	FirstTime=1;

	Width=4*SampleRate/1000;
	m_Source=new int[Width];
	m_Differ=new int[Width];
	for(Pos=0;Pos<Width;Pos++) {
		m_Source[Pos]=0;
		m_Differ[Pos]=0;
	}
	Pos=0;
}
CPaceMaker::~CPaceMaker()
{
	delete []m_Reserve;
	delete []m_Source;
	delete []m_Differ;
}
void CPaceMaker::Init()
{
	Init(m_nLowPassFreq,m_bSmooth);
}
void CPaceMaker::Init(int nLowPassFreq,BOOL bSmooth)
{
	int SampleRate=fs;
	int LowPassFreq=nLowPassFreq;
	BOOL Smooth=bSmooth;

	WidthCount=SpikeWidth;
	PaceFound=0;
	ThresholdValue=750;             //not make sure of uV
	if(Smooth) SmoothPhaseShift=24*SampleRate/1000;
	else SmoothPhaseShift=0;
	if(LowPassFreq==0) LowPassPhaseShift=0;
	else if(LowPassFreq<30) LowPassPhaseShift=10*SampleRate/1000;
	else if(LowPassFreq<40) LowPassPhaseShift=8*SampleRate/1000;
	else if(LowPassFreq<50) LowPassPhaseShift=7*SampleRate/1000;
	else if(LowPassFreq<60) LowPassPhaseShift=6*SampleRate/1000;
	else if(LowPassFreq<70) LowPassPhaseShift=5*SampleRate/1000;
	else if(LowPassFreq<80) LowPassPhaseShift=4*SampleRate/1000;
	else if(LowPassFreq<100) LowPassPhaseShift=3*SampleRate/1000;
	else if(LowPassFreq<120) LowPassPhaseShift=2*SampleRate/1000;
	else if(LowPassFreq<=150) LowPassPhaseShift=1*SampleRate/1000;
	else LowPassPhaseShift=0;
	PhaseShift=SmoothPhaseShift+LowPassPhaseShift+1;
	ShiftCount=PhaseShift;
//	m_Reserve=new int[SpikeWidth];
	for(iRes=0;iRes<=SpikeWidth;iRes++) m_Reserve[iRes]=0;
	iRes=jRes=0;
	Start=0;
	MaxPace=0;
	MinPace=4*SampleRate/1000;
	PaceCount=0;
	FirstTime=1;

	Width=4*SampleRate/1000;
//	m_Source=new int[Width];
//	m_Differ=new int[Width];
	for(Pos=0;Pos<Width;Pos++) {
		m_Source[Pos]=0;
		m_Differ[Pos]=0;
	}
	Pos=0;
}
int CPaceMaker::Remove(int xt)
{
	int i,xv;
	xv=m_Source[Pos];
	Pos=(Pos+1)%Width;
	m_Differ[Pos]=xt-xv;
	xv=m_Source[Pos];
	m_Source[Pos]=xt;
	if(PaceFound) {
		WidthCount--;
		if(WidthCount==0) PaceFound=0;
		RestoreValue=xv;
		xv=ReserveValue;
	}
	else if(abs(m_Differ[(Pos+1)%Width])>ThresholdValue) {
		for(i=1;i<Width;i++) {
			if(abs(m_Differ[(Pos+1+i)%Width])>ThresholdValue &&
				m_Differ[(Pos+1)%Width]*m_Differ[(Pos+1+i)%Width]<0) PaceFound=1;
		}
		if(PaceFound) {
			WidthCount=SpikeWidth;
//			RestoreValue=xv;
			ReserveValue=xv;
		}
	}
	return xv;
}
int CPaceMaker::Recover(int xt)
{	
	int xv;
	xv=xt;
	if(PaceFound) {
		if(WidthCount==SpikeWidth) {
			Start=1;
			iRes=0;
			jRes=0;
			ShiftCount=PhaseShift;
		}
		else m_Reserve[iRes++]=RestoreValue;
	}
	if(Start&&ShiftCount>0) ShiftCount--;
	if(ShiftCount==0) {
		Start=0;
		if(jRes<iRes) xv=m_Reserve[jRes++];
	}
	if(jRes==1) {
		if(FirstTime) FirstTime=0;
		else {
			if(PaceCount>MaxPace) MaxPace=PaceCount;
			if(PaceCount<MinPace) MinPace=PaceCount;
		}
		PaceCount=0;
	}
	else PaceCount++;
	return xv;
}
int CPaceMaker::GetSpikes()
{
	if(jRes>0&&jRes<SpikeWidth/3) return m_Reserve[jRes-1];
	else return 0;
}
int CPaceMaker::Type()
//right now, 0: no; 1: single; 2: double;
{
	if(MaxPace==0) return 0;
	if(MaxPace>3*MinPace) return 2;
	else return 1;
}

/////////////////////////////////////////////////////////////////////
//Multi-Channel Lowpass filter
CMultiChannelLowpass2::CMultiChannelLowpass2(double Freq,int SampleRate,int Channel)
{
	fc=Freq;
	ChlNum=Channel;	fs=SampleRate;
	pass=new CLowpassFilter2 *[ChlNum];
	for(int i=0;i<ChlNum;i++) 
		pass[i]=new CLowpassFilter2(Freq,SampleRate);
}

void CMultiChannelLowpass2::Init(double Freq,int SampleRate)
{
	fc=Freq;	fs=SampleRate;
	for(int i=0;i<ChlNum;i++) 
		pass[i]->Init(Freq,SampleRate);
}

void CMultiChannelLowpass2::Filter(short *data)
{
	for(int i=0;i<ChlNum;i++) 
		data[i]=pass[i]->Filter(data[i]);
}

CMultiChannelLowpass2::~CMultiChannelLowpass2()
{
	for(int i=0;i<ChlNum;i++) delete pass[i];
	delete []pass;
}

////////////////////////////////////////////////////////////////////////////
//Multi-Channel Highpass filter
CMultiChannelHighpass::CMultiChannelHighpass(double Freq,int SampleRate,int Channel)
{
	hfreq=Freq;
	ChlNum=Channel;	fs=SampleRate;
	pass=new CHighpassFilter *[ChlNum];
	for(int i=0;i<ChlNum;i++) 
		pass[i]=new CHighpassFilter(Freq,SampleRate);
}

void CMultiChannelHighpass::Init(double Freq,int SampleRate)
{
	for(int i=0;i<ChlNum;i++) 
		pass[i]->Init(Freq,SampleRate);
}

void CMultiChannelHighpass::Filter(short *data)
{
	for(int i=0;i<ChlNum;i++) 
		data[i]=pass[i]->Filter(data[i]);
}

CMultiChannelHighpass::~CMultiChannelHighpass()
{
	for(int i=0;i<ChlNum;i++) delete pass[i];
	delete []pass;
}

/////////////////////////////////////////////////////////////////
//Multi-Channel Notch filter
CMultiChannelNotch::CMultiChannelNotch(double Freq,int SampleRate,int Channel,double q)
{
	Q=q;	fc=Freq;	fs=SampleRate;
	ChlNum=Channel;
	pass=new CNotchFilter *[ChlNum];
	for(int i=0;i<ChlNum;i++) 
		pass[i]=new CNotchFilter(Freq,SampleRate,q);
}

void CMultiChannelNotch::Init(double Freq,int SampleRate,double q)
{
	Q=q;	fc=Freq;	fs=SampleRate;
	for(int i=0;i<ChlNum;i++) 
		pass[i]->Init(Freq,SampleRate,q);
}

void CMultiChannelNotch::Filter(short *data)
{
	for(int i=0;i<ChlNum;i++) 
		data[i]=pass[i]->Filter(data[i]);
}

CMultiChannelNotch::~CMultiChannelNotch()
{
	for(int i=0;i<ChlNum;i++) delete pass[i];
	delete []pass;
}
	
//class CMultiChannelSmooth
CMultiChannelSmooth::CMultiChannelSmooth(int SampleRate,int Channel)
{
	ChlNum=Channel;
	smooth=new CSmoothFilter *[ChlNum];
	for(int i=0;i<ChlNum;i++)
		smooth[i]=new CSmoothFilter(SampleRate);
}
CMultiChannelSmooth::~CMultiChannelSmooth()
{
	for(int i=0;i<ChlNum;i++) delete smooth[i];
	delete []smooth;
}
void CMultiChannelSmooth::Init()
{
	for(int i=0;i<ChlNum;i++) smooth[i]->Init();
}

void CMultiChannelSmooth::Smooth(short *data)
{
	for(int i=0;i<ChlNum;i++) 
		data[i]=smooth[i]->Smooth((int)data[i]);
}
	
//class CMultiChannelPaceMaker
CMultiChannelPaceMaker::CMultiChannelPaceMaker(int LowPassFreq,BOOL Smooth,int SampleRate,int Channel)
{
	ChlNum=Channel;
	pace=new CPaceMaker *[ChlNum];
	for(int i=0;i<ChlNum;i++)
		pace[i]=new CPaceMaker(LowPassFreq,Smooth,SampleRate);
}
CMultiChannelPaceMaker::~CMultiChannelPaceMaker()
{
	for(int i=0;i<ChlNum;i++) delete pace[i];
	delete []pace;
}
void CMultiChannelPaceMaker::Init()
{
	for(int i=0;i<ChlNum;i++) pace[i]->Init();
}
void CMultiChannelPaceMaker::Init(int LowPassFreq,BOOL Smooth)
{
	for(int i=0;i<ChlNum;i++) pace[i]->Init(LowPassFreq,Smooth);
}
void CMultiChannelPaceMaker::Remove(short *data)
{
	for(int i=0;i<ChlNum;i++) 
		data[i]=pace[i]->Remove((int)data[i]);
}
void CMultiChannelPaceMaker::Recover(short *data)
{
	for(int i=0;i<ChlNum;i++) 
		data[i]=pace[i]->Recover((int)data[i]);
}
void CMultiChannelPaceMaker::GetSpikes(short *Spikes)
{
	for(int i=0;i<ChlNum;i++) 
		Spikes[i]=pace[i]->GetSpikes();
}
void CMultiChannelPaceMaker::Type(short *types)
{
	for(int i=0;i<ChlNum;i++) 
		types[i]=pace[i]->Type();
}

///////////////////////////////////////////////////////////////////////////
//class CLengthTransform
CLengthTransform::CLengthTransform(int SampleRate,int ms)
{
	w=0;
	wms=-1;fs=-1;
	L=NULL;
	Init(SampleRate,ms);
}

CLengthTransform::~CLengthTransform()
{
	if(L) delete[]L;
}

void CLengthTransform::Init(int SampleRate,int ms)
{
	if(ms!=wms || SampleRate!=fs) {
		if(L) delete[]L;	L=NULL;
	}

	wms=ms;
	fs=SampleRate;
	w=wms*fs/1000;
	if(!L) L=new int[w];

	Init();
}

void CLengthTransform::Init()
{
	p=0;
	Li=0;
	pre[0]=pre[1]=pre[2]=0;
	memset(L,0,w*sizeof(int));
}

int CLengthTransform::Length(int xt)
{
	L[p]=abs(xt-pre[0]);	
	pre[0]=xt;
	Li+=L[p];
	p=(p+1)%w;
	Li-=L[p];
	return Li;
}

int CLengthTransform::Length(int xt,int yt,int zt)
{
	L[p]=(abs(xt-pre[0])+abs(yt-pre[1])+abs(zt-pre[2]))/3;	
	pre[0]=xt;pre[1]=yt;pre[2]=zt;
	Li+=L[p];
	p=(p+1)%w;
	Li-=L[p];
	return Li;
}
//////////////////////////////////////////////////////////////////////////
//class CrtRRdetect
CrtRRdetect::CrtRRdetect(int SampleRate,int freqNotch)
{
	fs=SampleRate;	notch=freqNotch;
	sec2=fs*2;			//2 seconds
	sec3=fs*3;			//3 seconds
	ms240=fs*240/1000;	//240ms		//max 250bpm
	lw=100;				//100ms
	q=fs*lw/1000;		//100ms;
//	win=new int[q];

    xLow=new CLowpassFilter2(10,fs);
    yLow=new CLowpassFilter2(10,fs);
    zLow=new CLowpassFilter2(10,fs);
        
    xHigh=new CHighpassFilter(7,fs);
    yHigh=new CHighpassFilter(7,fs);
    zHigh=new CHighpassFilter(7,fs);

//	xDiff=new Difference();
//	yDiff=new Difference();
//	zDiff=new Difference();

	if(notch>0) {
		xNotch=new CNotchFilter(notch,fs);
		yNotch=new CNotchFilter(notch,fs);
		zNotch=new CNotchFilter(notch,fs);
	}
	else {
		xNotch=NULL;	yNotch=NULL;	zNotch=NULL;
	}
	
	xyzLength=new CLengthTransform(fs,lw);

	Init();
}
CrtRRdetect::~CrtRRdetect()
{
//	delete[]win;

    delete xLow;
    delete yLow;
    delete zLow;
        
    delete xHigh;
    delete yHigh;
    delete zHigh;

//	delete xDiff;
//	delete yDiff;
//	delete zDiff;

    if(xNotch) delete xNotch;
    if(yNotch) delete yNotch;
    if(zNotch) delete zNotch;

	delete xyzLength;
}

void CrtRRdetect::Init()
{
//	memset(win,0,q*sizeof(int));
	w=0;
	count=0;
	rr=0;
	maxp=0;minp=0x7fff;
	er=0;
	pre=fs/2;	//0.5s
	doing=0;repeat=0;
	status=0;
//	start=find=false;
//	first=true;

    xLow->Init(10,fs);
    yLow->Init(10,fs);
    zLow->Init(10,fs);
        
    xHigh->Init(7,fs);
    yHigh->Init(7,fs);
    zHigh->Init(7,fs);

//	xDiff->Init();
//	yDiff->Init();
//	zDiff->Init();

	if(notch>0) {
		xNotch->Init(notch,fs);
		yNotch->Init(notch,fs);
		zNotch->Init(notch,fs);
	}

	xyzLength->Init();
}

int CrtRRdetect::QRSdetect(int xt)
{
	if(xNotch) xt=xNotch->Filter(xt);
	xt=xLow->Filter(xt);
	xt=xHigh->Filter(xt);
//	xt=xDiff->Differ(xt);
	xt=xyzLength->Length(xt);

	return RRdetect(xt);
}

int CrtRRdetect::QRSdetect(int xt,int yt,int zt)
{
	if(notch>0) {
		xt=xNotch->Filter(xt);
		yt=yNotch->Filter(yt);
		zt=zNotch->Filter(zt);
	}
	xt=xLow->Filter(xt);
	yt=yLow->Filter(yt);
	zt=zLow->Filter(zt);
	xt=xHigh->Filter(xt);
	yt=yHigh->Filter(yt);
	zt=zHigh->Filter(zt);
//	xt=xDiff->Differ(xt);
//	yt=yDiff->Differ(yt);
//	zt=zDiff->Differ(zt);

	xt=xyzLength->Length(xt,yt,zt);

	return RRdetect(xt);
}

int CrtRRdetect::RRdetect(int xt)
{//约需3秒找到第1个心拍
	rr=0;
	switch(status) {
	case 0:	//学习2秒钟，找到最大值
		if(doing<pre) {xt=0;doing++;}
//		if(peak<xt) peak=xt;
		if(minp>xt)	minp=xt;
		if(maxp<xt) maxp=xt;
		count++;
		if(count>sec2) {
			count=0;
//			er=peak/3;	//取最大值的1/3为判据
			er=minp+(maxp-minp)/3;	//取最大值的1/3为判据
			if(er>0) {
				doing=0;	repeat=0;
				maxp=0;		minp=0x7fff;
				if(xt<er) status=2;	//学习完后，处于静息期
				else	  status=1;	//学习完后，处于心拍中
			}//否则重新学习
		}
		break;
	case 1:	//学习完后，如果处于一个心拍中，则寻找静息期，否则寻找第一心拍
		doing++;	count++;
		if(xt<er && doing>q) {
			doing=0;	status=2;
		}
		break;
	case 2:	//寻找第一心拍
		count++;
		if(xt>=er) {//找到第一心拍
			rr=count;
			count=0;	
			repeat++;		
			if(repeat>=5) {//每5个心拍更新一次判据
//				if(peak>3) er=peak/3;
				if(maxp-minp>3) er=minp+(maxp-minp)/3;
				repeat=0;	//peak=0;
				maxp=0;		minp=0x7fff;
			}
			status=3;		//处于心拍中
		}
		if(count>sec3) status=0;	//3秒未检出，重新学习
		break;
	case 3:	//处于第一心拍中，240ms无检出区间
		count++;
		if(count>ms240 && xt<er) status=2;	//寻找下一心拍
		if(count>sec3) status=0;	//3秒未检出，重新学习
//		if(peak<xt) peak=xt;
		if(minp>xt)	minp=xt;
		if(maxp<xt) maxp=xt;
		break;
	}
/*HDC hDC=GetDC(NULL);
CDC *pDC=CDC::FromHandle(hDC);
CString str;
str.Format("status=%d,er=%d,peak=%d,repeat=%d,rr=%d,xt=%d",status,er,peak,repeat,rr,xt);
pDC->TextOut(100,0,str);
ReleaseDC(NULL,hDC);*/
	return rr;
}

//Multi-Channel Baseline Drife
MultiChannelBaseline::MultiChannelBaseline(int SampleRate,int Channel)
{
	ChlNum=Channel;	fs=SampleRate;
	baseline=new RealtimeBaselineDrift *[ChlNum];
	for(int i=0;i<ChlNum-1;i++) 
		baseline[i]=new RealtimeBaselineDrift(SampleRate,0);
	baseline[ChlNum-1]=new RealtimeBaselineDrift(SampleRate,1);
}

void MultiChannelBaseline::Init(int SampleRate,int Channel)
{
	ChlNum=Channel;	fs=SampleRate;
	for(int i=0;i<ChlNum-1;i++) 
		baseline[i]->Init(SampleRate,0);
	baseline[ChlNum-1]->Init(SampleRate,1);
}

int MultiChannelBaseline::Drift(short *data,int isR,BOOL BL)
{
	int Rp;
	Rp=isR;
	for(int i=0;i<ChlNum;i++) 
		data[i]=baseline[i]->drift(data[i],&Rp,BL);
	return Rp;
}

BOOL MultiChannelBaseline::ReplayAvailable()
{
	for(int i=0;i<ChlNum;i++) {
		if(baseline[i]->ReplayAvailable()) return TRUE;
	}
	return FALSE;
}

MultiChannelBaseline::~MultiChannelBaseline()
{
	for(int i=0;i<ChlNum;i++) delete baseline[i];
	delete []baseline;
}

///////////////////////////////////////////////////////////////////////////////////
//2013-5-15恢复使用  //2013-5-24 最后修改 //2014-12-26修改早搏加提前
RealtimeBaselineDrift::RealtimeBaselineDrift(int SampleRate,int return_hr)
{
	x=NULL;
	hr=NULL;
	avrgX=NULL;
	Init(SampleRate,return_hr);
}

RealtimeBaselineDrift::~RealtimeBaselineDrift()
{
  Delete();
}

void RealtimeBaselineDrift::Init(int SampleRate,int return_hr)
{
  Delete();
  sRate=SampleRate;
  n=sRate*12;    //12 seconds
  prPos=(55+66+15)*sRate/1000;  //66ms for P-R, 15ms for half of moving window, 55ms for prosible VE
  x=new int[n]; memset(x,0,n*sizeof(int));
  if(return_hr) {hr=new int[n];memset(hr,0,n*sizeof(int));}
  else hr=NULL;
  rrRange=15*sRate/10;    //1.5 seconds
  avrgX=new int[rrRange]; memset(avrgX,0,rrRange*sizeof(int));
  InitDrift();
}

void RealtimeBaselineDrift::Delete()
{
  if(x) delete[]x;	x=NULL;
  if(hr) delete[]hr;  hr=NULL;
  if(avrgX) delete []avrgX;  avrgX=NULL;
}

void RealtimeBaselineDrift::InitDrift()
{
	rrCount=0;
	averageV=0.0;
	aP=0;
	Rtimes=0;
	p=q=0;
	p4=-1;
	p0=p1=p2=p3=0;
	y=y0=y1=y2=y3=y4=0;
	y1t=y2t=y3t=0;
}

int RealtimeBaselineDrift::prPosition()
{
	int temp,minV,maxV,minD;
	int wT=20*sRate/1000;
	int k,i,prP,prTemp;
	
	minV=maxV=x[(p-prPos+n)%n];
	prTemp=prPos;
	for(i=1;i<wT;i++) {
		temp=x[(p-prPos+i+n)%n];
		if(temp<minV) {
			minV=temp;
			prTemp=prPos-i;
		}
		if(temp>maxV) maxV=temp;
	}
	minD=maxV-minV;
	prP=prTemp;
	for(k=1;k<prPos/2;k++) {
		minV=maxV=x[(p-prPos+k+n)%n];
		prTemp=prPos-k;
		for(i=1;i<wT;i++) {
			temp=x[(p-prPos+k+i+n)%n];
			if(temp<minV) {
				minV=temp;
				prTemp=prPos-k-i;
			}
			if(temp>maxV) maxV=temp;
		}  
		if(maxV-minV<minD) {
			minD=maxV-minV;
			prP=prTemp;
		}
	}
	return prP;
}

void RealtimeBaselineDrift::calculatedrift()
{
  double t1, t2;
  double tt1,temp;
  t1=(p1>p0)? p1-p0:p1+n-p0;
  t2=(p2>p0)? p2-p0:p2+n-p0;
  tt1=t1*t1;
  temp=(y2-y0)/t2;
  y3t=12.*(y0-y1)/(t1*tt1)+6*(y1t+temp)/tt1;
  y2t=-6*(y0-y1)/tt1-2*(2*y1t+temp)/t1;
}

bool RealtimeBaselineDrift::ReplayAvailable()
{
	if(Rtimes==3) return TRUE;
	else return FALSE;
}

// 2015-1-26 早搏处理和提前量（合并case1，case2）
int RealtimeBaselineDrift::drift(int xt, int *isR,BOOL BL)
{
   int xn,hrate;
   int prV=0;
   int b0=prPos;

   if(Rtimes==3) {  //4 11/2,2000
	  if(BL) xn=x[q]-(int)y;
	  else xn=x[q];
      if(hr) hrate=hr[q];
      y+=(y1t+y2t/2+y3t/6);
      y1t+=(y2t+y3t/2);
      y2t+=y3t;
      if(q==p1) {   //q==p1  11/2,2000
	     p0=p1; p1=p2;   // p2=p3;
	     y0=y1; y1=y2;   // y2=y3;
		 y=y0;    //2014.6.16 加
		 if(p3>=0) {
			 p2=p3;
			 y2=y3;
		 }
		 else {
			 p2=(p>=p2)? (p2+(p-p2)/2)%n:(p2+(p+n-p2)/2)%n;
			 y2=(y2+(int)averageV)/2;
		 }
	     if(p4>=0) {
	        p3=p4;
	        y3=y4;
	        p4=-1;
	     }
	     else  p3=-1;
	     calculatedrift();
      }
      q=(q+1)%n;
   }
   else {
       xn=0;
       hrate=0;
   }
   x[p]=xt;
   if(hr) hr[p]=*isR;
//   if(hr) hr[(p-b+n)%n]=*isR;  //为显示PR样条点
   if(*isR || rrCount>rrRange) {
	  rrCount=0;
	  if(*isR) {
		  b0=prPosition();
		  prV=x[(p-b0+n)%n];
	  }
	  else {
		  b0=rrRange/2;
		  prV=(int)averageV;  //(prV+int(averageV))/2;
	  }
	  if(Rtimes<3) {
		 Rtimes++;
	     switch(Rtimes) {
	     case 1:
			p0=q=0;
			p1=(p>=2*b0)? p-b0 : p;
			y=y0=y1=prV;   //x[p1];
		    break;
		case 2:
	        p2=(p-b0+n)%n;
	        y2=prV;  //x[p2];
	        if(p1>p0)  y1t=(y1-y0)/(p1-p0);
	        else if(p1-p0+n>0) y1t=(y1-y0)/(p1+n-p0);
		    else y1t=0;
	        calculatedrift();
	        break;
	     case 3:
	        p3=(p-b0+n)%n;
	        y3=prV;  //x[p3];
	        break;
	     }     //switch
	  }
	  else {        // Rtimes>=3
	     if(p3<0) {
	        p3=(p-b0+n)%n;
	        y3=prV;  //x[p3];
	     }
		 else {
			 p4=(p-b0+n)%n;
			 y4=prV;   //x[p4];
		 }
      }
   }

   p=(p+1)%n;
   if(hr) *isR=hrate;

   rrCount++;
   averageV-=(double)avrgX[aP]/(double)rrRange;
   averageV+=(double)xt/(double)rrRange;
   avrgX[aP]=xt;
   aP=(aP+1)%rrRange;

   return(xn);
}

//////////////////////////////////////////
/*
RealtimeBaselineDrift::RealtimeBaselineDrift(int SampleRate,int return_hr)
{
//  rt_hr=return_hr;
	x=NULL;
	hr=NULL;
	Init(SampleRate,return_hr);
}

RealtimeBaselineDrift::~RealtimeBaselineDrift()
{
  Delete();
}

void RealtimeBaselineDrift::Init(int SampleRate,int return_hr)
{
	Delete();

  n=SampleRate*9;    //5 seconds
  b=(66+4*RR_SHIFTWINDOW+RR_SHIFTWINDOW/2)*SampleRate/1000+6;  //66 msec for P-R, 15 msc for half of moving window
  x=new int[n];memset(x,0,n*sizeof(int));
  if(return_hr) {hr=new int[n];memset(hr,0,n*sizeof(int));}
  else hr=NULL;
  InitDrift();
}

void RealtimeBaselineDrift::Delete()
{
  if(x)	delete[]x;	x=NULL;
  if(hr) delete[]hr;hr=NULL;
}

void RealtimeBaselineDrift::InitDrift()
{
	Rtimes=0;
	p=q=0;
	p4=-1;
	p0=p1=p2=p3=0;
	y0=y1=y2=y3=0;
	y=y1t=y2t=y3t=0;
//	for(int i=0;i<n;i++) x[i]=0;
	Count=0;
}

void RealtimeBaselineDrift::calculatedrift()
{
  double t1, t2;
  double tt1,temp;
  t1=(p1>p0)? p1-p0:p1+n-p0;
  t2=(p2>p0)? p2-p0:p2+n-p0;
  tt1=t1*t1;
  temp=(y2-y0)/t2;
  y3t=12.*(y0-y1)/(t1*tt1)+6*(y1t+temp)/tt1;
  y2t=-6*(y0-y1)/tt1-2*(2*y1t+temp)/t1;
}

int RealtimeBaselineDrift::drift(int xt, int *isR)
{
   int xn,hrate;
	if(Rtimes>=4) {  //4 11/2,2000
	  xn=x[q]-(int)y;
      if(hr) hrate=hr[q];
      y=y+y1t+y2t/2+y3t/6;
      y1t=y1t+y2t+y3t/2;
      y2t=y2t+y3t;
      if(q==p1) {   //q==p1  11/2,2000
	     p0=p1; p1=p2;// p2=p3;
	     y0=y1; y1=y2;// y2=y3;
		 //problem solving 2000.1.10
		 if(p3>=0) {
			 p2=p3;
			 y2=y3;
		 }
		 else {
			 p2=(p>=p2)? (p2+(p-p2)/2)%n:(p2+(p+n-p2)/2)%n;
//			 y2=x[p2];   //y2=y2;?
		 }
		 //above
	     if(p4>=0) {
	        p3=p4;
	        y3=x[p3];
	        p4=-1;
	     }
	     else  p3=-1;
	     calculatedrift();
      }
      q=(q+1)%n;
   }
   else {
      xn=0;
      hrate=0;
   }
   x[p]=xt;
   if(hr) hr[p]=*isR;
   if(*isR) {
      if(Rtimes<4) {
  	     Rtimes++;
	     switch(Rtimes) {
	     case 1:
	       if(p>=b) {
		      p0=p-b;
		      q=p0;
		      y0=x[p0];
		      y=y0;
	       }
	       else Rtimes--;
	       break;
	     case 2:
	       p1=p-b;
		   p1=max(0,p1);	//made by husheping, maybe incorrect;
	       y1=x[p1];
	       break;
	     case 3:
	       p2=p-b;
		   p2=max(0,p2);	//made by husheping, maybe incorrect;
	       y2=x[p2];
	       if(p1>p0)  y1t=(y1-y0)/(p1-p0);
	       else if(p1-p0+n>0) y1t=(y1-y0)/(p1+n-p0);
		   else y1t=0;
	       calculatedrift();
	       break;
	     case 4:
	       p3=p-b;
		   p3=max(0,p3);	//made by husheping, maybe incorrect;
	       y3=x[p3];
	       break;
	     }     //switch
      }
      else {        // Rtimes>=4
	     if(p3<0) {
	        p3=(p>=b)? p-b:p+n-b;
	        y3=x[p3];
	     }
	     else {
	       p4=(p>=b)? p-b:p+n-b;
	     }
      }
   }
   p=(p+1)%n;
   if(hr) *isR=hrate;
   return(xn);
}
*/
/*
RealtimeBaselineDrift::RealtimeBaselineDrift(int SampleRate,int return_hr)
{
//  rt_hr=return_hr;
	x=NULL;
	hr=NULL;
	Init(SampleRate,return_hr);
}

RealtimeBaselineDrift::~RealtimeBaselineDrift()
{
  Delete();
}

void RealtimeBaselineDrift::Init(int SampleRate,int return_hr)
{
	Delete();

  n=9*SampleRate;    //5 seconds
//  b=(66+4*RR_SHIFTWINDOW+RR_SHIFTWINDOW/2)*SampleRate/1000+6;  //66 msec for P-R, 15 msc for half of moving window
  b=88*SampleRate/1000;  //66 msec for P-R, 15 msc for half of moving window
  x=new int[n];memset(x,0,n*sizeof(int));
  if(return_hr) {hr=new int[n];memset(hr,0,n*sizeof(int));}
  else hr=NULL;
  InitDrift();
}

void RealtimeBaselineDrift::Delete()
{
  if(x)	delete[]x;	x=NULL;
  if(hr) delete[]hr;hr=NULL;
}

void RealtimeBaselineDrift::InitDrift()
{
//	Rtimes=0;
	Ready=FALSE;
	p=q=0;
	p4=-1;
	p0=p1=p2=p3=0;
	y0=y1=y2=y3=0;
	y=y1t=y2t=y3t=0;
//	for(int i=0;i<n;i++) x[i]=0;
//	Count=0;
}

void RealtimeBaselineDrift::calculatedrift()
{
  double t1, t2;
  double tt1,temp;
  t1=(p1>p0)? p1-p0:p1+n-p0;
  t2=(p2>p0)? p2-p0:p2+n-p0;
  tt1=t1*t1;
  temp=(y2-y0)/t2;
  y3t=12.*(y0-y1)/(t1*tt1)+6*(y1t+temp)/tt1;
  y2t=-6*(y0-y1)/tt1-2*(2*y1t+temp)/t1;
}

BOOL RealtimeBaselineDrift::ReplayAvailable()
{
	if(Ready) return TRUE;
	else return FALSE;
}

int RealtimeBaselineDrift::drift(int xt, int *isR)
{
	int xn,hrate;
	if(Ready) {      
	  xn=x[q]-(int)y;
      if(hr) hrate=hr[q];
      y=y+y1t+y2t/2+y3t/6;
      y1t=y1t+y2t+y3t/2;
      y2t=y2t+y3t;
      if(q==p1) {   //q==p1  11/2,2000
//		  xn=10000;            //for P_R position test
	     p0=p1; p1=p2;// p2=p3;
	     y0=y1; y1=y2;// y2=y3;
		 //problem solving 2000.1.10
		 if(p3>=0) {
			 p2=p3;
			 y2=y3;
			 if(p4>=0) {
				p3=p4;
				y3=y4;
				p4=-1;
			 }
			 else  p3=-1;
		 }
		 else {
		     if(p4>=0) {
		        p2=p4;
			    y2=y4;
				p4=-1;
			 }
			 else {
				 p2=(p>=p2)? (p2+(p-p2)/2)%n:(p2+(p+n-p2)/2)%n;
			 }
		 }
	     calculatedrift();
      }
      q=(q+1)%n;
   }
   else {
      xn=0;
      hrate=0;
   }
   x[p]=xt;
   if(hr) hr[p]=*isR;
   if(*isR) {
      if(!Ready) {
		 if(p>n/8) {
			 p3=p-b;
			 p1=p3/3;
			 p2=p1+p1;
//			 p0=0;
//			 q=p0;
			 y=y0=y1=y2=y3=(x[(p3-2+n)%n]+x[(p3-1+n)%n]+x[p3]+x[(p3+1)%n]+x[(p3+2)%n])/5;
			 y1t=0;
			 calculatedrift();
			 Ready=TRUE;
	         if(hr) hr[p]=int(p/2.8);//*isR;
		 }
	  }		 
      else {        // Rtimes==3
	     if(p3<0) {
	        p3=(p>=b)? p-b:p+n-b;
	        y3=(x[(p3-2+n)%n]+x[(p3-1+n)%n]+x[p3]+x[(p3+1)%n]+x[(p3+2)%n])/5;
	     }
	     else {
	        p4=(p>=b)? p-b:p+n-b;
		    y4=(x[(p4-2+n)%n]+x[(p4-1+n)%n]+x[p4]+x[(p4+1)%n]+x[(p4+2)%n])/5;
	     }
      }
   }
   p=(p+1)%n;
   if(hr) *isR=hrate;
   return(xn);
}
*/
void ButterwirthFilter(int Freq,int SampleRate,double *data,int length,int maxp)
{
	int i,j;
	double a,ab[2][5];
    double c[2]= { 0.5883003, 0.24350705 };
    double fsfc=SampleRate/Freq;
    double tscp=fsfc/M_PI;

   for(i=0;i<2;i++){
	 a=1.0+c[i]*fsfc+tscp*tscp;
	 ab[i][0]=tscp*tscp/a;      //  a0
	 ab[i][1]=-2.0*ab[i][0];   //  a1
	 ab[i][2]=ab[i][0];         //   a2
	 ab[i][3]=(2.0-2.0*tscp*tscp)/a;  //   b1
	 ab[i][4]=(1.0-c[i]*fsfc+tscp*tscp)/a;  //  b2
   }
   
   double *y=new double[length];
   for(i=0;i<2;i++){
      y[0]=y[1]=0.0;
      for(j=2;j<=maxp;j++) {
	  y[j]=(ab[i][0]*data[j]+ab[i][1]*data[j-1]+ab[i][2]*data[j-2])
	      -(ab[i][3]*y[j-1]+ab[i][4]*y[j-2]);
      }
      for(j=0;j<maxp;j++) data[j]=y[j];
   }
   for(i=0;i<2;i++){
      y[length-1]=y[length-2]=0.0;
      for(j=length-3;j>=maxp;j--) {
	  y[j]=(ab[i][0]*data[j]+ab[i][1]*data[j+1]+ab[i][2]*data[j+2])
	      -(ab[i][3]*y[j+1]+ab[i][4]*y[j+2]);
      }
      for(j=length-1;j>=maxp;j--) data[j]=y[j];
   }
   delete []y;
}

//used for all channels of notch filter after-process
void PostProcess(int Channel,double *data[],int n,int NotchFreq,int SampleRate)
{
	int i;
	int ch=Channel==1? 0:1;
	double f;
	f=NotchFreqDetect(data[ch],n,NotchFreq,SampleRate);
	CNotchFilter notch(f,SampleRate);
	for(ch=0;ch<Channel;ch++) {
		for(i=0;i<n;i++) *(data[ch]+i)=notch.Filter(*(data[ch]+i));
		for(i=n-1;i>=0;i--) *(data[ch]+i)=notch.Filter(*(data[ch]+i));
	}
}

double NotchFreqDetect(double *data,int n,int NotchFreq,int SampleRate)
{
	double f0,f1,div0,div1;
	double step=0.0001;
	f0=double(NotchFreq)-0.5;
	f1=double(NotchFreq)+0.5;
	div0=NotchDiv(f0,data,n,SampleRate);
	div1=NotchDiv(f1,data,n,SampleRate);
	while(f1-f0>step) {
		if(div0<div1) {
			f1-=((f1-f0)/4.);
			div1=NotchDiv(f1,data,n,SampleRate);
		}
		else {
			f0+=((f1-f0)/4.);
			div0=NotchDiv(f0,data,n,SampleRate);
		}
	}
	return (f0+(f1-f0)/2.);
}

double NotchDiv(double f,double *data,int n,int SampleRate)
{
	short int i;
	double *temp=new double[n];
	CNotchFilter notch(f,SampleRate);
	double average=0.,div=0.;

	for(i=0;i<n;i++) temp[i]=data[i];
	for(i=0;i<n;i++) temp[i]=notch.Filter(temp[i]);
	for(i=n/2-n/4;i<n/2+n/4;i++) average+=temp[i];
	average/=(n/2);
	for(i=n/2-n/4;i<n/2+n/4;i++) div+=((temp[i]-average)*(temp[i]-average));
	delete []temp;
	return div;
}

