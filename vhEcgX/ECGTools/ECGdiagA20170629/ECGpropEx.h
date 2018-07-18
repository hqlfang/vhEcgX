// ECGpropEx.h: interface for the ECGprop class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ECGPROP_H__1BC85D12_86C9_485B_A5DD_C07D6C0BA37F__INCLUDED_)
#define AFX_ECGPROP_H__1BC85D12_86C9_485B_A5DD_C07D6C0BA37F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ECGbeats.h"
#include "ECGtempl.h"
//#include "..\Patient\Patient.h"

#define		Max(a,b)	((a>=b)?a:b)
#define		Max3(a,b,c)	(Max(Max(a,b),c))
#define		Min(a,b)	((a<=b)?a:b)
#define		Min3(a,b,c)	(Min(Min(a,b),c))
#define		Abs(a)		((a>=0)?a:-a)
#define		Upper(ch)	((ch>='a' && ch<='z')?(ch-'a'+'A'):ch)
#define		Lower(ch)	((ch>='A' && ch<='Z')?(ch-'A'+'a'):ch)

//typedef struct {
//} ECGbeats;

typedef struct {//in ms and uV
	union {
		char	ecg_parm[128];
		struct {
			short RR,HR;
			short Pd,PR,QRS,QT,QTC;
			char  WPW;  //return: A/a B/b W/w  A型, B型, W型, 小写: 可疑, 其它: 无WPW
			char  empty;
			short msQTdis,msMax,msMin;
			short MaxLeadNo,MinLeadNo;
//	QTdiscretion QTd;
			short axisP,axisQRS,axisT;  //return Degree,  for ChNumber==12 only
			short uvRV5;   //return unit: uV,         for ChNumber==12 only
			short uvRV6;   //return unit: uV,         for ChNumber==12 only
			short uvSV1;   //return unit: uV,         for ChNumber==12 only
			short uvSV2;   //return unit: uV,         for ChNumber==12 only
			short OnOff[6];
			short uvRV1;
			short uvSV5;
		};
	};
} ECGparm;

typedef struct {//in ms and uV
	union {
		char	ecg_lead[128];
		struct {
			short OnOff[6];	//Pb,Pe,QRSb,QRSe,Tb,Te;	//特征点位置
			short Pstatus;   //0: none, 1: +, 2: -; 3: +-; 4: -+
			short Tstatus;
			short Pd,Qd,Rd1,Rd2,Sd1,Sd2,Td,PR,QT,QRS;
			short Pa1,Pa2,Qa,Ra1,Ra2,Sa1,Sa2,Ta1,Ta2;
			short Rnotch;	//0,1,2,3: none,上升边，下降边，两边
			short ST[8];	//STj,ST1,ST2,ST3,ST20,ST40,ST60,ST80
			float STslope[4];
			char  morpho[8];
		};
	};
} ECGlead;

//class rCodeDiag;
class ECGprop  
{
//	MultiLead_ECG(short ChNumber,short **DataIn,short Seconds,short Samplerate,double Uvperbit);
//	MultiLead_Templates(int ChNumber,short **MultiLeadData,int Lenght,int SampleRate,float Uvperbit);
public:
	MultiLead_ECG		*m_classBeats;		//声明逐拍分析类
	MultiLead_Templates *m_classTemplates;	//声明模板分析类
//	rCodeDiag	*m_rCode;

	short m_nDataChN,m_nSeconds;
	short m_nSamplerate;
//	short m_nTemplChN;
	double m_fuVperbit;
	short  m_nPACpercent,m_nPVCpercent;	//default: 10 means 10%.用于早搏判定的提早率

public:
	short **m_pDataIn;	//8通道原始数据。会改变(抗零漂与微弱低通) (对12导分析)
//	short **m_DataTempl;//12通道模板数据
	short m_nTemplMaxlen;//模板最大长度
//	short m_nTemplLength;，用于分析的实际模板长度
	ECG_Parameters *m_pOutPut;	//逐拍分析结果
	Template Temp;		//12导模板数据，不同于pOutPut中8导模板(对12导分析)

	BOOL  AutoProcess(short nTemplChN=0,bool all=true);//由原始数据自动逐拍分析和模板分析,<=0:auto
public:
	long	m_nBeats,m_nBeatsMax;//心搏数,最大心搏数5次/秒
	char	*Beats;		//心搏类型。目前只有N:正常，V:室性，B:边界无法判定，P:起搏
	BYTE	*BeatAdd;	//心搏是否参与叠加，只有0,1两种状态
public:
	ECGparm	*Parm;	//公共参数
	ECGlead *Lead;	//各导参数
	ECGlead *I,*II,*III,*aVR,*aVL,*aVF,*V1,*V2,*V3,*V4,*V5,*V6,*X,*Y,*Z,*A1,*A2,*A3,*A4;
public:
	void	CommParameters();	//获取公共参数，填充ECGparm	*Parm;
	void	LeadParameters();	//获取各导参数，填充ECGlead *Lead;
	BOOL	BeatsClassify();	//心搏重分类，填充char	*Beats;
	BOOL	IsPacedECG();		//是否起博心电图
public:
//	ECGprop();
	ECGprop(short ChNumber,short **DataIn,short Seconds,short Samplerate,double uVperbit);
//	ECGprop(MultiLead_Templates *Templates);
	virtual ~ECGprop();
	void	SetPrematurePpercent(short percentPAC,short percentPVC) {m_nPACpercent=percentPAC;m_nPVCpercent=percentPVC;}//设置早搏提早率
	short	GetSampleRate() {return m_nSamplerate;}
public:
	BOOL	TemplIsOk();// {return m_classTemplates?TRUE:FALSE;}
//	void	GetStringOfCommonParameter(char *szString);
	short	GetTemplChN();
protected:
	short	m_lenCorr;
	short	*m_corrTempl[2],*m_corrBeat;
	void	InitTemplate(short nTemplChN);
public:

	BOOL isPositiveQRS(short lead) {return (uvQa(lead)+uvRa1(lead)+uvRa2(lead)+uvSa1(lead)+uvSa2(lead)>=0)?1:0;}	//QRS波为正?
	BOOL isNegativeQRS(short lead) {return (uvQa(lead)+uvRa1(lead)+uvRa2(lead)+uvSa1(lead)+uvSa2(lead)<0)?1:0;}	//QRS波为负?
	BOOL isPositiveP(short lead)	{ return (uvPa1(lead)>0 && uvPa2(lead) == 0) ? TRUE : FALSE; }	//return (Lead[lead].Pstatus==1)?TRUE:FALSE;}
	BOOL isNegativeP(short lead)	{ return (uvPa1(lead)<0 && uvPa2(lead) == 0) ? TRUE : FALSE; }	//return (Lead[lead].Pstatus==2)?TRUE:FALSE;}
	BOOL isDualP(short lead)		{ return (uvPa1(lead)*uvPa2(lead)<0) ? TRUE : FALSE; }	//return (Lead[lead].Pstatus>2)?TRUE:FALSE;}
	BOOL isPositiveT(short lead)	{ return (uvTa1(lead)>0 && uvTa2(lead) == 0) ? TRUE : FALSE; }	//return (Lead[lead].Tstatus==1)?TRUE:FALSE;}
	BOOL isNegativeT(short lead)	{ return (uvTa1(lead)<0 && uvTa2(lead) == 0) ? TRUE : FALSE; }	//return (Lead[lead].Tstatus==2)?TRUE:FALSE;}
	BOOL isDualT(short lead)		{ return (uvTa1(lead)*uvTa2(lead)<0) ? TRUE : FALSE; }	//return (Lead[lead].Tstatus>2)?TRUE:FALSE;}
	BOOL isFlatT(short lead);	  //{	return (Min(Lead[i].Pa1,Lead[i].Pa2)>=0 && Max(Lead[i].Pa1,Lead[i].Pa2)<100)?TRUE:FALSE;}//is flat T if <100uV?
	BOOL isQS(short lead);//	{	return (Upper(Lead[i].morpho[0])=='Q' && Upper(Lead[i].morpho[1])=='S')?TRUE:FALSE;}//该导联是QS型吗?
	BOOL isQr(short lead);//	{	return (Lead[i].morpho[0]=='Q' && Lead[i].morpho[1]=='r')?TRUE:FALSE;}//该导联是Qr型吗?
	BOOL isrsR(short lead);//	{	return (Lead[i].morpho[0]=='r' && Lead[i].morpho[1]=='s' && Lead[i].morpho[2]=='R')?TRUE:FALSE;}//该导联是rsR'型吗?
	BOOL isrsr(short lead);//	{	return (Lead[i].morpho[0]=='r' && Lead[i].morpho[1]=='s' && Lead[i].morpho[2]=='r')?TRUE:FALSE;}//该导联是rsr'型吗?
	BOOL isRSrs(short lead);//	{	return (Lead[i].morpho[0]=='r' || Lead[i].morpho[1] && Lead[i].morpho[2]=='s' || Lead[i].morpho[3]=='S')?TRUE:FALSE;}//是R(r)S(s)?

//get individual parameters
	char* QRSmorpho(short lead) {return Lead[lead].morpho;}	//形态编码
	char  GetPaceMaker()	{if(m_pOutPut->PaceMaker == 'A' || m_pOutPut->PaceMaker == 'V' || m_pOutPut->PaceMaker == 'B') return m_pOutPut->PaceMaker;	else return '\0';}
	short GetAflutAfib()	{return m_pOutPut->AflutAfib;}
//uV
//	short Pa1,Pa2,Qa,Ra1,Ra2,Sa1,Sa2,Ta1,Ta2;
	short	uvPa1(short lead) {return Lead[lead].Pa1;}
	short	uvPa2(short lead) {return Lead[lead].Pa2;}
	short	uvQa(short lead);// {return (isQS(lead))?Min3(Lead[lead].Qa,Lead[lead].Sa1,Lead[lead].Sa2):Lead[lead].Qa;}
	short	uvRa1(short lead) {return Lead[lead].Ra1;}
	short	uvRa2(short lead) {return Lead[lead].Ra2;}
	short	uvSa1(short lead) {return Lead[lead].Sa1;}
	short	uvSa2(short lead) {return Lead[lead].Sa2;}
	short	uvTa1(short lead) {return Lead[lead].Ta1;}
	short	uvTa2(short lead) {return Lead[lead].Ta2;}

	short	uvQRSa(short lead);//	{return Max(Ra1(lead),Ra2(lead))-Min3(Qa(lead),Sa1(lead),Sa2(lead));}//QRS振幅
	short	uvRa(short lead);// {return Max(Ra1(lead),Ra2(lead));}
	short	uvSa(short lead);// {return Min(Sa1(lead),Sa2(lead));}
	short	uvPa(short lead);	//绝对值较大之Pa
	short	uvTa(short lead);	//绝对值较大之Ta
//ms
//	short Pd,Qd,Rd1,Rd2,Sd1,Sd2,Td,PRd,QTd,QRSd;
	short	msPd(short lead) {return Lead[lead].Pd;}
	short	msQd(short lead);// {return (isQS(lead))?Max3(m_pstProp[lead].Qd,m_pstProp[lead].Sd1,m_pstProp[lead].Sd2):m_pstProp[lead].Qd;}
	short	msRd1(short lead) {return Lead[lead].Rd1;}
	short	msRd2(short lead) {return Lead[lead].Rd2;}
	short	msSd1(short lead) {return Lead[lead].Sd1;}
	short	msSd2(short lead) {return Lead[lead].Sd2;}
	short	msTd(short lead) {return Lead[lead].Td;}

	short	msPR(short lead) {return Lead[lead].PR;}
	short	msQT(short lead) {return Lead[lead].QT;}
	short	msQRS(short lead) {return Lead[lead].QRS;}

	short	msSd(short lead) {return msSd1(lead)+msSd2(lead);}
	short	msRd(short lead) {return msRd1(lead)+msRd2(lead);}
//uV
	short	STj(short lead) {return Lead[lead].ST[0];}
	short	ST1(short lead) {return Lead[lead].ST[1];}
	short	ST2(short lead) {return Lead[lead].ST[2];}
	short	ST3(short lead) {return Lead[lead].ST[3];}
	short	ST20(short lead) {return Lead[lead].ST[4];}
	short	ST40(short lead) {return Lead[lead].ST[5];}
	short	ST60(short lead) {return Lead[lead].ST[6];}
	short	ST80(short lead) {return Lead[lead].ST[7];}
	short	Rnotch(short lead) {return Lead[lead].Rnotch;}
	//derived
	short	positivePa(short lead);// {return Max3(Pa1(lead),Pa2(lead),0);}//返回正P波幅值uV,没有返回0
	short	negativePa(short lead);// {return Min3(Pa1(lead),Pa2(lead),0);}//返回负P波幅值uV,没有返回0
	short	positiveTa(short lead);// {return Max3(Ta1(lead),Ta2(lead),0);}//返回正T波幅值uV,没有返回0
	short	negativeTa(short lead);// {return Min3(Ta1(lead),Ta2(lead),0);}//返回负T波幅值uV,没有返回0
	short	positivePd(short lead);// {//如果是正P波,返回间期ms,否则返回0
	short	negativePd(short lead);// {//返回负P波间期ms,没有返回0
	short	positiveTd(short lead);// {//返回正T波间期ms,没有返回0
	short	negativeTd(short lead);// {//返回负T波间期ms,没有返回0
//
//get common parameters
	//ms
	static short	HR(short msRR) {return short(60*1000L/msRR);}
	static short	QTc(short msQT,short msRR) {return (msRR>0)?short(msQT*sqrt(1000.)/sqrt((double)msRR)):0;}	

	short	HR()	{return Parm->HR;}
	short	RR()	{return Parm->RR;}
	short	Pd_ms()	{return Parm->Pd;}
	short	PR()	{return Parm->PR;}
	short	QRS()	{return Parm->QRS;}
	short	QT()	{return Parm->QT;}
	short	QTc()	{return Parm->QTC;}
	short	QTdis()	{return (Parm->msQTdis>=0)?Parm->msQTdis:0;}
	short	QTmax()	{return (Parm->msMax>Parm->msMin)?Parm->msMax:0;}
	short	QTmin()	{return (Parm->msMax>Parm->msMin)?Parm->msMin:0;}
	short	QTmaxLead()	{return (Parm->MaxLeadNo>=0)?Parm->MaxLeadNo:-1;}
	short	QTminLead()	{return (Parm->MinLeadNo>=0)?Parm->MinLeadNo:-1;}
	//uV	
	//short	RV5()	//{return Parm->uvRV5;}
	//short	RV6()	//{return Parm->uvRV6;}
	//short	SV1()	//{return Parm->uvSV1;}
	//short	SV2()	//{return Parm->uvSV2;}
	//short	RV1()	//{return Parm->uvRV1;}
	//short	SV5()	//{return Parm->uvSV5;}
	//度
	short	Paxis()	  {return Parm->axisP;}
	short	QRSaxis() {return Parm->axisQRS;}
	short	Taxis()	  {return Parm->axisT;}
//	short*	Axis()	  {return Parm->Axis;}
	char	WPW()	  {return Parm->WPW;}//return: A/a B/b W/w  A型, B型, W型, 小写: 可疑, 其它: 无WPW

//get Beat 	parameters
	short	RR(int index)	{return (short)((index>0)?(m_pOutPut->Beats[index].Pos-m_pOutPut->Beats[index-1].Pos)*1000L/m_nSamplerate:0);}	//RR(i) in ms
	short	beatPnum(int index)		{return m_pOutPut->Beats[index].Pnum;} //P波个数
};

#endif // !defined(AFX_ECGPROP_H__1BC85D12_86C9_485B_A5DD_C07D6C0BA37F__INCLUDED_)
