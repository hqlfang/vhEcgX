// ECGprop.cpp: implementation of the ECGprop class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>
#include "ECGpropEx.h"
//#include "ECGcodeEx.h"
#include "MyMath.h"

//static char *g_szEcgLead[12]={"I","II","III","aVR","aVL","aVF","V1","V2","V3","V4","V5","V6"};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*
ECGprop::ECGprop()
{
	Lead=NULL;
	Parm=NULL;

	Beats=NULL;
//	m_DataTempl=NULL;

	m_classBeats=NULL;
	m_classTemplates=NULL;
//	m_rCode=NULL;//		new rCodeDiag;
	
	m_nPACpercent=10;m_nPVCpercent=10;
}
*/
ECGprop::ECGprop(short ChNumber,short **DataIn,short Seconds,short Samplerate,double uVperbit)
{
	m_nDataChN=ChNumber;	m_nSeconds=Seconds;	m_nSamplerate=Samplerate;
	m_fuVperbit=uVperbit;
//	m_nTemplChN=0;//(ChNumber<8)?ChNumber:ChNumber+4;

	short i;
	if(DataIn) {
		m_pDataIn=DataIn;	//m_DataTempl=NULL;
		for(i=0;i<m_nDataChN;i++) {m_pDataIn[i]=DataIn[i];}//m_DataTempl[i]=NULL;}
	}
	else m_pDataIn=NULL;
	//ZeroMemory(&Temp,sizeof(Temp));
	
	m_nTemplMaxlen=m_nSamplerate+m_nSamplerate/5;//模板最大长度
//	m_nTemplLength=0;							//用于分析的实际模板长度
//	m_DataTempl=new short *[m_nTemplChN];//12通道模板数据
//	for(i=0;i<m_nTemplChN;i++) m_DataTempl[i]=new short[m_nTemplMaxlen];
//Temp模板其他成员变量待分析后赋值
	Temp.SampleRate=Samplerate;
	Temp.Uvperbit=m_fuVperbit;
	Temp.ChN=0;//m_nTemplChN;
////	Temp.Length=0;
//	Temp.Data=new short *[m_nTemplChN];
//	for(i=0;i<m_nTemplChN;i++) Temp.Data[i]=new short[m_nTemplMaxlen];

	m_nBeatsMax=Seconds*6;	m_nBeats=0;	//最大300bpm
	Beats=new char[m_nBeatsMax+1];
	BeatAdd=new BYTE[m_nBeatsMax+1];

	m_classBeats	=NULL;
	m_classTemplates=NULL;
	m_pOutPut=NULL;
//	m_rCode=NULL;//		new rCodeDiag;
	
	m_nPACpercent=18;m_nPVCpercent=10;

//////////////////////////////
	Parm=new ECGparm;
	Lead=NULL;
	//Lead=new ECGlead[m_nTemplChN];
	//switch(m_nTemplChN) {
	//case 3:
	//	I=II=III=aVR=aVL=aVF=V1=V2=V3=V4=V5=V6=NULL;
	//	X=&Lead[0];	Y=&Lead[1];	Z=&Lead[2];
	//	break;
	//case 12:	case 15:
	//	I	=&Lead[0];	II	=&Lead[1];	III	=&Lead[2];
	//	aVR	=&Lead[3];	aVL	=&Lead[4];	aVF	=&Lead[5];
	//	V1	=&Lead[6];	V2	=&Lead[7];	V3	=&Lead[8];
	//	V4	=&Lead[9];	V5	=&Lead[10];	V6	=&Lead[11];  
	//	if(m_nTemplChN==15) {
	//		X=&Lead[12];	Y=&Lead[13];	Z=&Lead[14];
	//	}
	//	else X=Y=Z=NULL;
	//	break;
	//}
	
	m_lenCorr=short(250L*m_nSamplerate/1000L);
	m_corrTempl[0]=new short[m_lenCorr];	m_corrTempl[1]=new short[m_lenCorr];
	m_corrBeat=new short[m_lenCorr];
}

ECGprop::~ECGprop()
{
//	if(m_rCode) delete m_rCode;
	delete[]m_corrTempl[1];	delete[]m_corrTempl[0];
	delete[]m_corrBeat;

	if(Lead) delete[]Lead;
	delete Parm;

	delete[]Beats;
	delete[]BeatAdd;
		
//	for(short i=0;i<m_nTemplChN;i++) delete[]m_DataTempl[i];
//	delete[]m_DataTempl;//12通道模板数据
		
	for(short i=0;i<Temp.ChN;i++) if(Temp.Data[i]) delete[]Temp.Data[i];
	if(Temp.Data) delete[]Temp.Data;//12通道模板数据

	if(m_classBeats)		delete m_classBeats;		m_classBeats=NULL;
	if(m_classTemplates)	delete m_classTemplates;	m_classTemplates=NULL;
}
	
void ECGprop::InitTemplate(short nTemplChN)
{
	short i;
	if(nTemplChN-m_nDataChN==4) {}
//	else if(nTemplChN==0 && m_nDataChN==8) nTemplChN=12;
	else nTemplChN=m_nDataChN;

	if(nTemplChN!=Temp.ChN && Temp.ChN>0) {
		for(i=0;i<Temp.ChN;i++) {
			if(Temp.Data[i]) delete[]Temp.Data[i];	Temp.Data[i]=NULL;
		}
		if(Temp.Data) delete[]Temp.Data;	Temp.Data=NULL;//12通道模板数据
		if(Lead) delete[]Lead;	Lead=NULL;
	}
	Temp.ChN=nTemplChN;
//	Temp.Length=0;
    if(true) {//}(!Temp.Data) {
		Temp.Data=new short *[Temp.ChN];
		for(i=0;i<Temp.ChN;i++) Temp.Data[i]=new short[m_nTemplMaxlen];
	}
	if(!Lead) Lead=new ECGlead[Temp.ChN];
	if(Temp.ChN>=12) {
		I=&Lead[0];    II=&Lead[1];   III=&Lead[2];
		aVR=&Lead[3];  aVL=&Lead[4];  aVF=&Lead[5];
		V1=&Lead[6];   V2=&Lead[7];   V3=&Lead[8];
		V4=&Lead[9];   V5=&Lead[10];  V6=&Lead[11];  
		if(Temp.ChN>=15) {
			i=Temp.ChN-3;
			X=&Lead[i++];  Y=&Lead[i++];  Z=&Lead[i++];
			i=12;
			A1=&Lead[i++]; A2=&Lead[i++]; A3=&Lead[i++];
			if(Temp.ChN>15) A4=&Lead[i++];	else A4=NULL;
		}
		else X=Y=Z=A1=A2=A3=A4=NULL;
	}
	else if(Temp.ChN==3) {
		I=II=III=aVR=aVL=aVF=V1=V2=V3=V4=V5=V6=NULL;
		A1=A2=A3=A4=NULL;
		X=&Lead[0];	Y=&Lead[1];	Z=&Lead[2];
	}
	else if(Temp.ChN>=6 || Temp.ChN<12) {
		I=&Lead[0];    II=&Lead[1];   III=&Lead[2];
		aVR=&Lead[3];  aVL=&Lead[4];  aVF=&Lead[5];
		V1=V2=V3=V4=V5=V6=NULL;
		X=Y=Z=A1=A2=A3=A4=NULL;
	}
}

BOOL ECGprop::AutoProcess(short nTemplChN,bool all)
{
	InitTemplate(nTemplChN);

	if(m_classBeats)		delete m_classBeats;		m_classBeats=NULL;
	if(m_classTemplates)	delete m_classTemplates;	m_classTemplates=NULL;

	m_classBeats=new MultiLead_ECG(m_nDataChN,m_pDataIn,m_nSeconds,m_nSamplerate,m_fuVperbit);
	if(m_classBeats->ProcessSucceed()) {
		m_pOutPut=&(m_classBeats->OutPut);
		if(m_classBeats->GetProcStatus()==PROC_OK) {

//Temp其他成员变量在此赋值
			Temp.Pos	=m_pOutPut->Temp.Pos;     //叠加位置。***********************新增
			Temp.Length	=m_pOutPut->Temp.Length;	//template data
			Temp.Left	=m_pOutPut->Temp.Left;
			Temp.Right	=m_pOutPut->Temp.Right;  //analysis range is from Left to Right
			Temp.SpikeA	=m_pOutPut->Temp.SpikeA;
			Temp.SpikeV	=m_pOutPut->Temp.SpikeV; // 房室起搏钉位置
//			Temp.msRR	=m_pOutPut->AverageRR;
	
			short i,j;
			m_nBeats=m_pOutPut->BeatsNum;	if(m_nBeats>m_nBeatsMax) m_nBeats=m_nBeatsMax;
			if(Temp.ChN-m_nDataChN==4) {
				for(j=0;j<Temp.Length;j++) {
					Temp.Data[0][j]=m_pOutPut->Temp.Data[0][j];				//I
					Temp.Data[1][j]=m_pOutPut->Temp.Data[1][j];				//II
					Temp.Data[2][j]=m_pOutPut->Temp.Data[1][j]-m_pOutPut->Temp.Data[0][j];		//III=II-I
					Temp.Data[3][j]=-m_pOutPut->Temp.Data[0][j]/2-m_pOutPut->Temp.Data[1][j]/2;	//aVR=-(I+II)/2
					Temp.Data[4][j]=m_pOutPut->Temp.Data[0][j]-m_pOutPut->Temp.Data[1][j]/2;	//aVL=I-II/2
					Temp.Data[5][j]=m_pOutPut->Temp.Data[1][j]-m_pOutPut->Temp.Data[0][j]/2;	//aVF=II-I/2
					for(i=6;i<Temp.ChN;i++) Temp.Data[i][j]=m_pOutPut->Temp.Data[i-4][j];	//V1,...,V6
				}
			}
			else {
				for(j=0;j<Temp.Length;j++) {
					for(i=0;i<Temp.ChN;i++) Temp.Data[i][j]=m_pOutPut->Temp.Data[i][j];	//V1,...,V6
				}
			}
//			m_classTemplates=new MultiLead_Templates(m_nTemplChN,m_DataTempl,m_nTemplLength,m_nSamplerate,m_fuVperbit);
//			m_classTemplates->AutoAnalysis(m_pOutPut->Temp.Pos+start,short(long(m_pOutPut->AverageRR)*m_nSamplerate/1000L));
			if(all) {
				m_classTemplates=new MultiLead_Templates(&Temp,m_pOutPut);
				m_classTemplates->AutoAnalysis();

				LeadParameters();
				CommParameters();
			}
			BeatsClassify();
			return TRUE;
		}
	}
	return FALSE;
}

short ECGprop::GetTemplChN()
{
	return (Temp.ChN>0)?Temp.ChN:0;
}

	
BOOL ECGprop::TemplIsOk()
{// {return m_classTemplates?TRUE:FALSE;}
	if(m_classTemplates && m_pOutPut->Status==PROC_OK) return TRUE;
	else return FALSE;
}
void ECGprop::CommParameters()
{
	if(!m_classTemplates) return;

	QTdiscretion QTd;
	Parm->RR	=m_classTemplates->RR();
	Parm->HR	=m_classTemplates->HR();
	Parm->Pd	=m_classTemplates->msInterval(cPd);
	Parm->PR	=m_classTemplates->msInterval(cPRd);
	Parm->QRS	=m_classTemplates->msInterval(cQRSd);
	Parm->QT	=m_classTemplates->msInterval(cQTd);	if(Parm->QT<0) Parm->QT=0;
	if(Parm->QT>0) Parm->QTC	=m_classTemplates->msQTc(Parm->QT);	else Parm->QTC=0;
	if(GetTemplChN()>=6) {
		Parm->axisP	=m_classTemplates->Axis(Pwave);  //return Degree,  for ChNumber==12 only
		Parm->axisQRS	=m_classTemplates->Axis(QRScomplex);  //return Degree,  for ChNumber==12 only
		Parm->axisT	=m_classTemplates->Axis(Twave);  //return Degree,  for ChNumber==12 only
	}
	else {
		Parm->axisP	=0;//m_classTemplates->Axis(Pwave);  //return Degree,  for ChNumber==12 only
		Parm->axisQRS	=0;//m_classTemplates->Axis(QRScomplex);  //return Degree,  for ChNumber==12 only
		Parm->axisT	=0;//m_classTemplates->Axis(Twave);  //return Degree,  for ChNumber==12 only
	}
	int TemplChN=GetTemplChN();
//	if(TemplChN>=12) {
		m_classTemplates->QTdiscrete(QTd);
		if(QTd.MinLeadNo<0 || QTd.MinLeadNo>=TemplChN) QTd.MinLeadNo=0;
		if(QTd.MaxLeadNo<0 || QTd.MaxLeadNo>=TemplChN) QTd.MaxLeadNo=0;
		if(QTd.msMin<0) QTd.msMin=0;
		if(QTd.msMax<0) QTd.msMax=0;
		Parm->msQTdis	=QTd.msQTd;
		Parm->msMax	=QTd.msMax;
		Parm->msMin	=QTd.msMin;
		Parm->MaxLeadNo	=QTd.MaxLeadNo;
		Parm->MinLeadNo	=QTd.MinLeadNo;
	if(TemplChN>=12) {
		Parm->WPW = m_classTemplates->WPW();  //return: A/a B/b W/w  A型, B型, W型, 小写: 可疑, 其它: 无WPW
		Parm->empty = '\0';
		Parm->uvRV5 = m_classTemplates->uvRV5();   //return unit: uV,         for ChNumber==12 only
		Parm->uvRV6	=m_classTemplates->uvRV6();   //return unit: uV,         for ChNumber==12 only
		Parm->uvSV1	=m_classTemplates->uvSV1();   //return unit: uV,         for ChNumber==12 only
		Parm->uvSV2	=m_classTemplates->uvSV2();   //return unit: uV,         for ChNumber==12 only
		Parm->uvRV1 =uvRa(6);
		Parm->uvSV5	=-uvSa(10);
	}
/*	else {
		//if(QTd.MinLeadNo<0 || QTd.MinLeadNo>11) QTd.MinLeadNo=0;
		//if(QTd.MaxLeadNo<0 || QTd.MaxLeadNo>11) QTd.MaxLeadNo=0;
		//if(QTd.msMin<0) QTd.msMin=0;
		//if(QTd.msMax<0) QTd.msMax=0;
		Parm->msQTdis	=0;//QTd.msQTd;
		Parm->msMax	=0;//QTd.msMax;
		Parm->msMin	=0;//QTd.msMin;
		Parm->MaxLeadNo	=0;//QTd.MaxLeadNo;
		Parm->MinLeadNo	=0;//QTd.MinLeadNo;
		Parm->uvRV5	=0;//m_classTemplates->uvRV5();   //return unit: uV,         for ChNumber==12 only
		Parm->uvRV6	=0;//m_classTemplates->uvRV6();   //return unit: uV,         for ChNumber==12 only
		Parm->uvSV1	=0;//m_classTemplates->uvSV1();   //return unit: uV,         for ChNumber==12 only
		Parm->uvSV2	=0;//m_classTemplates->uvSV2();   //return unit: uV,         for ChNumber==12 only
		Parm->uvRV1 =0;//uvRa(6);
		Parm->uvSV5	=-0;//uvSa(11);
	}*/
	for(short j=0;j<6;j++) Parm->OnOff[j]=m_classTemplates->OnOff[j];

	if(GetAflutAfib()>0) {//hspecg 20150429增加
		Parm->Pd	=Parm->PR	=0;
	}
}
	
void ECGprop::LeadParameters()
{
	if(!m_classTemplates) return;

	short i,j;
	short Incrs=0,Decrs=0;
	for(i=0;i<Temp.ChN;i++) {
//		Lead[i].Pb	=m_classTemplates->Lead[i]->OnOff[0];
//		Lead[i].Pe	=m_classTemplates->Lead[i]->OnOff[1];
//		Lead[i].QRSb=m_classTemplates->Lead[i]->OnOff[2];
//		Lead[i].QRSe=m_classTemplates->Lead[i]->OnOff[3];
//		Lead[i].Tb	=m_classTemplates->Lead[i]->OnOff[4];
//		Lead[i].Te	=m_classTemplates->Lead[i]->OnOff[5];
		for(j=0;j<6;j++) Lead[i].OnOff[j]=m_classTemplates->Lead[i]->OnOff[j];
		Lead[i].Pd	=m_classTemplates->Lead[i]->msInterval(Pd);
		Lead[i].Qd	=m_classTemplates->Lead[i]->msInterval(Qd);
		Lead[i].Rd1	=m_classTemplates->Lead[i]->msInterval(Rd1);
		Lead[i].Rd2	=m_classTemplates->Lead[i]->msInterval(Rd2);
		Lead[i].Sd1	=m_classTemplates->Lead[i]->msInterval(Sd1);
		Lead[i].Sd2	=m_classTemplates->Lead[i]->msInterval(Sd2);
		Lead[i].Td	=m_classTemplates->Lead[i]->msInterval(Td);
		Lead[i].PR	=m_classTemplates->Lead[i]->msInterval(PRd);
		Lead[i].QT	=m_classTemplates->Lead[i]->msInterval(QTd);
		Lead[i].QRS	=m_classTemplates->Lead[i]->msInterval(QRSd);
		Lead[i].Pa1	=m_classTemplates->Lead[i]->uvValue(Pa1);
		Lead[i].Pa2	=m_classTemplates->Lead[i]->uvValue(Pa2);
		Lead[i].Qa	=m_classTemplates->Lead[i]->uvValue(Qa);
		Lead[i].Ra1	=m_classTemplates->Lead[i]->uvValue(Ra1);
		Lead[i].Ra2	=m_classTemplates->Lead[i]->uvValue(Ra2);
		Lead[i].Sa1	=m_classTemplates->Lead[i]->uvValue(Sa1);
		Lead[i].Sa2	=m_classTemplates->Lead[i]->uvValue(Sa2);
		Lead[i].Ta1	=m_classTemplates->Lead[i]->uvValue(Ta1);
		Lead[i].Ta2	=m_classTemplates->Lead[i]->uvValue(Ta2);
		Lead[i].Pstatus=m_classTemplates->Lead[i]->P->Status;
		Lead[i].Tstatus=m_classTemplates->Lead[i]->T->Status;
//		Lead[i].res=0;
		for(j=0;j<8;j++) {
			Lead[i].ST[j]=m_classTemplates->Lead[i]->uvSTvalue((j<4)?j:((j-3)*20));	//STj,ST1,ST2,ST3,ST20,ST40,ST60,ST80
		}
		for(j=0;j<4;j++) {
			Lead[i].STslope[j]=m_classTemplates->Lead[i]->STslope(j*20+20);
		}
		char *qrs=m_classTemplates->Lead[i]->QRSmorpho();
// Du 2010-4-21
//		if((qrs[0]=='Q'&&qrs[1]=='S'&&qrs[2]=='\0')||(qrs[0]=='q'&&qrs[1]=='s'&&qrs[2]=='\0')) {
		if((qrs[0]=='Q' || qrs[0]=='q') && (qrs[1]=='S' || qrs[1]=='s') && qrs[2]=='\0') {
			Lead[i].Qd=Lead[i].QRS;
			Lead[i].Qa=Min3(Lead[i].Qa,Lead[i].Sa1,Lead[i].Sa2);
			Lead[i].Sd1=Lead[i].Sd2=0;
			Lead[i].Sa1=Lead[i].Sa2=0;
		}
//		for(j=0;j<8;j++) Lead[i].morpho[j]='\0';
		for(j=0;j<8;j++) {
			Lead[i].morpho[j]=qrs[j];	if(qrs[j]=='\0') break;
		}
		m_classTemplates->Lead[i]->QRS->Notch(Incrs,Decrs);
		Lead[i].Rnotch=0;
		if(Incrs>0) Lead[i].Rnotch|=0x01;
		if(Decrs>0) Lead[i].Rnotch|=0x10;
	}

	if(GetAflutAfib()>0) {//hspecg 20150429增加
		for(i=0;i<Temp.ChN;i++) {
			Lead[i].Pd=Lead[i].PR=0;
			Lead[i].Pa1=Lead[i].Pa2=0;
		}
	}
}
//		m_classTemplates->Lead[lead]->QRS
//		return 0;
//	}//Lead[lead].nRNotch;}//R波缺口，0,1,2,3:无，上升边，下降边,两边 not done
BOOL ECGprop::IsPacedECG()		//是否起博心电图
{
	char pace=GetPaceMaker();
	if(pace=='\0' || pace=='N') return FALSE;
	else return TRUE;
}

BOOL ECGprop::BeatsClassify()
{
	long i,j,k,n,beats=m_pOutPut->BeatsNum;	if(beats<1) return FALSE;
	for(i=0;i<beats;i++) if(Beats[i]==OK) BeatAdd[i]=1;	else BeatAdd[i]=0;

	short s0=m_lenCorr/2;
	long pos=m_pOutPut->Temp.Pos-s0;
	long len=(long)m_lenCorr,Length=m_classBeats->GetLength();

	BYTE type='0';
	for(i=0;i<beats;i++) Beats[i]=type;		Beats[i]='\0';	n=beats;	//初始化，未分类

	if(IsPacedECG()) {//是起博心电
		short SpikesN=m_pOutPut->SpikesN;
		long *SpikesPos=m_pOutPut->SpikesPos;
		short pre=250*m_nSamplerate/1000;	//看其前有无起博脉冲
		for(i=k=0;i<beats;i++) {
			for(j=k;j<SpikesN;j++) {
				if(m_pOutPut->Beats[i].Pos-SpikesPos[j]>0 && m_pOutPut->Beats[i].Pos-SpikesPos[j]<pre) {
					Beats[i]='Q';	k=j+1;	break;
				}
			}
		}
		if(Beats[0]=='\0') {
			if(m_pOutPut->Beats[0].Pos-pre<0) {Beats[0]='{';}	//左边边界，不便分类Border(unknown)
		}
	}
	else {
		if(m_pOutPut->Beats[n-1].Pos-s0+len>=Length) {n--;Beats[n]='}';}	//右边边界，不便分类Border(unknown)
		if(m_pOutPut->Beats[0].Pos-s0<0) {Beats[0]='{';}	//左边边界，不便分类Border(unknown)
	}

	CMaxLinearCorrelation corr;//(100L*m_nSamplerate/1000L);
	double r=0,rs=0;
	while(n>0) {
		k=-1;
		if(type=='0') {
			for(j=0;j<len;j++) {
				m_corrTempl[0][j]=m_pOutPut->Temp.Data[m_pOutPut->LeadNo][pos+j];
				m_corrTempl[1][j]=m_pOutPut->Temp.Data[m_pOutPut->SubLeadNo][pos+j];
			}
			type++;	//用于叠加模板的心搏，暂为'1'
			for(i=0;i<beats;i++) {
				if(Beats[i]=='Q') continue;
				if(m_pOutPut->Beats[i].Status==OK) {n--;Beats[i]=type;}
			}
		}
		else {
			for(i=0;i<beats;i++) {
				if(Beats[i]=='Q') continue;
				if(Beats[i]=='0') {k=i;break;}
			}
			if(k<0) break;
			pos=m_pOutPut->Beats[k].Pos-s0;
			for(j=0;j<len;j++) {
				m_corrTempl[0][j]=m_pDataIn[m_pOutPut->LeadNo][pos+j];
				m_corrTempl[1][j]=m_pDataIn[m_pOutPut->SubLeadNo][pos+j];
			}
			type++;	n--;	Beats[k]=type;
		}
		k++;

		for(i=k;i<beats;i++) {
			if(Beats[i]=='Q') continue;
			if(Beats[i]!='0') continue;
			pos=m_pOutPut->Beats[i].Pos-s0;	if(pos<0) continue;

			for(j=0;j<len;j++) m_corrBeat[j]=m_pDataIn[m_pOutPut->LeadNo][pos+j];
			r=corr.MaxLinearCorrelation(m_corrTempl[0],m_corrBeat,len);
			for(j=0;j<len;j++) m_corrBeat[j]=m_pDataIn[m_pOutPut->SubLeadNo][pos+j];
			rs=corr.MaxLinearCorrelation(m_corrTempl[1],m_corrBeat,len);
			if((r>0.92 && rs>0.92)) {	n--;Beats[i]=type;	}
			else if((r>0.98 && rs>0.80) || (r>0.80 && rs>0.98)) {
				if(i==0 || RR(i)*100L>long((100L-m_nPACpercent)*RR())) {	n--;Beats[i]=type;	}
			}
		}
	}
	if(type=='1') {//只有一种类型
		for(i=0;i<beats;i++) {
			if(Beats[i]=='Q') continue;
			Beats[i]='N';
			if(i>0) {
				if(QRS()>=120 && PR()<=0) Beats[i]='V';	//宽QRS且无P
				else {
					if(RR(i)*100L<=long((100L-m_nPACpercent)*RR())) {
						Beats[i]='S';
					}
				}
			}
		}
	}
	else {
		if(QRS()>=130) {
			BOOL bN=FALSE;
			n=type;
			while(n>'1') {
				long QRSw=0;
				for(i=k=0;i<beats;i++) {
					if(Beats[i]==(BYTE)n) {
						QRSw+=(m_pOutPut->Beats[i].QRSw+m_pOutPut->Beats[i].SubQRSw);	k++;
					}
				}
				if(k>0) {
					QRSw/=(k*2);	j=max(130,QRS()-20);	j=min(j,QRS());
					if(QRSw<j) {
						for(i=k=0;i<beats;i++) {
							if(Beats[i]==(BYTE)n) {Beats[i]='N';	bN=TRUE;}
						}
					}
				}
				n--;
			}
			if(bN) {
				n=type;
				while(n>'0') {
					short pn=0,pt=0;
					for(i=0;i<beats;i++) {
						if(Beats[i]==(BYTE)n) {
							if(beatPnum(i)>0) pn++;
							pt++;
						}
					}
					if(pn*2>=pt && pt>0) {
						for(i=0;i<beats;i++) {
							if(Beats[i]==(BYTE)n && Beats[i]!='N') Beats[i]='N';
						}
					}
					else {
						for(i=0;i<beats;i++) {
							if(Beats[i]==(BYTE)n && Beats[i]!='N') Beats[i]='V';
						}
					}
					n--;
				}
			}
			else {
				for(i=0;i<beats;i++) {
					if(Beats[i]=='Q') continue;
					if(Beats[i]=='1') Beats[i]='N';
					else Beats[i]='V';
				}
			}
		}
		else {
			BOOL bV=FALSE;
			long nw=0;
			for(i=k=0;i<beats;i++) {
				if(Beats[i]==(BYTE)'1') {
					nw+=(m_pOutPut->Beats[i].QRSw+m_pOutPut->Beats[i].SubQRSw);	k++;
				}
			}
			if(k>0) nw/=(k+k);	nw+=20;	if(nw<110) nw=110;
			if(nw>114) nw=max(nw+10,114);
			n=type;
			while(n>'1') {
				long QRSw=0;
				for(i=k=0;i<beats;i++) {
					if(Beats[i]==(BYTE)n) {
						QRSw+=(m_pOutPut->Beats[i].QRSw+m_pOutPut->Beats[i].SubQRSw);	k++;
					}
				}
				if(k>0) QRSw/=(k*2);//	j=max(130,QRS()-20);	j=min(j,QRS());
				bV=TRUE;
				short pn=0,pt=0;
				for(i=0;i<beats;i++) {
					if(Beats[i]==(BYTE)n) {
						if(beatPnum(i)>0) pn++;
						pt++;
					}
				}
				if(pn*2>=pt || QRSw<nw) bV=FALSE;
				if(bV) {
					for(i=0;i<beats;i++) {
						if(Beats[i]==(BYTE)n) Beats[i]='V';
					}
				}
				else {
					for(i=0;i<beats;i++) {
						if(Beats[i]==(BYTE)n) {
							if(QRSw>=nw) {
								if(/*m_pOutPut->Beats[i].Status==RR_Int && */(i>0 && RR(i)*100L<=RR()*(100L-m_nPVCpercent))) {
									if(QRSw>=120 || beatPnum(i)<1) Beats[i]='V';
									else Beats[i]='S';
								}
								else Beats[i]='N';
							}
							else {
								if(/*m_pOutPut->Beats[i].Status==RR_Int && */(i>0 && RR(i)*100L<=RR()*(100L-m_nPACpercent))) Beats[i]='S';
								else Beats[i]='N';
							}
						}
					}
				}
				n--;
			}
			for(i=k=0;i<beats;i++) {
				if(Beats[i]=='1') {
					if(i>0 && RR(i)*100L<=RR()*(100L-m_nPACpercent)) Beats[i]='S';
					else Beats[i]='N';
				}
			}
		}
	}
	for(i=3;i<beats;i++) {
		if(Beats[i]=='S') {
			if(Beats[i-1]=='N' && Beats[i-2]=='V') Beats[i]='N';
		}
	}

	return TRUE;
}
//	
//void ECGprop::GetStringOfCommonParameter(char *szString)//Only for 12-Lead
//{
//	if(!m_classTemplates) {
//		strcpy(szString,"");	return;
//	}
//
//	char *leads[12]={"I","II","III","aVR","aVL","aVF","V1","V2","V3","V4","V5","V6"};
//	char szQTmaxLead[4],szQTminLead[4];
//	if(QTmaxLead()>=0 && QTmaxLead()<12) strcpy(szQTmaxLead,leads[QTmaxLead()]);	else strcpy(szQTmaxLead,"");
//	if(QTminLead()>=0 && QTminLead()<12) strcpy(szQTminLead,leads[QTminLead()]);	else strcpy(szQTminLead,"");
//	sprintf(szString,
//		"RR=%dms, HR=%dbpm;\n\r"
//		"Pd=%dms, PR=%dms, QRS=%dms;\n\r"
//		"QT=%dms, QTc=%dms;\n\r"
//		"QTd=%dms(%s:%dms,%s:%dms);\n\r"
//		"Axis(P,QRS,T)=%d,%d,%d deg;\n\r"
//		"SV1(%d)+RV5(%d)=%.2fmV;\n\r"
//		"SV2(%d)+RV6(%d)=%.2fmV",
//		RR(),HR(),
//		Pd_ms(),PR(),QRS(),
//		QT(),QTc(),
//		QTdis(),szQTmaxLead,QTmax(),szQTminLead,QTmin(),
//		Paxis(),QRSaxis(),Taxis(),
//		SV1(),RV5(),(SV1()+RV5())*0.001,SV2(),RV6(),(SV2()+RV6())*0.001
//		);
//}

/*
BOOL ECGprop::IsPositiveP(short i)
{
	return (Min(Lead[i].Pa1,Lead[i].Pa2)>0)?TRUE:FALSE;
}

BOOL ECGprop::IsNegativeP(short i)
{
	return (Max(Lead[i].Pa1,Lead[i].Pa2)<0)?TRUE:FALSE;
}

BOOL ECGprop::IsPositiveT(short i)
{
	return (Min(Lead[i].Ta1,Lead[i].Ta2)>0)?TRUE:FALSE;
}

BOOL ECGprop::IsNegativeT(short i)
{
	return (Max(Lead[i].Ta1,Lead[i].Ta2)<0)?TRUE:FALSE;
}
*/
BOOL ECGprop::isFlatT(short i)
{//is flat T if <100uV?
	return (Min(Lead[i].Ta1,Lead[i].Ta2)>=0 && Max(Lead[i].Ta1,Lead[i].Ta2)<100)?TRUE:FALSE;
}

BOOL ECGprop::isQS(short i)	//该导联是QS型吗?
{
	return (Upper(Lead[i].morpho[0])=='Q' && Upper(Lead[i].morpho[1])=='S')?TRUE:FALSE;
}

BOOL ECGprop::isQr(short i)			//该导联是Qr型吗?
{
	return (Lead[i].morpho[0]=='Q' && Lead[i].morpho[1]=='r')?TRUE:FALSE;
}

BOOL ECGprop::isrsR(short i)			//该导联是rsR'型吗?
{
	return (Lead[i].morpho[0]=='r' && Lead[i].morpho[1]=='s' && Lead[i].morpho[2]=='R')?TRUE:FALSE;
}

BOOL ECGprop::isrsr(short i)			//该导联是rsr'型吗?
{
	return (Lead[i].morpho[0]=='r' && Lead[i].morpho[1]=='s' && Lead[i].morpho[2]=='r')?TRUE:FALSE;
}

BOOL ECGprop::isRSrs(short i) //是R(r)S(s)?
{
	return ((Lead[i].morpho[0]=='R' || Lead[i].morpho[0]=='r') && (Lead[i].morpho[1]=='s' || Lead[i].morpho[1]=='S'))?TRUE:FALSE;
}
	
short ECGprop::uvQa(short lead)
{
	return (isQS(lead))?Min3(Lead[lead].Qa,Lead[lead].Sa1,Lead[lead].Sa2):Lead[lead].Qa;
}
	
short ECGprop::msQd(short lead)
{
	return (isQS(lead))?Max3(Lead[lead].Qd,Lead[lead].Sd1,Lead[lead].Sd2):Lead[lead].Qd;
}
	
short ECGprop::uvQRSa(short lead)//QRS振幅
{
	return Max(uvRa1(lead),uvRa2(lead))-Min3(uvQa(lead),uvSa1(lead),uvSa2(lead));
}
	
short ECGprop::uvRa(short lead)
{
	return Max(uvRa1(lead),uvRa2(lead));
}
	
short ECGprop::uvSa(short lead)
{
	return (isQS(lead))?Min3(Lead[lead].Qa,Lead[lead].Sa1,Lead[lead].Sa2):Min(Lead[lead].Sa1,Lead[lead].Sa2);
}

short ECGprop::uvPa(short lead)	//绝对值较大之Pa
{
	if(Abs(uvPa1(lead))>Abs(uvPa2(lead))) return uvPa1(lead);
	else return uvPa2(lead);
}

short ECGprop::uvTa(short lead)	//绝对值较大之Ta
{
	if(Abs(uvTa1(lead))>Abs(uvTa2(lead))) return uvTa1(lead);
	else return uvTa2(lead);
}

short ECGprop::positivePa(short lead)//返回正P波幅值uV,没有返回0
{
	return Max3(uvPa1(lead),uvPa2(lead),0);
}
	
short ECGprop::negativePa(short lead)//返回负P波幅值uV,没有返回0
{
	return Min3(uvPa1(lead),uvPa2(lead),0);
}
	
short ECGprop::positiveTa(short lead)//返回正T波幅值uV,没有返回0
{
	return Max3(uvTa1(lead),uvTa2(lead),0);
}
	
short ECGprop::negativeTa(short lead)//返回负T波幅值uV,没有返回0
{
	return Min3(uvTa1(lead),uvTa2(lead),0);
}
	
short ECGprop::positivePd(short lead)
{//如果是正P波,返回间期ms,否则返回0
	return (negativePa(lead)==0 && positivePa(lead)>0)?msPd(lead):0;
}
	
short ECGprop::negativePd(short lead)
{//返回负P波间期ms,没有返回0
	return (positivePa(lead)==0 && negativePa(lead)<0)?msPd(lead):0;
}
	
short ECGprop::positiveTd(short lead)
{//返回正T波间期ms,没有返回0
	return (positiveTa(lead)>0)?msTd(lead):0;
}
	
short ECGprop::negativeTd(short lead)
{//返回负T波间期ms,没有返回0
	return (negativeTa(lead)<0)?msTd(lead):0;
}
