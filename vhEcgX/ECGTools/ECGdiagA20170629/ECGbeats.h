/*//////////////////////////////////////////////////
Du Xiaodong, VH Medical
1st Ver. 01-July-2009 
*///////////////////////////////////////////////////
#define		Max(a,b)	((a>=b)?a:b)
#define		Max3(a,b,c)	(Max(Max(a,b),c))
#define		Min(a,b)	((a<=b)?a:b)
#define		Min3(a,b,c)	(Min(Min(a,b),c))
#define		Abs(a)		((a>=0)?a:-a)

#include "math.h"
#include "Filters.h"
#include "EcgDiagDefines.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef aaa
#define aaa 100000

typedef enum PROC_STATUS{MUSH_NOISE=-3,MUCH_QRS=-2,NO_TEMPL=-1,FEW_QRS=0,PROC_OK=1};//hspecg
//逐拍处理结果状态
typedef enum BEAT_STATUS_TYPE{OK,BORDER,RR_Int,QRS_W,SubQRS_W,QRS_Dir,SubQRS_Dir,T_Dir,P_Dir,fOK,PR_Int,QRS_Range}; //m_P,
//心搏叠加状态: 不叠加优先级:: Border,QRSw,QRSDir,TDir,PDir  
static 	char *StatusName[]={"Ok","B","RR","Rw","sRw","Rd","sRd","Td","Pd","fOK","PR","Rh"}; //"m_P",
//2016 newly added: fOK-Fause-OK,  m_P: multi_P  //2016  //2017-1: QRS_Range/"Rh"

struct SpikeV {
	short V1,V2;
};

struct QRSfeature {
	long Pos;   //3(II,V1,V5)导联QRS动量最大值位置
	long Start,End,Onset,Offset;
	long MaxP,MinP;
	unsigned short Range; //QRS幅值(in uV)  2017-1
};

struct PTcommon {
	short Dir;
	short pV,nV;
};

struct Pfeature: PTcommon{ long Onset; short Wide; };   //Wide=mS
struct Tfeature: PTcommon { long Offset; };
struct Ufeature { 
	short Dir; 
	long Onset,Offset;
};

struct BeatFeature {  //result //Du
	QRSfeature QRS;
	Pfeature P[3];//P0,P1;
	Tfeature T;
	Ufeature U;
//	int SubT;   //(V3)'s T position if !=-1
};

struct BeatParameters {  //for Hu
	BEAT_STATUS_TYPE Status;  //叠加状态
	long QRSonset,Pos;
	short PtoR;
	short QRSw,PR,QT;    //mS
	unsigned short QRSh; //QRS Range  max-min in QRS range in uV  //2017-1
	short Pdir,Pwide,QRSdir,Tdir,Udir;  //0: none, 1: +, -1: -, 2: +- (+>-), -2: +- (->+)
	short Pnum;    //0, 1, 2 (P0,P1)
	short PR1,PR2;
	short SubQRSw,SubQRSdir;
//	long SubQRSon; //for test
};

struct Template {
	short ChN, SampleRate;//通道数，采样频率。********新增
	double Uvperbit;	//每位微伏数
	short Length;  //length of template data
	short Pos;     //叠加位置。***********************新增
	short **Data;	//template data (8通道)
	short Left, Right;  //analysis range is from Left to Right
	short SpikeA, SpikeV; // 房室起搏钉位置
	//	short msRR;  //average RR in ms***********************新增
};

struct ECG_Parameters {
	short Status;
	short AflutAfib;	//0:none, 1:Aflut(房扑), 2:Afib(房颤)
//	short VfibVTech; 
	short LeadNo,SubLeadNo;  //
	short Vrate,Arate;//,Trate;
	short MaxRR, MinRR;      // add at Y2015/5/12
	short AverageRR;	//ms 
	short TemplateRR;   //ms  add at Y2015/7/8   
	short BeatsNum;  //Number of Beats
	BeatParameters *Beats;
	Template Temp;
	char PaceMaker; //'N': none, 'A': A-Type, 'V': V-type, 'B': Both
	short SpikesN;
	long *SpikesPos;
};

///////////////////////////////////////////////////////////////////////
class MultiLead_ECG {
	int ChN,SampleRate;
	long Length;
	double uVperBit;
//	double Ori_uVperBit;
	short MinWave;
	short ProcStatus;
public:
	MultiLead_ECG(int ChNumber,short **DataIn,int Seconds,int Samplerate,double Uvperbit);
	~MultiLead_ECG();

	ECG_Parameters OutPut;
	BOOL ProcessSucceed();

	short GetProcStatus() {return ProcStatus;}
	long  GetLength()	  {return Length;}

private:
	short **DataOut,*Data[3],*sData;

	bool MuchNoise(short *data);
//public:
	BeatFeature *Beats;     //private
private:
	short PreProcess();
	void FinalProcess();

	short SpikesN;
	long *Spikes;    //private
	SpikeV **SpikesV;
    void SpikeDetect();  //private
	void SpikeRemoval();  //private
	void SpikeReset();  //private
	void NoiseRemoval();  //private

	short QRSsN;        //private
	void QRSsDetect();
	BOOL CestQRS(short *data,int len);

	void SelectLeads(short **LeadsData,short *SelectedLeads);    //2018-4-25

	short LeadSelectDependsOnQRS();   //2017 only consider QRS's height
	short LeadSelectDependsOnP();

//	int MaxPsAverage(short *data,int midRR); //for LeadSelect()  ???
	int MaxPsAverage(short *data,int smallRR,int largeRR); //2016-03-28 rewrite
	int MaxTsAverage(short *data); //for LeadSelect()  ???
	int MaxPowerValue(short *data,int len);
	void SumPower(short *data,int *power,int len,int w0);

	short SubLeadSelect();
	void SubQRSanalysis(int QRSi,short *data);
//	void SubLeadTs();
//	int SubLeadT(short *data,int len);

	BOOL GoodSelectPs();
	short LeadSelectDependsOnT(short LeadNo);

	BOOL AnalysisDone,TemplateDone;
	void ECG_Analysis();
	char PaceMaker_Analysis();
	void QRSanalysis(int QRSi);
	void PTUanalysis(int QRSi);
	void Tanalysis(short *data,int len,Tfeature *Twave); 
	void Panalysis(short *data,int len,Pfeature **P);
	BOOL PositiveP(short *data,int seg1,int seg2,Pfeature *P,BOOL FirstTime);
	BOOL NegativeP(short *data,int seg,Pfeature *P);
//	BOOL CorrectP(short *data,int seg,int pSt,Pfeature *P);
	void CorrectP0(Pfeature *P0,Pfeature *tP);
	void Sub_P(Pfeature *subP,Pfeature *stdP);
	int PositiveU(short *data,int len,Ufeature *U);
	int NagtiveU(short *data,int len,Ufeature *U);

	void CorrectPTs();   //Y2015-7
	void ParametersCalculation();
	void StatusConfirm();
	void ComplementPs();  //琛
	void OK_StatusCorrect();  //2016-04-25

	short ArateDetecte();

	BOOL Addible(short *data,int len);  //  Du 2011-5-6
	void AverageTemplate();
	void SingleTemplate(long Pos);

	BOOL Hill(short *data,long p,short w,short Zero);  //not in use
	BOOL Vale(short *data,long p,short w,short Zero);  //not in use
	void AntiDrift(short *data);
	void AntiDrift2(short *data);
	long TurnPoint(short *data,long l,long r,int sign);
	long TurnPoint(long *data,long l,long r,int sign);   //Y2015/5/28
	void Average(short *data,long len,short ms);
	void Average(long *data,long len,short ms);   //Y2015/5/28

//  for A_Fib, A_Flt and V_Fib 
private:
	short F_Analysis();
	int AllBeatsP();    //Y2015-6-30  return 1: 90% good P, -1: 60% bad P, 0: others
	BOOL P_QRSrelations();
	short RR_relations();     //Y2015/5
//	BOOL FlutterDetect(short *fPN,short *fData,int len,int Range,int LF);

	BOOL FlutterDetect();  //2016-8
	typedef enum FLUTTER_WAVE_STAGE{SMALL,MIDDLE,BIG}; //2016-10-27,
	bool FlutterWave(short *fDAta,FLUTTER_WAVE_STAGE Stage); 
//	void Get_fPN(short *fPN,short *fData,int fLen);   //prepare fPN for FlutterDetect Y2015

//	BOOL FibDecide(short *fData,short *gData,int fLength);
	BOOL FibDecide();
//	BOOL FibDetect(short *fData,int fLength);

//	short V_Analysis();
//	short VtechDetect(short *vPN,int len,int Range);
//	short VfibDetect(short *vPN,int len,int Range);
//public: 
//	int V_CH;
//	short *vData;
//	int vLength;
};

//Tools
void KinetEnergy(short *in0,short *in1,short *in2,long *out,int length,int q);
void EnergyCorrect(long *Edata,int length,int step);
double SquareError(short *tData,int Range);
//void QrsKinetEnergy(short *in0,short *in1,short *in2,short *out,int length,int q);
//Du 2010-11-8
//void ModifiedKinetEnergy(short *in0,short *in1,short *in2,short *out,int length,int SmplRt);

#endif
