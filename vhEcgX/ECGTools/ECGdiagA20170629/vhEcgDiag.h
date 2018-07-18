#pragma once

#include "EcgDiagDefines.h"

class CvhEcgDiag {
protected:
//	short	m_nSex,m_nAge;
	short	m_nVindex[12];//V1-V6,V3R-V5R,V7,V9
	VH_ECGparm		*m_pEcgParm;
	VH_ECGlead		*m_pEcgLead;
	VH_ECGbeat		*m_pEcgBeat;
	VH_ECGinfo		*m_pEcgInfo;
//	VH_Template		*m_pTemplate;
	char			m_szLeadName[64];
//	char	*m_pBeatsType;		//心搏类型。目前只有N:正常，V:室性，B:边界无法判定，P:起搏
public:
	typedef enum	{
		V1=0,V2,V3,V4,V5,V6,V3R,V4R,V5R,V7,V8,V9
	} Vindex;
	CvhEcgDiag();
	virtual ~CvhEcgDiag();
	// TODO: 在此添加您的方法。
	void	SetVindex(Vindex index,short chn);
public:
	BOOL CreateEcgDiag(short ChNumber,short **DataIn,short Seconds,short Samplerate,double uVperbit);
//	BOOL EcgAutoProcess(short TemplChnNumber=0);
	BOOL EcgCode(char bySex,short age,short ageYmd=0);//bySex='M','F'
	void SetPrematurePpercent(short percentPAC,short percentPVC);
	BOOL CommenManual(short *OnOff);
	BOOL IndividualManual(short **OnOffs);
	BOOL ECGparmManual(VH_ECGparm *parm);
	void SomeManual(BOOL bOnOff,BOOL bOnOffs);
public:
	short	GetFirstMcode(char *szLeadName=NULL);
	short	GetNextMcode(char *szLeadName=NULL);
	short	GetFirstRcode(char *szLeadName=NULL);
	short	GetNextRcode(char *szLeadName=NULL);
	short	GetMcCodeCount();
	short	GetVhCodeCount();
	static char	*mcCode(short code);
	static char	*vhCode(short code);
	//char	*CodeLeads(unsigned short code);
	//char	*GetCseCode(unsigned short rCode);
	short	GetCriticalValue();
public:
	short GetTemplChNumber();
	VH_ECGinfo *GetEcgInfo();	//逐拍分析结果
	VH_ECGbeat *GetEcgBeats();	//逐拍分析结果
//	VH_Template *GetTemplate();		//12导模板数据，不同于pOutPut中8导模板(对12导分析)
	short	GetTemplLength();
	short	**GetTemplData();
	short	GetTemplPos();
	
public:
	VH_ECGparm *GetECGparm();	//公共参数
	VH_ECGlead *GetECGlead();	//各导参数
public:
//uV
//	short Pa1,Pa2,Qa,Ra1,Ra2,Sa1,Sa2,Ta1,Ta2;
	short	uvPa1(short lead);
	short	uvPa2(short lead);
	short	uvQa(short lead);
	short	uvRa1(short lead);
	short	uvRa2(short lead);
	short	uvSa1(short lead);
	short	uvSa2(short lead);
	short	uvTa1(short lead);
	short	uvTa2(short lead);

	short	uvQRSa(short lead);
	short	uvRa(short lead);
	short	uvSa(short lead);

//ms
//	short Pd,Qd,Rd1,Rd2,Sd1,Sd2,Td,PRd,QTd,QRSd;
	short	msPd(short lead);
	short	msQd(short lead);
	short	msRd1(short lead);
	short	msRd2(short lead);
	short	msSd1(short lead);
	short	msSd2(short lead);
	short	msTd(short lead);

	short	msPR(short lead);
	short	msQT(short lead);
	short	msQRS(short lead);

	short	msSd(short lead);
	short	msRd(short lead);
	//uV
	short	STj(short lead);
	short	ST1(short lead);
	short	ST2(short lead);
	short	ST3(short lead);
	short	ST20(short lead);
	short	ST40(short lead);
	short	ST60(short lead);
	short	ST80(short lead);
	short	Rnotch(short lead);
//
//get common parameters
//ms
	short	HR();
	short	RR();
	short	Pd();
	short	PR();
	short	QRS();
	short	QT();
	short	QTc();
	short	QTdis();
	short	QTmax();
	short	QTmin();
	short	QTmaxLead();
	short	QTminLead();
	//uV	
	short	RV5();
	short	SV5();	//算术值，是负数，请以绝对值参与计算
	short	RV6();
	short	SV6();	//算术值，是负数，请以绝对值参与计算
	short	SV1();	//算术值，是负数，请以绝对值参与计算
	short	RV1();
	short	SV2();	//算术值，是负数，请以绝对值参与计算
	short	RV2();
//度
	short	Paxis();
	short	QRSaxis();
	short	Taxis();
//	short*	Axis();
	char	WPW();

public:
	long	GetBeatNum();
	char	*GetBeats(long &BeatNum);		//心搏类型。目前只有N:正常，V:室性，B:边界无法判定，P:起搏
	BYTE	*GetBeatAdd(long &BeatNum);		//心搏是否参与叠加，只有0,1两种状态
public:
//get Beat 	parameters
	short	RR(int index);
	short	beatPnum(int index);
protected:
	void SetParameters();
public:
//derived
	short	positivePa(short lead);
	short	negativePa(short lead);
	short	positiveTa(short lead);
	short	negativeTa(short lead);
	short	positivePd(short lead);
	short	negativePd(short lead);
	short	positiveTd(short lead);
	short	negativeTd(short lead);
public:
	BOOL isPositiveQRS(short lead);
	BOOL isNegativeQRS(short lead);
	BOOL isPositiveP(short lead);
	BOOL isNegativeP(short lead);
	BOOL isDualP(short lead);
	BOOL isPositiveT(short lead);
	BOOL isNegativeT(short lead);
	BOOL isDualT(short lead);
	BOOL isFlatT(short lead);;
	BOOL isQS(short lead);
	BOOL isQr(short lead);
	BOOL isrsR(short lead);
	BOOL isrsr(short lead);
	BOOL isRSrs(short lead);
public:
	int uvSTvalue(short lead,int MSor123);    //ms=0 STj Value, 1,2,3 -- ST1,ST2,ST3, otherwise ms
	float STslope(short lead,int msStep);   //return uV/ms, msStep -- ms to j_point 10ms to 100ms
public:
	long	Samples2ms(long Samples);
	long	ms2Samples(long ms);

//get individual parameters
	char* QRSmorpho(short lead);
	char  GetPaceMaker();
	short GetAflutAfib();
public:
	BOOL	IsPacedECG();	//是否起博心电图
	BOOL	TemplIsOk();
public:
	static short	HR(short msRR);
	static short	QTc(short msQT,short msRR);
};
