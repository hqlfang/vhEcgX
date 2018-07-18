/*//////////////////////////////////////////////////
Du Xiaodong, VH Medical
1st Ver. 09-Jun-2009
*///////////////////////////////////////////////////
#define		Max(a,b)	((a>=b)?a:b)
#define		Max3(a,b,c)	(Max(Max(a,b),c))
#define		Min(a,b)	((a<=b)?a:b)
#define		Min3(a,b,c)	(Min(Min(a,b),c))
#define		Abs(a)		((a>=0)?a:-a)

#ifndef __ECGTEMPLATE__H
#define __ECGTEMPLATE__H


#define __MY_VC_TEST__
#ifndef __MY_VC_TEST__
#include "DataType.h"
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <math.h>
#include "filters.h"
#include "ECGbeats.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef enum WAVE_TYPE{Pwave,QRScomplex,Twave};
typedef enum COMMEN_INTERVAL_TYPE{cPd,cPRd,cQRSd,cQTd};

typedef enum INDIVIDUAL_INTERVAL_TYPE{Pd,Qd,Rd1,Rd2,Sd1,Sd2,Td,PRd,QTd,QRSd};
typedef enum INDIVIDUAL_VALUE_TYPE{Pa1,Pa2,Qa,Ra1,Ra2,Sa1,Sa2,Ta1,Ta2};

struct QTdiscretion {
	short msQTd,msMax,msMin;
	short MaxLeadNo,MinLeadNo;
};

//////////////////////////////////////////////////////////////////////
class ECG_Base {
protected:
	int m_nSampleRate;
	double m_dUvperbit;

	int MinimumWave;
//    int WaveAngle;  //degree for 125mm/sec,20mm/mV  
    int FlateAngle;   //degree for 125mm/sec,20mm/mV 
protected:
	ECG_Base(int SampleRate,double Uvperbit);
    int TurnPoint(short *data,int l,int r,int sign); //求转折点
//    int TurnPoint(int *data,int l,int r,int sign); //求转折点  //2017-3-30 added
	int FlatestPoint(short *data,int p);
    BOOL Flat(short *data,int p,int w,int Angle);
	void Average(short *data,long len,short ms);
};

///////////////////////////////////////////////////////////////////////
class QRS_Complex: private ECG_Base {
	short *VHs;
	int length;
	
    void ValesAndHills();  //,int msStep,int uvThreshold,int Angle);
    void Blocks();  //,int msStep,int uvThreshold);
	BOOL VHsProcess(BOOL Auto);
	int OnsetPoint(int l,int r,int sign);
	int OffsetPoint(int l,int r,int sign);
	void ComplexFeatures();
	void CorrectFeatures();    //Y2015-12-21 test

	short bpHigh,bpLow;  //2015-8 (for WPW)

public:
	short *Data;
	int OnSet,OffSet;    // OnSet=-2: without QRS by Momentum, -1 without QRS by V&H
	int Q[4],R1[4],S1[4],R2[4],S2[4];  //0--Status, 1--start, 2--peak, 3--end
                                       //value: 0: without, 1: q/r/s, 2: Q/R/S
    void SubSetting(int l,int p,int r,int *QRSsub);

	QRS_Complex(int SampleRate,double Uvperbit);
	~QRS_Complex();
	void SetData(short *TemplateData,int QRSstart,int QRSend);  //int ZeroValue,
	void Analysis();
//	void OnsetCorrect();      //Y2015-7-31  after Hu
//	void OffsetCorrect();    //Y2015-12-10
	void FeatureSet(short *TemplateData,int start,int end);  //int zeroV,  2011-4-6  by Du
//	void QS_Type_Setting(); //2010-4-26 deal with QR type

	void CorrectDependsOnZero();

//// following functions are for (Hu) outside using  //Individual Parameters
	BOOL Delta();  
	BOOL Split();   //not done
//	void Notch(short &Incrs,short &Decrs);    //not done
	void Notch(short &Incrs,short &Decrs);    //not done
//	BOOL IncreaseNotch(int Start,int End);
//	BOOL DecreaseNotch(int Start,int End);
	short IncreaseNotch(int Start,int End);
	short DecreaseNotch(int Start,int End);
	int	GetLength()	{return length;}
};

///////////////////////////////////////////////////////////////////////
class PT_Wave: private ECG_Base {
	int length;
	int minWave,Width;
	int OnsetPoint(int l,int r,int sign);
	int OffsetPoint(int l,int r,int sign);
	int Positive_P(int *Pos);
	int Negative_P(int *Pos);

	void NP_Correct(int pV, int *pPos, int nV, int *nPos);

	void P_OnOffCorrect();
	void T_OnOffCorrect();

	unsigned short AbstractV();    //2017-12-18

public:
//	int length;
	short *Data;  //*OriData;
	int Status;   //0: none, 1: +, 2: -; 3: +-; 4: -+
	int OnSet,OffSet;
	int OnePos,TwoPos;  //position

	unsigned short AbV;  //  /Bigest_Value - OffSet_Value 

	PT_Wave(char WaveType,int SampleRate,double Uvperbit);
	~PT_Wave();
	void SetData(short *TemplateData,int Start,int End);
//	void Analysis();
	void FeatureSet(short *TemplateData,int start,int end);
	void TwaveAnalysis();
	void PwaveAnalysis();

//	void P_Set_Test(int Pos);
	int  GetLength()	{return length;}
};

/////////////////////////////////////////////////////////////////////////
class ECG_Template: private ECG_Base {
	int m_nLength;
	char Morpho[8];
	
    void Monentum(short *data,int length);
    void Potential(short *data,int length);
	int StartEndAdjust(short *data,int p);

public:
	short *TemplateData;
	ECG_Template(short *ECGdata,int Lenght,int SampleRate,double Uvperbit);
    ~ECG_Template();

	short CalculateZeroValue();

	QRS_Complex *QRS;
	PT_Wave *P;
	PT_Wave *T;

	BOOL QRSok;
	int QRSstart,QRSend; 
// 	short ZeroValue;    //2016-4-6

	void QRS_Location(int Center);

//	int P_Range(short *data,int len);

//	BOOL Pok,Tok;
	int Pstart,Pend,Tstart,Tend; 

	void FeatureSet();  //Individual Manual

//// following functions are for (Hu) outside using  //Individual Parameters
	short AvantOnOff[6], OnOff[6]; //P: on/off, QRS: on/off, T: on/off: positions //AvantOnOff added in 2017-1

	int msInterval(INDIVIDUAL_INTERVAL_TYPE Type);   //return unit: ms
	int uvValue(INDIVIDUAL_VALUE_TYPE Type);      //return unit: mV
	int uvSTvalue(int MSor123);    //ms=0 STj Value, 1,2,3 -- ST1,ST2,ST3, otherwise ms
	float STslope(int msStep);   //return uV/ms, msStep -- ms to j_point 10ms to 100ms
	char * QRSmorpho();   //retur Morpho
};

/////////////////////////////////////////////////////////////////////////
class MultiLead_Templates: private ECG_Base {
	int m_nLength,m_nCenter;
	int m_nLeft;
	int m_nRight;
	int ChN;
	int msRR;

    void QRSTanalysis();
	void SetOnOff(bool Auto);
//	void Correct_Pwaves();  //Y2015-7-24
//	void Correct_Positions();
	int FindMaxPch(int ChFound);   //find the maxV channel which != ChFound  Y2015/5
	int FindMaxTch(int Ch1,int Ch2);   //find the maxV channel which != Ch1 and Ch2  Y2015/5
//	int SelectMaxQRS();
	ECG_Parameters *Beats_OutPut;
	void AflutAfibCorrect();   //Y2015-7-10

	void P_Added_Depends();
	void P_OnOff_Correct();

//	void P_OnOff_Correct(bool *LeadNo,int *StartP,int *EndP);
//	void Delete_PR_TooShort();

//	void QRS_Form_Correct_Onset(int LeadN);

//	void QRS_OnOff_Correct();

//	int SetQRS_Position(QRS_Complex *pQRS,int left,int right);

	int Axis(double vI, double vIII);

//	bool FindWrownOnset(short *OnPos);
//	bool FindWrownOffset(short *OffPos);

public:
	ECG_Template *I,*II,*III,*aVR,*aVL,*aVF,*V1,*V2,*V3,*V4,*V5,*V6,*X,*Y,*Z;
	ECG_Template **Lead;

	MultiLead_Templates(short ChNumber,short **MultiLeadData,short Lenght,short SampleRate,double Uvperbit);
//	MultiLead_Templates(Template *Temp,short AverageRR);
	MultiLead_Templates(Template *Temp,ECG_Parameters *BeatsOutPut);
	~MultiLead_Templates();
	void InitLead();
	void AutoAnalysis();//all in samples
	void IndividualManual();
	void CommenManual();

//// following functions are for (Hu) outside using //Commen Parameters
	short OnOff[6];  //P: on/off,  QRS: on/off,  T: on/off: positions

	int msInterval(COMMEN_INTERVAL_TYPE Type);   //return unit: ms
	int msQTc(int msQTd);   //return unit: ms
	int Axis(WAVE_TYPE Type);  //return Degree,  for ChNumber==12 only. Angle range: -90~+270
	int uvRV5();   //return unit: uV,         for ChNumber==12 only
	int uvRV6();   //return unit: uV,         for ChNumber==12 only
	int uvSV1();   //return unit: uV,         for ChNumber==12 only
	int uvSV2();   //return unit: uV,         for ChNumber==12 only
	void QTdiscrete(QTdiscretion &QTd);   //QTd==-1: failed
	char WPW();  //return: A/a B/b W/w  A型, B型, W型, 小写: 可疑, 其它: 无WPW
	int RR() {return msRR;}
	int HR() {return 60000l/msRR;}
	int ChNumber() {return ChN;}
	int Length() {return m_nLength;}
	int Center() {return m_nCenter;}
};

//void KinetEnergy(short *in0,short *in1,short *in2,short *out,int length,int q);
//void EnergyCorrect(long *Edata,int length,int step);

#endif

