/*//////////////////////////////////////////////////
Du Xiaodong, VH Medical
1st Ver. 09-Jun-2009 
2013.12.25
QRS_Complex::QRS_Complex(int SampleRate,double Uvperbit): ECG_Base(SampleRate,Uvperbit)
length=0;OnSet=OffSet=0;
2014.06.04 hspecg,modify error for QTmax,QTmin in function void MultiLead_Templates::QTdiscrete(QTdiscretion &QTd)
2014.11.24 QT is unchanged by adjust OnOff[5]
*///////////////////////////////////////////////////

#include "stdafx.h"

//#define __MY__DOS_DEBUG__

#ifdef __MY__DOS_DEBUG__
#include <stdio.h>
#include <conio.h>
#endif

#include "ECGtempl.h"

///////////////////////////////////////////////////////////////////////////////////////////
ECG_Base::ECG_Base(int SampleRate,double Uvperbit)
{
	m_nSampleRate=SampleRate;
	m_dUvperbit=Uvperbit;

	MinimumWave=int(36/m_dUvperbit);  //30before 2010-4-30
 //   WaveAngle=22; //degree, (22d: 125mm/sec,20mm/mV 相当于 45degree 25mm/s 10mm/mV) //Y2015-12-27 move to Vales&Hills
    FlateAngle=5;  //degree (125mm/sec,20mm/mV 相当于 ?degree 25mm/s 10mm/mV)
}

int ECG_Base::TurnPoint(short *data,int l,int r,int sign)
//sign=+1: 求凸点, sign=-1: 求凹点
{
    int i,turnP;
    double slop,s,dif,maxV;
	int start=l;
	int end=r;
//	int margin=8*m_nSampleRate/1000;
//	while(end-start>2*margin) {
	    maxV=0.;
		turnP=start+(end-start)/2;
		slop= double((data[end]-data[start]))/double(end-start); //求直线斜率
		for(i=start;i<end;i++)      
		{
			s=data[start]+(i-start)*slop; //直线方程
			dif=(sign>0)? (data[i]-s):(s-data[i]); //信号与直线(+)差值     
			if(dif>maxV)              //最大差值点为所提取特征点位置
			{
				maxV=dif;
				turnP=i;
			}
		}
	return turnP;
}  //end of short point int	

/*
//added in 2017-3-30
int ECG_Base::TurnPoint(int *data,int l,int r,int sign)
//sign=+1: 求凸点, sign=-1: 求凹点
{
    int i,turnP;
    double slop,s,dif,maxV;
	int start=l;
	int end=r;
//	int margin=8*m_nSampleRate/1000;
//	while(end-start>2*margin) {
	    maxV=0.;
		turnP=start+(end-start)/2;
		slop= double((data[end]-data[start]))/double(end-start); //求直线斜率
		for(i=start;i<end;i++)      
		{
			s=data[start]+(i-start)*slop; //直线方程
			dif=(sign>0)? (data[i]-s):(s-data[i]); //信号与直线(+)差值     
			if(dif>maxV)              //最大差值点为所提取特征点位置
			{
				maxV=dif;
				turnP=i;
			}
		}
	return turnP;
}  //end of short point int	
*/

BOOL ECG_Base::Flat(short *data,int p,int w,int Angle)
{
	float x,y;
	float stdRate=float(tan(Angle*M_PI/180.));   //0.7-35d, 0.8-39d, 0.6-31d 0.9-42d 1.2-50d
	float xRate=125.f/m_nSampleRate;  //  125mm/sec
	float yRate=float((m_dUvperbit/1000.)*20.);   // 20mm/mV
	x=w*xRate;
	y=yRate*abs(data[p+w]-data[p]);
	if(y/x<stdRate) return TRUE;
	else return FALSE;
}

int ECG_Base::FlatestPoint(short *data,int p)   //最平处
{
	int w=16l*m_nSampleRate/1000;
	int difV,minV,minP;
	minP=p;
	minV=abs(data[p+w]-data[p])+abs(data[p]-data[p-w]);
	for(int i=p-w;i<p+w;i++) {
		difV=abs(data[i+w]-data[i])+abs(data[i]-data[i-w]);
		if(difV<minV) {
			minV=difV;
			minP=i;
		}
	}
	return minP;
}

void ECG_Base::Average(short *data,long len,short ms)
{
	int w=(ms/2)*m_nSampleRate/1000;
	int i,j,v;
	short *in;
	in=new short[len];
	for(i=0;i<len;i++) in[i]=data[i];
	for(i=1;i<w;i++) {
		v=0;
		for(j=0;j<2*i;j++) v+=in[j];
		data[i]=v/(2*i);
		v=0;
		for(j=0;j<2*i;j++) v+=in[len-1-j];
		data[len-1-i]=v/(2*i);
	}
	for(i=w;i<len-w;i++) {
		v=0;
		for(j=-w;j<w;j++) v+=in[i+j];
		data[i]=v/(2*w);
	}
	delete []in;
}

/////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
MultiLead_Templates::MultiLead_Templates(short ChNumber,short **MultiLeadData,short Length,short SampleRate,double Uvperbit)
: ECG_Base(SampleRate,Uvperbit)
{
	ChN=ChNumber;
	m_nLength=Length;
	Lead=new ECG_Template *[ChN];
	for(int i=0;i<ChN;i++) Lead[i]=new ECG_Template(MultiLeadData[i],Length,SampleRate,Uvperbit);

//	for(int i=0;i<ChN;i++) for(int k=0;k<Length;k++) Lead[i]->TemplateData[k]=short(Lead[i]->TemplateData[k]*Uvperbit);

	InitLead();
}

//MultiLead_Templates::MultiLead_Templates(Template *Temp,short AverageRR)
MultiLead_Templates::MultiLead_Templates(Template *Temp,ECG_Parameters *BeatsOutPut)
: ECG_Base(Temp->SampleRate,Temp->Uvperbit)
{
	ChN=Temp->ChN;
	m_nLength=Temp->Length;
	m_nCenter=Temp->Pos;
	m_nLeft=Temp->Left+10l*Temp->SampleRate/1000;
	m_nRight=Temp->Right-10l*Temp->SampleRate/1000;
	Beats_OutPut=BeatsOutPut;
	msRR=Beats_OutPut->TemplateRR;    //AverageRR;
//	msRR=Temp->msRR;
	Lead=new ECG_Template *[ChN];
	for(int i=0;i<ChN;i++) Lead[i]=new ECG_Template(Temp->Data[i],Temp->Length,Temp->SampleRate,Temp->Uvperbit);
	InitLead();
}

MultiLead_Templates::~MultiLead_Templates()
{
	for(int i=0;i<ChN;i++) delete Lead[i];
	delete []Lead;
}

void MultiLead_Templates::InitLead()
{
	int i;
	if (ChN >= 12) {
		I = Lead[0];    II = Lead[1];   III = Lead[2];
		aVR = Lead[3];  aVL = Lead[4];  aVF = Lead[5];
		V1 = Lead[6];   V2 = Lead[7];   V3 = Lead[8];
		V4 = Lead[9];   V5 = Lead[10];  V6 = Lead[11];
		if (ChN >= 15) {
			i = ChN - 3;
			X = Lead[i++];  Y = Lead[i++];  Z = Lead[i++];
		}
	}
	else if (ChN == 3 || ChN == 2) {
		I = II = III = aVR = aVL = aVF = V1 = V2 = V3 = V4 = V5 = V6 = NULL;
		if (ChN == 3) { X = Lead[0];  Y = Lead[1];  Z = Lead[2]; }
		else { I = Lead[0]; II = Lead[1]; X = Y = Z = NULL; }
	}
	else if (ChN >= 6 || ChN<12) {
		I = Lead[0];    II = Lead[1];   III = Lead[2];
		aVR = Lead[3];  aVL = Lead[4];  aVF = Lead[5];
		V1 = V2 = V3 = V4 = V5 = V6 = NULL;
		X = Y = Z = NULL;
	}
}

int MultiLead_Templates::FindMaxPch(int ChFound)
{
	int MaxV=0;
    int MaxLead=-1;
	int tempV;
	for(int i=0;i<ChN;i++) {
		if(Lead[i]->P->Status>0) {
			tempV=Max(abs(Lead[i]->uvValue(Pa1)),abs(Lead[i]->uvValue(Pa2)));
			if(tempV>MaxV && MaxLead!=ChFound) {
				MaxV=tempV;
				MaxLead=i;
			}
		}
	}
	return MaxLead;
}

int MultiLead_Templates::FindMaxTch(int Ch1,int Ch2)
{
	int MaxV=0;
    int MaxLead=-1;
	int tempV;
	for(int i=0;i<ChN;i++) {
		if(Lead[i]->T->Status>0) {
			tempV=Max(abs(Lead[i]->uvValue(Ta1)),abs(Lead[i]->uvValue(Ta2)));
			if(tempV>MaxV && MaxLead!=Ch1 && MaxLead!=Ch2) {
				MaxV=tempV;
				MaxLead=i;
			}
		}
	}
	return MaxLead;
}

void MultiLead_Templates::SetOnOff(bool Auto)
{
	int i,j;
	if(Auto) {  //2017-1
		for(i=0;i<ChN;i++) for(j=0;j<6;j++) Lead[i]->OnOff[j]=0;
		for(i=0;i<ChN;i++) {
			if(Lead[i]->P->Status>0 && Lead[i]->P->OffSet>Lead[i]->P->OnSet) {
				Lead[i]->OnOff[0]=Lead[i]->P->OnSet+Lead[i]->Pstart;
				Lead[i]->OnOff[1]=Lead[i]->P->OffSet+Lead[i]->Pstart;
			}
			if(Lead[i]->QRS->OnSet>=0 && Lead[i]->QRS->OffSet-Lead[i]->QRS->OnSet>0) {
				Lead[i]->OnOff[2]=Lead[i]->QRS->OnSet+Lead[i]->QRSstart;
				Lead[i]->OnOff[3]=Lead[i]->QRS->OffSet+Lead[i]->QRSstart;
			}
			if(Lead[i]->T->Status>0 && Lead[i]->T->OffSet>Lead[i]->T->OnSet) {
				Lead[i]->OnOff[4]=Lead[i]->T->OnSet+Lead[i]->Tstart;
        		Lead[i]->OnOff[5]=Lead[i]->T->OffSet+Lead[i]->Tstart;
			}
		}
	}
	for(i=0;i<ChN;i++) for(j=0;j<6;j++) Lead[i]->AvantOnOff[j]=Lead[i]->OnOff[j]; //2017-1
///////////////////////////////////////////////////////////Y2015 above
	for(j=0;j<6;j++) OnOff[j]=0;

// modified at 2010/6/24
//	int t;
	for(j=0;j<5;j+=2) {   //0,2,4 for On
		int n=0;
		for(i=0;i<ChN;i++) if(Lead[i]->OnOff[j]!=0) n++;
		if(n>3) {
			int *temp=new int[n];
			int t=0;
			for(i=0;i<ChN;i++) if(Lead[i]->OnOff[j]!=0) temp[t++]=Lead[i]->OnOff[j];
			BubbleSort(temp,n);
//			OnOff[j]=(temp[0]+temp[1]+temp[2])/3;
			OnOff[j]=(temp[1]+temp[2])/2;
			delete []temp;
		}
		else if(n>0) {
			for(i=0;i<ChN;i++) if(Lead[i]->OnOff[j]!=0) OnOff[j]+=Lead[i]->OnOff[j];
			OnOff[j]/=n;
		}
	}
	for(j=1;j<6;j+=2) {   //1,3,5 for Off
		int n=0;
		for(i=0;i<ChN;i++) if(Lead[i]->OnOff[j]!=0) n++;
		if(n>3) {
			int *temp=new int[n];
			int t=0;
			for(i=0;i<ChN;i++) if(Lead[i]->OnOff[j]!=0) temp[t++]=Lead[i]->OnOff[j];
			BubbleSort(temp,n);
//			OnOff[j]=(temp[n-1]+temp[n-2]+temp[n-3])/3;
			if(n==4) OnOff[j]=(temp[n-2]+temp[n-3])/2;
			else OnOff[j]=(temp[n-2]+temp[n-3]+temp[n-4])/3;
//			for(int k=1;k<n-1;k++) OnOff[j]+=temp[k];
//			OnOff[j]/=(n-2);
			delete []temp;
		}
		else if(n>0) {
			for(i=0;i<ChN;i++) if(Lead[i]->OnOff[j]!=0) OnOff[j]+=Lead[i]->OnOff[j];
			OnOff[j]/=n;
		}
	}
}

int MultiLead_Templates::uvRV5()
{
	if(V5->QRS->R1[0]<=0 && V5->QRS->R2[0]<=0) return 0;
	if(V5->QRS->R1[0]>0 && V5->QRS->R2[0]<=0) return int(V5->QRS->Data[V5->QRS->R1[2]]*m_dUvperbit);
	if(V5->QRS->Data[V5->QRS->R1[2]]>V5->QRS->Data[V5->QRS->R2[2]]) return int(V5->QRS->Data[V5->QRS->R1[2]]*m_dUvperbit);
	else return int(V5->QRS->Data[V5->QRS->R2[2]]*m_dUvperbit);
}

int MultiLead_Templates::uvRV6()
{
	if(V6->QRS->R1[0]<=0 && V6->QRS->R2[0]<=0) return 0;
	if(V6->QRS->R1[0]>0 && V6->QRS->R2[0]<=0) return int(V6->QRS->Data[V6->QRS->R1[2]]*m_dUvperbit);
	if(V6->QRS->Data[V6->QRS->R1[2]]>V6->QRS->Data[V6->QRS->R2[2]]) return int(V6->QRS->Data[V6->QRS->R1[2]]*m_dUvperbit);
	else return int(V6->QRS->Data[V6->QRS->R2[2]]*m_dUvperbit);
}

int MultiLead_Templates::uvSV1()
{
	if(V1->QRS->S1[0]<=0 && V1->QRS->S2[0]<=0) return 0;
	if(V1->QRS->S1[0]>0 && V1->QRS->S2[0]<=0) return int(-V1->QRS->Data[V1->QRS->S1[2]]*m_dUvperbit);
	if(V1->QRS->Data[V1->QRS->S1[2]]<V1->QRS->Data[V1->QRS->S2[2]]) return int(-V1->QRS->Data[V1->QRS->S1[2]]*m_dUvperbit);
	else return int(-V1->QRS->Data[V1->QRS->S2[2]]*m_dUvperbit);
}

int MultiLead_Templates::uvSV2()
{
	if(V2->QRS->S1[0]<=0 && V2->QRS->S2[0]<=0) return 0;
	if(V2->QRS->S1[0]>0 && V2->QRS->S2[0]<=0) return int(-V2->QRS->Data[V2->QRS->S1[2]]*m_dUvperbit);
	if(V2->QRS->Data[V2->QRS->S1[2]]<V2->QRS->Data[V2->QRS->S2[2]]) return int(-V2->QRS->Data[V2->QRS->S1[2]]*m_dUvperbit);
	else return int(-V2->QRS->Data[V2->QRS->S2[2]]*m_dUvperbit);
}

int MultiLead_Templates::msInterval(COMMEN_INTERVAL_TYPE Type)
{
	switch(Type) {
	case cPd: 
		if(OnOff[0]>0 && OnOff[1]>0) return (OnOff[1]-OnOff[0])*1000l/m_nSampleRate;
	case cPRd:
		if(OnOff[2]>0 && OnOff[0]>0) return (OnOff[2]-OnOff[0])*1000l/m_nSampleRate;
		break;
	case cQRSd:
		return (OnOff[3]-OnOff[2])*1000l/m_nSampleRate;
	case cQTd:
/*
		int r=0;
		if((ChN==12||ChN==15) && !(V1->OnOff[5]==0 && V2->OnOff[5]==0 && V3->OnOff[5]==0)) 
			r=Max(Max(V1->OnOff[5],V2->OnOff[5]),V3->OnOff[5]);
		else r=OnOff[5];   //hspecg 2014.11.24
		if(OnOff[2]>0 && r>OnOff[2]) return (r-OnOff[2])*1000l/m_nSampleRate;
		break;
*/
		int count=0;   //2017-12-18
		for(int i=0;i<ChN;i++) if(Lead[i]->T->AbV>0) count++;
		unsigned short maxT1=0;
		unsigned short maxT2=0;
		int offT1=0;
		int offT2=0;
		int LeadNo=0;
		if(count>1) {
			for(int i=0;i<ChN;i++) {
				if(Lead[i]->T->AbV>maxT1) {
					maxT1=Lead[i]->T->AbV;
					offT1=Lead[i]->OnOff[5];
					LeadNo=i;
				}
			}
			Lead[LeadNo]->T->AbV=0;
			for(int i=0;i<ChN;i++) {
				if(Lead[i]->T->AbV>maxT2) {
					maxT2=Lead[i]->T->AbV;
					offT2=Lead[i]->OnOff[5];
				}
			}
			offT1=(offT1+offT2)/2;
			Lead[LeadNo]->T->AbV=maxT1;  
		}
		else offT1=OnOff[5];   //hspecg 2014.11.24
		if(OnOff[2]>0 && offT1>OnOff[2]) return (offT1-OnOff[2])*1000l/m_nSampleRate;
		break;
	}
	return 0;
}

int MultiLead_Templates::msQTc(int msQTd) 
{
	return int(msQTd*sqrt(1000.)/sqrt((double)msRR));
}

void MultiLead_Templates::QTdiscrete(QTdiscretion &QTd)
{
	int i,t;
	QTd.msMax=0;
	QTd.msMin=m_nLength;
	for(i=0;i<ChN;i++) {
		if(Lead[i]->QRS->OnSet>0 && Lead[i]->T->Status>0) {
//			t=Lead[i]->T->OffSet-Lead[i]->QRS->OnSet;
			if(Lead[i]->OnOff[5]>Lead[i]->OnOff[2] && Lead[i]->OnOff[2]>0) {//2014.06.04	by hspecg
				t=Lead[i]->OnOff[5]-Lead[i]->OnOff[2];
				if(t>QTd.msMax) { QTd.MaxLeadNo=i;  QTd.msMax=t;  }
				if(t<QTd.msMin) { QTd.MinLeadNo=i;  QTd.msMin=t;   }
			}
		}
	}
	if(QTd.msMax!=0 && QTd.msMin!=m_nLength) {
		QTd.msMax=short(QTd.msMax*(1000L/m_nSampleRate));   //ms
		QTd.msMin=short(QTd.msMin*(1000L/m_nSampleRate));   //ms
		QTd.msQTd=QTd.msMax-QTd.msMin;     //ms
	}
	else QTd.msQTd=-1;  //failed to get QTD
}
//
//int MultiLead_Templates::Axis(WAVE_TYPE Type)
//{
//	if(ChN<6) return 0;
//
//	int i;
//	int Value1=0;
//	int Value3=0;
//	switch(Type) {
//	case Pwave:
//		if(I->P->Status>0) for(i=I->P->OnSet;i<I->P->OffSet;i++) Value1+=I->P->Data[i];
//		if(III->P->Status>0) for(i=III->P->OnSet;i<III->P->OffSet;i++) Value3+=III->P->Data[i];
//		break;
//	case QRScomplex:
//		if(I->QRS->OnSet>0) for(i=I->QRS->OnSet;i<I->QRS->OffSet;i++) Value1+=I->QRS->Data[i];
//		if(III->QRS->OnSet>0) for(i=III->QRS->OnSet;i<III->QRS->OffSet;i++) Value3+=III->QRS->Data[i];
//		break;
//	case Twave:	
//		if(I->T->Status>0) for(i=I->T->OnSet;i<I->T->OffSet;i++) Value1+=I->T->Data[i];
//		if(III->T->Status>0) for(i=III->T->OnSet;i<III->T->OffSet;i++) Value3+=III->T->Data[i];
//		break;
//	}
//	return Axis(Value1, Value3);
////deleted by Hu 2011.04.02
//    float a=0;
//    if(Value1==0) {
//        if(Value3==0) return 0;
//        else if(Value3<0) return -90;
//        else return 90;
//    }
//    if(Value1>0) {
//	if(Value3>0) a=(float)(atan((Value1/2.+Value3)/(Value1*cos(M_PI/6)))*180/M_PI);
//        else a=(float)(atan((Value1/2.-Value3)/(Value1*cos(M_PI/6)))*180/M_PI);
//    }
//    else {
//       if(Value3>0) a=180-(float)(atan((Value3-Value1/2.)/(Value1*cos(M_PI/6)))*180/M_PI);
//       else a=180+(float)(atan((Value1/2.+Value3)/(Value1*cos(M_PI/6)))*180/M_PI);
//    }
////added by Hu 2011.04.02
//	double vI,vIII,a;
//    vI=abs(Value1);
//    vIII=abs(Value3);
//
//    if(Value1==0) {
//        if(Value3==0) return 0;
//        else if(Value3<0) return -90;
//        else return 90;
//    }
//
//	double sqrt3d3=tan(M_PI/6);
//	double vIII2=2*vIII;
//
//	if(Value1>0) {
//		if(Value3<=0) {
//			if(vI<=vIII2) a=-atan(sqrt3d3*(vIII2/vI-1));	//0 ~ -90
//			else if(vI>vIII2) a=atan(sqrt3d3*(1-vIII2/vI));	//0 ~ 60
//		}
//		else if(Value3>0) {
//			a=atan(sqrt3d3*(vIII2/vI+1));					//60 ~ 90
//		}
//	}
//	else {
//		if(Value3>=0) {
//			a=M_PI-atan(sqrt3d3*(vIII2/vI-1));			//90 ~ 180
//		}
//		else if(Value3<0) {
//			a=M_PI+atan(sqrt3d3*(vIII2/vI+1));			//180 ~ 270
//		}
//	}
//
//	a=a*180/M_PI+((a>=0)?0.5:-0.5);
//	return int(a);
//}
/*
int MultiLead_Templates::Axis(WAVE_TYPE Type)
{
	if (ChN < 6) return 0;

	int i, on, off;
	int Value1 = 0, Value3 = 0;
	int maxV = 0, minV = 0;
	switch (Type) {
	case Pwave:
		if (I->P->Status>0) {
			on = I->P->OnSet;	off = I->P->OffSet;
		}
		else {
			on = I->P->GetLength() / 5;	off = I->P->GetLength() * 4 / 5;
		}
		if (off>on && on>0) {
			maxV = minV = 0;
			for (i = on; i < off; i++) {//Value1 += I->P->Data[i];
				if (maxV < I->P->Data[i]) maxV = I->P->Data[i];
				if (minV > I->P->Data[i]) minV = I->P->Data[i];
			}
			Value1 = maxV + minV;
		}
		if (III->P->Status > 0) {
			on = III->P->OnSet;	off = III->P->OffSet;
		}
		else {
			on = III->P->GetLength() / 5;	off = III->P->GetLength() * 4 / 5;
		}
		if (off>on && on>0) {
			maxV = minV = 0;
			for (i = on; i < off; i++) {//Value3 += III->P->Data[i];
				if (maxV < III->P->Data[i]) maxV = III->P->Data[i];
				if (minV > III->P->Data[i]) minV = III->P->Data[i];
			}
			Value3 = maxV + minV;
		}
		break;
	case QRScomplex:
		if (I->QRS->OnSet > 0) {
			on = I->QRS->OnSet;	off = I->QRS->OffSet;
		}
		else {
			on = I->QRS->GetLength() / 5;	off = I->QRS->GetLength() * 4 / 5;
		}
		if (off>on && on>0) {
			maxV = minV = 0;
			for (i = on; i < off; i++) {//Value1 += I->QRS->Data[i];
				if (maxV < I->QRS->Data[i]) maxV = I->QRS->Data[i];
				if (minV > I->QRS->Data[i]) minV = I->QRS->Data[i];
			}
			Value1 = maxV + minV;
		}
		if (III->QRS->OnSet > 0) {
			on = III->QRS->OnSet;	off = III->QRS->OffSet;
		}
		else {
			on = III->QRS->GetLength() / 5;	off = III->QRS->GetLength() * 4 / 5;
		}
		if (off>on && on>0) {
			maxV = minV = 0;
			for (i = on; i < off; i++) {//Value3 += III->QRS->Data[i];
				if (maxV < III->QRS->Data[i]) maxV = III->QRS->Data[i];
				if (minV > III->QRS->Data[i]) minV = III->QRS->Data[i];
			}
			Value3 = maxV + minV;
		}
		break;
	case Twave:
		if (I->T->Status > 0) {
			on = I->T->OnSet;	off = I->T->OffSet;
		}
		else {
			on = I->T->GetLength() / 5;	off = I->T->GetLength() * 4 / 5;
		}
		if (off>on && on>0) {
			maxV = minV = 0;
			for (i = on; i < off; i++) {//Value1 += I->T->Data[i];
				if (maxV < I->T->Data[i]) maxV = I->T->Data[i];
				if (minV > I->T->Data[i]) minV = I->T->Data[i];
			}
			Value1 = maxV + minV;
		}
		if (III->T->Status > 0) {
			on = III->T->OnSet;	off = III->T->OffSet;
		}
		else {
			on = III->T->GetLength() / 5;	off = III->T->GetLength() * 4 / 5;
		}
		if (off>on && on>0) {
			maxV = minV = 0;
			for (i = on; i < off; i++) {//Value3 += III->T->Data[i];
				if (maxV < III->T->Data[i]) maxV = III->T->Data[i];
				if (minV > III->T->Data[i]) minV = III->T->Data[i];
			}
			Value3 = maxV + minV;
		}
		break;
	}
	return Axis(Value1, Value3);
}
*/
int MultiLead_Templates::Axis(WAVE_TYPE Type)
{
	if (ChN < 6) return 0;

	int i, on=0, off=0;
	int Value1 = 0, Value3 = 0;
	int maxV = 0, minV = 0;
	switch (Type) {
	case Pwave:
		if (OnOff[0]>0 && OnOff[1] > OnOff[0]) {
			on = OnOff[0];	off = OnOff[1];
		}
		break;
	case QRScomplex:
		if (OnOff[2]>0 && OnOff[3] > OnOff[2]) {
			on = OnOff[2];	off = OnOff[3];
		}
		break;
	case Twave:
		if (OnOff[4]>0 && OnOff[5] > OnOff[4]) {
			on = OnOff[4];	off = OnOff[5];
		}
		break;
	}
	if (off>on && on>0) {
		maxV = minV = 0;
		for (i = on; i < off; i++) {//Value1 += I->P->Data[i];
			if (maxV < I->TemplateData[i]) maxV = I->TemplateData[i];
			if (minV > I->TemplateData[i]) minV = I->TemplateData[i];
		}
		Value1 = maxV + minV;
		maxV = minV = 0;
		for (i = on; i < off; i++) {//Value3 += III->P->Data[i];
			if (maxV < III->TemplateData[i]) maxV = III->TemplateData[i];
			if (minV > III->TemplateData[i]) minV = III->TemplateData[i];
		}
		Value3 = maxV + minV;
	}
	return Axis(Value1, Value3);
}
int MultiLead_Templates::Axis(double vI, double vIII)
{
	double a=0,VI = fabs(vI), VIII = fabs(vIII);
	if (vI == 0) {
		if (vIII > 0) return 90;
		else if (vIII<0) return -90;
		else return 0;
	}

	double tan30 = tan(M_PI / 6);
	if (vI > 0) {
		if (vIII < 0) {
			if (VIII < VI / 2) {	//0 ~ +60
				a = atan((1 - 2 * VIII / VI)*tan30);
			}
			else if (VIII < 2 * VI) {	//0 ~ -60
				a = -atan((2 * VIII / VI - 1)*tan30);
			}
			else {	//-60 ~ -90
				a = -(atan((1 - 2 * VI / VIII)*tan30) + M_PI / 3);
			}
		}
		else if(VIII > 0) {
			a = 2 * M_PI / 3 - atan((1 + 2 * VI / VIII)*tan30);			//60 ~ 90
		}
		else a = -M_PI/2;
	}
	else {
		if (vIII < 0) {		//-90 ~ -150
			a = -(M_PI / 3 + atan((1 + 2 * VI / VIII)*tan30));			
		}
		else if (vIII>0) {
			if (VIII < VI / 2) {	//-150 ~ -180
				a = -(M_PI - atan((1 - 2 * VIII / VI)*tan30));
			}
			else if (VIII < 2 * VI) {	//90 ~ 120
				a = 2 * M_PI / 3 - atan((1 - 2 * VI / VIII)*tan30);
			}
			else {	//120 ~ 180
				a = M_PI - atan((2 * VIII / VI - 1)*tan30);
			}
		}
		else a = M_PI / 2;
	}

	a = a * 180 / M_PI + ((a >= 0) ? 0.5 : -0.5);
	return int(a);
}

char MultiLead_Templates::WPW()
{
	int i,count=0;
	for(i=0;i<ChN;i++) if(Lead[i]->QRS->Delta()) count++;
	BOOL WPWc=FALSE;
	int pAxis,uvPa,msPR;
	int pArea1,nArea1,Area2;
	BOOL QRSc1,QRSc2,QRSc3;
	if(count>=2) {
		QRSc1=QRSc2=QRSc3=FALSE;
		pAxis=Axis(Pwave);
		uvPa=aVF->uvValue(Pa1)+aVF->uvValue(Pa2);
		msPR=msInterval(cPRd);
		pArea1=nArea1=0;
		if(V1->QRS->OnSet>0) {
			for(i=V1->QRS->OnSet;i<V1->QRS->OffSet;i++) {
				if(V1->QRS->Data[i]>0) pArea1+=V1->QRS->Data[i];
				else nArea1+=abs(V1->QRS->Data[i]);
			}
		}
		if(pArea1>nArea1 && msPR<=180) QRSc1=TRUE;
		if(pArea1<nArea1 && msPR<=140) QRSc3=TRUE;
		Area2=0;
		if(V2->QRS->OnSet>0) {
			for(i=V2->QRS->OnSet;i<V2->QRS->OffSet;i++) Area2+=V2->QRS->Data[i];
		}
		if(Area2>0 && msPR<=160) QRSc2=TRUE;
		if(msPR>0 && pAxis>-40 && pAxis<160 && uvPa>-50 && (QRSc1||QRSc2||QRSc3)) WPWc=TRUE;   //pAxis -30 to 120
    }  
	if(WPWc) {  // 小写：可疑
		if(pArea1>0.8*(pArea1+nArea1)) {
			if(count>2) return 'A';
			else return 'a';
		}
		if(nArea1>0.8*(pArea1+nArea1)) {
			if(count>2) return 'B';
			else return 'b';
		}
		if(count>2) return 'W';
		else return 'w';
	}
	return ' ';
}

void MultiLead_Templates::AflutAfibCorrect()  //Y2015-7-6 correct AflutAfib (by beats) depends on 12-lead templates
{                      
	short CorrectAF=abs(Beats_OutPut->AflutAfib);
	short Arate=Beats_OutPut->Arate;
	short Vrate=Beats_OutPut->Vrate;
	short MaxRR=Beats_OutPut->MaxRR;
	short MinRR=Beats_OutPut->MinRR;
	short AveRR=Beats_OutPut->AverageRR;

//	short pdCount,pCount;
	if(Beats_OutPut->AflutAfib==0) {
		short pdCount=0;
		if(Arate==0 || (Arate!=0 && Arate<3*Vrate/4)) {   //  2/3
			for(int i=0;i<ChN;i++) if(Lead[i]->P->Status!=0) pdCount++;
			if(pdCount==0 && MaxRR-MinRR>MinRR/5 && HR()>24 && HR()<160) CorrectAF=2;  //Afib
		} 
	}
	else {
		short pCount=0;
		for(int i=0;i<ChN;i++) 
			if(Lead[i]->P->Status!=0 && Lead[i]->msInterval(Pd)>40 && 
				Max(abs(Lead[i]->uvValue(Pa1)),abs(Lead[i]->uvValue(Pa2)))>40) pCount++;
		if(pCount>6) {  
			int *PRds=new int[pCount];
			int nPR=0;
			for(int i=0;i<ChN;i++) if(Lead[i]->P->Status!=0 && Lead[i]->msInterval(Pd)>40 && 
				Max(abs(Lead[i]->uvValue(Pa1)),abs(Lead[i]->uvValue(Pa2)))>40) PRds[nPR++]=Lead[i]->msInterval(PRd);
			BubbleSort(PRds,pCount);
			int mid=pCount/2;
			if(Beats_OutPut->AflutAfib==1 && pCount>1-ChN/3 && PRds[mid]<120) {//hspecg 对房扑严格条件 2016-07-01
				if(PRds[mid+1]-PRds[mid-3]<12 && PRds[mid+2]-PRds[mid-2]<12 && PRds[mid+3]-PRds[mid-1]<12) CorrectAF=0;//FibFlut=FALSE;
				else if(Arate==Vrate && PRds[mid+1]-PRds[mid-3]<16 && PRds[mid+2]-PRds[mid-1]<16 ) CorrectAF=0;//FibFlut=FALSE;
			}
			if(Beats_OutPut->AflutAfib==2) {
				if(PRds[mid+1]-PRds[mid-2]<12 && PRds[mid+2]-PRds[mid-2]<12 && PRds[mid+2]-PRds[mid-1]<12 &&
					PRds[mid+2]-PRds[mid-3]<36 && PRds[mid+3]-PRds[mid-2]<36) CorrectAF=0;  //FibFlut=FALSE;
				else if(Arate==Vrate && PRds[mid+1]-PRds[mid-3]<16 && PRds[mid+2]-PRds[mid-1]<16 ) CorrectAF=0;  //FibFlut=FALSE;
			}
			if (Beats_OutPut->AflutAfib==-2) { 
				if(PRds[mid+1]-PRds[mid-2]<16 && PRds[mid+2]-PRds[mid-2]<16 && PRds[mid+2]-PRds[mid-1]<16) CorrectAF=0;//FibFlut=FALSE;
				else if(Arate==Vrate && PRds[mid+1]-PRds[mid-3]<20 && PRds[mid+2]-PRds[mid-1]<20 ) CorrectAF=0;  //FibFlut=FALSE;
			}  
			if(CorrectAF!=0 && pCount>8 && 
				12*(MaxRR-MinRR)<AveRR && 30*(MaxRR-AveRR)<AveRR && 25*(AveRR-MinRR)<AveRR) {
				if(PRds[mid+4]-PRds[mid-4]<60 && PRds[mid+3]-PRds[mid-3]<35 && 
					PRds[mid+3]-PRds[mid-1]<20 && PRds[mid+1]-PRds[mid-3]<25 &&
					PRds[mid+2]-PRds[mid-2]<20 && PRds[mid+1]-PRds[mid-1]<10) CorrectAF=0;    //FibFlut=FALSE;
			}
			if(CorrectAF!=0 && pCount>9 && AveRR>1000 &&
				10*(MaxRR-MinRR)<AveRR && 20*(MaxRR-AveRR)<AveRR && 20*(AveRR-MinRR)<AveRR) {
				if(PRds[mid+4]-PRds[mid-4]<60 && PRds[mid+3]-PRds[mid-3]<40 && 
					PRds[mid+3]-PRds[mid-1]<30 && PRds[mid+1]-PRds[mid-3]<30 &&
					PRds[mid+2]-PRds[mid-2]<25 && PRds[mid+1]-PRds[mid-1]<15) CorrectAF=0;    //FibFlut=FALSE;
			}
			delete []PRds; 
		} 
	}

	if(CorrectAF!=0) {
		for(int i=0;i<ChN;i++) Lead[i]->P->Status=0;    //Templates P波相关参数置零
		Beats_OutPut->Arate=0;							//Beats P波相关参数置零
		for(int i=1;i<Beats_OutPut->BeatsNum;i++) {
			(Beats_OutPut->Beats+i)->Pdir=0;
			(Beats_OutPut->Beats+i)->Pnum=0;
			(Beats_OutPut->Beats+i)->PR=0;
		}
	}

	Beats_OutPut->AflutAfib=CorrectAF;  
}

void MultiLead_Templates::AutoAnalysis()
{
//	msRR=nRR*1000l/m_nSampleRate;
	int i;

//	BOOL QRSok;
////QRS analysis

	int QRSstart=m_nCenter;
	int QRSend=m_nCenter;

	int aveQRSstart=0;
	int aveQRSend=0;
	int aveN=0;
/* test part
int *Da=new int[m_nLength];
for(int k=0;k<m_nLength;k++) {
	Da[k]=Lead[3]->TemplateData[k];
	Lead[3]->TemplateData[k]=Lead[6]->TemplateData[k];
}
*/
	for(i=0;i<ChN;i++) {
	    Lead[i]->QRS_Location(m_nCenter);
//		if(SelQRSok) {
//			Lead[i]->QRSstart=(3*SelStart+2*Lead[i]->QRSstart)/5;
//			Lead[i]->QRSend=(3*SelEnd+2*Lead[i]->QRSend)/5;
//		}
        //above two lines added in 2010-7-6
		if(Lead[i]->QRSok) {
//			if(Lead[i]->QRSstart<QRSstart) QRSstart=Lead[i]->QRSstart;
//			if(Lead[i]->QRSend>QRSend) QRSend=Lead[i]->QRSend;
			aveQRSstart+=Lead[i]->QRSstart;
			aveQRSend+=Lead[i]->QRSend;
			aveN++;
		}
	}

//for(int k=0;k<m_nLength;k++) Lead[3]->TemplateData[k]=Da[k];
//delete []Da;  test part

	if(aveN>0) {
		aveQRSstart/=aveN;
		aveQRSend/=aveN;
	}
	for(i=0;i<ChN;i++) {
		if(Lead[i]->QRSok) {
			Lead[i]->QRSstart=(3*aveQRSstart+2*Lead[i]->QRSstart)/5;
			Lead[i]->QRSend=(3*aveQRSend+2*Lead[i]->QRSend)/5;
		}
	}

	for(i=0;i<ChN;i++) {
		if(Lead[i]->QRSok) {
			if(Lead[i]->QRSstart<QRSstart) QRSstart=Lead[i]->QRSstart;
			if(Lead[i]->QRSend>QRSend) QRSend=Lead[i]->QRSend;
		}
	}

	if(QRSstart>=QRSend) {  //without QRS in all leads
		QRSstart-=(80l*m_nSampleRate/1000);
		QRSend+=(80l*m_nSampleRate/1000);
	}

	int w=20l*m_nSampleRate/1000;     
//	int ZeroV;
	for(i=0;i<ChN;i++) {
		Lead[i]->QRS=new QRS_Complex(m_nSampleRate,m_dUvperbit);
		if(Lead[i]->QRSok) {
			Lead[i]->QRSstart=FlatestPoint(Lead[i]->TemplateData,Lead[i]->QRSstart+w);
			Lead[i]->QRSend=FlatestPoint(Lead[i]->TemplateData,Lead[i]->QRSend-w);
		}
		else {
			Lead[i]->QRSstart=FlatestPoint(Lead[i]->TemplateData,QRSstart+w);
			Lead[i]->QRSend=FlatestPoint(Lead[i]->TemplateData,QRSend-w);  //?/
		}
//		Lead[i]->ZeroV=0;
//		int ZeroV=0;
//		for(int j=-w/4;j<w/4;j++) ZeroV+=Lead[i]->TemplateData[Lead[i]->QRSstart+j]; 
//	    ZeroV/=(w/2);
//		for(int j=0;j<m_nLength;j++) Lead[i]->TemplateData[j]-=ZeroV;//	Lead[i]->ZeroV=0;//added by hspecg 2011.04.02 !!!
//	}
//	Lead[6]->QRSstart=Lead[9]->QRSstart;   // 2017-3-2
//	for(i=0;i<ChN;i++) {
		if(Lead[i]->QRSok) {
			Lead[i]->QRS->SetData(Lead[i]->TemplateData,Lead[i]->QRSstart,Lead[i]->QRSend);  //Lead[i]->ZeroV,
			Lead[i]->QRS->Analysis(); 
		}
	}

//	QRS_OnOff_Correct();  

/////////////////////////////////////////////  end of QRS analysis

// 新改动
	int PR_d=msRR<1000? 300l*m_nSampleRate/1000 : 350l*m_nSampleRate/1000;
	
	for(i=0;i<ChN;i++) {   //New 2016-4-28
		int pLeft=(2*msRR/5)*m_nSampleRate/1000;
		if(Lead[i]->QRS->OnSet>0) Lead[i]->Pstart=Max(m_nLeft,Lead[i]->QRS->OnSet-pLeft+Lead[i]->QRSstart);
		else Lead[i]->Pstart=m_nLeft;       

		if(Lead[i]->QRS->OnSet>0) 
			Lead[i]->Pend=Min(Lead[i]->QRS->OnSet+Lead[i]->QRSstart,Lead[i]->Pstart+300l*m_nSampleRate/1000);
//			Lead[i]->Pend=Lead[i]->QRS->OnSet+Lead[i]->QRSstart;
		else if(Lead[i]->QRSok) Lead[i]->Pend=Lead[i]->QRSstart;
		else Lead[i]->Pend=Lead[i]->Pstart+PR_d;
//		int P_Pos=Lead[i]->P_Range(&Lead[i]->TemplateData[Lead[i]->Pstart],Lead[i]->Pend-Lead[i]->Pstart);
//		int hPw=100l*m_nSampleRate/1000;
//		if(Lead[i]->Pstart+P_Pos+hPw<Lead[i]->Pend) Lead[i]->Pend=Lead[i]->Pstart+P_Pos+hPw;
//		if(P_Pos-hPw>0) Lead[i]->Pstart+=(P_Pos-hPw);  // Lead[i]->Pstart=m_nLeft;
		Lead[i]->P=new PT_Wave('P',m_nSampleRate,m_dUvperbit);  //without P
		Lead[i]->P->SetData(Lead[i]->TemplateData,Lead[i]->Pstart,Lead[i]->Pend);
		Lead[i]->P->PwaveAnalysis(); 
	}

	P_Added_Depends();
	if(ChN>=8) 	P_OnOff_Correct();  //2016-10-10

/*	if(ChN>=8) {    //2016-10-10 deleted
		bool *LeadNo=new bool[ChN];
		int *StartP=new int[ChN];
		int *EndP=new int[ChN];
		for(i=0;i<ChN;i++) LeadNo[i]=FALSE;
		P_OnOff_Correct(LeadNo,StartP,EndP);
		for(i=0;i<ChN;i++) {
			if(LeadNo[i]) {
				Lead[i]->P->SetData(Lead[i]->TemplateData,StartP[i],EndP[i]);
				Lead[i]->P->PwaveAnalysis(); 
			}
		}
		delete []LeadNo;
		delete []StartP;
		delete []EndP;
	}
*/

// 考虑用P_OnOff_Correct 对需要的导联重新确定Pstart，Pend 再分析一遍P
//	for(i=0;i<ChN;i++) {   //New 2016-9-22
//		Lead[i]->P->SetData(Lead[i]->TemplateData,Lead[i]->Pstart,Lead[i]->Pend);
//		Lead[i]->P->PwaveAnalysis(); 
//	}

//	Delete_PR_TooShort();
//在此可以对数据显示操作
//for(i=Lead[0]->Pstart;i<Lead[0]->Pend;i++) Lead[0]->TemplateData[i]=200;

//// end of P analysis

//	Correct_Pwaves();
//	Correct_Positions();   //Y2015

//	可以在这里确定ZeroValue零点  //2016-4-6
	for(i=0;i<ChN;i++) {
		short zeroV=Lead[i]->CalculateZeroValue();
		for(int k=0;k<m_nLength;k++) Lead[i]->TemplateData[k]-=zeroV;
		Lead[i]->QRS->CorrectDependsOnZero();    //2017-3-2
	}

//// T analysis start   //2016/04/25  move T analysis after zeroV
	int AveOff=0;
	int ChCount=0;
	for(i=0;i<ChN;i++) {
		if(Lead[i]->QRS->OnSet>0) {
			ChCount++;
			AveOff+=(Lead[i]->QRS->OffSet+Lead[i]->QRSstart);
		}
	}
	if(ChCount>0) 	AveOff/=ChCount;
//above add 2017-3-22
	int wT=(100l*m_nSampleRate/1000);
	for(i=0;i<ChN;i++) {
		if(Lead[i]->QRS->OnSet>0) Lead[i]->Tstart=Max(Lead[i]->QRS->OffSet+Lead[i]->QRSstart,AveOff); //2017-3-22
//		if(Lead[i]->QRS->OnSet>0) Lead[i]->Tstart=Lead[i]->QRS->OffSet+Lead[i]->QRSstart;
		else if(Lead[i]->QRSok) Lead[i]->Tstart=Lead[i]->QRSend;
		else Lead[i]->Tstart=m_nCenter+(60l*m_nSampleRate/1000);
		if(m_nRight-Lead[i]->Tstart<wT) Lead[i]->Tstart=m_nRight-wT;
		Lead[i]->Tend=m_nRight;  //Lead[i]->Tstart+Twidth;  // 新改动
		if(msRR<450) Lead[i]->Tend-=(20l*m_nSampleRate/1000);   //HR>133
	}

	for(i=0;i<ChN;i++) {
		Lead[i]->T=new PT_Wave('T',m_nSampleRate,m_dUvperbit);
	    Lead[i]->T->SetData(Lead[i]->TemplateData,Lead[i]->Tstart,Lead[i]->Tend);
	    Lead[i]->T->TwaveAnalysis();
	}
//////////////////////////////////////////   end of T
	SetOnOff(TRUE);
	AflutAfibCorrect();      //Y2015-7-6  Correct A-flut and A-fib depends on templates
}

///   2017-2-20 delete templately
/*
void MultiLead_Templates::QRS_OnOff_Correct()  //问题：每个导联的起始点不一样。
{
	int msD=60l*m_nSampleRate/1000;
	int AveOnSet=0;
	int AveOffSet=0;
	int AveW=0;
	int nCh=0;
	for(int i=0;i<ChN;i++) {
		if(Lead[i]->QRS->OnSet>0) {
			nCh++;
			AveOnSet+=Lead[i]->QRS->OnSet+Lead[i]->QRSstart;
			AveOffSet+=Lead[i]->QRS->OffSet+Lead[i]->QRSstart; 
			AveW+=(Lead[i]->QRS->OffSet-Lead[i]->QRS->OnSet);
		}
	}
	if(nCh>0) {
		AveOnSet/=nCh;
		AveOffSet/=nCh;
		AveW/=nCh;
	}
	for(int i=0;i<ChN;i++) {
		if(AveOffSet-(Lead[i]->QRS->OffSet+Lead[i]->QRSstart) > msD &&
			AveOffSet-(Lead[i]->QRS->OffSet+Lead[i]->QRSstart) > Lead[i]->QRS->OffSet-Lead[i]->QRS->OnSet &&
			3*(Lead[i]->QRS->OffSet-Lead[i]->QRS->OnSet) < 2*AveW) {
		 	   Lead[i]->QRS->OffSet=Min(AveOffSet-Lead[i]->QRSstart,Lead[i]->QRS->length-10l*m_nSampleRate/1000);
		}
	}
}
*/


void MultiLead_Templates::P_Added_Depends()  //问题：每个导联的起始点不一样。
{
	if(ChN<8) return;
	int Pw=30*m_nSampleRate/1000;
	int minWave=int(30/m_dUvperbit);
// Limb Leads correct
	if(I->P->Status!=0 && II->P->Status!=0 && abs((I->Pstart+I->P->OnSet)-(II->Pstart+II->P->OnSet))<Pw && 
		abs((I->Pstart+I->P->OffSet)-(II->Pstart+II->P->OffSet))<Pw && 
		I->P->OffSet-I->P->OnSet>2*Pw && II->P->OffSet-II->P->OnSet>2*Pw) {  
		for(int i=2;i<6;i++) { //III and aVR etc.
			if(Lead[i]->P->Status==0 || (Lead[i]->P->Status!=0 && 
				2*(Lead[i]->P->OffSet-Lead[i]->P->OnSet)<(II->P->OffSet-II->P->OnSet))) {

				if((I->P->OffSet-I->P->OnSet)<(II->P->OffSet-II->P->OnSet)) {
					Lead[i]->Pstart=I->Pstart;
					Lead[i]->P->OnSet=I->P->OnSet;
					Lead[i]->P->OffSet=I->P->OffSet;
				}   
				else {
					Lead[i]->Pstart=II->Pstart;
					Lead[i]->P->OnSet=II->P->OnSet;
					Lead[i]->P->OffSet=II->P->OffSet;
				}  //On and Off are all set							
				int minP=Lead[i]->P->OnSet+1;
				int maxP=minP;
				for(int k=Lead[i]->P->OnSet+2;k<Lead[i]->P->OffSet-1;k++) {
					if(Lead[i]->P->Data[k]<Lead[i]->P->Data[minP]) minP=k;
					if(Lead[i]->P->Data[k]>Lead[i]->P->Data[maxP]) maxP=k;
				}
				short maxV=Lead[i]->P->Data[maxP];
				short minV=Lead[i]->P->Data[minP];
				short BaseV=(Lead[i]->P->Data[Lead[i]->P->OnSet]+Lead[i]->P->Data[Lead[i]->P->OffSet])/2;
				if(maxV-BaseV<minWave && BaseV-minV<minWave) Lead[i]->P->Status=0;
				else if(maxV-BaseV>=BaseV-minV) {
					if(maxP-Lead[i]->P->OnSet>Pw/2 && Lead[i]->P->OffSet-maxP>Pw/2) {
						Lead[i]->P->Status=1;
						Lead[i]->P->OnePos=maxP;
					}
					else Lead[i]->P->Status=0;
				}
				else {
					if(minP-Lead[i]->P->OnSet>Pw/2 && Lead[i]->P->OffSet-minP>Pw/2) {
						Lead[i]->P->Status=2;
						Lead[i]->P->OnePos=minP;
					}
					else Lead[i]->P->Status=0;
				}
			}
		} //for
	} //Limb Leads only

//for all leads
	int Pcount=0;
	for(int i=0;i<ChN;i++) if(Lead[i]->P->Status!=0) Pcount++;
	if(Pcount<3) return;
	int *OnSs=new int[Pcount]; 
	int *OffSs=new int[Pcount]; 
	int k=0;
	for(int i=0;i<ChN;i++) {
		if(Lead[i]->P->Status!=0) {
			OnSs[k]=Lead[i]->Pstart+Lead[i]->P->OnSet;
			OffSs[k]=Lead[i]->Pstart+Lead[i]->P->OffSet;
			k++;
		}
	}
	BubbleSort(OnSs,Pcount);
	BubbleSort(OffSs,Pcount);
	int MidOn=OnSs[Pcount/2];
	int MidOff=OffSs[Pcount/2];
	delete []OnSs;
	delete []OffSs;

	if(MidOff-MidOn<2*Pw) return;
	for(int i=0;i<ChN;i++) {
		if(Lead[i]->P->Status==0 || (Lead[i]->P->Status!=0 && (Lead[i]->P->OffSet-Lead[i]->P->OnSet<Pw ||
			Lead[i]->Pstart+Lead[i]->P->OffSet<MidOn || Lead[i]->Pstart+Lead[i]->P->OnSet>MidOff || 
			MidOn-(Lead[i]->Pstart+Lead[i]->P->OnSet)>3*Pw || (Lead[i]->Pstart+Lead[i]->P->OffSet)-MidOff>3*Pw))) {

			if(Lead[i]->Pstart<MidOn) Lead[i]->P->OnSet=MidOn-Lead[i]->Pstart;
			else Lead[i]->P->OnSet=Lead[i]->Pstart+1;
			if(MidOff<Lead[i]->Pend) Lead[i]->P->OffSet=MidOff-Lead[i]->Pstart;
			else Lead[i]->P->OffSet=Lead[i]->Pend-2;
		// above set On and Off position
			int minP=Lead[i]->P->OnSet+1;
			int maxP=minP;
			for(k=Lead[i]->P->OnSet+2;k<Lead[i]->P->OffSet-1;k++) {
				if(Lead[i]->P->Data[k]<Lead[i]->P->Data[minP]) minP=k;
				if(Lead[i]->P->Data[k]>Lead[i]->P->Data[maxP]) maxP=k;
			}
			short maxV=Lead[i]->P->Data[maxP];
			short minV=Lead[i]->P->Data[minP];
			short BaseV=(Lead[i]->P->Data[Lead[i]->P->OnSet]+Lead[i]->P->Data[Lead[i]->P->OffSet])/2;
			if(maxV-BaseV<minWave && BaseV-minV<minWave) Lead[i]->P->Status=0;
			else if(maxV-BaseV>=BaseV-minV) {
				if(maxP-Lead[i]->P->OnSet>Pw/2 && Lead[i]->P->OffSet-maxP>Pw/2) {
					Lead[i]->P->Status=1;
					Lead[i]->P->OnePos=maxP;
				}
				else Lead[i]->P->Status=0;
			}
			else {
				if(minP-Lead[i]->P->OnSet>Pw/2 && Lead[i]->P->OffSet-minP>Pw/2) {
					Lead[i]->P->Status=2;
					Lead[i]->P->OnePos=minP;
				}
				else Lead[i]->P->Status=0;
			}
		}
	}
}

void MultiLead_Templates::P_OnOff_Correct()
{
	int i,k,nP=0;

	int MaxPd=0;
//	int nQRS=0;  
	int P_OffSet=0;
	int QRSonset=0;
	for(i=0;i<ChN;i++) {          //for very short PR only  //M01-050
		if(Lead[i]->P->Status!=0) {
			nP++;
			if(Lead[i]->P->OffSet-Lead[i]->P->OnSet>MaxPd) {
				MaxPd=Lead[i]->P->OffSet-Lead[i]->P->OnSet;
				P_OffSet=Lead[i]->Pstart+Lead[i]->P->OffSet;
				QRSonset=Lead[i]->QRSstart+Lead[i]->QRS->OnSet;
			}
		}
	}
/*	int *QRSonset=new int[nQRS];
	int *QRSw=new int[nQRS];
	k=0; 
	for(i=0;i<ChN;i++) {
		if(Lead[i]->QRS->OnSet>0) {
			QRSonset[k]=Lead[i]->QRSstart+Lead[i]->QRS->OnSet; 
			QRSw[k]=Lead[i]->QRS->OffSet-Lead[i]->QRS->OnSet;
			k++;
		}
	}  
	BubbleSort(QRSonset,nQRS);
	BubbleSort(QRSw,nQRS);     
	if(nP<5 && nQRS>4 && MaxPd*1000l/m_nSampleRate<55 && 
		((QRSonset[1]+QRSonset[2])/2-P_OffSet)*1000l/m_nSampleRate<20 && QRSw[nQRS-2]*1000l/m_nSampleRate>150)
			for(i=0;i<ChN;i++) Lead[i]->P->Status=0;
	delete []QRSonset;
	delete []QRSw;  
	nP=0;
	for(i=0;i<ChN;i++) if(Lead[i]->P->Status!=0) nP++; 
	if(nP<4) return;
*/
	if(nP<4 && MaxPd*1000l/m_nSampleRate<50 && (QRSonset-P_OffSet)*1000l/m_nSampleRate<30) {
			for(i=0;i<ChN;i++) Lead[i]->P->Status=0;
			return;
	}

	if(nP<4) return;

	int *OnSs=new int[nP]; 
	int *OffSs=new int[nP]; 
	k=0;
	for(i=0;i<ChN;i++) {
		if(Lead[i]->P->Status!=0) {
			OnSs[k]=Lead[i]->Pstart+Lead[i]->P->OnSet;
			OffSs[k]=Lead[i]->Pstart+Lead[i]->P->OffSet;
			k++;
		}
	}
	BubbleSort(OnSs,nP);
	BubbleSort(OffSs,nP);
	int StdOn=OnSs[nP/3];
	int StdOff=OffSs[2*nP/3];
	delete []OnSs;
	delete []OffSs;

	int Pw=28*m_nSampleRate/1000;
	bool *LeadNo=new bool[ChN];
	for(i=0;i<ChN;i++) LeadNo[i]=FALSE;
	for(i=0;i<ChN;i++) {
		if(Lead[i]->P->Status!=0) {
			if(abs(StdOn-(Lead[i]->Pstart+Lead[i]->P->OnSet))>Pw) {   //Onset
				Lead[i]->P->OnSet=StdOn-Lead[i]->Pstart;
				LeadNo[i]=TRUE;
			}
			if(abs((Lead[i]->Pstart+Lead[i]->P->OffSet)-StdOff)>Pw) {
				Lead[i]->P->OffSet=StdOff-Lead[i]->Pstart;
				LeadNo[i]=TRUE;
			}
			if(Lead[i]->P->OffSet<=Lead[i]->P->OnSet) {
				Lead[i]->P->Status=0;
				LeadNo[i]=FALSE;
			}
		}
	}
	for(i=0;i<ChN;i++) {
		if(LeadNo[i]) {
			int pMax,pMin;
			pMax=pMin=Lead[i]->P->OnSet+1;
			for(int k=Lead[i]->P->OnSet+2;k<Lead[i]->P->OffSet;k++) {
				if(Lead[i]->P->Data[k]>Lead[i]->P->Data[pMax]) pMax=k;
				if(Lead[i]->P->Data[k]<Lead[i]->P->Data[pMin]) pMin=k;
			}
			short vMax=Lead[i]->P->Data[pMax];
			short vMin=Lead[i]->P->Data[pMin];
			if(vMax>0 && abs(vMax)>abs(vMin) && 
				vMax>Lead[i]->P->Data[Lead[i]->P->OnSet] && vMax>Lead[i]->P->Data[Lead[i]->P->OffSet]) {
					Lead[i]->P->Status=1;
					Lead[i]->P->OnePos=pMax;
//					Lead[i]->P->OnSet=Lead[i]->P->OnsetPoint(Lead[i]->P->Data,Lead[i]->P->OnSet,pMax,1);
			}
			else if(vMin<0 && abs(vMin)>abs(vMax) && 
				vMin<Lead[i]->P->Data[Lead[i]->P->OnSet] && vMin<Lead[i]->P->Data[Lead[i]->P->OffSet]) {
					Lead[i]->P->Status=2;
					Lead[i]->P->OnePos=pMin;
			}
			else Lead[i]->P->Status=0;
		}
	}
	delete []LeadNo;
}
/*    //to be corrected
void MultiLead_Templates::P_OnOff_Correct(bool *LeadNo,int *StartP,int *EndP)
{
	int i,k,nP=0;

	int MaxPd=0;
	int P_OffSet=0;
	int QRSonset=0;
	for(i=0;i<ChN;i++) {          //for very short PR only  //M01-050
		if(Lead[i]->P->Status!=0) {
			nP++;
			if(Lead[i]->P->OffSet-Lead[i]->P->OnSet>MaxPd) {
				MaxPd=Lead[i]->P->OffSet-Lead[i]->P->OnSet;
				P_OffSet=Lead[i]->Pstart+Lead[i]->P->OffSet;
				QRSonset=Lead[i]->QRSstart+Lead[i]->QRS->OnSet;
			}
		}
	}

	if(nP<4 && MaxPd*1000l/m_nSampleRate<50 && (QRSonset-P_OffSet)*1000l/m_nSampleRate<30) {
			for(i=0;i<ChN;i++) Lead[i]->P->Status=0;
			return;
	}

	if(nP<4) return;

	int *OnSs=new int[nP]; 
	int *OffSs=new int[nP]; 
	k=0;
	for(i=0;i<ChN;i++) {
		if(Lead[i]->P->Status!=0) {
			OnSs[k]=Lead[i]->Pstart+Lead[i]->P->OnSet;
			OffSs[k]=Lead[i]->Pstart+Lead[i]->P->OffSet;
			k++;
		}
	}
	BubbleSort(OnSs,nP);
	BubbleSort(OffSs,nP);
	int StdOn=OnSs[nP/2-1];
	int StdOff=OffSs[nP/2+1];
	delete []OnSs;
	delete []OffSs;

	int Pw=30*m_nSampleRate/1000;
//	for(i=0;i<ChN;i++) LeadNo[i]=FALSE;
	for(i=0;i<ChN;i++) {
		if(Lead[i]->P->Status!=0) {
			int CurrentOn=Lead[i]->Pstart+Lead[i]->P->OnSet;
			if(abs(StdOn-CurrentOn)>Pw) {   //Onset
//				Lead[i]->P->OnSet=StdOn-Lead[i]->Pstart;
				LeadNo[i]=TRUE;
				if(StdOn>CurrentOn) StartP[i]=CurrentOn+(StdOn-CurrentOn)/3;
				else StartP[i]=StdOn;
				EndP[i]=Lead[i]->Pstart+Lead[i]->P->length; 
			}
			int CurrentOff=Lead[i]->Pstart+Lead[i]->P->OffSet;
			if(abs(CurrentOff-StdOff)>Pw) {
//				Lead[i]->P->OffSet=StdOff-Lead[i]->Pstart;
				if(!LeadNo[i]) StartP[i]=Lead[i]->Pstart;
				LeadNo[i]=TRUE;
				if(CurrentOff>StdOff) EndP[i]=StdOff+3*(CurrentOff-StdOff)/5;
				else EndP[i]=StdOff;
			}
			if(Lead[i]->P->OffSet<=Lead[i]->P->OnSet) {
				Lead[i]->P->Status=0;
				LeadNo[i]=FALSE;
			}
		}
	}
}
*/
/*
void MultiLead_Templates::Delete_PR_TooShort()
{
	int nP=0;
	int MaxPd=0;
	int MaxPR=0;
	for(int i=0;i<ChN;i++) {
		if(Lead[i]->P->Status!=0) {
			nP++;
			if(Lead[i]->P->OffSet-Lead[i]->P->OnSet>MaxPd) MaxPd=Lead[i]->P->OffSet-Lead[i]->P->OnSet;
			if((Lead[i]->QRSstart+Lead[i]->QRS->OnSet)-(Lead[i]->Pstart+Lead[i]->P->OnSet)>MaxPR) 
				MaxPR=(Lead[i]->QRSstart+Lead[i]->QRS->OnSet)-(Lead[i]->Pstart+Lead[i]->P->OnSet);
		}
	}
	if(nP<5 && MaxPd*1000l/m_nSampleRate<55 && MaxPR*1000l/m_nSampleRate<75) for(int i=0;i<ChN;i++) Lead[i]->P->Status=0;
}
*/
/*
int MultiLead_Templates::SetQRS_Position(QRS_Complex *pQRS,int left,int right)
{
	int minVp=left;
	for(int i=left;i<right;i++) if(abs(pQRS->Data[i])<abs(pQRS->Data[minVp])) minVp=i;
	return minVp;
}
*/
void MultiLead_Templates::QRSTanalysis()
//for stress test only
{
	int i,j;
	int w=16l*m_nSampleRate/1000;
	int QRSstart=m_nCenter;
	int QRSend=m_nCenter;
	for(i=0;i<ChN;i++) {
	    Lead[i]->QRS_Location(m_nCenter);
		if(Lead[i]->QRSok) {
			if(Lead[i]->QRSstart<QRSstart) QRSstart=Lead[i]->QRSstart;
			if(Lead[i]->QRSend>QRSend) QRSend=Lead[i]->QRSend;
		}
	}
	if(QRSstart==QRSend) {  //without QRS in all leads
		QRSstart-=(80l*m_nSampleRate/1000);
		QRSend+=(80l*m_nSampleRate/1000);
	}

	int ZeroV=0;
	for(i=0;i<ChN;i++) {
		Lead[i]->QRS=new QRS_Complex(m_nSampleRate,m_dUvperbit);
		Lead[i]->QRSstart=FlatestPoint(Lead[i]->TemplateData,QRSstart+w);
		Lead[i]->QRSend=FlatestPoint(Lead[i]->TemplateData,QRSend-w);
		ZeroV=0;
		for(j=-w/4;j<w/4;j++) ZeroV+=Lead[i]->TemplateData[Lead[i]->QRSstart+j]; 
		ZeroV/=(w/2);
		for(j=0;j<m_nLength;j++) Lead[i]->TemplateData[j]-=ZeroV; //	Lead[i]->ZeroV=0;//added by hspecg 2011.04.02
		if(Lead[i]->QRSok) {
			Lead[i]->QRS->SetData(Lead[i]->TemplateData,Lead[i]->QRSstart,Lead[i]->QRSend);  //Lead[i]->ZeroV,
			Lead[i]->QRS->Analysis();
		}
	}
}
/*
bool MultiLead_Templates::FindWrownOnset(short *OnPos)
{
	int error=30l*m_nSampleRate/1000;
	int i,k;
	for(i=0;i<ChN;i++) OnPos[i]=0;
	int count=0;
	for(i=0;i<ChN;i++) if(Lead[i]->P->Status!=0) count++;
	if(count<5) return FALSE;
	int *Pos=new int[count];
	k=0; 
	for(i=0;i<ChN;i++) if(Lead[i]->P->Status!=0) Pos[k++]=Lead[i]->Pstart+Lead[i]->P->OnSet;
	BubbleSort(Pos,count);
	bool Found=FALSE;
	for(i=0;i<ChN;i++) {
		if(Lead[i]->P->Status!=0) {
			if(Lead[i]->Pstart+Lead[i]->P->OnSet-Pos[count/2]>error) { OnPos[i]=1;  Found=TRUE; }
			if(Pos[count/2]-Lead[i]->Pstart+Lead[i]->P->OnSet>error) { OnPos[i]=-1;  Found=TRUE; }
		}
	}
	delete []Pos;
	return Found;
}
*/
void MultiLead_Templates::IndividualManual()
{
	for(int i=0;i<ChN;i++) Lead[i]->FeatureSet();
	SetOnOff(FALSE);
}

void MultiLead_Templates::CommenManual()
{
	int i,j;
	int w=16l*m_nSampleRate/1000;

	int ZeroV;
	for(i=0;i<ChN;i++) {
		Lead[i]->QRSstart=OnOff[2]-w;
		Lead[i]->QRSend=OnOff[3]+w;
		ZeroV=0;
		for(j=-w/4;j<w/4;j++) ZeroV+=Lead[i]->TemplateData[Lead[i]->QRSstart+j]; 
        ZeroV/=(w/2);
		for(j=0;j<m_nLength;j++) Lead[i]->TemplateData[j]-=ZeroV;	// Lead[i]->ZeroV=0;//added by hspecg 2011.04.02
		Lead[i]->QRS->SetData(Lead[i]->TemplateData,Lead[i]->QRSstart,Lead[i]->QRSend); //Lead[i]->ZeroV,
		Lead[i]->QRS->Analysis();
//		Lead[i]->QRS->QS_Type_Setting();

		Lead[i]->Pstart=OnOff[0]-w;
		Lead[i]->Pend=OnOff[1]+w;
		Lead[i]->P->SetData(Lead[i]->TemplateData,Lead[i]->Pstart,Lead[i]->Pend);
//		Lead[i]->P->Analysis();  
		Lead[i]->P->PwaveAnalysis(); 
	}
// Recalculate ZeroV   2011-4-6 by Du
	int MidZp;
	for(i=0;i<ChN;i++) {
		if(Lead[i]->P->Status>0&&Lead[i]->QRS->OnSet>0) {
			w=((Lead[i]->QRSstart+Lead[i]->QRS->OnSet)-(Lead[i]->Pstart+Lead[i]->P->OffSet))/3;
			if(w>0) {
				MidZp=(Lead[i]->Pstart+Lead[i]->P->OffSet)+w/2;
				ZeroV=0;
				for(j=0;j<w;j++) ZeroV+=Lead[i]->TemplateData[MidZp+j];
				ZeroV/=w;
			}
			else ZeroV=Lead[i]->TemplateData[Lead[i]->Pstart+Lead[i]->P->OffSet];
   			for(j=0;j<m_nLength;j++) Lead[i]->TemplateData[j]-=ZeroV;
		}

	    w=16l*m_nSampleRate/1000;
		Lead[i]->Tstart=OnOff[4]-w;
		Lead[i]->Tend=OnOff[5]+w;
		Lead[i]->T->SetData(Lead[i]->TemplateData,Lead[i]->Tstart,Lead[i]->Tend);
//		Lead[i]->T->Analysis('T');
		Lead[i]->T->TwaveAnalysis();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
ECG_Template::ECG_Template(short *ECGdata,int Length,int SampleRate,double Uvperbit)
: ECG_Base(SampleRate,Uvperbit)
{
	m_nLength=Length;
	TemplateData=ECGdata;
	Morpho[0]=Morpho[1]=Morpho[2]=Morpho[3]=Morpho[4]=Morpho[5]=Morpho[6]=Morpho[7]='\0';

	QRS=NULL;
	P=NULL;
	T=NULL;

	QRSok=FALSE;
	QRSstart=QRSend=0; 
//	ZeroV=0;   //2011-4-26 by Du
//	Pok=Tok=FALSE;
	Pstart=Pend=Tstart=Tend=0; 
//  50Hz?
//	int w=20l*SampleRate/1000;
//	for(int i=0;i<Length-w;i++) ECGdata[i]=(ECGdata[i]+ECGdata[i+w])/2;
}

ECG_Template::~ECG_Template()
{
	if(QRS) delete QRS;
	if(P) delete P;
	if(T) delete T;
}

short ECG_Template::CalculateZeroValue()
{
	short zeroV=0;
	if(P->Status>0 && QRS->OnSet>=0 && QRS->OffSet-QRS->OnSet>0) {
		int QRSon=QRS->OnSet+QRSstart;
		int Poff=P->OffSet+Pstart;
		int w=(QRSon-Poff)/3;
		if(w>1) {
			for(int k=0;k<w;k++) zeroV+=TemplateData[Poff+w+k];
			zeroV/=w;
		}
		else if(w>=0) zeroV=TemplateData[(Poff+QRSon)/2];
	} 
	return zeroV;
}


void ECG_Template::FeatureSet()  //modified by Du 2011-5-19
{	//0   1   2    3   4  5
	//Pb,Pe,QRSb,QRSe,Tb,Te
	int j,w=8l*m_nSampleRate/1000;   //ZeroValue

	if((OnOff[1]!=AvantOnOff[1] || OnOff[2]!=AvantOnOff[2]) && OnOff[2]>OnOff[1]) {
		int ZeroV=0;
		if(OnOff[1]>w && (OnOff[2]-OnOff[1])>0 && OnOff[2]<m_nLength-w) {
			for(j=0;j<w;j++) ZeroV+=TemplateData[(OnOff[1]+OnOff[2])/2+j-w/2]; 
		}
		else if(OnOff[2]>4*w && OnOff[2]<m_nLength) {
			for(j=0;j<w;j++) ZeroV+=TemplateData[OnOff[2]-3*w+j]; 
		}
		ZeroV/=w;
		for(j=0;j<m_nLength;j++) TemplateData[j]-=ZeroV;
	}

//  P-Wave set
	if((OnOff[0]!=AvantOnOff[0] || OnOff[1]!=AvantOnOff[1]) && OnOff[1]>OnOff[0]) {
		if(OnOff[0]>0 && (OnOff[1]-OnOff[0])>0 && OnOff[1]<m_nLength) {
			Pstart=OnOff[0];
		 P->FeatureSet(TemplateData,OnOff[0],OnOff[1]);
		}
		else if(P->Status!=-2) P->Status=0;
	}

//  QRS Complex set
	if((OnOff[2]!=AvantOnOff[2] || OnOff[3]!=AvantOnOff[3]) && OnOff[3]>OnOff[2]) {
		if(OnOff[2]>0 && (OnOff[3]-OnOff[2])>0 && OnOff[3]<m_nLength) {
			QRSstart=OnOff[2];
			QRS->FeatureSet(TemplateData,OnOff[2],OnOff[3]); //ZeroV,
		}
		else if(QRS->OnSet!=-2) QRS->OnSet=0; 
	}

//  T-Wave set
	if((OnOff[4]!=AvantOnOff[4] || OnOff[5]!=AvantOnOff[5]) && OnOff[5]>OnOff[4]) {
		if(OnOff[4]>0 && (OnOff[5]-OnOff[4])>0 && OnOff[5]<m_nLength) {
			Tstart=OnOff[4];
			T->FeatureSet(TemplateData,OnOff[4],OnOff[5]);
		}
		else if(T->Status!=-2) T->Status=0;
	}
}


void ECG_Template::QRS_Location(int Center)
{
	int i;
	short *temp;   //before short
	int w=30l*m_nSampleRate/1000;   //ms
	int start=Center-6*w;	if(start<w) start=w;  //hspecg 2011-4-19  //Du 2015-7-30
	int end=Center+8*w;     if(end>=m_nLength-w) end=m_nLength-w;  //Du 2015-7-30
	int length=end-start;

	temp=new short[length+1];   //before short
	for(i=0;i<length;i++) temp[i]=TemplateData[start+i];
	Monentum(temp,length);

	int maxP=3*w;
	for(i=maxP;i<length-3*w;i++) if(temp[i]>temp[maxP]) maxP=i;

	int average=0;
	int count=0;
	for(i=w;i<maxP-2*w;i++) {
		average+=temp[i];
		count++;
	}
	for(i=maxP+2*w;i<length-w;i++) {
		average+=temp[i];
		count++;
	}
	if(count>0) average/=count;
		
	if(temp[maxP]<=1.25*average||temp[maxP]<MinimumWave) QRSok=FALSE;   //无QRS (max<0.025mV)
	else {
        QRSok=TRUE;
		int left,right;
		int hold=4*temp[maxP]/7;   //before /2     2017-3-30
		if(hold>1.5*average) {                    //following: modified in Y2015-8-3
			for(i=2*w;i<length-2*w;i++) if(temp[i]>hold) temp[i]=hold; 
			left=2*w;                //3*
			while(temp[left]<hold && left<maxP) left++;
			right=length-2*w-1;   //3*
			while(temp[right]<hold && right>maxP) right--;  
		}
		else left=right=maxP;  
		QRSstart=TurnPoint(temp,w/2,left,-1);
		if(QRSstart>w) QRSstart+=(start-w);
		else QRSstart+=start;
		if(QRSstart-start-w/3>0) QRSstart-=(w/3);
		QRSend=TurnPoint(temp,right,length-w/2,-1);
		if(QRSend<length-w) QRSend+=(start+w);
		else QRSend+=start;
		if(QRSend+w-start<length-w/2) QRSend+=w;                   
	}
	
	delete []temp;
}

/*
int ECG_Template::P_Range(short *data,int len)
{
	int w=40l*m_nSampleRate/1000;
	short minV=short(30/m_dUvperbit);
	int MaxPos=len/2;
	short MaxPv=0;
	for(int i=w;i<len-w;i++) {
		if(data[i]-data[i-w]>minV && data[i]-data[i+w]>minV) {
			if(data[i]-data[i-w]+data[i]-data[i+w]>MaxPv) {
				MaxPv=data[i]-data[i-w]+data[i]-data[i+w];
				MaxPos=i;
			}
		}
		if(data[i-w]-data[i]>minV && data[i+w]-data[i]>minV) {
			if(data[i-w]-data[i]+data[i+w]-data[i]>MaxPv) {
				MaxPv=data[i-w]-data[i]+data[i+w]-data[i];
				MaxPos=i;
			}
		}
	}
	return MaxPos;
}
*/

void ECG_Template::Monentum(short *data,int length)
{
	int i,j,t;
//	short *temp1,*temp2;
 	int *temp1,*temp2;
    int w=10l*m_nSampleRate/1000;   //8毫秒

//	temp1=new short[length+1];
//	temp2=new short[length+1];
	temp1=new int[length+1];
	temp2=new int[length+1];

	temp1[0]=data[0];
	temp1[length-1]=data[length-1];
	int w0=1;
	for(i=1;i<w;i++) {
		int t1=0;
		int t2=0;
		for(j=-w0;j<w0;j++) {
			t1+=data[i+j];
			t2+=data[length-1-i+j];
		}
		temp1[i]=t1/(2*w0);
		temp1[length-1-i]=t2/(2*w0);
		w0++;
	}
	for(i=w;i<length-w;i++) {   //2*w毫秒平均
		t=0;
		for(j=-w;j<w;j++) t+=data[i+j];
		temp1[i]=t/(2*w);
	}

	for(i=0;i<w;i++) temp2[i]=temp2[length-w+i]=0;

	for(i=w;i<length-w;i++) //2*w毫秒动量定义
		temp2[i]=abs(temp1[i+w]-temp1[i])+abs(temp1[i-w]-temp1[i]);

	for(i=3*w;i<length-3*w;i++) { //6*w毫秒平均动量  for QRS
		t=0;
		for(j=-3*w;j<3*w;j++) t+=temp2[i+j];
		data[i]=t/(6*w);   
	}
	data[0]=data[length-1]=0;
	int t1,t2;
	for(i=1;i<3*w;i++) {
		t1=t2=0;
		for(j=0;j<2*i;j++) {
			t1+=temp2[j];
            t2+=temp2[length-1-j];
		}
		data[i]=t1/(2*i);
		data[length-1-i]=t2/(2*i);
	}
	delete []temp1;
	delete []temp2;
}

void ECG_Template::Potential(short *data,int length)
{
	int i,j,t,w;
	short *temp;
    w=10l*m_nSampleRate/1000;   //8毫秒

	temp=new short[length+1];

	float tan=float((data[length-w]-data[0])/(length-w));
	for(i=0;i<length;i++) temp[i]=abs(data[i]-int(tan*i)-data[0]);

	for(i=3*w;i<length-3*w;i++) { //6*w毫秒平均量  for QRS
		t=0;
		for(j=-3*w;j<3*w;j++) t+=temp[i+j];
		data[i]=t/(6*w);   
	}

	data[0]=data[length-1]=0;
	int t1,t2;
	for(i=1;i<3*w;i++) {
		t1=t2=0;
		for(j=0;j<2*i;j++) {
			t1+=temp[j];
            t2+=temp[length-1-j];
		}
		data[i]=t1/(2*i);
		data[length-1-i]=t2/(2*i);
	}
	delete []temp;
}

int ECG_Template::msInterval(INDIVIDUAL_INTERVAL_TYPE Type)   
{     //return unit: ms 
	switch(Type) {
	case Pd:
		return (OnOff[1]-OnOff[0])*1000l/m_nSampleRate;  //Y2016-4-6 OnOff[1]=OnOff[0]=0 if without P
	case Qd:
		if(QRS->Q[0]!=0) return	(QRS->Q[3]-QRS->Q[1])*1000l/m_nSampleRate;
		return 0;
	case Rd1:
		if(QRS->R1[0]!=0) return (QRS->R1[3]-QRS->R1[1])*1000l/m_nSampleRate;
		return 0;
	case Rd2:
		if(QRS->R2[0]!=0) return (QRS->R2[3]-QRS->R2[1])*1000l/m_nSampleRate;
		return 0;
	case Sd1:
		if(QRS->S1[0]!=0) return (QRS->S1[3]-QRS->S1[1])*1000l/m_nSampleRate;
		return 0;
	case Sd2:
		if(QRS->S2[0]!=0) return (QRS->S2[3]-QRS->S2[1])*1000l/m_nSampleRate;
		return 0;
	case Td:
		return (OnOff[5]-OnOff[4])*1000l/m_nSampleRate;
	case PRd:
		if(P->Status!=0 && OnOff[2]>OnOff[0]) return (OnOff[2]-OnOff[0])*1000l/m_nSampleRate;  //Y2015-7-6 增加：P->Status!=0
		return 0;
	case QTd:
		if(OnOff[5]>0 && OnOff[2]>0) return (OnOff[5]-OnOff[2])*1000l/m_nSampleRate;
		return 0;
	case QRSd:
		return (OnOff[3]-OnOff[2])*1000l/m_nSampleRate;
	}
	return 0;
}

int ECG_Template::uvValue(INDIVIDUAL_VALUE_TYPE Type)
{
	switch(Type) {
	case Pa1:
		if(P->Status>0) return int((P->Data[P->OnePos])*m_dUvperbit);
		return 0;
	case Pa2:
		if(P->Status>2) return int((P->Data[P->TwoPos])*m_dUvperbit);
		return 0;
	case Qa:
		if(QRS->Q[0]>0) return int((QRS->Data[QRS->Q[2]])*m_dUvperbit);
		return 0;
	case Ra1:
		if(QRS->R1[0]>0) return int((QRS->Data[QRS->R1[2]])*m_dUvperbit);
		return 0;
	case Ra2:
		if(QRS->R2[0]>0) return int((QRS->Data[QRS->R2[2]])*m_dUvperbit);
		return 0;
	case Sa1:
		if(QRS->S1[0]>0) return int((QRS->Data[QRS->S1[2]])*m_dUvperbit);
		return 0;
	case Sa2:
		if(QRS->S2[0]>0) return int((QRS->Data[QRS->S2[2]])*m_dUvperbit);
		return 0;
	case Ta1:                                                       //Status 1+, 2-, 3+-, 4-+
		if(T->Status>0) return int((T->Data[T->OnePos])*m_dUvperbit);
		return 0;
	case Ta2:
		if(T->Status>2) return int((T->Data[T->TwoPos])*m_dUvperbit);
		return 0;
	}
	return 0;
}
/*
//I think following two members(Du) is not correct and rewrite by Hu
int ECG_Template::uvSTvalue(int MSor123)
{
	int QT,step;
	if(QRS->OnSet>0)  {
		if(MSor123==1||MSor123==2||MSor123==3) {
			if(T->Status>0) QT=(T->OffSet-QRS->OnSet);
			else QT=0;
            return int((TemplateData[QRS->OffSet+MSor123*QT/10]-ZeroV)*m_dUvperbit);
		}
        else {
			step=long(MSor123)*m_nSampleRate/1000;
			return int((TemplateData[QRS->OffSet+step]-ZeroV)*m_dUvperbit);
		}
	}
	else return 0;
}

float ECG_Template::STslope(int msStep)
{
	int Step=long(msStep)*m_nSampleRate/1000;
	if(QRS->OnSet>0) return 
		(TemplateData[QRS->OffSet+Step]-TemplateData[QRS->OffSet])*m_dUvperbit/msStep;
	else return 0;
}
*/

int ECG_Template::uvSTvalue(int MSor123)
{
	int QT,step,msST=OnOff[3];
// int Correct=4l*m_nSampleRate/1000;  //by Du 2011-4-30
	int Correct=6l*m_nSampleRate/1000;  //by gdh 2011-5-10
	if(QRS->OnSet>0)  {
		if(MSor123>=0 && MSor123<4) {//MSor123==1||MSor123==2||MSor123==3) {
			if(T->Status>0) QT=(T->OffSet-QRS->OnSet);
			else QT=400l*m_nSampleRate/1000;
 //           return int((TemplateData[OnOff[3]+MSor123*QT/10]-ZeroV)*m_dUvperbit);
			msST+=(Correct+MSor123*QT/10);
//            return int((TemplateData[OnOff[3]+Correct+MSor123*QT/10])*m_dUvperbit);  //2011-4-5 by Du
		}
        else {
			step=long(MSor123)*m_nSampleRate/1000;
//   return int((TemplateData[OnOff[3]+step]-ZeroV)*m_dUvperbit);  
			msST+=(Correct+step);
//   return int((TemplateData[OnOff[3]+Correct+step])*m_dUvperbit);  //2011-4-5 by Du
		}
	}
	if(msST>=OnOff[3] && msST<m_nLength && TemplateData) return int((TemplateData[msST])*m_dUvperbit); //2016-08-03 by hspecg
	else return 0;
}


float ECG_Template::STslope(int msStep)
{
	int Correct=4l*m_nSampleRate/1000;  //by Du 2011-4-30
	int Step=long(msStep)*m_nSampleRate/1000;
	if(QRS->OnSet>0) return 
		float((TemplateData[OnOff[3]+Correct+Step]-TemplateData[OnOff[3]+Correct])*m_dUvperbit/msStep);
	else return 0;
}

char * ECG_Template::QRSmorpho()
{   //Morpho[6]
	int k=0;
	if(QRS->Q[0]==1) Morpho[k++]='q';
	if(QRS->Q[0]==2) Morpho[k++]='Q';
	if(QRS->R1[0]==1) Morpho[k++]='r';
	if(QRS->R1[0]==2) Morpho[k++]='R';
	if(QRS->S1[0]==1) Morpho[k++]='s';
	if(QRS->S1[0]==2) Morpho[k++]='S';
	if(QRS->R2[0]==1) { Morpho[k++]='r'; Morpho[k++]='\''; }
	if(QRS->R2[0]==2) { Morpho[k++]='R'; Morpho[k++]='\''; }
	if(QRS->S2[0]==1) { Morpho[k++]='s'; Morpho[k++]='\''; }
	if(QRS->S2[0]==2) { Morpho[k++]='S'; Morpho[k++]='\''; }
    Morpho[k]='\0';
	if(Morpho[0]=='s' && Morpho[1]=='\0') {
		Morpho[0]='q';
		Morpho[1]='s';
		Morpho[2]='\0';
	}
	if(Morpho[0]=='S' && Morpho[1]=='\0') {
		Morpho[0]='Q';
		Morpho[1]='S';
		Morpho[2]='\0';
	}
	return Morpho;
}
///////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
QRS_Complex::QRS_Complex(int SampleRate,double Uvperbit): ECG_Base(SampleRate,Uvperbit)
{
//	OnSet=-2;     //initial without QRS
	OnSet=-1;
	Q[0]=R1[0]=S1[0]=R2[0]=S2[0]=0;//hspecg
	VHs=NULL; Data=NULL;//hspecg
	length=0;OnSet=OffSet=0;
	bpLow=bpHigh=0;   //2015-8 for WPW
}

QRS_Complex::~QRS_Complex()
{
	if(OnSet!=-2) if(VHs) delete []VHs;
}

void QRS_Complex::SetData(short *TemplateData,int QRSstart,int QRSend) //int ZeroValue,
{
	int i;
	length=QRSend-QRSstart;

	if(OnSet!=-2) if(VHs) delete []VHs;      //for manual functions reset the Data
		
	Data=&TemplateData[QRSstart];
	VHs=new short[length];   //+1
	for(i=0;i<length;i++) VHs[i]=0;

	OnSet=-1;
	Q[0]=R1[0]=S1[0]=R2[0]=S2[0]=0;
	
	int bW=20*m_nSampleRate/1000;                             //2015-8 (for WPW)
	bpHigh=QRSstart>bW? TemplateData[QRSstart-bW] : TemplateData[0];
	bpLow=QRSstart>5*bW? TemplateData[QRSstart-5*bW] : TemplateData[0];
}

void QRS_Complex::FeatureSet(short *TemplateData,int start,int end)  //int zeroV,
{  
	SetData(TemplateData,start,end);  //zeroV,
	OnSet=0;
	OffSet=length-1;

	if(!VHsProcess(FALSE)) return;   

	int i,maxP,minP,VsN,HsN;
	VsN=HsN=0;
	maxP=minP=OnSet+1;
	for(i=OnSet+2;i<OffSet;i++) {
		if(VHs[i]>0 && Data[i]>0) HsN++;
		if(VHs[i]<0 && Data[i]<0) VsN++;
		if(Data[i]>Data[maxP]) maxP=i;
		if(Data[i]<Data[minP]) minP=i;
	}
	if(HsN==0) { SubSetting(OnSet,minP,OffSet,S1);  return;  }  // QS/qs form
	if(VsN==0) { SubSetting(OnSet,maxP,OffSet,R1);  return;  }  // R/r form

// Looks for if Q and S(S2) exist, from left and right seperately
	int l,r,p,q;   //p--from left; q--from right;
	for(p=0;p<length;p++) {
		if(VHs[p]!=0) {
			if(Data[p]>0) break;  //must have one
			if(Data[p]<=0) break;  //maybe, consider OffsetV //before is <
		}
	}
	if(VHs[p]<0) {   //q/Q found (data[p]<OnsetV)
		minP=p;
		for(p=minP+1;p<length;p++) if(VHs[p]>0 && Data[p]>0) break;
		for(i=minP;i<p;i++) if(VHs[i]<0&&Data[i]<Data[minP]) minP=i;
		for(r=p;r>minP;r--) if(Data[r]<=Data[OnSet]) break;
        SubSetting(OnSet,minP,r,Q);   //q/Q found
	}   //now p is at a hill point (VHs[p]>0 now), wether Q is found or not

	for(q=length-1;q>0;q--) {
		if(VHs[q]>0 && Data[q]>0) break;  //must have one
		if(VHs[q]<0) break;//OffsetV||found) break;  //maybe, consider OnsetV
	}

	if(VHs[q]<0) {    // s/S found but not sure S1 or S2
		minP=q;
		for(q=minP-1;q>0;q--) if(VHs[q]>0 && Data[q]>0) break;
		for(i=minP;i>q;i--) if(VHs[i]<0&&Data[i]<Data[minP]) minP=i;
		for(l=q;l<minP;l++) if(Data[l]<=Data[OffSet]) break;
        SubSetting(l,minP,OffSet,S1);   //not sure S1 or S2
	}   //now q is at a hill point (VHs[q]>0 now), wether S(S2) is found or not

// to check if sigle R or float R
	for(i=p;i<=q;i++) if(VHs[i]<-1 && Data[i]<0) break;
	if(i>=q) {   //Sigle R
		maxP=p;
		for(i=p;i<=q;i++) if(VHs[i]>1 && Data[i]>Data[maxP]) maxP=i;
		if(Q[0]==0) l=OnSet;  //without Q
		else l=Q[3];
		if(S1[0]==0) r=OffSet;  //without S
		else r=S1[1];
        SubSetting(l,maxP,r,R1);   //R1 set
	}
	else {  //  Double R (with S1 beteen R1 and R2
		for(minP=i;i<q;i++) if(VHs[i]<0 && Data[i]<Data[minP]) minP=i;  //find S1;
// looks for R1		
		for(maxP=i=p;i<minP;i++) if(VHs[i]>0 && Data[i]>Data[maxP]) maxP=i;  //R1
		if(Q[0]==0) l=OnSet;  //without Q
		else l=Q[3];
		for(r=maxP;r<minP;r++) if(Data[r]<=Data[l]) break;
        SubSetting(l,maxP,r,R1);   //R1 set
//  looks for R2
		for(maxP=i=minP+1;i<=q;i++) if(VHs[i]>0 && Data[i]>Data[maxP]) maxP=i;  //R2
		if(S1[0]==0) r=OffSet;  //without S
		else {    //S2 set
			for(i=0;i<4;i++) S2[i]=S1[i];
			r=S1[1];
		}
		for(l=maxP;l>minP;l--) if(Data[l]<=Data[r]) break;
        SubSetting(l,maxP,r,R2);   //R2 set
        SubSetting(R1[3],minP,R2[1],S1);   //S1 set
	}
}

BOOL QRS_Complex::Delta()
{
	int i,turnP;
	int w=4l*m_nSampleRate/1000;

	if(R1[0]==2 && Q[0]!=2) {    //R1[0]==2 && Q[0]!=2
		turnP=TurnPoint(Data,R1[1],R1[2],-1);
		for(i=-w;i<w;i++) if(VHs[turnP+i]!=0) return FALSE;
		if(turnP-R1[1]>7*w  &&  17*(turnP-R1[1])>10*(R1[2]-R1[1])  &&   //7*w=28ms
			17*(Data[turnP]-Data[R1[1]])<7*(Data[R1[2]]-Data[R1[1]]))	{
			return TRUE;
		}  // M01_098(V6) 5/9<x<5/8   x=5/8.5=10/17 
	}
	if(Q[0]==2) {
		turnP=TurnPoint(Data,Q[1],Q[2],1);
		for(i=-w;i<w;i++) if(VHs[turnP+i]!=0) return FALSE;
		if(turnP-Q[1]>7*w  &&  17*(turnP-Q[1])>10*(Q[2]-Q[1])  && 
			17*(Data[Q[1]]-Data[turnP])<7*(Data[Q[1]]-Data[Q[2]]))	{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL QRS_Complex::Split()
{
	return FALSE;
}
/*
void QRS_Complex::Notch(short &Incrs,short &Decrs)
{
	short Pn=0;
	short Nn=0;
	for(int i=0;i<length;i++) {
		if(VHs[i]!=3) continue;
		if(R1[0]!=0&&i==R1[2]) continue;
		if(R2[0]!=0&&i==R2[2]) continue;
		if(R2[0]==0) {
			if(R1[0]!=0) {
				if(i<R1[2]) Pn++;
				else Nn++;
			}
		}
		else {
			if(i<R1[2]) Pn++;
			else {
				if(i<S1[2]) Nn++;
				else if(i<R2[2]) Pn++;
				else Nn++;
			}
		}
	}
	Incrs=Pn;
	Decrs=Nn;
}
*/
/*
void QRS_Complex::Notch(short &Incrs,short &Decrs)
{
	short Pn=0;
	short Nn=0;
	int start=OnSet;
	if(Q[0]!=0) start=Q[2];
	if(R1[0]!=0) {
		if(IncreaseNotch(start,R1[2])) Pn++;
		start=R1[2];
	}
	if(S1[0]!=0) {
		if(DecreaseNotch(start,S1[2])) Nn++;
		if(R2[0]!=0) {
			if(IncreaseNotch(S1[2],R2[2])) Pn++;
			if(S2[0]!=0) if(DecreaseNotch(R2[2],S2[2])) Nn++;
			else if(DecreaseNotch(R2[2],OffSet)) Nn++;
		}
	}
	if(R1[0]!=0&&S1[0]==0) if(DecreaseNotch(R1[2],OffSet)) Nn++; 
	Incrs=Pn;
	Decrs=Nn;
}

BOOL QRS_Complex::IncreaseNotch(int Start,int End)
{
	int w=6*m_nSampleRate/1000;
	if(End-Start<4*w) return FALSE;
	for(int i=Start+2*w;i<End-2*w;i++) if(Data[i+w]<Data[i-w]) return TRUE;
	return FALSE;
}

BOOL QRS_Complex::DecreaseNotch(int Start,int End)
{
	int w=6*m_nSampleRate/1000;
	if(End-Start<4*w) return FALSE;
	for(int i=Start+2*w;i<End-2*w;i++) if(Data[i+w]>Data[i-w]) return TRUE;
	return FALSE;
}
*/
void QRS_Complex::Notch(short &Incrs,short &Decrs)
{
	short Pn=0;
	short Nn=0;
	int start=OnSet;
	if(Q[0]!=0) start=Q[2];
	if(R1[0]!=0) {
		Pn+=IncreaseNotch(start,R1[2]);
		start=R1[2];
	}
	if(S1[0]!=0) {
		Nn+=DecreaseNotch(start,S1[2]);
		if(R2[0]!=0) {
			Pn+=IncreaseNotch(S1[2],R2[2]);
			if(S2[0]!=0) Nn+=DecreaseNotch(R2[2],S2[2]);
			Nn+=DecreaseNotch(R2[2],OffSet);
		}
	}
	if(R1[0]!=0&&S1[0]==0) Nn+=DecreaseNotch(R1[2],OffSet); 
	Incrs=Pn;
	Decrs=Nn;
}

short QRS_Complex::IncreaseNotch(int Start,int End)
{
	int w=6*m_nSampleRate;
	int st=Start+2*w;
	short Count=0;
	int t;
	while(st<End-2*w) {
		if(Data[st+w]<Data[st-w]) {
			Count++;
			t=st+w+1;
			while(Data[t]<Data[st-w]&&t<End-2*w) t++;
			st=t;
		}
		st++;
	}
	return Count;
}

short QRS_Complex::DecreaseNotch(int Start,int End)
{
	int w=6*m_nSampleRate;
	int st=Start+2*w;
	short Count=0;
	int t;
	while(st<End-2*w) {
		if(Data[st+w]>Data[st-w]) {
			Count++;
			t=st+w+1;
			while(Data[t]>Data[st-w]&&t<End-2*w) t++;
			st=t;
		}
		st++;
	}
	return Count;
}


void QRS_Complex::Analysis()
{
	if(!VHsProcess(TRUE)) return; // Without QRS 
	ComplexFeatures();  
	CorrectFeatures();
}

void QRS_Complex::ComplexFeatures()
{
	int i,minP,maxP,l,r;
	
	int stdV=int(140/m_dUvperbit);
	int tAve=10*m_nSampleRate/1000;
	int OnsetV=0;
	for(i=0;i<tAve;i++) OnsetV+=Data[i];
	OnsetV/=tAve;
	int OffsetV=0;
	for(i=1;i<=tAve;i++) OffsetV+=Data[length-i];
	OffsetV/=tAve;

	int OnsetWidth=50l*m_nSampleRate/1000;
	int sbHold=int(500/m_dUvperbit);

	for(i=tAve;i<length;i++) if(VHs[i]>0 && Data[i]>OnsetV && Data[i]>0) break;
	if(i>length-tAve) {   //10mS single s/S  //2015-12-2
		for(l=tAve;l<=length-tAve;l++) if(VHs[l]<-1) break;
		for(r=length-tAve;r>=l;r--) if(VHs[r]<-1) break;
		minP=l;
		for(i=l;i<r;i++) if(Data[i]<Data[minP]) minP=i;
		int minPl=minP;
		for(i=l;i<minP;i++) if(VHs[i]>1/* && Data[i]<0*/) break;
		if(i<minP) {    // find a hill (the leftest)
			minPl=l;
			for(int k=l;k<i;k++) if(Data[k]<Data[minPl]) minPl=k;
			if(Data[i]-Data[minPl]<stdV/2) minPl=minP;   //????????????????????
		}
		int minPr=minP;
		for(i=minP;i<r;i++) if(VHs[i]>0) break;
		if(i<r) {
			minPr=r;
			for(int k=r;k>i;k--) if(Data[k]<Data[minPr]) minPr=k;
		}
		l=minPl;        // following 10 lines corrected on 2015-12-16
		while(l>2*tAve && Data[l]<-3*stdV/2) l--; 
		OnSet=OnsetPoint(tAve,l,1);
		if(Data[OnSet]<-stdV && OnSet-3*tAve>0) {
			int lMinP=OnSet-2*tAve;
			for(i=lMinP;i<OnSet;i++) if(Data[i]<Data[lMinP]) lMinP=i;
			if(Data[lMinP]<Data[lMinP+(OnSet-lMinP)/2] && Data[lMinP+(OnSet-lMinP)/2]<Data[OnSet] && l-lMinP<3*tAve) 
				OnSet=OnsetPoint(tAve,lMinP,1);
		}
		int rPos=length-(length-minPr)/3;  //&2015-12-12 following 7 lines added
		int lPos=length-2*(length-minPr)/3;
		r=length-tAve;
		while(Data[r]>3*stdV/2 && r>rPos) r--;
		l=minPr;
		while(Data[l]<-3*stdV && l<lPos) l++;
		OffSet=OffsetPoint(l,r,1);
        SubSetting(OnSet,minP,OffSet,S1); 
		return;
	}

	for(i=tAve;i<length;i++) if(VHs[i]<0 && (Data[i]<OnsetV||Data[i]<OffsetV) && Data[i]<0) break;
// 可能找不到谷（M02-042 avl）拟在correct中解决
	if(i>=length-tAve) {  //single r/R  //2015-12-7  
		for(l=0;l<length;l++) if(VHs[l]>1) break;
		for(r=length-1;r>=l;r--) if(VHs[r]>1) break;
		maxP=l;
		for(i=l;i<r;i++) if(Data[i]>Data[maxP]) maxP=i;
		int maxPl=maxP;
		for(i=l;i<maxP;i++) if(VHs[i]<-1) break;
		if(i<maxP) {    // find a hill (the leftest)
			maxPl=l;
			for(int k=l;k<i;k++) if(Data[k]>Data[maxPl]) maxPl=k;
			if(Data[maxPl]-Data[i]<stdV/2) maxPl=maxP;    //???????????????????????
		}
		int maxPr=maxP;
		for(i=maxP;i<r;i++) if(VHs[i]<0) break;
		if(i<r) {
			maxPr=r;
			for(int k=r;k>i;k--) if(Data[k]>Data[maxPr]) maxPr=k;
		}
		l=maxPl;
		while(l>2*tAve && Data[l]>2*stdV) l--;
		OnSet=OnsetPoint(tAve,l,-1);
	
		if(Data[OnSet]>stdV && OnSet-3*tAve>0) {
			int lMaxP=OnSet-2*tAve;
			for(i=lMaxP;i<OnSet;i++) if(Data[i]>Data[lMaxP]) lMaxP=i;
			if(Data[lMaxP]>Data[lMaxP+(OnSet-lMaxP)/2] && Data[lMaxP+(OnSet-lMaxP)/2]>Data[OnSet] && l-lMaxP<3*tAve) 
				OnSet=OnsetPoint(tAve,lMaxP,-1);
		}
		r=maxPr;
		while(r<length-tAve && Data[r]>3*stdV) r++;
		OffSet=OffsetPoint(r,length-tAve,-1);
        SubSetting(OnSet,maxP,OffSet,R1);  
		return;
	}

// Looks for if Q and S(S2) exist, from left and right seperately
	int p,q;   //p--from left; q--from right;
	for(p=0;p<length;p++) {
		if(VHs[p]!=0) {
			if(Data[p]>OnsetV) break;  //must have one
			if(Data[p]<OnsetV) break;  //maybe, consider OffsetV
		}
	}
	if(VHs[p]<0) {   //q/Q found (data[p]<OnsetV)
		minP=p;
		OnSet=OnsetPoint(tAve,minP,1);  
		if(abs(Data[minP])<sbHold && minP-OnSet>OnsetWidth) OnSet=OnsetPoint(OnSet,minP,1);
		for(p=minP+1;p<length;p++) if(VHs[p]>0 && Data[p]>OnsetV) break;
		for(i=minP;i<p;i++) if(VHs[i]<0) if(Data[i]<Data[minP]) minP=i;
		for(r=p;r>minP;r--) if(Data[r]<=Data[OnSet]) break;
        SubSetting(OnSet,minP,r,Q);   //q/Q found
	}   //now p is at a hill point (VHs[p]>0 now), wether Q is found or not

	for(q=length-1;q>0;q--) {
		if(VHs[q]>0 && Data[q]>OnsetV) break;  //must have one
		if(VHs[q]<0) break;//OffsetV||found) break;  //maybe, consider OnsetV
	}

	if(VHs[q]<0) {    // s/S found but not sure S1 or S2
		minP=q;
		int rPos=length-(length-minP)/3;  //&2015-12-12 following 7 lines added
		int lPos=length-2*(length-minP)/3;
		l=minP;
		while(Data[l]<-4*stdV && l<lPos) l++;
		r=length-tAve;
		while((Data[r]>3*stdV/2 && r>rPos) || r-l>8*tAve) r--;
		OffSet=OffsetPoint(l,r,1);      //???????????? r-tAve OK M01-121
		if(OffSet+2*tAve<length && Data[OffSet]>Data[OffSet+tAve] && Data[OffSet+tAve/2]>Data[OffSet+3*tAve/2]) {
			int tMinP=OffSet-tAve/2;
			for(i=tMinP;i<OffSet+2*tAve;i++) if(Data[i]<Data[tMinP]) tMinP=i;
			if(tMinP-l<2*tAve && tMinP<r) OffSet=OffsetPoint(tMinP,r,1);
		}  
		if(Data[OffSet]<-stdV && OffSet+5*tAve<length) {
			int tMinP=OffSet-tAve/2;  //tAve=10ms
			int tMaxP=tMinP;
			for(i=tMinP;i<OffSet+4*tAve;i++) {
				if(Data[i]<Data[tMinP]) tMinP=i;
				if(Data[i]>Data[tMaxP]) tMaxP=i;
			}
			if(Data[tMaxP]>Data[OffSet] && Data[OffSet]>Data[tMinP] && tMaxP<tMinP && tMinP<r) OffSet=OffsetPoint(tMinP,r,1);
		}
		r=OffSet;

		for(q=minP-1;q>0;q--) if(VHs[q]>0 && Data[q]>OnsetV/2) break;
		for(i=minP;i>q;i--) if(VHs[i]<0) if(Data[i]<Data[minP]) minP=i;
		for(l=q;l<minP;l++) if(Data[l]<=Data[r]) break;     
        SubSetting(l,minP,r,S1);   //not sure S1 or S2
	}   //now q is at a hill point (VHs[q]>0 now), wether S(S2) is found or not

// to check if sigle R or float R
	for(i=p;i<=q;i++) if(VHs[i]<-1 && (Data[i]<OnsetV||Data[i]<OffsetV)) break;
	if(i>=q) {   //Sigle R
		maxP=p;
		for(i=p;i<=q;i++) if(VHs[i]>1 && Data[i]>Data[maxP]) maxP=i;
		if(Q[0]==0) {  
			int minPk=0;
			for(int k=0;k<p;k++) if(Data[k]<Data[minPk]) minPk=k;
			int pt=p;
			if(Data[pt]<stdV/2) while(pt<maxP && Data[pt]<3*stdV) pt++;
			while(pt>minPk && Data[pt]>3*stdV) pt--;
			OnSet=l=OnsetPoint(minPk/2,pt,-1);  //without Q
		}
		else l=Q[3];
		if(S1[0]==0) {
			if(VHs[q]<0 || Data[q]<OnsetV) OffSet=r=q;   //Y2015-12-12 old: if(before Data[q]-OnsetV<stdV)
			else OffSet=r=OffsetPoint(q,length-tAve,-1);  //without S
		}
		else r=S1[1];
        SubSetting(l,maxP,r,R1);   //R1 set
	}
	else {  //  Double R (with S1 beteen R1 and R2
		for(minP=i;i<q;i++) if(VHs[i]<0 && Data[i]<Data[minP]) minP=i;  //find S1;
// looks for R1		
		for(maxP=i=p;i<minP;i++) if(VHs[i]>0 && Data[i]>Data[maxP]) maxP=i;  //R1
		if(Q[0]==0) {
			int minPk=0;
			for(int k=0;k<p;k++) if(Data[k]<Data[minPk]) minPk=k;
			int pt=p;
			if(Data[pt]<stdV/2) while(pt<maxP && Data[pt]<3*stdV) pt++;
			while(pt>minPk && Data[pt]>3*stdV) pt--;
			OnSet=l=OnsetPoint(minPk/2,pt,-1);  //without Q
		}
		else l=Q[3];
		for(r=maxP;r<minP;r++) if(Data[r]<=Data[l]) break;
        SubSetting(l,maxP,r,R1);   //R1 set
//  looks for R2
		for(maxP=i=minP+1;i<=q;i++) if(VHs[i]>0 && Data[i]>Data[maxP]) maxP=i;  //R2
		if(S1[0]==0) OffSet=r=OffsetPoint(q,length-tAve/2,-1);  //without S
		else {    //S2 set
			for(i=0;i<4;i++) S2[i]=S1[i];
			r=S1[1];    
		}
		for(l=maxP;l>minP;l--) if(Data[l]<=Data[r]) break;
        SubSetting(l,maxP,r,R2);   //R2 set
        SubSetting(R1[3],minP,R2[1],S1);   //S1 set
	}  
}

void QRS_Complex::CorrectFeatures()
{  
//following OnSet Correct
	int w=16*m_nSampleRate/1000;    //big error before 2017-3-6  16*1000/m_nSampleRate
	if(Q[0]==1 && R1[0]!=0) {  //only for short PR
		int maxP=1;
		for(int i=2;i<Q[1]-w;i++) if(Data[i]>Data[maxP]) maxP=i;
		if(maxP<w/2) maxP=w/2;
		int sum0=0;
		for(int k=0;k<w;k++) sum0+=Data[Q[1]-w/2+k];
		int sum=sum0;
		for(int i=maxP-w/2;i<Q[1]-w/2-1;i++) {
			sum=0;
			for(int k=0;k<w;k++) sum+=Data[i+k];
			if(sum<sum0) break;
		}
		if(sum>=sum0) {
			int minP=Q[1];
			for(int i=Q[1]+1;i<Q[2];i++) if(abs(Data[i])<abs(Data[minP])) minP=i;
			if(Data[minP]-Data[Q[2]]>short(20/m_dUvperbit)) {
				OnSet=Q[1]=minP;
			}
			else {
				Q[0]=0;
				OnSet=R1[1]=Q[2];
			}	
		}
	}

// 可能找不到谷（M02-042 avl）拟在correct中解决  //2017-3-26
	if(R1[0]!=0 && S1[0]==0) {
		int wT=100*m_nSampleRate/1000;
		short vV=short(100/m_dUvperbit);
		if(OffSet-OnSet<wT && length-OffSet>2*wT/3) {
			int minP=OffSet;
			for(int i=OffSet;i<Min(OffSet+wT,length-wT/10);i++) if(Data[i]<Data[minP]) minP=i;
			if(Data[OffSet]-Data[minP]>vV/2 && Data[Min(minP+wT,length-wT/10)]-Data[minP]>vV && 
				Data[OnSet]-Data[minP]>vV && minP-OffSet<2*wT/3) {
				OffSet=OffsetPoint(minP,Min(minP+wT,length-wT/10),1);
				SubSetting(R1[3],minP,OffSet,S1);
			}
		}
	}

	if(Q[0]==1 && R1[0]!=0) {
		double ZuV=Min(Data[Q[1]]-Data[Q[2]],Data[Q[3]]-Data[Q[2]])*m_dUvperbit;
		double ZmS=(Q[3]-Q[1])*1000/m_nSampleRate;
		if(ZuV<200 && 3*ZmS>ZuV+70) {     //line formular  mS=uV/3+70/3  Y=aX+b
			OnSet=OnsetPoint(Q[1],R1[1]+(R1[2]-R1[1])/2,-1);
			SubSetting(OnSet,R1[2],R1[3],R1);
			Q[0]=0;
		}
	} 

	short stV=short(100/m_dUvperbit);
	int bT=10*m_nSampleRate/1000;
/*  // Simulator 124594-018 III Onset problem 2016-5-16
	if(Q[0]==0 && R1[0]!=0 && OnSet>5*bT) {
		int maxP=OnSet-3*bT;
		for(int i=maxP;i<=OnSet;i++) if(Data[i]>Data[maxP]) maxP=i;
		if(maxP<OnSet && Data[maxP]-Data[bT]>stV && 2*(Data[maxP]-Data[bT])>(Data[R1[2]]-Data[maxP])) {
			OnSet=OnsetPoint(bT,maxP,-1);
			SubSetting(OnSet,R1[2],R1[3],R1);
		}
	}
*/
	if((Q[0]!=0 || (Q[0]==0 && R1[0]==0 && S1[0]!=0)) && OnSet>5*bT) { //?????????????
		int minP=OnSet-3*bT;
		for(int i=minP;i<=OnSet;i++) if(Data[i]<Data[minP]) minP=i;
		int t= Q[0]==0? S1[2]:Q[2];
		if(minP<OnSet && Data[bT]-Data[minP]>stV && 3*(Data[bT]-Data[minP])>2*(Data[minP]-Data[t])) {
			t=OnsetPoint(bT,minP,1);
			if(Data[t]-Data[minP]>stV) {
				OnSet=t;
				if(Q[0]==0) SubSetting(OnSet,S1[2],S1[3],S1);
				else SubSetting(OnSet,Q[2],Q[3],Q);
			}
		}
	}

	if(Q[0]==0 && R1[0]==1 && S1[0]==2) {   //很小的r不要
		if(Data[R1[2]]-Data[OnSet]<stV && Data[S1[2]]<-10*stV && R1[3]-OnSet<5*bT) {
			R1[0]=0;
			OnSet=R1[2];
			SubSetting(OnSet,S1[2],S1[3],S1);
		}
	}

	stV=short(50/m_dUvperbit);
	if(Q[0]==0 && R1[0]==1 && S1[0]==1 && R2[0]==2) {   //小且宽的r不要
		if(Data[R1[2]]<stV && R1[3]-R1[1]>5*bT) {
			OnSet=R1[2];
			SubSetting(OnSet,S1[2],S1[3],Q);
			SubSetting(R2[1],R2[2],R2[3],R1);
			R2[0]=0;
			if(S2[0]!=0) {
				SubSetting(S2[1],S2[2],S2[3],S1);
				S2[0]=0;
			}
			else S1[0]=0;
		}
	}

	if(Q[0]==1 && R1[0]!=0) {    //小且宽的q不要
		if(Data[Q[2]]>-stV && Q[3]-Q[1]>5*bT) {
			Q[0]=0;
			OnSet=Q[2];
			SubSetting(Q[2],R1[2],R1[3],R1); 
		}
	}

// 考虑处理小且宽度s     
	stV=short(80/m_dUvperbit);
	if(S2[0]==1) {
		if(abs(Data[S2[2]])<stV && S2[3]-S2[1]>4*bT) {
			S2[0]=0;
			OffSet=OffsetPoint(R2[2],S2[2],-1);
			SubSetting(R2[1],R2[2],OffSet,R2);
		}
	}
// M02-008 III offset  problem
	if(S2[0]==0 && R2[0]==1) {
		if(abs(Data[R2[2]])<2*stV/3 && R2[3]-R2[1]>4*bT) {
			R2[0]=0;
			OffSet=OffsetPoint(S1[2],R2[2],1);
			SubSetting(S1[1],S1[2],OffSet,S1);
		}
	}
//belowing
	if(R2[0]==0 && S1[0]==1) {
		if(abs(Data[S1[2]])<2*stV/3 && S1[3]-S1[1]>5*bT && S1[3]-S1[2]>3*bT) {
			S1[0]=0;
			OffSet=OffsetPoint(R1[2],S1[2],-1);
			SubSetting(R1[1],R1[2],OffSet,R1);
		}
	}
// aboving
	stV=short(200/m_dUvperbit);
	int stdT=30*m_nSampleRate/1000;

	int tCTS=12*m_nSampleRate/1000;   //for CTS
	short vCTS=short(15/m_dUvperbit);

	if(Q[0]==1 && R1[0]!=0 && Data[Q[1]]-Data[Q[2]]<stV/2 && Q[2]-OnSet>stdT) {   //小q并且短PR 不要q
		int dA=0;
		int dT=Min(OnSet,Q[2]-OnSet);
		for(int i=0;i<dT;i++) dA+=(Data[OnSet-dT+i]-Data[OnSet+i]);
		if(4*dA>stV*dT && (Data[R1[2]]-Data[R1[1]])-(Data[Q[1]]-Data[Q[2]])>stV/4) {      //???还要增加条件
			Q[0]=0;
			OnSet=Q[2];
			SubSetting(OnSet,R1[2],R1[3],R1);
		}
	} 

	if(Q[0]==0 && R1[0]!=0 && OnSet-3*tCTS>0 && R1[2]-R1[1]>2*tCTS) {  //for CTS small q
		if(abs(Data[OnSet-2*tCTS]-Data[OnSet-tCTS])<vCTS && Data[OnSet-2*tCTS]-Data[OnSet]>vCTS) {
			int OnP=OnsetPoint(OnSet-3*tCTS,OnSet,1);
			if(OnSet-OnP<tCTS) {
				SubSetting(OnP,OnSet,OnSet+(OnSet-OnP),Q);
				SubSetting(Q[3],R1[2],R1[3],R1);
				OnSet=OnP;
			}
		}
	} 
//   OffSet Correct
//S-correct
	stV=short(150/m_dUvperbit);
	if((S2[0]!=0 || (S1[0]!=0 && R2[0]==0)) && Data[OnSet]-Data[OffSet]>stV) {
		int minP=OffSet;
		int endP=length-20*m_nSampleRate/1000;
		if(endP>OffSet+60*m_nSampleRate/1000) endP=OffSet+60*m_nSampleRate/1000;
		for(int i=OffSet;i<endP;i++) if(Data[i]<Data[minP]) minP=i;
		if(Data[OnSet]-Data[OffSet]>3*stV/2) {
			int pTemp=OffSet;    //OffSet 2017-2-22
			while(Data[OnSet]-Data[pTemp]>3*stV/2 && pTemp<endP-10*m_nSampleRate/1000) pTemp++;
			if(Data[pTemp]-Data[OffSet]>2*stV/3) OffSet=OffsetPoint(pTemp,endP,1);
			if(S2[0]!=0) SubSetting(S2[1],S2[2],OffSet,S2);
			else SubSetting(S1[1],S1[2],OffSet,S1);
		}
		else if(Data[OffSet]-Data[minP]>stV/3 && minP-OffSet<30*m_nSampleRate/1000) {  //Data[OffSet]>Data[minP] //2017-2-22
			OffSet=OffsetPoint(minP,endP,1);
			if(S2[0]!=0) SubSetting(S2[1],S2[2],OffSet,S2);  //????
			else SubSetting(S1[1],S1[2],OffSet,S1);
		}
	}

	//following for 43
	if((S2[0]!=0 || (S1[0]!=0 && R2[0]==0)) && Data[OffSet]-Data[OnSet]>stV) {
		int t=10*m_nSampleRate/1000;
		if(S2[0]!=0) {
			int d=Min(length-OffSet-1,OffSet-S2[2]);
			if(Data[OffSet+d]-Data[OffSet-d]>stV && abs((Data[OffSet+d]-Data[OffSet])-(Data[OffSet]-Data[OffSet-d]))<stV/2) {
				int minP=S2[2]+t;
				for(int i=S2[2]+t;i<S2[3]-t;i++) if(abs(Data[i])<abs(Data[minP])) minP=i;
				OffSet=S2[3]=minP;
				SubSetting(S2[1],S2[2],OffSet,S2);
			}
		}
		else if(S1[0]!=0 && R2[0]==0) {
			int d=Min(length-OffSet-1,OffSet-S1[2]);
			if(Data[OffSet+d]-Data[OffSet-d]>stV && abs((Data[OffSet+d]-Data[OffSet])-(Data[OffSet]-Data[OffSet-d]))<stV/2) {
				int minP=S1[2]+t;
				for(int i=S1[2]+t;i<S1[3]-t;i++) if(abs(Data[i])<abs(Data[minP])) minP=i;
				OffSet=S1[3]=minP;
				SubSetting(S1[1],S1[2],OffSet,S1);
			}
		}
	}  

	if(((R2[0]!=0 && S2[0]==0) || (R1[0]!=0 && S1[0]==0)) && Data[OnSet]-Data[OffSet]>stV) {
		int t=10*m_nSampleRate/1000;
		if(R2[0]!=0 && S2[0]==0) {
			int d=Min(length-OffSet-1,OffSet-R2[2]);
			if(Data[OffSet-d]-Data[OffSet+d]>2*stV/3 && abs((Data[OffSet]-Data[OffSet+d])-(Data[OffSet-d]-Data[OffSet]))<stV/2) {
				int minP=R2[2]+t;
				for(int i=R2[2]+t;i<R2[3]-t;i++) if(abs(Data[i])<abs(Data[minP])) minP=i;
				OffSet=R2[3]=minP;
				SubSetting(R2[1],R2[2],OffSet,R2);
			}
		}
		else if(R1[0]!=0 && S1[0]==0) {
			int d=Min(length-OffSet-1,OffSet-R1[2]);
			if(Data[OffSet-d]-Data[OffSet+d]>stV && abs((Data[OffSet]-Data[OffSet+d])-(Data[OffSet-d]-Data[OffSet]))<stV/2) {
				int minP=R1[2]+t;
				for(int i=R1[2]+t;i<R1[3]-t;i++) if(abs(Data[i])<abs(Data[minP])) minP=i;
				OffSet=R1[3]=minP;
				SubSetting(R1[1],R1[2],OffSet,R1);
			}
		}
	}
	//above test for 43

// feature correct
	stV=short(160/m_dUvperbit);
	if(R1[0]!=0 && R2[0]!=0 && S2[0]!=0) {
		short onV=Data[bT];
		short offV=Data[length-bT];
		if((Data[S1[2]]-onV>stV && (Data[S1[2]]-offV>stV/2 || Data[S1[2]]-Data[OffSet]>stV/2)) ||
			((R1[0]>1 || S2[0]>1) && Min(Data[R1[2]],-Data[S2[2]])>3*stV/2 && Data[R2[2]]-Data[S1[2]]<2*stV/3)) {
			SubSetting(R1[1],Max(R1[2],R2[2]),R2[3],R1);
			SubSetting(S2[1],S2[2],S2[3],S1);
			R2[0]=S2[0]=0;
		}
	}
	if(R1[0]==0 && S1[0]!=0) {
		int maxP=OnSet;
		for(int i=OnSet; i<OffSet;i++) if(Data[i]>Data[maxP]) maxP=i;
		if(Data[maxP]-Data[OnSet]>stV/2 && Data[maxP]-Data[OffSet]>stV/2) {
			int minPl=OnSet;
			for(int i=OnSet;i<maxP;i++) if(Data[i]<Data[minPl]) minPl=i;
			int minPr=maxP;
			for(int i=maxP;i<OffSet;i++) if(Data[i]<Data[minPr]) minPr=i;
			int tA=10*m_nSampleRate/1000;
			if(maxP-minPl>tA && minPr-maxP>tA && minPl>OnSet && minPr<OffSet) {
				SubSetting(OnSet,minPl,minPl+(maxP-minPl)/2,Q);
				SubSetting(minPl+(maxP-minPl)/2,maxP,maxP+(minPr-maxP)/2,R1);
				SubSetting(maxP+(minPr-maxP)/2,minPr,OffSet,S1);
			}
		}
	}	
// following deal with some of WPW 
	if(Q[0]==0 && R1[0]!=0) {
		stV=short(110/m_dUvperbit);
		if(Data[R1[2]]>short(400/m_dUvperbit)) stV=short(200/m_dUvperbit);
		if(Data[R1[1]]>stV) {
			int maxP=R1[1];
			int s=Max(R1[1]-30*m_nSampleRate/1000,20*m_nSampleRate/1000);
			for(int i=s;i<R1[1];i++) if(Data[i]>Data[maxP]) maxP=i;
			if(maxP<R1[1]) s=Max(maxP-40*m_nSampleRate/1000,20*m_nSampleRate/1000);
			else s=Max(maxP-60*m_nSampleRate/1000,20*m_nSampleRate/1000);
			OnSet=OnsetPoint(s,maxP,-1);
			SubSetting(OnSet,R1[2],R1[3],R1);
		}
	}  

// 2017-3-6
	stV=int(315/m_dUvperbit);     //350
	w=10*m_nSampleRate/1000;
	if(S1[0]!=0 && R2[0]==0 && Data[S1[2]]<stV && 
		Data[OffSet]<2*stV/3 && Data[OnSet]-Data[OffSet]>2*stV/5 && length-OffSet>6*w) {
			int t=S1[3];
			while(Data[t]-Data[S1[3]]<stV/10 && t<S1[3]+3*w) t++;
			int OffSet0=OffsetPoint(t,Min(S1[3]+7*w,length-w),1);
			if(Data[OffSet0]-Data[OffSet]>stV/3) {
				OffSet=OffSet0;
				SubSetting(S1[1],S1[2],OffSet,S1);
			}
	}
//2017-3-21
//	stV=int(300/m_dUvperbit);     //350
	if(R1[0]!=0 && S1[0]==0 && Data[R1[2]]>stV &&
		Data[OffSet]>stV/2 && Data[OffSet]-Data[OnSet]>2*stV/5 && length-OffSet>6*w) {
			int t=R1[3];
			while(Data[R1[3]]-Data[t]<stV/10 && t<R1[3]+3*w) t++;
			int OffSet0=OffsetPoint(t,Min(R1[3]+7*w,length-w),-1);
			if(Data[OffSet]-Data[OffSet0]>stV/3) {
				OffSet=OffSet0;
				SubSetting(R1[1],R1[2],OffSet,R1);
			}
	}
}

void QRS_Complex::CorrectDependsOnZero()
{ 
	if(S2[0]>0 && Data[S2[2]]>=0) {   //S2
		S2[0]=0;
		SubSetting(R2[1],R2[2],S2[2],R2);
		OffSet=R2[3];
	}
	if(R2[0]>0 && Data[R2[2]]<=0) {   //R2
		R2[0]=0;
		if(S2[0]>0) {
			if(Data[S2[2]]<Data[S1[2]]) S1[2]=S2[2];
			SubSetting(S1[1],S1[2],S2[3],S1);
			S2[0]=0;
		}
		else SubSetting(S1[1],S1[2],R2[2],S1);
		OffSet=S1[3];
	}
	if(S1[0]>0 && Data[S1[2]]>=0) {   //S1
		S1[0]=0;
		if(R2[0]>0) {
			if(Data[R2[2]]>Data[R1[2]]) R1[2]=R2[2];
			SubSetting(R1[1],R1[2],R2[3],R1);
			if(S2[0]>0) SubSetting(S2[1],S2[2],S2[3],S1);
			else OffSet=R1[3];;
			R2[0]=S2[0]=0;
		}
		else  {
			if(R1[0]>0) SubSetting(R1[1],R1[2],S1[2],R1);
			OffSet=S1[2];
		}
	} 
// following the problem
	if(R1[0]>0 && Data[R1[2]]<=0) { //R1
		if(R2[0]==0) {
			R1[0]=0;
			if(Q[0]>0 && S1[0]>0) {
				if(Data[S1[2]]<Data[Q[2]]) Q[2]=S1[2];
				SubSetting(Q[1],Q[2],S1[3],Q);
			}
			else if(Q[0]>0) SubSetting(Q[1],Q[2],R1[2],Q);
			else if(S1[0]>0) SubSetting(R1[1],S1[2],S1[3],Q);
			if(Q[0]>0) OffSet=Q[3];
		}
		else {
			if(Q[0]>0 && Data[S1[2]]<Data[Q[2]]) Q[2]=S1[2];
			if(Q[0]==0) {
				Q[1]=OnsetPoint(Max(1,R1[1]-25*m_nSampleRate/1000),R1[1],1);
				Q[2]=R1[1];
			}
			SubSetting(Q[1],Q[2],S1[3],Q);  // Q[2] may be without value
			SubSetting(R2[1],R2[2],R2[3],R1);
			if(S2[0]>0) SubSetting(S2[1],S2[2],S2[3],S1);
			R2[0]=S2[0]=0;
		}
		OnSet=Q[1];
	}   
// above the problem
	if(Q[0]>0 && Data[Q[2]]>=0) {  //Q
		Q[0]=0;
		if(R1[0]>0) {
			int msT=10*m_nSampleRate/1000;
			int pt=OnsetPoint(Max(Q[1]-3*msT,msT),Q[1]-msT/2,-1);
			if(Data[pt]>-(50/m_dUvperbit) && Data[pt]<Data[Q[2]]) SubSetting(pt,R1[2],R1[3],R1);
			else SubSetting(Q[2],R1[2],R1[3],R1);
			OnSet=R1[1];
		}
	}

	short sbHold=short(500/m_dUvperbit);
	if(Q[0]>0) {
		if(Data[Q[2]]<-sbHold) Q[0]=2;
		else Q[0]=1;
	}
	if(R1[0]>0) {
		if(Data[R1[2]]>sbHold) R1[0]=2;
		else R1[0]=1;
	}
	if(S1[0]>0) {
		if(Data[S1[2]]<-sbHold) S1[0]=2;
		else S1[0]=1;
	}
	if(R2[0]>0) {
		if(Data[R2[2]]>sbHold) R2[0]=2;
		else R2[0]=1;
	}
	if(S2[0]>0) {
		if(Data[S2[2]]<-sbHold) S2[0]=2;
		else S2[0]=1;
	}
}

/*
int QRS_Complex::OnsetPoint(int l,int r,int sign)
{
	int w=20l*m_nSampleRate/1000;
	int w0=5l*m_nSampleRate/1000;

	int V0=int(500/m_dUvperbit);
	while(r-l>2*w&&abs(Data[l]-Data[r])>V0) r--;
	int turnP=TurnPoint(Data,l,r,sign);

	if(turnP>w && abs(Data[turnP])>150/m_dUvperbit) turnP=TurnPoint(Data,l,turnP,sign);
//  avoid Delta wave
	int l1=turnP-w>l? turnP-w:l;
	int r1=turnP+w0<r? turnP+w0:r;
//	int r1=turnP+w<r? turnP+w:r;
	if(!Flat(Data,l1,w,25)) turnP=TurnPoint(Data,l,r1,sign);  
	if(turnP==l) turnP++;
	return turnP;
}
*/
int QRS_Complex::OnsetPoint(int l,int r,int sign)
{
	int turnP1,turnP2;

	int w=40l*m_nSampleRate/1000;
	int V0=int(500/m_dUvperbit);
	while(r-l>w&&abs(Data[l]-Data[r])>V0) r--;

	double slop= double((Data[r]-Data[l]))/double(r-l); //求直线斜率
	turnP1=TurnPoint(Data,l,r,sign);
	turnP2=turnP1;
	if(turnP1-l>w/2) turnP2=TurnPoint(Data,l,turnP1,sign);
	if(turnP1==turnP2) return turnP1;

	double s1=Data[l]+(turnP1-l)*slop; //直线方程
	double s2=Data[l]+(turnP2-l)*slop; //直线方程
	double dif1=(sign>0)? (Data[turnP1]-s1):(s1-Data[turnP1]); 
	double dif2=(sign>0)? (Data[turnP2]-s2):(s2-Data[turnP2]); 

	if(dif1-dif2<dif1/5) return turnP2;
	return turnP1;
}
/*
int QRS_Complex::OffsetPoint(int l,int r,int sign)
{
	int w=20l*m_nSampleRate/1000;
	int w0=5l*m_nSampleRate/1000;
	int SmV=int(150/m_dUvperbit);

	int V0=int(500/m_dUvperbit);
	while(r-l>2*w&&abs(Data[l]-Data[r])>V0) l++;
	int turnP=TurnPoint(Data,l,r,sign);
	
	int r1=turnP+w<r? turnP+w:r;
	int l1=turnP-w0>l? turnP-w0:l;
//	int l1=turnP-w>l? turnP-w:l;
    if(!Flat(Data,r1-w,w,25)) turnP=TurnPoint(Data,l1,r,sign);
//	if(sign>0 && Data[turnP]*m_dUvperbit<-250) turnP=TurnPoint(Data,turnP,r,sign);
	if(sign>0&&turnP-l>3*w) {
		if(Data[turnP]-Data[turnP-2*w]>SmV&&Data[turnP-w]-Data[turnP-3*w]>SmV) {
			r1=turnP+w<r? turnP+w:r;
			turnP=TurnPoint(Data,l+w,r1,sign);
		}
	}
//	if(sign<0 && Data[turnP]*m_dUvperbit>250) turnP=TurnPoint(Data,turnP,r,sign); 
//	if(sign<0) {
//		if(r-3*w>l) TurnPoint(Data,l,r-3*w,sign);
//	}
	return turnP;
}
*/

int QRS_Complex::OffsetPoint(int l,int r,int sign)
{
	int turnP1,turnP2;

//	int CorrectW=4l*m_nSampleRate/1000;     //by Du 2011-4-30????
	int w=40l*m_nSampleRate/1000;
	int V0=int(500/m_dUvperbit);
	while(r-l>w&&abs(Data[l]-Data[r])>V0) l++;
	double slop= double((Data[r]-Data[l]))/double(r-l); //求直线斜率
	turnP1=TurnPoint(Data,l,r,sign);
	turnP2=turnP1;
	if(r-turnP1>w/2) turnP2=TurnPoint(Data,turnP1,r,sign);
	if(turnP1==turnP2) {
//		if(turnP1+CorrectW<r) turnP1+=CorrectW;
		return turnP1;
	}
	double s1=Data[l]+(turnP1-l)*slop; //直线方程
	double s2=Data[l]+(turnP2-l)*slop; //直线方程
	double dif1=(sign>0)? (Data[turnP1]-s1):(s1-Data[turnP1]); 
	double dif2=(sign>0)? (Data[turnP2]-s2):(s2-Data[turnP2]); 

	if(dif1-dif2<dif1/5) {
//		if(turnP2+CorrectW<r) turnP2+=CorrectW;
		return turnP2;
	}
//	if(turnP1+CorrectW<r) turnP1+=CorrectW;
	return turnP1;
}

void QRS_Complex::SubSetting(int l,int p,int r,int *QRSsub)
{
	QRSsub[1]=l;
	QRSsub[2]=p;
	QRSsub[3]=r;
//	if(abs(Data[p]-Data[l])>sbHold && abs(Data[p]-Data[r])>sbHold) QRSsub[0]=2;  //Q/R/S
	if(abs(Data[p])>short(500/m_dUvperbit)) QRSsub[0]=2;  //Q/R/S
	else QRSsub[0]=1;  //q/r/s
}

BOOL QRS_Complex::VHsProcess(BOOL Auto)
{ 
	ValesAndHills();
	Blocks();

	int w=30*m_nSampleRate/1000;
	int minV=int(90/m_dUvperbit);
	int i,l,r,maxP,minP;
	int hillN,valeN;

	if(Auto) {
		l=w<length? w : 0;
		r=length-w>0? length-w : length-1;
	}
	else {
		l=OnSet+1;
		r=OffSet-1;
	}
	hillN=valeN=0;
	maxP=minP=l;
	for(i=l;i<r;i++) {
		if(VHs[i]>0 && Data[i]>0) hillN++;
		if(VHs[i]<0 && Data[i]<0) valeN++;
		if(Data[i]>Data[maxP]) maxP=i;
		if(Data[i]<Data[minP]) minP=i;
	}
	if((hillN>0 && valeN>0) || (hillN>0 && Data[maxP]>minV) || (valeN>0 && Data[minP]<-minV)) return TRUE;

	// 特殊情况：无峰谷或值很小时的处理。2*w: 假定一般左边升降快于右边
	int midV=0;
	for(i=0;i<l;i++) midV+=Data[i];
	if(l>0) midV/=l;

	if(Data[maxP]==Data[minP]) {  //maxP==minP included
		if(Auto) OnSet=l;
		if(Auto) OffSet=r;
	}
	else if((Data[maxP]-midV > midV-Data[minP]) && (midV-Data[minP] < minV/3)) {  
		if(Auto) OnSet=maxP-w>l? maxP-w : l;
		if(Auto) OffSet=maxP+2*w<r? maxP+2*w : r;
		SubSetting(OnSet,maxP,OffSet,R1);
	}
	else if((Data[maxP]-midV < midV-Data[minP]) && (Data[maxP]-midV < minV/3)){
		if(Auto) OnSet=minP-w>l? minP-w : l;
		if(Auto) OffSet=minP+2*w<r? minP+2*w : r;
		SubSetting(OnSet,minP,OffSet,S1);
	}
	else { 
		if(maxP<minP) {
			if(Auto) OnSet=maxP-w>l? maxP-w : l;
			if(Auto) OffSet=minP+w<r? minP+w : r;
			SubSetting(OnSet,maxP,maxP+(minP-maxP)/2,R1);
			SubSetting(maxP+(minP-maxP)/2,minP,OffSet,S1);
		}
		else {  //maxP>minP
			if(Auto) OnSet=minP-w>l? minP-w : l;
			if(Auto) OffSet=maxP+w<r? maxP+w : r;
			SubSetting(OnSet,minP,minP+(maxP-minP)/2,Q);
			SubSetting(minP+(maxP-minP)/2,maxP,OffSet,R1);
		}
	}
	return FALSE;
}

void QRS_Complex::ValesAndHills()
//VHs[i]:  -3: vale, +3: hill
{
	int i,k,l,r;
	int l0,r0,p1,p2;
	int midV,leftV,rightV,leftV0,rightV0;
	float xl,xr,yl,yr,tanL,tanR;
	
	int w=4l*m_nSampleRate/1000;   //4ms Step毫秒
	float hold=float(30*(MinimumWave*m_dUvperbit)/1000.);    // 20mm/mV   30uV - 2015-11-11
//afterward, consider to select different "hold" depends on the singal (two times, 20 and 35) 2010-7-7 

//	int WaveAngle=20; //degree, (22d: 125mm/sec,20mm/mV 相当于 45degree 25mm/s 10mm/mV) 
//	float stdRate=float(tan(WaveAngle*M_PI/180.));   //
	float stdRate=float(tan(22*M_PI/180.));   //
	float stdRate_RH=float(tan(19*M_PI/180.));   //RH: right hill
	float xRate=125.f/m_nSampleRate;  //  125mm/sec
	float yRate=float((m_dUvperbit/1000.)*20.);   // 20mm/mV

	i=3*w;
    while(i<length-3*w) {  //look for vales 
		midV=leftV=rightV=0;
		for(k=-w;k<w;k++) {
			leftV+=Data[i-2*w+k];
		    midV+=Data[i+k];
		    rightV+=Data[i+2*w+k];
		}
		if(midV<leftV && midV<rightV) { //found a vale
		    l=i-2*w;
			leftV0=midV;
		    while(leftV0<leftV && l>3*w) {
				leftV0=leftV0-Data[l+3*w-1]+Data[l+w-1];
		        leftV=leftV-Data[l+w-1]+Data[l-w-1];
			    l--;
			}
		    r=i+2*w;
            rightV0=midV;
		    while(rightV0<rightV && r<length-3*w) {
				rightV0=rightV0-Data[r-3*w]+Data[r-w];
		        rightV=rightV-Data[r-w]+Data[r+w];
			    r++;
			}
			
			p1=i-2*w;
			for(k=p1+1;k<i+2*w;k++) if(Data[k]<Data[p1]) p1=k;
			p2=p1+1;
			while(Data[p1]==Data[p2] && p2<i+2*w) p2++;

			xl=xr=yl=yr=tanL=tanR=0;

			l0=l+2*w-1;   //!!!must be 2*w but not w
			for(k=l0-1;k>=l-w;k--) if(Data[k]>Data[l0]) l0=k;
			  xl=xRate*float(p1-l0);   //mm   25mm/s
			yl=yRate*float(Data[l0]-Data[p1]);         //10mm/mV
				if(xl>0) tanL=yl/xl;

			r0=r-2*w+1;
			for(k=r0+1;k<r+w;k++) if(Data[k]>Data[r0]) r0=k;
			  xr=xRate*float(r0-p2);   //mm
			yr=yRate*float(Data[r0]-Data[p2]);
				if(xr>0) tanR=yr/xr;

			if(tanL>stdRate && tanR>stdRate && yl>hold && yr>hold) {
				 VHs[(p1+p2)/2]=-3;
				 i=r-w;
			}
		}
		i++;
	}  //while vales

	i=3*w;
	 while(i<length-3*w) {  //look for hills
		midV=leftV=rightV=0;
		for(k=-w;k<w;k++) {
			leftV+=Data[i-2*w+k];
		    midV+=Data[i+k];
		    rightV+=Data[i+2*w+k];
		}
		if(midV>leftV && midV>rightV) {  //found a hill
		    l=i-2*w;
			leftV0=midV;
		    while(leftV0>leftV && l>3*w) {
				leftV0=leftV0-Data[l+3*w-1]+Data[l+w-1];
		        leftV=leftV-Data[l+w-1]+Data[l-w-1];
			    l--;
			}
		    r=i+2*w;
            rightV0=midV;
		    while(rightV0>rightV && r<length-3*w) {
				rightV0=rightV0-Data[r-3*w]+Data[r-w];
		        rightV=rightV-Data[r-w]+Data[r+w];
			    r++;
			}
			
			p1=i-2*w;
			for(k=p1+1;k<i+2*w;k++) if(Data[k]>Data[p1]) p1=k;
			p2=p1+1;
			while(Data[p1]==Data[p2] && p2<i+2*w) p2++;

 			xl=xr=yl=yr=tanL=tanR=0;

		    l0=l+2*w-1;
		    for(k=l0-1;k>l-w;k--) if(Data[k]<Data[l0]) l0=k;
			xl=xRate*float(p1-l0);   //ms
			yl=yRate*float(Data[p1]-Data[l0]);
				if(xl>0) tanL=yl/xl;

			r0=r-2*w+1;
		    for(k=r0+1;k<r+w;k++) if(Data[k]<Data[r0]) r0=k;
			xr=xRate*float(r0-p2);   //ms
			yr=yRate*float(Data[p2]-Data[r0]);
			if(xr>0) tanR=yr/xr;

			if(tanL>stdRate && tanR>stdRate_RH && yl>hold && yr>hold) {
			    VHs[(p1+p2)/2]=3;
			    i=r-w;
			}
		}
		i++;
	}  //while

	int VHleft=0;  //处理左边界
	while(VHleft<length-2*w  &&  VHs[VHleft]==0) VHleft++;

	i=3*w;
    while(i<VHleft-w) {  //look for vales 
		midV=leftV=rightV=0;
		for(k=-w;k<w;k++) {
			leftV+=Data[i-2*w+k];
		    midV+=Data[i+k];
		    rightV+=Data[i+2*w+k];
		}
		if(midV<leftV && midV<rightV) { //found a vale
		    r=i+2*w;
            rightV0=midV;
		    while(rightV0<rightV && r<length-3*w) {
				rightV0=rightV0-Data[r-3*w]+Data[r-w];
		        rightV=rightV-Data[r-w]+Data[r+w];
			    r++;
			}

			p1=i-2*w;
			for(k=p1+1;k<i+2*w;k++) if(Data[k]<Data[p1]) p1=k;
			p2=p1+1;
			while(Data[p1]==Data[p2] && p2<i+2*w) p2++;

			xl=xr=yl=yr=tanL=tanR=0;

			l0=TurnPoint(Data,0,p1,1);
			xl=xRate*float(p1-l0);   //mm   25mm/s
			yl=yRate*float(Data[l0]-Data[p1]);         //10mm/mV
				if(xl>0) tanL=yl/xl;

			r0=r-2*w+1;
			for(k=r0+1;k<r+w;k++) if(Data[k]>Data[r0]) r0=k;
			xr=xRate*float(r0-p2);   //mm
			yr=yRate*float(Data[r0]-Data[p2]);
			if(xr>0) tanR=yr/xr;

			if(tanL>stdRate && tanR>stdRate && yl>hold && yr>hold) {
			    VHs[(p1+p2)/2]=-3;
				break;
			}  
		}
		if(midV>leftV && midV>rightV) {  //found a hill
		    r=i+2*w;
            rightV0=midV;
		    while(rightV0>rightV && r<length-3*w) {
				rightV0=rightV0-Data[r-3*w]+Data[r-w];
		        rightV=rightV-Data[r-w]+Data[r+w];
			    r++;
			}
			
			p1=i-2*w;
			for(k=p1+1;k<i+2*w;k++) if(Data[k]>Data[p1]) p1=k;
			p2=p1+1;
			while(Data[p1]==Data[p2] && p2<i+2*w) p2++;

			xl=xr=yl=yr=tanL=tanR=0;

			l0=TurnPoint(Data,0,p1,-1);
			xl=xRate*float(p1-l0);   //ms
			yl=yRate*float(Data[p1]-Data[l0]);
				if(xl>0) tanL=yl/xl;

			r0=r-2*w+1;
		    for(k=r0+1;k<r+w;k++) if(Data[k]<Data[r0]) r0=k;
			xr=xRate*float(r0-p2);   //ms
			yr=yRate*float(Data[p2]-Data[r0]);
			if(xr>0) tanR=yr/xr;

			if(tanL>stdRate && tanR>stdRate && yl>hold && yr>hold) {
			    VHs[(p1+p2)/2]=3;
			    break;
			}
		}
		i++;
	}  //while vales

	int VHright=length-1;  //处理右边界
	while(VHright>2*w  &&  VHs[VHright]==0) VHright--;
	i=length-3*w;

    while(i>VHright+w) {  
		midV=leftV=rightV=0;
		for(k=-w;k<w;k++) {
			leftV+=Data[i-2*w+k];
		    midV+=Data[i+k];
		    rightV+=Data[i+2*w+k];
		}
		if(midV<leftV && midV<rightV) { //found a vale
		    l=i-2*w;
			leftV0=midV;
		    while(leftV0<leftV && l>3*w) {
				leftV0=leftV0-Data[l+3*w-1]+Data[l+w-1];
		        leftV=leftV-Data[l+w-1]+Data[l-w-1];
			    l--;
			}
			
			p1=i-2*w;
			for(k=p1+1;k<i+2*w;k++) if(Data[k]<Data[p1]) p1=k;
			p2=p1+1;
			while(Data[p1]==Data[p2] && p2<i+2*w) p2++;

			xl=xr=yl=yr=tanL=tanR=0;

			l0=l+2*w-1;   //!!!must be 2*w but not w
			for(k=l0-1;k>=l-w;k--) if(Data[k]>Data[l0]) l0=k;
			xl=xRate*float(p1-l0);   //mm   25mm/s
			yl=yRate*float(Data[l0]-Data[p1]);         //10mm/mV
			if(xl>0) tanL=yl/xl;

			r0=TurnPoint(Data,p2,length-1,1);
			xr=xRate*float(r0-p2);   //mm
			yr=yRate*float(Data[r0]-Data[p2]);
			if(xr>0) tanR=yr/xr;

			if(tanL>stdRate && tanR>stdRate && yl>hold && yr>hold) {
			    VHs[(p1+p2)/2]=-3;
			    break;
			}  
		}
		if(midV>leftV && midV>rightV) {  //found a hill
		    l=i-2*w;
			leftV0=midV;
		    while(leftV0>leftV && l>3*w) {
				leftV0=leftV0-Data[l+3*w-1]+Data[l+w-1];
		        leftV=leftV-Data[l+w-1]+Data[l-w-1];
			    l--;
			}
			
			p1=i-2*w;
			for(k=p1+1;k<i+2*w;k++) if(Data[k]>Data[p1]) p1=k;
			p2=p1+1;
			while(Data[p1]==Data[p2] && p2<i+2*w) p2++;

 			xl=xr=yl=yr=tanL=tanR=0;

		    l0=l+2*w-1;
		    for(k=l0-1;k>l-w;k--) if(Data[k]<Data[l0]) l0=k;
			xl=xRate*float(p1-l0);   //ms
			yl=yRate*float(Data[p1]-Data[l0]);
			if(xl>0) tanL=yl/xl;

			r0=TurnPoint(Data,p2,length-1,-1);
			xr=xRate*float(r0-p2);   //ms
			yr=yRate*float(Data[p2]-Data[r0]);
			if(xr>0) tanR=yr/xr;

			if(tanL>stdRate && tanR>stdRate && yl>hold && yr>hold) {
			    VHs[(p1+p2)/2]=3;
			    break;
			}
		}
		i--;
	}  //while vales
}

void QRS_Complex::Blocks()  
// return found or not  +-1: egdge, +-2 peak
{
	int i,k,l,r;

	int w=8l*m_nSampleRate/1000;   //8ms Step毫秒
	int hold=2*w*MinimumWave;
	
	int maxP,minP;
	int maxV,minV,leftV,rightV;
	maxP=minP=4*w;
	for(i=4*w;i<length-4*w;i++) {  //block is at either max or min position
		if(Data[i]>Data[maxP]) maxP=i;
		if(Data[i]<Data[minP]) minP=i;
	}
	maxV=minV=0;
	for(k=-w;k<w;k++) {
		maxV+=Data[maxP+k];
		minV+=Data[minP+k];
	}

//look for positive block
	leftV=maxV+(Data[maxP-w-1]-Data[maxP+w-1]);
	rightV=maxV+(Data[maxP+w]-Data[maxP-w]);
	l=maxP-1;
	while(maxV-leftV<hold && l>4*w) {
		leftV+=(Data[l-w-1]-Data[l+w-1]);
		l--;
	}
	r=maxP+1;
	while(maxV-rightV<hold && r<length-4*w-1) {
		rightV+=(Data[r+w]-Data[r-w]);
		r++;
	}

	if(r-l>3*w) { //found a positive block
		leftV=rightV=0;
		for(k=-w;k<w;k++) {
			leftV+=Data[l-2*w+k];
			rightV+=Data[r+2*w+k];
		}
		if(maxV-leftV>2*hold && maxV-rightV>2*hold) {
            l=TurnPoint(Data,l-2*w,(l+r)/2,1);
	        r=TurnPoint(Data,(l+r)/2,r+2*w,1);
			if(l>=maxP) l=maxP-1;
			if(r<=maxP) r=maxP+1;
			if(r-l>=4*w) {
				for(k=l-w;k<r+w;k++) VHs[k]=0;
		        VHs[l]=VHs[r]=1;
			    VHs[maxP]=2;
			}
		}
	}	

//look for negative block
	leftV=minV+(Data[minP-w-1]-Data[minP+w-1]);
	rightV=minV+(Data[minP+w]-Data[minP-w]);
	l=minP-1;
	while(leftV-minV<hold && l>4*w) {
		leftV+=(Data[l-w-1]-Data[l+w-1]);
		l--;
	}
	r=minP+1;
	while(rightV-minV<hold && r<length-4*w-1) {
		rightV+=(Data[r+w]-Data[r-w]);
		r++;
	}

	if(r-l>3*w) { //found a negative block
		leftV=rightV=0;
		for(k=-w;k<w;k++) {
			leftV+=Data[l-2*w+k];
			rightV+=Data[r+2*w+k];
		}
		if(leftV-minV>2*hold && rightV-minV>2*hold) {
            l=TurnPoint(Data,l-2*w,(l+r)/2,-1);
	        r=TurnPoint(Data,(l+r)/2,r+2*w,-1);
			if(l>=minP) l=minP-1;
			if(r<=minP) r=minP+1;
			if(r-l>=4*w) {
				for(k=l-w;k<r+w;k++) VHs[k]=0;
		        VHs[l]=VHs[r]=-1;
			    VHs[minP]=-2;
			}
		}
	}	
}
///////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
PT_Wave::PT_Wave(char WaveType,int SampleRate,double Uvperbit): ECG_Base(SampleRate,Uvperbit)
{
	Status=-2;     //initial without P/T
	if(WaveType=='P') {
		minWave=int(30/m_dUvperbit);
		Width=100l*m_nSampleRate/1000;
	}
	else {
		minWave=int(45/m_dUvperbit);
	    Width=200l*m_nSampleRate/1000;   //'T'
	}
#ifdef __MY__DOS_DEBUG__
	printf("Width=%d,minWave=%d\n",Width,minWave);getch();
#endif
}

PT_Wave::~PT_Wave()
{
}

void PT_Wave::SetData(short *TemplateData,int Start,int End)
{
	length=End-Start;	//hspecg add+1，2014.01.23
	Data=&TemplateData[Start];  //new short[length];
}
/*
void PT_Wave::P_Set_Test(int Pos)   //for test only
{
	Status=1;
	OnSet=10;
	OffSet=length-10;
	OnePos=Pos;
}
*/
void PT_Wave::PwaveAnalysis()
{
	int nPos[3],pPos[3];
	int pV=Positive_P(pPos);
	int nV=Negative_P(nPos);

	NP_Correct(pV,pPos,nV,nPos);
/*
nPos[0]=nPos[1]=nPos[2]=0;
nV=0;
pPos[0]=1;
pPos[2]=length-2;
pPos[1]=length/2;
pV=3*minWave;
*/

	if(pV>5*nV&&nV<3*minWave/2) nV=0;     //minWave=int(30/m_dUvperbit);
	if(nV>5*pV&&pV<3*minWave/2) pV=0;

	int Msw=120l*m_nSampleRate/1000;
	if(pV>0&&nV>0) {
		if(pPos[1]<nPos[1]&&nPos[2]-pPos[0]>Msw) {
//			if(pV>3*nV&&nV<2*minWave) nV=0;
//			if(nV>3*pV&&pV<2*minWave) pV=0;
			if(Data[pPos[1]]>-5*Data[nPos[1]]&&nV<2*minWave) nV=0;
			if(-Data[nPos[1]]>5*Data[pPos[1]]&&pV<2*minWave) pV=0;
		}
		if(pPos[1]>nPos[1]&&pPos[2]-nPos[0]>Msw) {
//			if(pV>3*nV&&nV<2*minWave) nV=0;
//			if(nV>3*pV&&pV<2*minWave) pV=0;
			if(Data[pPos[1]]>-5*Data[nPos[1]]&&nV<2*minWave) nV=0;
			if(-Data[nPos[1]]>5*Data[pPos[1]]&&pV<2*minWave) pV=0;
		}
	}

	if(pV>0&&nV>0) {
		if(pPos[1]<nPos[1]) {  //+-
			Status=3;
			OnSet=pPos[0];
			OffSet=nPos[2];
			OnePos=pPos[1];
			TwoPos=nPos[1];
		}
		else {  //-+
			Status=4;
			OnSet=nPos[0];
			OffSet=pPos[2];
			OnePos=nPos[1];
			TwoPos=pPos[1];
		}
	}
	else if(pV>0) {  //+
		Status=1;
		OnSet=pPos[0];
		OffSet=pPos[2];
		OnePos=pPos[1]; 
	}
	else if(nV>0) {  //-
		Status=2;
		OnSet=nPos[0];
		OffSet=nPos[2];
		OnePos=nPos[1];
	}
	else Status=0;
	P_OnOffCorrect();
}

void PT_Wave::P_OnOffCorrect()
{
	int w=5l*m_nSampleRate/1000;
	int i,t;
// Onset Correct
	if(OnSet-2*w>0) {
		t=OnSet;
		if(Status==1||Status==3) {
			for(i=OnSet-w;i<OnSet+3*w;i++)
				if(((Data[i-w]-Data[i])+(Data[i+w]-Data[i]))>
				   ((Data[t-w]-Data[t])+(Data[t+w]-Data[t]))) t=i;
			for(i=t-w;i<t+w;i++) if(Data[i]<Data[t]) t=i;
		}
		if(Status==2||Status==4) {
			for(i=OnSet-w;i<OnSet+3*w;i++)
				if(((Data[i]-Data[i-w])+(Data[i]-Data[i+w]))>
				   ((Data[t]-Data[t-w])+(Data[t]-Data[t+w]))) t=i;
			for(i=t-w;i<t+w;i++) if(Data[i]>Data[t]) t=i;
		}
		OnSet=t;
	}
//  Offset Correct
	if(OffSet+2*w<length-1) {
		t=OffSet;
		if(Status==1||Status==4) {
			for(i=OffSet+w;i>OffSet-3*w;i--)
				if(((Data[i-w]-Data[i])+(Data[i+w]-Data[i]))>
				   ((Data[t-w]-Data[t])+(Data[t+w]-Data[t]))) t=i;
			for(i=t-w;i<t+w;i++) if(Data[i]<Data[t]) t=i;
		}
		if(Status==2||Status==3) {
			for(i=OffSet+w;i>OffSet-3*w;i--)
				if(((Data[i]-Data[i-w])+(Data[i]-Data[i+w]))>
				   ((Data[t]-Data[t-w])+(Data[t]-Data[t+w]))) t=i;
			for(i=t-w;i<t+w;i++) if(Data[i]>Data[t]) t=i;
		}
		OffSet=t;
	}
}

void PT_Wave::T_OnOffCorrect()
{
	int w=10l*m_nSampleRate/1000;
	int stV=int(150/m_dUvperbit);
//OnSet Correct
	if(Status!=0) {
		if(OnSet<4*w && OffSet-OnSet>24*w) {
			int maxP,minP;
			maxP=minP=OnSet+4*w;
			for(int i=OnSet+4*w;i<OffSet-2*w;i++) {
				if(Data[i]>Data[maxP]) maxP=i;
				if(Data[i]<Data[minP]) minP=i;
			}
			if(Max(abs(Data[maxP]),abs(Data[minP]))<stV) {
				if(abs(Data[maxP])>=abs(Data[minP])) {
					OnSet=TurnPoint(Data,OnSet+4*w,maxP,-1);
					Status=1;
					OnePos=maxP;
				}
				else {
					OnSet=TurnPoint(Data,OnSet+4*w,minP,1);
					Status=2;
					OnePos=minP;
				}
			}
		}
	}
// OffSet correct 
//	stV=int(200/m_dUvperbit);
	if(Status==3) {  //+-
		if(Data[OnePos]>stV && abs(Data[TwoPos])<stV/10 && 10*abs(Data[TwoPos])<Data[OnePos]) {
			OffSet=TwoPos;
			Status=1;
		}
	}

	if(Status==4) {  //-+
		if(abs(Data[OnePos])>stV && Data[TwoPos]<stV/10 && 10*Data[TwoPos]<abs(Data[OnePos])) {
			OffSet=TwoPos;
			Status=2;
		}
	}
}

void PT_Wave::NP_Correct(int pV, int *pPos, int nV, int *nPos)
{
	int i;
	int w=50l*m_nSampleRate/1000;
	if((pV>0 && nV==0) || (pV>0 && nV!=0 && pPos[1]<nPos[1])) {
		int maxP=pPos[0];
		for(i=pPos[0]-1;i>Max(1,pPos[0]-w);i--) if(Data[i]>Data[maxP]) maxP=i;
		if(abs(Data[maxP]-Data[pPos[1]])<minWave && Data[maxP]-Data[pPos[0]]>minWave && Data[maxP]>2*minWave)
			pPos[0]=TurnPoint(Data,1,maxP,-1);
	}
}

int PT_Wave::Positive_P(int *Pos)  //Y2015-7 after HU, to be corrected 姜震亦（预激）
{
	int w0=10l*m_nSampleRate/1000;   //ms
	int dW=90l*m_nSampleRate/1000;   //ms

	int maxP=w0;
	for(int k=w0;k<length-w0;k++) if(Data[k]>Data[maxP]) maxP=k;  

	if(maxP<3*w0||maxP>length-3*w0) return 0;  

	int St=maxP-dW>w0? maxP-dW:w0;
	int Ed=maxP+dW<length-w0? maxP+dW:length-w0;

	if(Data[maxP]-Data[St]<minWave/2||Data[maxP]-Data[Ed]<minWave/2) return 0;
	if(Data[maxP-w0/2]-Data[St]<2*minWave/5||Data[maxP-w0/2]-Data[Ed]<2*minWave/5) return 0;
	if(Data[maxP+w0/2]-Data[St]<2*minWave/5||Data[maxP+w0/2]-Data[Ed]<2*minWave/5) return 0;

	int LmaxP=maxP-w0;
	int RmaxP=maxP+w0;
	if(Data[maxP]-Data[maxP-3*w0]<minWave/2||Data[maxP]-Data[maxP+3*w0]<minWave/2) {
		if(Data[maxP-3*w0]-Data[St]<minWave/2&&Data[maxP+3*w0]-Data[Ed]<minWave/2) return 0;
		if(Data[maxP]-Data[maxP-3*w0]<minWave/2&&Data[maxP-3*w0]-Data[St]>minWave/2)
			LmaxP=maxP-3*w0;
		if(Data[maxP]-Data[maxP+3*w0]<minWave/2&&Data[maxP+3*w0]-Data[Ed]>minWave/2)
			RmaxP=maxP+3*w0;
	}

	int l=LmaxP;
	int r=RmaxP;

	while(St<l-2*w0&&abs(Data[St]-Data[St+w0])<minWave/4) St++;
	while(Ed>r+2*w0&&abs(Data[Ed]-Data[Ed-w0])<minWave/4) Ed--;

	int t0=l;
	while(l>St+2*w0&&Data[l]>3*minWave/2&&Data[l]>Data[maxP]/2) l--;
	if(t0-l>w0) l+=w0;
	t0=r;
	while(r<Ed-2*w0&&Data[r]>3*minWave/2&&Data[r]>Data[maxP]/2) r++;
	if(r-t0>w0) r-=w0;

	if(St>2*w0) St-=(2*w0);
	if(length-Ed>2*w0) Ed+=(2*w0);

//	int wt=16l*m_nSampleRate/1000;   //ms
	int minV=int(4.8/m_dUvperbit);
	int mS=4l*m_nSampleRate/1000;
	int On0=TurnPoint(Data,St,l,-1);
	int On=On0;
	while(On-mS>St&&Data[On]-Data[On-mS]>minV&&On0-On<=w0/2) On--;
	for(int i=On0;i<On0+w0;i++) if(Data[i]<Data[On]) On=i;
//	int On=TurnPoint(Data,On0-2*w0,On0+w0,-1);
	int Off0=TurnPoint(Data,r,Ed,-1);
	int Off=Off0;
	while(Off+mS<Ed&&Data[Off]-Data[Off+mS]>minV&&Off-Off0<=w0/2) Off++;
    for(int i=Off0;i>Off0-w0;i--) if(Data[i]<Data[Off]) Off=i;

	int pV=(Data[maxP]-Data[On]+Data[maxP]-Data[Off])/2;
	if(pV<minWave/2 || Off-On<2*w0) return 0;

	Pos[0]=On;  Pos[2]=Off;  Pos[1]=maxP;	
	
	return pV;
}

int PT_Wave::Negative_P(int *Pos)
{
	int w0=10l*m_nSampleRate/1000;   //ms
	int dW=65l*m_nSampleRate/1000;   //ms

	int minP=w0;
	for(int k=w0;k<length-w0;k++) if(Data[k]<Data[minP]) minP=k;
	if(minP<3*w0||minP>length-3*w0) return 0;

	int St=minP-dW>0? minP-dW:0;
	int Ed=minP+dW<length-1? minP+dW:length-1;

	if(Data[St]-Data[minP]<3*minWave/5||Data[Ed]-Data[minP]<minWave/2) return 0;
	if(Data[St]-Data[minP-w0/2]<minWave/2||Data[Ed]-Data[minP-w0/2]<2*minWave/5) return 0;
	if(Data[St]-Data[minP+w0/2]<minWave/2||Data[Ed]-Data[minP+w0/2]<2*minWave/5) return 0;

//	if(Data[minP-3*w0]-Data[minP]<minWave/2||Data[minP+3*w0]-Data[minP]<minWave/2) return 0;

	int LminP=minP-w0;
	int RminP=minP+w0;
//	if(Data[maxP]-Data[maxP-3*w0]<minWave/2||Data[maxP]-Data[maxP+3*w0]<minWave/2) {
	if(Data[minP-3*w0]-Data[minP]<minWave/2||Data[minP+3*w0]-Data[minP]<minWave/2) {
		if(Data[St]-Data[minP-3*w0]<minWave/2&&Data[Ed]-Data[minP+3*w0]<minWave/2) return 0;
		if(Data[minP-3*w0]-Data[minP]<minWave/2&&Data[St]-Data[minP-3*w0]>minWave/2)
			LminP=minP-3*w0;
		if(Data[minP+3*w0]-Data[minP]<minWave/2&&Data[Ed]-Data[minP+3*w0]>minWave/2)
			RminP=minP+3*w0;
	}

	int l=LminP;
	int r=RminP;

//	int l,r;
//	l=r=minP;
	while(St<l-2*w0&&abs(Data[St]-Data[St+w0])<minWave/4) St++;
	while(Ed>r+2*w0&&abs(Data[Ed]-Data[Ed-w0])<minWave/4) Ed--;

	int t0=l;
	while(l>St+2*w0&&Data[l]<-3*minWave/2&&Data[l]<Data[minP]/2) l--;
	if(t0-l>w0) l+=w0;
	t0=r;
	while(r<Ed-2*w0&&Data[r]<-3*minWave/2&&Data[r]<Data[minP]/2) r++;
	if(r-t0>w0) r-=w0;

	if(St>2*w0) St-=2*w0;
	if(length-Ed>2*w0) Ed+=2*w0;

	int minV=int(4.8/m_dUvperbit); 
	int mS=4l*m_nSampleRate/1000;
	int On0=TurnPoint(Data,St,l,1);
	int On=On0;
	while(On-mS>St&&Data[On-mS]-Data[On]>minV&&On0-On<=w0/2) On--;
	for(int i=On;i<=On0+w0;i++) if(Data[i]>Data[On]) On=i;
	int Off0=TurnPoint(Data,r,Ed,1);
	int Off=Off0;
	while(Off+mS<Ed&&Data[Off+mS]-Data[Off]>minV&&Off-Off0<=w0/2) Off++;
    for(int i=Off;i>=Off0-w0;i--) if(Data[i]>Data[Off]) Off=i;

	int nV=Data[On]-Data[minP]<Data[Off]-Data[minP]? Data[On]-Data[minP]:Data[Off]-Data[minP];
	if(nV<minWave/2||Off-On<2*w0) return 0;
	
	Pos[0]=On;  Pos[2]=Off;  Pos[1]=minP;	
	return nV;
}

/*
void PT_Wave::Analysis()
{
	int i,maxP,minP,maxPl,minPl,maxPr,minPr;

	int w=10l*m_nSampleRate/1000;   //ms
	int error=int(10./m_dUvperbit);

	maxP=minP=w;   //
	for(i=w;i<length-w;i++) {
		if(Data[i]>Data[maxP]) maxP=i;
		if(Data[i]<Data[minP]) minP=i;
	}

	maxPr=maxPl=maxP;
	while(Data[maxP]-Data[maxPr]<=error && maxPr<length-w) maxPr++;
	while(Data[maxP]-Data[maxPl]<=error && maxPl>w) maxPl--;
//	maxP=(maxPl+maxPr)/2;
	minPr=minPl=minP;
	while(Data[minPr]-Data[minP]<=error && minPr<length-w) minPr++; 
	while(Data[minPl]-Data[minP]<=error && minPl>w) minPl--;
//	minP=(minPl+minPr)/2;

	if(Data[maxP]<minWave && Data[minP]>-minWave) {
		Status=0;  //without PT
		return;
	}

	BOOL pWave=FALSE;
	int count,factor;
	if(Data[maxP]>=minWave) {
		while(Data[maxP]>2*minWave && Data[maxPr]>2*Data[maxP]/3 && maxPr<length-w) maxPr++;
		while(Data[maxP]>2*minWave && Data[maxPl]>2*Data[maxP]/3 && maxPl>w) maxPl--;
		count=0;
		if(Data[minP]>-minWave) {
			for(i=w;i<maxPl;i++) if(Data[i]>Data[maxP]/2) count++; 
		    for(i=maxPr;i<length-w;i++) if(Data[i]>Data[maxP]/2) count++; 
			factor=3;
		}
		else {
			if(maxP<minP) for(i=w;i<maxPl;i++) if(Data[i]>Data[maxP]/2) count++; 
			else for(i=maxPr;i<length-w;i++) if(Data[i]>Data[maxP]/2) count++;
			factor=6;
		}
		if(maxPr-maxPl<Width && count*factor<length) {   //P100ms/T200ms
			pWave=TRUE;
			while(Data[maxPr]<minWave) maxPr--;
			while(Data[maxPl]<minWave) maxPl++;
		}
	}

#ifdef __MY__DOS_DEBUG__
	printf("PT_Wave::Analysis(%c):Data[maxP]=%d,minWave=%d,pWave=%d\n",WaveType,Data[maxP],minWave,pWave);
	printf("maxPr=%d,maxPl=%d,Width=%d,count=%d,factor=%d,length=%d\n",maxPr,maxPl,Width,count,factor,length);
#endif
	BOOL nWave=FALSE;
	if(Data[minP]<=-minWave) {
		while(Data[minP]<-2*minWave && Data[minPr]<2*Data[minP]/3 && minPr<length-w) minPr++;
		while(Data[minP]<-2*minWave && Data[minPl]<2*Data[minP]/3 && minPl>w) minPl--;
		count=0;
		if(Data[maxP]<minWave) {
			for(i=w;i<minPl;i++) if(Data[i]<Data[minP]/2) count++; 
			for(i=minPr;i<length-w;i++) if(Data[i]<Data[minP]/2) count++; 
			factor=3;
		}
		else {
			if(minP<maxP) for(i=w;i<minPl;i++) if(Data[i]<Data[minP]/2) count++; 
			else for(i=minPr;i<length-w;i++) if(Data[i]<Data[minP]/2) count++;
			factor=6;
		}
		if(minPr-minPl<Width && count*factor<length) {   //100ms
			nWave=TRUE;
			while(Data[minPr]>-minWave) minPr--;
			while(Data[minPl]>-minWave) minPl++;
		}
	}

	if(pWave && nWave) {
		if(Data[maxP]>5*abs(Data[minP]) && abs(Data[minP])<2.5*minWave) nWave=FALSE;
		if(abs(Data[minP])>5*Data[maxP] && Data[maxP]<2.5*minWave) pWave=FALSE;
	}

	if((!pWave) && (!nWave)) {
		Status=0;  //without P
		return;
	}

	int onset,offset;
	int Angle=2;
	if(!nWave) {  // pWave==TRUE, P+ only possible
		onset=OnsetPoint(w,maxPl,-1);
		offset=OffsetPoint(maxPr,length-w-1,-1);
		if(Flat(Data,w,onset-w,Angle)) 
			onset=OnsetPoint((w+onset)/2,maxPl,-1);
		if(Flat(Data,offset,length-w-1-offset,Angle)) 
			offset=OffsetPoint(maxPr,(length-w-1+offset)/2,-1);
		if(Data[maxP]-Data[onset]>=0.75*minWave && Data[maxP]-Data[offset]>=0.75*minWave) {
			Status=1;  
			OnSet=onset;  //OnsetPoint(data,w,maxPl,-1,WaveType);
			OffSet=offset;  //OffsetPoint(data,maxPr,length-w,-1,WaveType);
			OnePos=maxP;
		}
		else Status=0;
		return;
	}

	if(!pWave) {  // nWave==TRUE, P- only possible
		onset=OnsetPoint(w,minPl,1);
		offset=OffsetPoint(minPr,length-w-1,1);
		if(Flat(Data,w,onset-w,Angle)) 
			onset=OnsetPoint((w+onset)/2,minPl,1);
		if(Flat(Data,offset,length-w-1-offset,Angle)) 
			offset=OffsetPoint(minPr,(length-w-1+offset)/2,1);
		if(Data[onset]-Data[minP]>=0.75*minWave && Data[offset]-Data[minP]>=0.75*minWave) {
			Status=2;  
			OnSet=onset;  //OnsetPoint(data,w,minPl,1,WaveType);
			OffSet=offset;  //OffsetPoint(data,minPr,length-w,1,WaveType);
			OnePos=minP;
		}
		else Status=0;
		return;
	}

	if(maxP<minP) {    //both pWave&nWave==TRUE,  P+- possible
		onset=OnsetPoint(w,maxPl,-1);
		offset=OffsetPoint(minPr,length-w-1,1);
		if(Flat(Data,w,onset-w,Angle)) 
			onset=OnsetPoint((w+onset)/2,maxPl,-1);
		if(Flat(Data,offset,length-w-1-offset,Angle)) 
			offset=OffsetPoint(minPr,(length-w-1+offset)/2,1);
		if(Data[maxP]-Data[onset]<0.75*minWave) pWave=FALSE;
		if(Data[offset]-Data[minP]<0.75*minWave) nWave=FALSE;
		if(pWave && nWave) {
			Status=3;  
			OnSet=onset;  //OnsetPoint(data,w,maxPl,-1,WaveType);
			OffSet=offset;  //OffsetPoint(data,minPr,length-w,1,WaveType);
			OnePos=maxP;
			TwoPos=minP;
		}
		else if(pWave) {
			Status=1;  
			OnSet=onset;  
			OffSet=OffsetPoint(maxPr,minPl,-1);  //minPl  
			OnePos=maxP;
		}
		else if(nWave) {
			Status=2;  
			OnSet=OnsetPoint(maxPr,minPl,1);//maxPr;  
			OffSet=offset;  
			OnePos=minP;
		}
		else Status=0;  
		return;
	}

	if(maxP>minP) {    //both pWave&nWave==TRUE, P-+ possible
	    onset=OnsetPoint(w,minPl,1);
		offset=OffsetPoint(maxPr,length-w-1,-1);
		if(Flat(Data,w,onset-w,Angle)) 
			onset=OnsetPoint((w+onset)/2,minPl,1);
		if(Flat(Data,offset,length-w-offset,Angle)) 
			offset=OffsetPoint(maxPr,(length-w-1+offset)/2,-1);
		if(Data[maxP]-Data[onset]<0.95*minWave) pWave=FALSE;
		if(Data[offset]-Data[minP]<0.95*minWave) nWave=FALSE;
		if(pWave && nWave) {
			Status=4;  
			OnSet=onset;  //OnsetPoint(data,w,maxPl,-1,WaveType);
			OffSet=offset;  //OffsetPoint(data,minPr,length-w,1,WaveType);
			OnePos=minP;
			TwoPos=maxP;
		}
		else if(pWave) {
			Status=1;  
			OnSet=OnsetPoint(minPr,maxPl,-1);//minPr;  
			OffSet=offset;  
			OnePos=maxP;
		}
		else if(nWave) {
			Status=2;  
			OnSet=onset;  
			OffSet=OffsetPoint(minPr,maxPl,1);//maxPl;  
			OnePos=minP;
		}
		else Status=0;  
		return;
	}
	Status=0;  //without P
	return;
}
*/

int PT_Wave::OnsetPoint(int l,int r,int sign)
{
	int w=16l*m_nSampleRate/1000;
	int w0=2l*m_nSampleRate/1000;
	int turnP=TurnPoint(Data,l,r,sign);

	int l1=turnP-w>l? turnP-w:l;
	int r1=turnP+w0<r? turnP+w0:r;
	if(!Flat(Data,l1,w,15)) turnP=TurnPoint(Data,l,r1,sign);
/*	if(turnP-2*w>l) l1=turnP-2*w;
	else l1=l;
	if(turnP+w<r) r1=turnP+w;
	else r1=r;
	turnP=TurnPoint(Data,l1,r1,sign);   */
	return turnP;
}

int PT_Wave::OffsetPoint(int l,int r,int sign)
{
	int w=16l*m_nSampleRate/1000;
	int w0=2l*m_nSampleRate/1000;
	int turnP=TurnPoint(Data,l,r,sign);
	
	int r1=turnP+w<r? turnP+w:r;
	int l1=turnP-w0>l? turnP-w0:l;
    if(!Flat(Data,r1-w,w,15)) turnP=TurnPoint(Data,l1,r,sign);
	return turnP;
}

void PT_Wave::FeatureSet(short *TemplateData,int start,int end)
{
	SetData(TemplateData,start,end);
	OnSet=0;
	OffSet=length-1;
	int maxP,minP;
	maxP=minP=0;
	for(int i=0;i<length;i++) {
		if(Data[i]>Data[maxP]) maxP=i;
		if(Data[i]<Data[minP]) minP=i;
	}
	if(Data[maxP]>minWave && Data[minP]<-minWave) {
		if(maxP<minP) {
			Status=3;
			OnePos=maxP;
			TwoPos=minP;
		}
		else {
			Status=4;
			OnePos=minP;
			TwoPos=maxP;
		}
	}
	else if(Data[maxP]>minWave && Data[minP]>=-minWave) {
		Status=1;
		OnePos=maxP;
	}
	else if(Data[maxP]<minWave && Data[minP]<-minWave) {
		Status=2;
		OnePos=minP;
	}
	else {
		if(Data[maxP]>=abs(Data[minP])) {
			Status=1;
			OnePos=maxP;
		}
		else {
			Status=2;
			OnePos=minP;
		}
	}
}

void PT_Wave::TwaveAnalysis()
{
	int i,k,maxP,minP;
	
	short stdTv=short(40/m_dUvperbit);
	int step=50*m_nSampleRate/1000;
	int hillP,valeP;
	hillP=valeP=2*step;
	for(i=2*step;i<length-step;i++) {
		if(Data[i]-Data[i-step]>stdTv&&Data[i]-Data[i+step]>stdTv) {
			hillP=i;
			for(k=i-step/2;k<i+step/2;k++) if(Data[k]>Data[hillP]) hillP=k;
			break;
		}
	}
	for(i=step;i<length-step;i++) {
		if(Data[i-step]-Data[i]>stdTv&&Data[i+step]-Data[i]>stdTv) {
			valeP=i;
			for(k=i-step/2;k<i+step/2;k++) if(Data[k]<Data[valeP]) valeP=k;
			break;
		}
	}

	maxP=minP=step;
	for(i=step;i<length-step;i++) {
		if(Data[i]>Data[maxP]) maxP=i;
		if(Data[i]<Data[minP]) minP=i;
	}

	if(hillP!=2*step&&Data[hillP]>=Data[maxP]) maxP=hillP;
	if(valeP!=2*step&&Data[valeP]<=Data[minP]) minP=valeP;

	step=20*m_nSampleRate/1000;

//  positive wave process
	int pl,pr;
	pl=pr=maxP;  //in [len+step,len-step]
	while(pl-step>0&&Data[pl]-Data[pl-step]>-MinimumWave/4&&Data[pl]>Data[step]) pl-=step;
	if(pl+step<maxP) pl+=step;
	while(pl+step<maxP&&(Data[pl+step]-Data[pl])<MinimumWave/2) pl+=step;
	if(pl-step>0) pl-=step;
	if(pl-step>0) pl-=step;
	while(pr+step<length&&Data[pr]-Data[pr+step]>-MinimumWave/4&&Data[pr]>Data[step]) pr+=step;
	if(pr-step>maxP) pr-=step;
	while(pr-step>maxP&&(Data[pr-step]-Data[pr])<MinimumWave/2) pr-=step;
	if(pr+step<length) pr+=step;
	if(pr+step<length) pr+=step;
	int tempP=pr-2*step>maxP+step? pr-2*step:maxP+step;
	for(i=pr;i>tempP;i--) if(Data[i]<Data[pr]) pr=i;
	BOOL pW=TRUE;
	if(Data[maxP]-Data[pl]<MinimumWave||Data[maxP]-Data[pr]<MinimumWave||pr-pl<2*step) pW=FALSE;
	if(pr+step<length) pr+=step;

//  negative wave process
	int nl,nr;
	nl=nr=minP;  //in [len+step,len-step]
	while(nl-step>0&&Data[nl-step]-Data[nl]>-MinimumWave/4&&Data[nl]<Data[step]) nl-=step;
	if(nl+step<minP) nl+=step;
	while(nl+step<minP&&(Data[nl]-Data[nl+step])<MinimumWave/2) nl+=step;
	if(nl-step>0) nl-=step;
	while(nr+step<length&&Data[nr+step]-Data[nr]>-MinimumWave/4&&Data[nr]<Data[step]) nr+=step;
	if(nr-step>minP) nr-=step;
	while(nr-step>minP&&(Data[nr]-Data[nr-step])<MinimumWave/2) nr-=step;
	if(nr+step<length) nr+=step;
	if(nr+step<length) nr+=step;
	tempP=nr-2*step>maxP+step? nr-2*step:maxP+step;
	for(i=nr;i>tempP;i--) if(Data[i]>Data[nr]) nr=i;
	BOOL nW=TRUE;
	if(Data[nl]-Data[minP]<MinimumWave||Data[nr]-Data[minP]<MinimumWave||nr-nl<2*step) nW=FALSE;
	if(nr+step<length) nr+=step;

	if(nW&&pW&&(Data[maxP]-Data[pr])>5*abs(Data[minP]-Data[nr])) nW=FALSE;
	if(nW&&pW&&abs(Data[minP]-Data[nr])>5*(Data[maxP]-Data[pr])) pW=FALSE;

	if(nW&&pW&&Data[maxP]>4*stdTv&&Data[minP]>-stdTv) nW=FALSE;
	if(nW&&pW&&Data[minP]>-4*stdTv&&Data[maxP]<stdTv) pW=FALSE;

	if(pW&&nW&&abs(maxP-minP)>150*m_nSampleRate/1000) {
		int seW=80*m_nSampleRate/1000;
		if(maxP<seW||maxP>length-seW) pW=FALSE;
		if(minP<seW||minP>length-seW) nW=FALSE;
	}

	if(pW&&nW&&abs(maxP-minP)>125*m_nSampleRate/1000) {
		if(length-maxP<2*step||maxP<2*step) pW=FALSE;
		if(length-minP<2*step||minP<2*step) nW=FALSE;
		if(pW&&nW) {
			if(Data[maxP]>2*abs(Data[minP])) nW=FALSE;
			if(abs(Data[minP])>2*Data[maxP]) pW=FALSE;
		}
		if(pW&&nW) {  ////????????
			if(abs(maxP-length/2)>abs(minP-length/2)) pW=FALSE;
			else nW=FALSE;
		}
	}
	pr=(pr+step)<(length-step)? (pr+step):(length-step);
	nr=(nr+step)<(length-step)? (nr+step):(length-step);

// Set Status 1+, 2-, 3+-, 4-+
	if(pW&&nW) {
		if(maxP>minP) {
			Status=4;
			OnePos=minP;
			TwoPos=maxP;
			OffSet=TurnPoint(Data,maxP+step/2,pr,-1);
			OnSet=TurnPoint(Data,step,minP,1);
		}
		else {
			Status=3;
			OnePos=maxP;
			TwoPos=minP;
			OffSet=TurnPoint(Data,minP+step/2,nr,1);
			OnSet=TurnPoint(Data,step,maxP,-1);
		}
	}
	else if(pW) {
		Status=1; 
		OnePos=maxP;
		OnSet=TurnPoint(Data,step,maxP,-1);
		while(Data[maxP]-Data[pr-step/2]>4*stdTv&&maxP+step<pr-step/2) maxP++;
		if(Data[pr]<-MinimumWave) pr-=step;
		OffSet=TurnPoint(Data,maxP+step/4,pr,-1);
//		if(Data[maxP]-Data[step]<MinWave) Twave->Dir=0;  //??
	}
	else if(nW) {
		Status=2;
		OnePos=minP;
		OnSet=TurnPoint(Data,step,minP,1);
		while(Data[nr-step/2]-Data[minP]>5*stdTv&&minP+step<nr-step/2) minP++;
		if(Data[nr]>MinimumWave) nr-=step;
		OffSet=TurnPoint(Data,minP+step/4,nr,1);
//		if(Data[step]-Data[minP]<MinWave) Twave->Dir=0;  //??
	}
	else Status=0;
	if(Status!=0) {
		int mS=4*m_nSampleRate/1000;
		int minV=int(4/m_dUvperbit);
		int Off0=OffSet;
		while(OffSet+mS<length&&abs(Data[OffSet]-Data[OffSet+mS])>minV&&OffSet-Off0<step/2) 
			OffSet++;
	}

	T_OnOffCorrect();
	AbV=AbstractV();   //2017-12-18
}

unsigned short PT_Wave::AbstractV()  // //2017-12-18
{
	if(Status==0 || OnSet<1 || OffSet<=OnSet) return 0;
	unsigned short maxV=0;
//	for(int i=OnSet+1; i<OffSet; i++) if(Abs(Data[i]-Data[OffSet])>maxV) maxV=Abs(Data[i]-Data[OffSet]);
//	return maxV;
	if(Status==1 || Status==3) 
		for(int i=OnSet+1; i<OffSet; i++) if(Data[i]-Data[OffSet]>maxV) maxV=Data[i]-Data[OffSet];
	if(Status==2 || Status==4)
		for(int i=OnSet+1; i<OffSet; i++) if(Data[OffSet]-Data[i]>maxV) maxV=Data[OffSet]-Data[i];
	return maxV;
}
/*
void KinetEnergy(short *in0,short *in1,short *in2,short *out,int length,int q)   //Y2015/5/28
{
	long i,j;
	double L0,L1,L2,L;
	for(i=q;i<length-q;i++) {
		L=0;
		for(j=i-q+1;j<i+q;j++) {
			L0=1+(in0[j]-in0[j-1])*(in0[j]-in0[j-1]);
			L1=1+(in1[j]-in1[j-1])*(in1[j]-in1[j-1]);
			L2=1+(in2[j]-in2[j-1])*(in2[j]-in2[j-1]);
			L+=sqrt(L0+L1+L2);
		}
//		if(L<32768) out[i]=short(L);
//		else out[i]=-1;
		out[i]=(short)L;  //Y2015-8-24
	}
	short minV=out[q];
	for(i=q;i<length-q;i++) if(out[i]<minV) minV=out[i];
	for(i=q;i<length-q;i++) out[i]-=minV;
	for(i=0;i<q+1;i++) out[i]=out[length-i-1]=0;
}

void EnergyCorrect(long *Edata,int length,int step)
{
	long Max[4];
	for(int k=0;k<4;k++) {
		Max[k]=0;
		for(int i=0;i<length/4;i++) if(Edata[k*length/4+i]>Max[k]) Max[k]=Edata[k*length/4+i];
	}
	long Max1=Max[0]<Max[2]? Max[0]:Max[2];
	long Max2=Max[1]<Max[3]? Max[1]:Max[3];
	long MidMax=Max1>Max2? Max1:Max2;
	for(int i=0;i<length;i++) {
		if(Edata[i]>5*MidMax) {
			int start=i-step>0? i-step:0;
			int end=i+step<length? i+step:length;
			for(int k=start;k<end;k++) Edata[k]=MidMax/length;
		}
	}
}
*/
