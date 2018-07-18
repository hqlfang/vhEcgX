#include <stdint.h>
#include <math.h>

#pragma once


#define		Max(a,b)	((a>=b)?a:b)
#define		Max3(a,b,c)	(Max(Max(a,b),c))
#define		Min(a,b)	((a<=b)?a:b)
#define		Min3(a,b,c)	(Min(Min(a,b),c))
#define		Abs(a)		((a>=0)?a:-a)
#define		abs(a)		((a>=0)?a:-a)

#define TRUE true
#define FALSE false
#define BOOL bool
#define BYTE unsigned char
#define max Max
#define min Min

typedef uint32_t DWORD;


#define	max3(a,b,c)		(fmax(a,fmax(b,c)))
#define	min3(a,b,c)		(fmin(a,fmin(b,c)))
#define square(a)		((a)*(a))
#define NULL            0

const	short	VH_EcgMaxLeads=18;

enum {
	I=0,II,III,aVR,aVL,aVF,V1,V2,V3,V4,V5,V6,V3R,V4R,V5R,V7,V8,V9
};// VH_EcgLeadIndex;

typedef struct {
	short	lead;
	short	chn;	//only this value will be changed
	DWORD	mask;
	char	name[4];
	short	quality;//-2:no channel,-1:unknown,0:normal,1:VF,2:Vf,3:no signal
} VH_EcgLeadInfo;

typedef struct {//in ms and uV
	union {
		char	ecg_parm[128];
		struct {
			short RR,HR;
			short Pd,PR,QRS,QT,QTC;
			char  WPW;  //return: A/a B/b W/w  A型, B型, W型, 小写: 可疑, 其它: 无WPW
			char  empty;
			short QTdis,QTmax,QTmin;
			short QTmaxLead,QTminLead;
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
} VH_ECGparm;

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
} VH_ECGlead;

typedef struct {  //Similar to BeatParameters
	bool  Status;  //0,1 是否叠加
	long  QRSonset,Pos;
	short QRSw,PR,QT;    //mS
	short Pdir,QRSdir,Tdir,Udir;  //0: none, 1: +, -1: -, 2: +- (+>-), -2: +- (->+)
	short Pnum;    //0, 1, 2 (P0,P1)
	short SubQRSw,SubQRSdir;
} VH_ECGbeat;

typedef struct {	//Similar to ECG_Parameters
	bool  Status;	//模板是否有效
	short AflutAfib;	//0:none, 1:Aflut(房扑), 2:Afib(房颤)
	short LeadNo,SubLeadNo;  //
	short Vrate,Arate;
	short BeatsNum;  //Number of Beats
	VH_ECGbeat *Beats;
	char PaceMaker; //'N': none, 'A': A-Type, 'V': V-type, 'B': Both
	short SpikesN;
	long *SpikesPos;
} VH_ECGinfo;

typedef struct  {
	short ChN,SampleRate;//通道数，采样频率。********新增
	double Uvperbit;	//每位微伏数
	short Length;  //length of template data
	short Pos;     //叠加位置。***********************新增
	short **Data;	//template data (8通道)
	short Left,Right;  //analysis range is from Left to Right
	short SpikeA,SpikeV; // 房室起搏钉位置
//	short msRR;  //average RR in ms***********************新增
} VH_Template;

const VH_EcgLeadInfo initialEcgLeadInfo[]=
{
	{I,		0,	0x000001,	"I",	-1},
	{II,	1,	0x000002,	"II",	-1},
	{III,	2,	0x000004,	"III",	-1},
	{aVR,	3,	0x000008,	"aVR",	-1},
	{aVL,	4,	0x000010,	"aVL",	-1},
	{aVF,	5,	0x000020,	"aVF",	-1},
	{V1,	6,	0x000040,	"V1",	-1},
	{V2,	7,	0x000080,	"V2",	-1},
	{V3,	8,	0x000100,	"V3",	-1},
	{V4,	9,	0x000200,	"V4",	-1},
	{V5,	10,	0x000400,	"V5",	-1},
	{V6,	11,	0x000800,	"V6",	-1},
	{V3R,	12,	0x001000,	"V3R",	-1},
	{V4R,	13,	0x002000,	"V4R",	-1},
	{V5R,	14,	0x004000,	"V5R",	-1},
	{V7,	15,	0x008000,	"V7",	-1},
	{V8,	16,	0x010000,	"V8",	-1},
	{V9,	17,	0x020000,	"V9",	-1}
};

const	DWORD	MI_L=initialEcgLeadInfo[aVL].mask|initialEcgLeadInfo[I].mask|initialEcgLeadInfo[aVR].mask;
const	DWORD	MI_I=initialEcgLeadInfo[II].mask|initialEcgLeadInfo[aVF].mask|initialEcgLeadInfo[III].mask;
const	DWORD	MI_S=initialEcgLeadInfo[V1].mask|initialEcgLeadInfo[V2].mask;
const	DWORD	MI_A=initialEcgLeadInfo[V3].mask|initialEcgLeadInfo[V3].mask|initialEcgLeadInfo[V5].mask|initialEcgLeadInfo[V6].mask;
