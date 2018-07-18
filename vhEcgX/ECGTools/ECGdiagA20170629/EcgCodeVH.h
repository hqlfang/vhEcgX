#pragma once

#include "EcgCodeMC.h"

class CvhCode :	public CmcCode
{
	typedef struct {
		short score;
		DWORD dwLeads;
	} STT_SCORE;
protected:
	STT_SCORE m_stQscore;
	STT_SCORE m_stSTDscore;
	STT_SCORE m_stSTEscore;
	STT_SCORE m_stTNscore;
	STT_SCORE m_stPLMIscore;
	bool	m_bPolymorphicPVC;
	bool	m_bAf;
protected:
	CodeMgr			vh;
	static char		vhcodebuf[8];
	unsigned short	vhcode(short g,short c,short i) {return g*1000+c*100+i;}
public:
	CvhCode(void);
	virtual ~CvhCode(void);
	unsigned short SetvhCode(unsigned short nCode,DWORD dwLeads=0);
	short	vhCodeCount();
	short	vhCodeGetFirst(char *szLeadName=NULL);	//szLeadName[64],	cseCode[32]
	short	vhCodeGetNext(char *szLeadName=NULL);	//szLeadName[64],	cseCode[32]
	unsigned short vhCodeFrommcCode(unsigned short mcCode,unsigned short vhCode,DWORD &dwLeads);
	static char	*vhCodeString(unsigned short vhcode);
protected:
	Code	cv;
public:
	void	SetCriticalValue(short cv,DWORD dwLeads=0);
	short	GetCriticalValue();
	short	GetCriticalValue(DWORD &dwLeads);
protected:
	short	Qscore(DWORD &dwLeads);
	short	STDscore(DWORD &dwLeads);
	short	STEscore(DWORD &dwLeads);
	short	TNscore(DWORD &dwLeads);
	short	plMIscore(DWORD &dwLeads);//后侧壁心梗评分
	bool	isPolymorphicPVC();
public:
	void	code();
	short	vhCode();
//3.1 Code 0, Baseline EGG Suppression Codes
//If any code 0 is present, no any other code.
	bool	vhCode0();
	short	vhCode010();	//0-1-0 ECG not available
	short	vhCode021();	// Inadequate quality
	short	vhCode022();	// Missing limb leads
	short	vhCode023();	// Missing chest leads
	short	vhCode031();	//  RA/RL reversal ? (Check limb lead electrodes again) 
//	short	vhCode032();	// Other limb lead connection interchanges (correction made and ECG coded)
	short	vhCode0321();	// RA/LA reversal? (Check limb lead electrodes again)
	short	vhCode0322();	// RA/LL reversal? (Check limb lead electrodes again)
	short	vhCode0323();	// LA/LL reversal? (Check limb lead electrodes again)
	short	vhCode033_4();	// 判断胸导R波高度变化规律
	short	vhCode033();	// Correctable chest lead in V1-V3 connection error? (ECG codable)
	short	vhCode034();	// Correctable chest lead in V4-V6 connection error? (ECG codable)
	short	vhCode035();	// Correctable other chest lead connection error? (ECG codable)
	short	vhCode040();	// Any conditions with ORS duration ≥ 120 ms
	short	vhCode050();	// Atrial fibrillation or flutter
	short	vhCode061_2();	// 061,062
//	short	vhCode061();	// Ventricular or dual-chamber electronic pacemaker
//	short	vhCode062();	// Atrial electronic pacemaker
//3.2.	Code 1, Rhythm Codes
	void	vhCode1();
	short	vhCode100_4();
	//short	vhCode100();	// Sinus rhythm (SR) 
	//short	vhCode101();	// Normal sinus rhythm(NSR)
	//short	vhCode102();	// Sinus bradycardia (SB) 
	//short	vhCode103();	// Sinus tachycardia (ST) 
	//short	vhCode104();	// Sinus arrhythmia (SAR) 
	short	vhCode101x();	//1010,1011,1012,1013,1016
	//short	vhCode1010();	// Ectopic (ectosinal) supraventricular complexes (ESVC)
	//short	vhCode1011();	// Atrial bigeminy (ABG) 
	//short	vhCode1012();	// Atrial trigeminy (ATG) 
	//short	vhCode1013();	// A doublet of ectopic atrial complexes (DEAC)
	//short	vhCode1016();	// Frequent ectopic atrial complexes(FEAC)
	//short	vhCode1017();	// Short runs of ectopic atrial complexes(FEVC)
	//short	vhCode1018();	// Long runs of ectopic atrial complexes(FEVC)
	short	vhCode1020();	// Aberrant supraventricular complexes (ASVC)
	short	vhCode103x();	// 1030,1031,1032,1033,1034,1035,1036
	//short	vhCode1030();	// Ectopic ventricular complexes (EVC) 
	//short	vhCode1031();	// A doublet of ectopic ventricular complexes (DEVC)
	//short	vhCode1032();	// Coalescent ventricular ectopic complexes (CEVC) 
	//short	vhCode1033();	// Polymorphic ectopic ventricular complexes (PEVC)
	//short	vhCode1034();	// Ventricular bigeminy (VBG) 
	//short	vhCode1035();	// Ventricular trigeminy (VTG) 
	//short	vhCode1036();	// Frequent ectopic ventricular complexes(FEVC)
	//short	vhCode1037();	// Short runs of ectopic ventricular complexes(FEVC)
	//short	vhCode1038();	// Long runs of ectopic ventricular complexes(FEVC)
	short	vhCode1040();	// Pause (possible sinoatrial arrest or block) 
	short	vhCode1050();	// Reduced heart rate variability (RHRV) 
	short	vhCode1060();	// Increased heart rate variability (IHRV)
	short	vhCode110();	// Wandering atrial pacemaker (WAP) 
	short	vhCode12x();	//120,121,122,123
	//short	vhCode120();	// Junctional rhythm (JR) 
	//short	vhCode121();	// Junctional rhythm, rate 45-64 bpm
	//short	vhCode122();	//  Junctional bradycardia(JBR) 
	//short	vhCode123();	//  Accelerated junctional rhythm (AJR) 
	short	vhCode13x();	// 130,131,132,133,1311
	//short	vhCode130();	// Ectopic atrial rhythm (EAR) 
	//short	vhCode131();	// Ectopic atrial rhythm, rate 50-90 bpm
	//short	vhCode132();	// Ectopic atrial bradycardia (EABR) 
	//short	vhCode133();	// Ectopic atrial tachycardia (EAT) 
	//short	vhCode1311();	//  Probably left atrial ectopic focus
	short	vhCode134();	// Coronary Sinus Rhythm (CSR) 
	short	vhCode14x();	// 140,141,142,143
	//short	vhCode140();	// Supraventricular (SVT) or narrow QRS tachycardia
	//short	vhCode141();	// Supraventricular tachycardia, rate less than 130 bpm
	//short	vhCode142();	// Supraventricular tachycardia, rate no less than 130 bpm
	//short	vhCode143();	// Supraventricular tachycardia
	short	vhCode150();	// Atrial flutter or fibrillation(AFLF)
	short	vhCode151();	// Atrial flutter type 1(AFL1)
	short	vhCode152();	// Atrial flutter type 2(AFL2)
	short	vhCode153();	// Atrial fibrillation(AF)
	short	vhCode153x();	// 1531,1532,1533
	//short	vhCode1531();	// Atrial fibrillation with slow ventricular response
	//short	vhCode1532();	// Atrial fibrillation with rapid ventricular response
	//short	vhCode1533();	// Atrial fibrillation with AV dissociation and junctional rhythm
	short	vhCode154();	// Atrial fibrillation/flutter with possible dominant AV conduction
	short	vhCode16x();	// 160,161,162
	//short	vhCode160();	// Electronic pacemaker (PM)
	//short	vhCode161();	// Ventricular pacemaker (VPM) or dual chamber pacemaker (CPM) 
	//short	vhCode162();	// Atrial electronic pacemaker (APM) 
	short	vhCode170_2();	// 170,171,172
	//short	vhCode170();	// Ventricular tachycardia (VT) 
	//short	vhCode171();	// Ventricular escape rhythm (VER) 
	//short	vhCode172();	// Accelerated idioventricular rhythm (AIVR) 
	short	vhCode173_4();	// 173,174
	//short	vhCode173();	// Ventricular Flutter (VF) 
	//short	vhCode174();	// Ventricular Fibrillation (Vf) 
	short	vhCode181();	// Extreme Tachycardia(ExT)
	short	vhCode182();	// Extreme Bradycardia(ExB)
	short	vhCode190();	// Other abnormal rhythm classification
//3.3.	Code 2, Atrioventricular Conduction Abnormalities
	void	vhCode2();
	short	vhCode210();	//  First-degree AV block (AVB1) 
	short	vhCode220();	//  Second-degree AV block (AVB2) 
	short	vhCode221();	//  Second-degree AV block type Wenckebach or Mobitz 1 (AVB2W) 
	short	vhCode222();	//  Second-degree singular AV block or type Mobitz 2 (AVB2S) 
	short	vhCode223();	//  Second-degree multiple AV block (AVB2M) 
	short	vhCode231x();	//	231,2330,2331,2332
	//short	vhCode231();	//  Third-degree (complete) AV block (AVB3) 
	//short	vhCode2330();	// Third-degree (complete) AV block with narrow QRS complex
	//short	vhCode2331();	// Third-degree (complete) AV block with wide QRS complex
	//short	vhCode2332();	// Third-degree (complete) AV block with wide QRS complex and slow ventricular rate
	short	vhCode240();	//  Wolff-Parkinson-White syndrome (WPW) 
	short	vhCode250();	//  Short PR interval
//3.4.	Code 3, Conduction Disturbances
	void	vhCode3();
//VH code 3 includes NC-3 and NC-10.
//3.4.1.	Prolonged Ventricular Excitation(3-1-x to 3-4-x)
	short	vhCode31x();	// 310,311
//	short	vhCode310();	// Left bundle branch block (LBBB) 
//	short	vhCode311();	// LBBB with possible MI
	short	vhCode32x();	// 320,321
//	short	vhCode320();	// Right bundle branch block (RBBB) 
//	short	vhCode321();	// RBBB with possible MI 
	short	vhCode33x();	// 330,331
//	short	vhCode330();	// Indeterminate ventricular conduction delay (IVCD) 
//	short	vhCode331();	// IVCD with possible MI
	short	vhCode341();	// Incomplete right bundle branch block (IRBBB) 
	short	vhCode342();	// Incomplete left bundle branch block(ILBBB)
//3.4.2.	Fascicular Blocks (3-5-x)
//If QRS axis > –45° and QRS axis ≤ 90°, no code 3-5-x(NC-10,Fascicular block).
//	short	vhCode350();	// Left anterior fascicular block (LAFB) 
	short	vhCode350_1();	// 350,351
//	short	vhCode351();	// Probably Left anterior fascicular block (PLAFB) 
	short	vhCode352();	// Left posterior fascicular block (LPFB) 
//	short	vhCode370();	// Brugada syndrome
	short	vhCode371();	// Brugada syndrome, Type1
	short	vhCode372();	// Brugada syndrome, Type2
	short	vhCode373();	// Brugada syndrome, Type3
	short	vhCode380();	// Fragmented QRS
//3.5.	Code 4, Repolarization abnormalities
//If QTI < 112%, no code 4-1-x(NC 4-1-x,Prolonged Ventricular Repolarization).
//No MC code for prolonged ventricular Repolarization.
	void	vhCode4();
	short	vhCode41x();	// 410,411,412
	//short	vhCode410();	// Prolonged ventricular repolarization
	//short	vhCode411();	// Marginal prolongation of ventricular repolarization
	//short	vhCode412();	// Definite prolongation of ventricalar repolarization
	short	vhCode421();	//  Early Repolarzation(ER)
	short	vhCode422();	//  Probable early repolarization(PER)
//3.6.	Code 5, Myocardial Infarction/Ischemia (MI Likelihood)
	void	vhCode5();
	short	vhCode501();	// Poor R progression
	short	vhCode502();	// Abnormal Q wave
	short	vhCode510();	// Q wave MI with major Q waves 
	//short	vhCode511();	// Lateral
	//short	vhCode512();	// Inferior
	//short	vhCode513();	// Septal
	//short	vhCode514();	// Anterior
	short	vhCode520_30();	//520,530
	//short	vhCode520();	// Q wave MI with moderate Q waves and with ST-T abnormalities
	//short	vhCode521();	// Lateral
	//short	vhCode522();	// Inferior
	//short	vhCode523();	// Septal
	//short	vhCode524();	// Anterior
	//bool	vhCode530();	// Possible Q wave MI with moderate Q waves and without ST-T abnormalities
	//short	vhCode531();	// Lateral
	//short	vhCode532();	// Inferior
	//short	vhCode533();	// Septal
	//short	vhCode534();	// Anterior
	short	vhCode540();	// Possible Q wave MI with minor Q waves and with ST-T abnormalities
	//short	vhCode541();	// Lateral
	//short	vhCode542();	// Inferior
	//short	vhCode543();	// Septal
	//short	vhCode544();	// Anterior
	short	vhCode550_6x();	//550,560,561
	//short	vhCode550();	// ST abnormalities without Q waves
	//short	vhCode551();	// ST abnormalities//in 520,530,540 for children
	//short	vhCode560();	// T wave inversion without Q waves
	//short	vhCode561();	// T wave flat without Q waves
	short	vhCode570();	// Minor Q waves without ST-T abnormalities
	short	vhCode580();	// Minor ST-T abnormalities
//3.7.	Code 6, Ventricular Hypertrophy and Atrial Enlargement
//VH code 6 includes NC-6,7,8,9.
	void	vhCode6();		//
//	short	vhCode610();	// Left ventricular hypertrophy (LVH) (NC 6-1)
//	short	vhCode611();	// LVH with ST-T abnormalities (NC 6-1-1)
	short	vhCode61x();	// 610,611x
	short	vhCode621();	// Left atrial enlargement (NC 7-1)
	short	vhCode631();	// Right ventricular hypertrophy (RVH) (NC 8-1)
	short	vhCode641();	// Right atrial enlargement (NC 9-1)
//3.8.	Code 7, Axis deviation
	void	vhCode7();
	short	vhCode710();	// Left QRS axis deviation
	short	vhCode720();	// Marked right QRS axis deviation
	short	vhCode730();	// Right QRS axis deviation
	short	vhCode740();	// Extreme right QRS axis deviation
	short	vhCode750();	// Indeterminate QRS axis
	short	vhCode76x();	// 761，762
	//short	vhCode761();	// Wide QRS-T angle
	//short	vhCode762();	// Very wide QRS-T angle
//3.9.	 Code 8, Other abnormalities
	void	vhCode8();
	short	vhCode810();	// Dexiocardia(Check electrode again?)
	short	vhCode82x();	// 820,821,822
	//short	vhCode820();	// Low voltage QRS(LOW)
	//short	vhCode821();	// Limb leads low voltage QRS(LLW)
	//short	vhCode822();	// Chest leads low voltage QRS(CLW)
	short	vhCode831();	// Counter clock wise rotation(CCW)
	short	vhCode832();	// Clock wise rotation(CW)
	short	vhCode840();	// High T wave(HT)
	short	vhCode850();	// ST Segment Elevation (STE)
//Critical Values
	enum	{
		CV_VSTOP=11,	//11	心室停搏(无心电活动)		//0-1-0
		CV_VF	=12,	//12	心室颤动或扑动				//1-7-3,1-7-4
		CV_VT	=13,	//13	室速(连续>2个室早)			//1-7-0,1-0-37,1-0-38
		CV_PAUSE=14,	//14	停搏>=2s					//1-0-40
		CV_HVR	=15,	//15	心室率>=180次/分			//1-8-1
		CV_LHR	=16,	//16	心室率<=45次/分				//1-8-2
		CV_STD	=17,	//17	ST段压低					//
		CV_STE_HT=18,	//18	ST段抬高，T波高尖			//8-5=0 and 8-4-0	**
		CV_STE_IT=19,	//19	ST段抬高，T波倒置			//8-5-0 and 5-6-0	**
		CV_MPVC	=20,	//20	多源室性期前收缩或室性期前收缩RonT	//1-0-33,1-0-32
		CV_FPVC	=21,	//21	频发室性期前收缩伴QT延长			//1-0-31,1-0-32,1-0-33,1-0-34,1-0-35,1-0-36,1-0-37,1-0,38 and 4-1-2
		CV_AVB	=22,	//22	房室传导阻滞				//2-2-2,2-2-3,2-3-30,2-3-31,2-3-32
	};
};
