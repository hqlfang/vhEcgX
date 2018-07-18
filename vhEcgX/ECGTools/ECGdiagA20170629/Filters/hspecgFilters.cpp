#include "stdafx.h"
#include "hspecgFilters.h"
////////////////////////////////////////////////////////////////////////////////////////////
//begin of CBaseNotchFilter
CBaseNotchFilter::CBaseNotchFilter()
{
	x=NULL;
	y=NULL;
	m_f=0;
	m_fs=0;
	//Init(NotFreq,SampleRate);
}
CBaseNotchFilter::CBaseNotchFilter(int NotFreq,int SampleRate)
{
	x=NULL;
	y=NULL;
	m_f=0;
	m_fs=0;
	Init(NotFreq,SampleRate);
}
CBaseNotchFilter::~CBaseNotchFilter()
{
	if(x) delete[]x;	x=NULL;
	if(y) delete[]y;	y=NULL;
}
	
void CBaseNotchFilter::Init(int Freq,int SampleRate)
{
	if(Freq>0 || SampleRate>0) {
		if(m_f!=Freq || m_fs!=SampleRate) {
			if(x) delete[]x;	x=NULL;
			if(y) delete[]y;	y=NULL;
		}
		if(Freq>0) m_f=Freq;	
		if(SampleRate>0) m_fs=SampleRate;
	}
	int f=m_f,fs=m_fs;
	if(f!=50) {
		f=50;	fs=fs*5/6;
	}
	L=fs/f;
	K=2*fs/L;
	M=3*fs/4-L/2;
	d1=fs/2;
	d2=fs;
	d3=d1+d2;
	if(!x) x=new int[d3+1];
	if(!y) y=new int[L+1];
	xi=yi=0;
	for(int i=0;i<=d3;i++) x[i]=0;
	for(int i=0;i<=L;i++) y[i]=0;
}

int CBaseNotchFilter::Filter(int xt)
{
	int i,j,k,m,n,d=d3+1;
	x[xi]=xt;
	i=xi-d1;	if(i<0) i+=d;
	j=xi-d2;	if(j<0) j+=d;
	k=xi-d3;	if(k<0) k+=d;
	m=yi-L;		if(m<0) m+=(L+1);
	n=xi-M;		if(n<0) n+=d;
	y[yi]=x[xi]+x[i]-x[j]-x[k]+y[m];
	int yt=x[n]-y[yi]/K;
	xi++;		if(xi>d3) xi=0;
	yi++;		if(yi>L)  yi=0;
	return yt;
}
//end of CBaseNotchFilter

////////////////////////////////////////////////////////////////////////////////////////////
//begin of CBlNotchFilter
CBlNotchFilter::CBlNotchFilter()
{
	m_f=0;
	m_fs=0;
	m_n=0;
	m_index=0;
	m_filters=NULL;
	//Init(NotFreq,SampleRate);
}
CBlNotchFilter::CBlNotchFilter(int NotFreq,int SampleRate)
{
	m_f=0;
	m_fs=0;
	m_n=0;
	m_index=0;
	m_filters=NULL;
	Init(NotFreq,SampleRate);
}
CBlNotchFilter::~CBlNotchFilter()
{
	if(m_filters) {
		for(int i=0;i<m_n;i++) delete m_filters[i];
		delete[]m_filters;	m_filters=NULL;
	}
}
void CBlNotchFilter::Destroy()
{
	if(m_filters) {
		for(int i=0;i<m_n;i++) delete m_filters[i];
		delete[]m_filters;	m_filters=NULL;
	}
}

void CBlNotchFilter::Init(int Freq,int SampleRate)
{
	if(Freq>0 || SampleRate>0) {
		if(m_f!=Freq || m_fs!=SampleRate) {
			Destroy();
		}
		if(Freq>0) m_f=Freq;	
		if(SampleRate>0) m_fs=SampleRate;
	}
	m_index=0;
	int fs=m_f*10;
	if(m_fs%fs==0) {
		m_n=m_fs/fs;
	}
	else {
		m_n=1;
		fs=m_fs;
	}
	m_filters=new CBaseNotchFilter *[m_n];
	for(int i=0;i<m_n;i++) m_filters[i]=new CBaseNotchFilter(m_f,fs);
}

int CBlNotchFilter::Filter(int xt)
{
	if(m_filters) {
		xt=m_filters[m_index++]->Filter(xt);
		if(m_index>=m_n) m_index=0;
	}
	return xt;
}
//end of CBlNotchFilter

//////////////////////////////////////////////////////////////////////////////////////////////////////
//begin of CMutiBlNotchFilter
CMutiBlNotchFilter::CMutiBlNotchFilter(int Freq,int SampleRate,int ChlNumber)
{
	m_chnum=ChlNumber;
	blac=new CBlNotchFilter *[m_chnum];
	for(int i=0;i<m_chnum;i++) blac[i]=new CBlNotchFilter(Freq,SampleRate);
}
CMutiBlNotchFilter::~CMutiBlNotchFilter()
{
	for(int i=0;i<m_chnum;i++) delete blac[i];
	delete[]blac;
}
void CMutiBlNotchFilter::Init(int Freq,int SampleRate)
{
	for(int i=0;i<m_chnum;i++) blac[i]->Init(Freq);
}
void CMutiBlNotchFilter::Filter(int *xt)
{
	for(int i=0;i<m_chnum;i++) xt[i]=blac[i]->Filter(xt[i]);
}
void CMutiBlNotchFilter::Filter(short *xt)
{
	for(int i=0;i<m_chnum;i++) xt[i]=(short)blac[i]->Filter((int)xt[i]);
}
//end of CMutiBlNotchFilter


//////////////////////////////////////////////////////////////////////////////////////////
//begin of FreqInterpretate
FreqInterpretate::FreqInterpretate()
{
	m_fsrc=m_fdst=0;	//原始频率，插值后频率
	m_n=m_maxn=0;		//实际插值点数，最大插值点数
	m_dt=0;			//比例系数
	m_y=NULL;
	//Init(fsrc,fdst);
}
FreqInterpretate::FreqInterpretate(int fsrc,int fdst)
{
	m_fsrc=m_fdst=0;	//原始频率，插值后频率
	m_n=m_maxn=0;		//实际插值点数，最大插值点数
	m_dt=0;			//比例系数
	m_y=NULL;
	Init(fsrc,fdst);
}
FreqInterpretate::~FreqInterpretate()
{
	if(m_y) delete[]m_y;	m_y=NULL;
}
	
void FreqInterpretate::Init(int fsrc,int fdst)
{
	if(fsrc>0 || fdst>0) {
		if(m_fsrc!=fsrc || m_fdst!=fdst) {
			if(m_y) delete[]m_y;	m_y=NULL;
		}
		if(fsrc>0) m_fsrc=fsrc;
		if(fdst>0) m_fdst=fdst;
	}
	if(m_fsrc>0 && m_fdst>m_fsrc) {
		m_dt=(double)m_fsrc/m_fdst;
		m_maxn=int(1/m_dt);
		if(m_fdst%m_fsrc) m_maxn++;
	}
	else {
		m_fdst=m_fsrc;
		m_dt=1;
		m_maxn=1;
	}
	m_y0=0;
	m_t0=0;
	if(!m_y) m_y=new short[m_maxn];
}
    
short FreqInterpretate::CalculateReturnValues(short CurrentValue)
{
	double dy=double(CurrentValue-m_y0)/(1-m_t0);
	double t=m_t0;
	m_n=0;
	while(t<0.999999) {
		m_y[m_n++]=(short)(m_y0+(t-m_t0)*dy);
		t+=m_dt;
	}
	//m_y0+=((t-m_t0)*dy);
	m_y0=CurrentValue;
	m_t0=t-1;
	return m_n;
}
	
short *FreqInterpretate::GetData()
{
	return m_y;
}

CHspecgNotchFilter::CHspecgNotchFilter()
{
	m_f=m_fs=0;
	m_bFint=false;
}
CHspecgNotchFilter::CHspecgNotchFilter(int NotFreq,int SampleRate)
{
	m_f=m_fs=0;
	Init(NotFreq,SampleRate);
}
CHspecgNotchFilter::~CHspecgNotchFilter()
{
}
void CHspecgNotchFilter::Init(int Freq,int SampleRate)
{
	m_bFint=false;
	if(Freq>0) m_f=Freq;
	if(SampleRate>0) m_fs=SampleRate;
	int f=m_f,fs=m_fs;
	if(m_fs%m_f==0) {
	}
	else {
		m_bFint=true;
		fs=short(m_fs/600.+19.5)*600;
		if(fs<300) fs=600;
		m_fint.Init(m_fs,fs);
	}
	if(f!=50) {
		fs=fs*50/f;
		f=50;
	}
	m_blac.Init(f,fs);
}
int CHspecgNotchFilter::Filter(int xt)
{
	if(!m_bFint) return m_blac.Filter(xt);
	else {
		short n=m_fint.CalculateReturnValues(xt);
		short *p=m_fint.GetData();
		for(int k=0;k<n;k++) p[k]=m_blac.Filter(p[k]);
		return p[0];
	}
}
//end of CHspecgNotchFilter

//////////////////////////////////////////////////////////////////////////
//begin of CHspecgMultiNotchFilter
CHspecgMultiNotchFilter::CHspecgMultiNotchFilter()
{
	m_chnum=0;
	m_pNotchFilter=NULL;
}
CHspecgMultiNotchFilter::CHspecgMultiNotchFilter(int NotFreq,int SampleRate,int ChlNum)
{
	m_chnum=ChlNum;
	m_pNotchFilter=new CHspecgNotchFilter *[ChlNum];
	for(int i=0;i<m_chnum;i++) {
		m_pNotchFilter[i]=new CHspecgNotchFilter(NotFreq,SampleRate);
	}
}
CHspecgMultiNotchFilter::~CHspecgMultiNotchFilter()
{
	for(int i=0;i<m_chnum;i++) {
		delete m_pNotchFilter[i];
        m_pNotchFilter[i]=NULL;
	}
	delete[]m_pNotchFilter;
}
void CHspecgMultiNotchFilter::Init(int Freq,int SampleRate)
{
	for(int i=0;i<m_chnum;i++) m_pNotchFilter[i]->Init(Freq,SampleRate);
}
void CHspecgMultiNotchFilter::Filter(int *xt)
{
	for(int i=0;i<m_chnum;i++) xt[i]=m_pNotchFilter[i]->Filter(xt[i]);
}
void CHspecgMultiNotchFilter::Filter(short *xt)
{
	for(int i=0;i<m_chnum;i++) xt[i]=m_pNotchFilter[i]->Filter(xt[i]);
}

//end of CHspecgMultiNotchFilter
