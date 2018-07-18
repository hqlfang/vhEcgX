#pragma once
#include "EcgCodeBase.h"

class CmcCode : public CEcgCodeBase
{
protected:
	CodeMgr			mc;
	bool	m_bIsExistCode1;
	static char		mccodebuf[8];
	unsigned short	mccode(short g,short c,short i) {return g*1000+c*10+i;}
public:
	CmcCode(void);
	virtual ~CmcCode(void);
	unsigned short SetmcCode(unsigned short nCode,DWORD dwLeads=0);
	short	mcCodeCount();
	short	mcCodeGetFirst(char *szLeadName=NULL);	//szLeadName[64],	cseCode[32]
	short	mcCodeGetNext(char *szLeadName=NULL);	//szLeadName[64],	cseCode[32]
	static char	*mcCodeString(unsigned short mccode);
public:
	short	mcCode();
//Q and QS Patterns(1-code)
//Do not code in the presence of WPW code 6-4-1, or artiﬁcial pacemaker code 6-8 or code 6-1, 8-2-1. 8-2-2, or 8-4-1 with a heart rate ≥ 140. To qualify as a Q-wave, the deﬂection should be at least 100 µV (1 mm in amplitude).
	void	mcCode1();
//Anterolateral Site (leads I, aVL, V6)
	short	mcCode111();	//111LIA
	short	mcCode112();	//112LIA
	short	mcCode121();	//121LIA
	short	mcCode122();	//122LIA
	short	mcCode123();	//123LI
	short	mcCode131();	//131LIA
	short	mcCode138();	//138LA
//	short	mcCode111L();	//	Q/R amplitude ratio ≥ 1/3, plus Q duration ≥ 30 ms in lead I or V6.
//	short	mcCode112L();	//	Q duration ≥ 40 ms in lead I or V6.
	short	mcCode113L();	//	Q duration ≥ 40 ms plus R amplitude ≥ 300 µV in lead aVL.
//	short	mcCode121L();	//	Q/R amplitude ratio ≥ 1/3, plus Q duration ≥ 20 ms and < 30 ms in lead I or V6.
//	short	mcCode122L();	//	Q duration ≥ 30 ms and < 40 ms in lead I or V6.
//	short	mcCode123L();	//	QS pattern in lead I. Do not code in the presence of 7-1-1.
//	short	mcCode131L();	//	Q/R amplitude ratio ≥ 1/5 and < 1/3, plus Q duration ≥ 20 ms and < 30 ms in lead I or V6.
	short	mcCode133L();	//	Q duration ≥ 30 ms and < 40 ms, plus R amplitude ≥ 300 µV in lead aVL.
//	short	mcCode138L();	//  Initial R amplitude decreasing to 200 µV or less in every beat (and absence of codes 3-2, 7-1-1, 7-2-1, or 7-3) between V5  and V6. (All beats in lead V5 must have an initial R > 200 µV.)
//Posterior (inferior) site (leads II, III, aVF)
//	short	mcCode111I();	//	Q/R amplitude ratio ≥ 1/3, plus Q duration ≥ 30 ms in lead II.
//	short	mcCode112I();	//	Q duration ≥ 40 ms in lead II.
	short	mcCode114I();	//	Q duration ≥ 50 ms in lead III, plus a Q-wave amplitude ≥ 100 µV in the majority of beats in lead aVF.
	short	mcCode115I();	//	Q duration ≥ 50 ms in lead aVF.
//	short	mcCode121I();	//	Q/R amplitude ratio ≥ 1/3, plus Q duration ≥ 20 ms and < 30 ms in lead II.
//	short	mcCode122I();	//	Q duration ≥ 30 ms and < 40 ms in lead II.
//	short	mcCode123I();	//	QS pattern in lead II. Do not code in the presence of 7-1-1.
	short	mcCode124I();	//	Q duration ≥ 40 ms and < 50 ms in lead III, plus a Q-wave ≥ 100 µV amplitude in the majority of beats in aVF.
	short	mcCode125I();	//	Q duration ≥ 40 ms and < 50 ms in lead aVF.
//	short	mcCode131I();	//	Q/R amplitude ratio ≥ 1/5 and < 1/3, plus Q duration ≥ 20 ms and < 30 ms in lead II.
	short	mcCode134I();	//	Q duration ≥ 30 ms and < 40 ms in lead III, plus a Q-wave ≥ 100 µV amplitude in the majority of beats in lead aVF.
	short	mcCode135I();	//	Q duration ≥ 30 ms and < 40 ms in lead aVF.
	short	mcCode136_7I();	//	136,137
//	short	mcCode136I();	//	QS pattern in each of leads III and aVF. (Do not code in the presence of 7-1-1.)
//	short	mcCode137I();	//   QS pattern in lead aVF only. (Do not code in the presence 7-1-1)
//Anterior Site (leads V1, V2, V3, V4, V5)
	short	mcCode117_27_32A();	//117A,127A,132A
//	short	mcCode111A();	//	Q/R amplitude ratio ≥ 1/3 plus Q duration ≥ 30 ms in any of leads V2, V3, V4, V5.
//	short	mcCode112A();	//	Q duration ≥ 40 ms in any of leads V1, V2, V3, V4, V5.
	short	mcCode116A();	//	QS pattern when initial R-wave is present in adjacent lead to the right on the chest,in any of leads V2, V3, V4, V5, V6.
//	short	mcCode117A();	//	QS pattern in all of leads V1-V4 or V1-V5.
//	short	mcCode121A();	//	Q/R amplitude ratio ≥ 1/3 plus Q duration ≥ 20 ms and < 30 ms, in any of leads V2,V3, V4, V5.
//	short	mcCode122A();	//	Q duration ≥ 30 ms and < 40 ms in any of leads V2, V3, V4, V5.
//	short	mcCode127A();	//	QS pattern in all of leads V1, V2, and V3. (Do not code in the presence of 7-1-1.)
//	short	mcCode131A();	//	Q/R amplitude ratio ≥ 1/5 and < 1/3, plus Q duration ≥ 20 ms and < 30 ms in any of leads V2, V3, V4, V5.
//	short	mcCode132A();	//	QS pattern in lead V1 and V2. (Do not code in the presence of 3-1 or 7-1-1.)
//	short	mcCode138A();	//  Initial R amplitude decreasing to 200 µV or less in every beat (and absence of codes 3-2, 7-1-1, 7-2-1, or 7-3) between any of leads V2 and V3, V3, and V4, or V4 and V5. (All beats in the lead immediately to the right on the chest must have an initial R >200 µV.)

//QRS Axis Deviation(2-code)
//Do not code in presence of low-voltage QRS code 9-1, WPW 6-4-1, artiﬁcial pacemaker code 6-8, ventricular conduction defects 7-1-1, 7-2-1, 7-4, or 7-8.
	void	mcCode2();
	//short	mcCode210();	//	Left. QRS axis from –30° through –90° in leads I, II and III. (The algebraic sum of major positive and major negative QRS waves must be zero or positive in I, negative in III, and zero or negative in II.)
	//short	mcCode220();	//	Right. QRS axis from +120° through –150° in leads I, II, and III. (The algebraic sum of major positive and major negative QRS waves must be negative in I, and zero or positive in III, and in I must be one-half or more of that in III.)
	//short	mcCode230();	//	Right (optional code when 2-2 is not present). QRS axis from +90° through +119° in leads I, II, and III. (The algebraic sum of major positive and major negative QRS waves must be zero or negative in I and positive in II and III.)
	//short	mcCode240();	//	Extreme axis deviation (usually S1, S2, S3 pattern). QRS axis from –90° through–149° in leads I, II and III. (The algebraic sum of major positive and major negative QRS waves must be negative in each of leads I, II, and III.)
	//short	mcCode250();	//	Indeterminate axis. QRS axis approximately 90° from the frontal plane. (The algebraic sum of major positive and major negative QRS waves is zero in each of leads I, II and III, or the information from these three leads is incongruous.)

//High Amplitude R Waves(3-code)
//Do not code in the presence of codes 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8.
	void	mcCode3();
	short	mcCode310();	//	Left: R amplitude > 2600 µV in either V5  or V6, or R amplitude > 2000 µV in any of leads I, II, III, aVF, or R amplitude > 1200 µV in lead aVL measured only on second to last complete normal beat.
	short	mcCode320();	//	Right: R amplitude ≥ 500 µV and R amplitude ≥ S amplitude in the majority of beats in lead V1, when S amplitude is > R amplitude somewhere to the left on the chest of V1 (codes 7-3 and 3-2, if criteria for both are present).
	short	mcCode330();	//	Left (optional code when 3-1 is not present): R amplitude > 1500 µV but ≤ 2000 µV in lead I, or R amplitude in V5 or V6, plus S amplitude in V1 > 3500 µV.
	short	mcCode340();	//	Criteria for 3-1 and 3-2 both present.

//ST Junction (J) and Segment Depression(4-code)
//Do not code in the presence of codes 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8. When 4-1, 4-2, or 4-3 is coded, then a 5-code most often must also be assigned except in lead V1.
	void	mcCode4();
	inline short mcCode411(short *L,short n);	//LIA
	inline short mcCode412(short *L,short n);	//LIA
	inline short mcCode420(short *L,short n);	//LIA
	short	mcCode430(short *L,short n);	//LIA
	short	mcCode440(short *L,short n);	//LIA
	inline short	mcCode41x_20(unsigned short code,short *L,short n);	//LIA
	//Anterolateral Site (leads I, aVL, V6)
	short	mcCode411L();	//	STJ depression ≥ 200 µV and ST segment horizontal or downward sloping in any of leads I, aVL, or V6.
	short	mcCode412L();	//	STJ depression ≥ 100 µV but < 200 µV, and ST segment horizontal or downward sloping in any of leads I, aVL, or V6.
	short	mcCode420L();	//	STJ depression ≥ 50 µV but < 100 µV and ST segment horizontal or downward sloping in any of leads I, aVL, or V6.
	short	mcCode430L();	//	No STJ depression as much as 50 µV, but ST segment downward sloping and segment or T-wave nadir ≥ 50 µV below P-R baseline, in any of leads I, aVL, or V6.
	short	mcCode440L();	//	STJ depression ≥ 100 µV and ST segment upward sloping or U-shaped, in any of leads I, aVL, or V6.
//Posterior (inferior) Site (leads II, III, aVF)
	short	mcCode411I();	//	STJ depression ≥ 200 µV and ST segment horizontal or downward sloping in lead II or aVF.
	short	mcCode412I();	//	STJ depression ≥ 100 µV but < 200 µV and ST segment horizontal or downward sloping in lead II or aVF.
	short	mcCode420I();	//	STJ depression ≥ 50 µV but < 100 µV and ST segment horizontal or downward sloping in lead II or aVF.
	short	mcCode430I();	//	No STJ depression as much as 50 µV, but ST segment downward sloping and segment or T-wave nadir ≥ 50 µV below P-R baseline in lead II.
	short	mcCode440I();	//	STJ depression ≥ 100 µV and ST segment upward sloping, or U-shaped, in lead II.
//Anterior Site (leads V1, V2, V3, V4, V5)
	short	mcCode411A();	//	STJ depression ≥ 200 µV and ST segment horizontal or downward sloping in any of leads V1, V2, V3, V4, V5.
	short	mcCode412A();	//	STJ depression ≥ 100 µV but < 200 µV and ST segment horizontal or downward sloping in any of leads V1, V2, V3, V4, V5.
	short	mcCode420A();	//	STJ depression ≥ 50 µV but < 100 µV and ST segment horizontal or downward sloping in any of leads V1, V2, V3, V4, V5.
	short	mcCode430A();	//	No STJ depression as much as 50 µV, but ST segment downward sloping and segment or T-wave nadir ≥ 50 µV below P-R baseline in any of leads V2, V3, V4, V5.
	short	mcCode440A();	//	STJ depression ≥ 100 µV and ST segment upward sloping or U-shaped in any of leads V1, V2, V3, V4, V5.

//T-Wave Items(5-code)
//Do not code in the presence of codes 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8.
	void	mcCode5();
	short	mcCode530(short *L,short n);
	short	mcCode540(short *L,short n);
//Anterolateral Site (leads I, aVL, V6)
	short	mcCode510L();	//	T amplitude negative 500 µV or more in either of leads I, V6, or in lead aVL when R amplitude is ≥ 500 µV.
	short	mcCode520L();	//	T amplitude negative or diphasic (positive–negative or negative–positive type) with negative phase at least 100 µV but not as deep as 500 µV in lead I or V6, or in lead aVL when R amplitude is ≥ 500 µV.
	short	mcCode530L();	//	T amplitude zero (ﬂat), or negative, or diphasic (negative–positive type only) with less than 100 µV negative phase in lead I or V6, or in lead aVL when R amplitude is ≥ 500 µV.
	short	mcCode540L();	//	T amplitude positive and T/R amplitude ratio < 1/20 in any of leads I, aVL, V6; R wave amplitude must be ≥ 1000 µV.
//Posterior (Inferior) Site (leads II, III, aVF)
	short	mcCode510I();	//	T amplitude negative 500 µV or more in lead II, or in lead aVF when QRS is mainly upright.
	short	mcCode520I();	//	T amplitude negative or diphasic with negative phase (negative–positive or positive– negative type) at least 100 µV but not as deep as 500 µV in lead II, or in lead aVF when QRS is mainly upright.
	short	mcCode530I();	//	T amplitude zero (ﬂat), or negative, or diphasic (negative-positive type only) with less than 100 µV negative phase in lead II; not coded in lead aVF.
	short	mcCode540I();	//	T amplitude positive and T/R amplitude ratio < 1/20 in lead II; R wave amplitude must be ≥ 1000 µV.
//Anterior Site (leads V2, V3, V4, V5)
	short	mcCode510A();	//	T amplitude negative 500 µV or more in any of leads V2, V3, V4, V5.
	short	mcCode520A();	//	T amplitude negative, or diphasic (negative–positive or positive–negative type) with negative phase at least 100 µV but not as deep as 500 µV, in any of leads V2, V3, V4, V5.
	short	mcCode530A();	//	T amplitude zero (ﬂat), or negative, or diphasic (negative–positive type only) with less than 100 µV negative phase, in any of leads V3, V4, V5.
	short	mcCode540A();	//	T amplitude positive and T/R amplitude ratio < 1/20 in any of leads V3, V4, V5; R wave amplitude must be ≥ 1000 µV.

//A-V Conduction Defect(6-code)
	void	mcCode6();
	short	mcCode610();	//	Complete (third degree) A-V block (permanent or intermittent) in any lead. Atrial and ventricular complexes independent, and atrial rate faster than ventricular rate, with ventricular rate < 60.
	short	mcCode62x();	//	621,622,623
//	short	mcCode621();	//	Mobitz Type II (occurrence of P-wave on time with dropped QRS and T).
//	short	mcCode622();	//	Partial (second degree) A-V block in any lead (2:1 or 3:1 block).
//	short	mcCode623();	//	Wenckebach’s Phenomenon (P-R interval increasing from beat to beat until QRS and T dropped).
	short	mcCode630();	//	P-R (P-Q) interval ≥ 220 ms in the majority of beats in any of leads I, II, III, aVL, aVF.
	short	mcCode641();	//	Wolff-Parkinson-White Pattern (WPW), persistent. Sinus P-wave. P-R interval < 120 ms, plus QRS duration ≥ 120 ms, plus R peak duration ≥ 60 ms, coexisting in the same beat and present in the majority of beats in any of leads I, II, aVL, V4, V5, V6. (6-4-1 suppresses 1-2-3, 1-2-7, 1-3-2, 1-3-6, 1-3-8, all 3, 4, 5, 7, 9-2, 9-4, 9-5 codes.)
	short	mcCode642();	//	WPW Pattern, intermittent. WPW pattern in ≤ 50% of beats in appropriate leads.
	short	mcCode650();	//	Short P-R interval. P-R interval < 120 ms in all beats of any two of leads I, II, III, aVL, aVF.
	short	mcCode660();	//	Intermittent aberrant atrioventricular conduction. P-R > 120 ms (except in presence of 6-5 or heart rate greater than 100), and wide QRS complex > 120 ms, and normal P-wave when most beats are sinus rhythm. (Do not code in the presence of 6-4-2.)
	short	mcCode680();	//	Electronic pacemaker.

//Ventricular Conduction Defect(7-code)
	void	mcCode7();
	short	mcCode711_60();	//	Complete left bundle branch block (LBBB). (Do not code in presence of 6-1, 6-4-1,6-8, 8-2-1 or 8-2-2.) QRS duration ≥ 120 ms in a majority of beats (of the same QRS pattern) in any of leads I, II, III, aVL, aVF, plus R peak duration ≥ 60 ms in a majority of beats (of the same QRS pattern) in any of leads I, II, aVL, V5, V6. (7-1-1 suppresses 1-2-3, 1-2-7, 1-3-2, 1-3-6, 1-3-7, 1-3-8, all 2, 3, 4, 5, 9-2, 9-4, 9-5 codes. If any other codable Q-wave coexists with the LBBB pattern, code the Q and diminish the 7-1-1 code to a 7-4 code.)
	short	mcCode712();	//	Intermittent left bundle branch block. Same as 7-1-1 but with presence of normally conducted QRS complexes of different shape than the LBBB pattern.
	short	mcCode72x();	//	Complete right bundle branch block (RBBB). (Do not code in the presence of 6-1,6-4-1, 6-8, 8-2-1 or 8-2-2.) QRS duration ≥ 120 ms in a majority of beats (of the same QRS pattern) in any of leads I, II, III, aVL, aVF, plus: R΄ > R in V1; or QRS mainly upright, plus R peak duration ≥ 60 ms in V1 or V2; or S duration > R duration in all beats in lead I or II. (Suppresses 1-3-8, all 2-, 3-, 4-and 5-codes, 9-2, 9-4, 9-5.)
//	short	mcCode722();	//	Intermittent right bundle branch block. Same as 7-2-1 but with presence of normally conducted QRS complexes of different shape than the RBBB pattern.
	short	mcCode730();	//	Incomplete right bundle branch block. QRS duration < 120 ms in each of leads I, II, III, aVL, aVF, and R΄ > R in either of leads V1, V2 (Code as 3-2 in addition if those criteria are met. 7-3 suppresses code 1-3-8.)
	short	mcCode740();	//	Intraventricular block. QRS duration ≥ 120 ms in a majority of beats in any of leads I, II, III, aVL, aVF. (7-4 suppresses all 2, 3, 4, 5, 9-2, 9-4, 9-5 codes.)
	short	mcCode750();	//	R-R΄ pattern in either of leads V1, V2 with R΄ amplitude ≤ R.
//	short	mcCode760();	//	Incomplete left bundle branch block. (Do not code in the presence of any codable Q- or QS-wave.) QRS duration ≥ 100 ms and < 120 ms in the majority of beats of each of leads I, aVL, and V5 or V6.
	short	mcCode770();	//	Left anterior hemiblock (LAH). QRS duration < 120 ms in the majority of beats in leads I, II,III, aVL, aVF, plus Q-wave amplitude ≥ 250 µV and < 30 ms duration in lead I or aVL, plus left axis deviation of –45° or more negative. (In presence of 7-2, code 7-8 if axis is < – 45° and the Q-wave in lead I meets the above criteria.) 
	short	mcCode780();	//	Combination of 7-7 and 7-2.
	short	mcCode791_2_3();
//	short	mcCode791();	//  Type 1 Brugada pattern convex (coved) ST segment elevation ≥ 200 µV plus T-wave negative with little or no isolelectric (baseline) separation in at least 2 leads of V1-V3.
//	short	mcCode792();	//  Type 2 Brugada pattern ST segment elevation ≥ 200 µV plus T-wave positive or diphasic that  results in a “saddle-back” shape in at least 2 leads of V1-V3.
//	short	mcCode793();	//  Type 3 Brugada pattern. 7-2-1 plus ST segment elevation ≤ 100 µV plus a “saddle-back”conﬁguration in at least 2 leads of V1–V3.
	short	mcCode7100();	//	Fragmented QRS.
							//Rules:
							//(1) notching is a sudden change in slope (down or up) of any part of the Q-, R-, or S-wave; 
							//(2) notching can occur anywhere from beginning to end of QRS; 
							//(3) no restrictions on height or dura- tion of the notch; 
							//(4) small initial R or small Q-waves at the start of the QRS complex are not considered notching; 
							//(5) the notch may not cross the baseline; 
							//(6) notching must be present in a majority of beats in a lead; 
							//(7) no 50/60 Hz AC power interference. 
							//(8)This could be noted with technical problem code 9-8-2.
//Arrhythmias(8-code)
	void	mcCode8();
	short	mcCode81x();	//811,812,813,814,815
	//short	mcCode811();	//	Presence of any atrial or junctional premature beats.
	//short	mcCode812();	//	Presence of any ventricular premature beats.
	//short	mcCode813();	//	Presence of both atrial and/or junctional premature beats and ventricular premature beats.
	short	mcCode814();	//	Wandering atrial pacemaker.
	short	mcCode815();	//	Presence of 8-1-2 and 8-1-4.
	short	mcCode821();	//	Ventricular ﬁbrillation or ventricular asystole.
	short	mcCode822();	//	Persistent ventricular (idioventricular) rhythm.
	short	mcCode823_42();	//823,842
//	short	mcCode823();	//	Intermittent ventricular tachycardia. Three or more consutive ventricular premature beats occurring at a rate ≥ 100. This includes more persistent ventricular tachycardia.
	short	mcCode824();	//	Ventricular parasystole (should not be coded in presence of 8-3-1).
	short	mcCode831_2();
//	short	mcCode831();	//	Atrial ﬁbrillation (persistent).
//	short	mcCode832();	//	Atrial ﬂutter (persistent).
	short	mcCode833();	//	Intermittent atrial ﬁbrillation (code if 3 or more clear-cut, consutive sinus beats are present in any lead).
	short	mcCode834();	//	Intermittent atrial ﬂutter (code if 3 or more clear-cut, consutive sinus beats are present in any lead).
	short	mcCode841();	//	Supraventricular rhythm persistent. QRS duration < 120 ms; and absent P-waves or presence of abnormal P-waves (inverted or ﬂat in II, III and aVF); and regular rhythm.
//	short	mcCode842();	//	Supraventricular tachycardia intermittent. Three consutive atrial or junctional pre- mature beats occurring at a rate ≥ 100.
	short	mcCode85x();	//	851,852
//	short	mcCode851();	//	Sinoatrial arrest. Unexpected absence of P, QRS and T, plus a R-R interval at a ﬁxed multiple of the normal interval, ±10%.
//	short	mcCode852();	//	Sinoatrial block. Unexpected absence of P, QRS and T, preceded by progressive shortening of P-P intervals. (R-R interval at a ﬁxed multiple of the normal interval,± 10%.)
	short	mcCode86x();	//	861,862,863,864
//	short	mcCode861();	//	A-V dissociation with ventricular pacemaker (without capture). Requires: P-P and R-R occur at variable rates with ventricular rate as fast as or faster than the atrial rate, plus variable P-R intervals, plus no capture beats.
//	short	mcCode862();	//	A-V dissociation with ventricular pacemaker (with capture).
//	short	mcCode863();	//	A-V dissociation with atrial pacemaker (without capture).
//	short	mcCode864();	//	A-V dissociation with atrial pacemaker (with capture).
	short	mcCode870_80();	//870,880
//	short	mcCode870();	//	Sinus tachycardia (≥100/min).
//	short	mcCode880();	//	Sinus bradycardia (≤50/min).
	short	mcCode890();	//	Other arthythmias. Heart rate may be recorded as a continuous variable.
//
	void	mcCode9();
//ST Segment Elevation(9-2-code)
//Do not code in the presence of codes 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8.
	short	mcCode920();
//Anterolateral Site (leads I, aVL, V6)
//	short	mcCode920L();	//	ST segment elevation ≥ 100 µV in any of leads I, aVL, V6.
//Posterior (Inferior) Site (leads II, III, aVF)
//	short	mcCode920I();	//	ST segment elevation ≥ 100 µV in any of leads II, III, aVF.
//Anterior site (Leads V1, V2, V3, V4, V5)
//	short	mcCode920A();	//	ST segment elevation ≥ 100 µV in lead V5 or ST segment elevation ≥ 200 µV in any of leads V1, V2, V3, V4.
//Miscellaneous Items(other 9-code)
	short	mcCode910();	//	Low QRS amplitude. QRS peak-to-peak amplitude < 500 µV in all beats in each of leads I, II, III, or < 1000 µV in all beats in each of leads V1, V2, V3, V4, V5, V6. (Check calibration before coding.)
	short	mcCode930();	//	P-wave amplitude ≥ 2.500 µV in any of leads II, III, aVF, in a majority of beats.
	short	mcCode941();	//	QRS transition zone at V3  or to the right of V3 on the chest.(all beats mainly positive in any lead V1~V3). (Do not code in the presence of 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8.)
	short	mcCode942();	//	QRS transition zone at V4  or to the left of V4 on the chest.(all beats mainly negative in any lead V4~V6 and all leads negative in V1~V3). (Do not code in the presence of 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8.)
	short	mcCode950();	//	T-wave amplitude > 1200 µV in any of leads I, II, III, aVL, aVF, V1, V2, V3, V4, V5, V6. (Do not  code in the presence of 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8.)
	short	mcCode960();	//  Notched and widened P wave (duration ≥ 120 ms.) in frontal plane (usually lead II), and/or deep negative component to the P wave in lead V1  duration ≥ 40 ms. and depth ≥ 100 µV.
	short	mcCode971_2();
//	short	mcCode971();	//  Definite Early Repolarization.STJ elevation ≥ 1mm in the majority of beats, T wave amplitude≥ 500 µV, prominent J point, upward concavity of the ST segment, and a distinct notch  or slur on the down-stroke of the R wave in any of V3 –V6, or STJ elevation ≥ 200 µV in the majority of beats and T wave amplitude ≥ 500 µV, prominent J point and upward concavity of the  ST segment in any of V3 –V6.
//	short	mcCode972();	//  Probable Early Repolarization. STJ elevation ≥ 100 µV in the majority of beats, prominent J point,  and upward concavity of the ST segment in any of V3 –V6 and T wave amplitude ≥ 800 µV in any of the leads V3 –V6.
	short	mcCode981();	//  Uncorrectable lead reversal.
	short	mcCode982();	//  Poor Quality/Technical problems which interfere with coding.
	short	mcCode983();	//  Correctable lead reversal
							//i.  Correctable limb lead connection error RA/LA Reserval
							//ii.  Correctable chest lead connection error in V1-V3 
							//iii. Correctable chest lead connection error in V4-V6
							//iv.  Correctable other chest lead connection error
	short	mcCode984();	//  Technical problems that do not interfere with coding.
};
