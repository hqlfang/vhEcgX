#include "stdafx.h"
#include "DoubleSampleRate.h"


///////////////////////////////////////////////////////////////////////////////
CDoubleSampleRate::CDoubleSampleRate()
{
	td=2;//Numbers=2;ReturnNumbers;
	ReturnValues[0]=ReturnValues[1]=0;
	Init(1);
}

CDoubleSampleRate::~CDoubleSampleRate()
{
//	if(ReturnValues) delete[]ReturnValues;
//	ReturnValues=NULL;
}

void CDoubleSampleRate::Init(double uVpb)
{
	y0=y1=y2=y3=0;
	y1t=y2t=y3t=0;
	ReturnValues[0]=ReturnValues[1]=0;
	judge=15/uVpb;	judge1=240/uVpb;
}

void CDoubleSampleRate::CalculateReturnValues(short CurrentValue) //计算返回点值
{
	double y,temp;
	y3=CurrentValue;
	temp=double(y3-y1)/(td+td);
	y3t=12.*(y1-y2)/(td*td*td)+6.*(y1t+temp)/(td*td);
	y2t=-6.*(y1-y2)/(td*td)-2.*(2*y1t+temp)/td;
	y=y1;

	short dy1=y1-y0,dy2=y2-y1,dy3=y3-y2;
	if(dy1<0) dy1=-dy1;
	if(dy2<0) dy2=-dy2;
	if(dy3<0) dy3=-dy3;
//	if(dy1>judge && dy3>judge) {
	if((dy1<=judge && dy3<=judge && dy2>judge1) ||
		(dy1<=judge && dy2<=judge && dy3>judge1) ||
		(dy2<=judge && dy3<=judge && dy1>judge1))
	{
		y2=y3;
	}
	else {
		y+=(y1t+y2t/2+y3t/6);
	}
		y1t+=(y2t+y3t/2);
		y2t+=y3t;
		ReturnValues[0]=short(y);
	ReturnValues[1]=y2;
	y0=y1;  y1=y2;	y2=y3;
}
	
short CDoubleSampleRate::GetNumbers()
{
	return 2;
}
//		
//short *CDoubleSampleRate::GetData()
//{
//	return ReturnValues;
//}

short *CDoubleSampleRate::GetDoubleSampleRateData(short xn)
{
	CalculateReturnValues(xn);
	return ReturnValues;
}
////////////////////////////////////////////////////////////////
CMultiChannelDoubleSampleRate::CMultiChannelDoubleSampleRate(short chnum)
{
	m_chnum=chnum;
	m_pDoubleSampleRate=new CDoubleSampleRate[chnum];
	ReturnValues=new short *[chnum];
	YangReturnValues[0]=new short[chnum];
	YangReturnValues[1]=new short[chnum];
//	for(short i=0;i<chnum;i++) ReturnValues[i]=m_pDoubleSampleRate[i].GetData();
}

CMultiChannelDoubleSampleRate::~CMultiChannelDoubleSampleRate()
{
	delete[]YangReturnValues[1];delete[]YangReturnValues[0];
	delete[]ReturnValues;
	delete[]m_pDoubleSampleRate;
}
void CMultiChannelDoubleSampleRate::Init(double uVpb)
{
	for(short i=0;i<m_chnum;i++) m_pDoubleSampleRate[i].Init(uVpb);
}
	
short CMultiChannelDoubleSampleRate::GetNumbers()
{
	return 2;
}

short **CMultiChannelDoubleSampleRate::GetDoubleSampleRateData(short *xn)
{
	for(short i=0;i<m_chnum;i++) ReturnValues[i]=m_pDoubleSampleRate[i].GetDoubleSampleRateData(xn[i]);
	return ReturnValues;
}
short **CMultiChannelDoubleSampleRate::GetData()
{
	short i,j;
	for(i=0;i<m_chnum;i++) {
		for(j=0;j<2;j++) YangReturnValues[j][i]=ReturnValues[i][j];
	}
	return YangReturnValues;
}
