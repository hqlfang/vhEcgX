#include "stdafx.h"
#include "EcgCodeMC.h"
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////////
//begin class CmcCode

char CmcCode::mccodebuf[8];

CmcCode::CmcCode(void) : CEcgCodeBase()
{
}

CmcCode::~CmcCode(void)
{
}

unsigned short CmcCode::SetmcCode(unsigned short nCode,DWORD dwLeads)
{
	CodeList *p=mc.found(nCode);
	if(p) {
		p->code.nLeads|=dwLeads;
	}
	else {
		Code	code;
		code.nCode=nCode;
		code.nLeads=dwLeads;
		code.nClass=0;
		code.nSort=0;
		code.nIndex=0;
		mc.add(code);
	}
	return nCode;
}
	
short CmcCode::mcCodeCount()
{
	return (short)mc.getCount();
}
	
short CmcCode::mcCodeGetFirst(char *szLeadName)
{
	CodeList *p=mc.getFirst();
	if(p) {
		if(szLeadName) {
			strcpy(szLeadName,LeadNames(p->code.nLeads));
		}
		return p->code.nCode;
	}
	return 0;
}
	
short CmcCode::mcCodeGetNext(char *szLeadName)
{
	CodeList *p=mc.getNext();
	if(p) {
		if(szLeadName) {
			strcpy(szLeadName,LeadNames(p->code.nLeads));
		}
		return p->code.nCode;
	}
	return 0;
}

char *CmcCode::mcCodeString(unsigned short mccode)
{
	sprintf(mccodebuf,"%d-%d",mccode/1000,(mccode%1000)/10);
	if(mccode%10>0) {
		char tmp[4];
		sprintf(tmp,"-%d",mccode%10);
		strcat(mccodebuf,tmp);
	}
	return mccodebuf;
}

short CmcCode::mcCode()
{
	mc.reset();
	m_bIsExistCode1=false;
	if(mcCode821()) return (short)mc.getCount();
	if(mcCode610()) {
		mcCode822();	return (short)mc.getCount();
	}
	if(mcCode831_2() || mcCode641() || mcCode680()) return (short)mc.getCount();;
	if(mcCode841() && bpmHR()>=140) {
		mcCode740();	mcCode62x();	return (short)mc.getCount();;
	}
	mcCode8();	//Arrhythmias
	if(!m_pEcgParm || !m_pEcgLead || !m_pEcgBeat || !m_pEcgInfo) return (short)mc.getCount();
	mcCode7();	//Ventricular Conduction
	bool code711=mc.found(mccode(7,1,1))?true:false;
	bool code721=mc.found(mccode(7,2,1))?true:false;
	bool code740=mc.found(mccode(7,4,0))?true:false;
	bool code780=mc.found(mccode(7,8,0))?true:false;
	mcCode6();	//A-V Conduction Defect
	mcCode1();	//Q-,QS patterns
	mcCode9();	//ST segment Elevation(9-2) and Miscellaneous Items
	if(mc.found(mccode(7,3,0))) mc.remove(mccode(1,3,8));
	if(!code711 && !code721 && !code740 && !code780) {
		mcCode2();	//QRS Axis Deviation
		mcCode3();	//High Amplitude R Waves
		mcCode4();	//ST junction(J) and segment depression
		mcCode5();	//T-wave Items
	}
	else {
		mc.remove(mccode(9,2,0));	mc.remove(mccode(9,4,0));	mc.remove(mccode(9,5,0));
	}
	if(code711 || code721 || code780) {
		mc.remove(mccode(1,3,8));	mc.remove(mccode(9,7,1));	mc.remove(mccode(9,7,2));
	}
	if(code711) {
		mc.remove(mccode(1,2,3));	mc.remove(mccode(1,2,7));	mc.remove(mccode(1,3,2));	mc.remove(mccode(1,3,6));	mc.remove(mccode(1,3,7));
	}
	if(m_bIsExistCode1) mc.remove(mccode(7,6,0));
	if(mc.found(mccode(3,1,0))) 	mc.remove(mccode(1,3,2));
	if(mc.found(mccode(3,2,0)))	{mc.remove(mccode(1,3,8));}//mc.remove(mccode(7,3,0));}

	if(mc.found(mccode(8,1,2))) mc.remove(mccode(8,2,4));
	if(mc.found(mccode(8,1,4))) {mc.remove(mccode(8,1,1));	mc.remove(mccode(9,3,0));}
	if(mc.found(mccode(8,2,3))) mc.remove(mccode(8,1,2));
	if(mc.found(mccode(8,3,1)) || mc.found(mccode(8,3,2)) || mc.found(mccode(8,3,3)) ) {
		mc.remove(mccode(8,1,1));	mc.remove(mccode(8,1,2));
	}
	if(mc.found(mccode(8,3,2)) || mc.found(mccode(8,3,4))) mc.remove(mccode(6,2,2));
	if(mc.found(mccode(8,4,1))) mc.remove(mccode(6,5,0));
	if(mc.found(mccode(8,4,2))) mc.remove(mccode(8,1,1));

	return (short)mc.getCount();
}

//Q and QS Patterns(1-code)
//Do not code in the presence of WPW code 6-4-1, or artiﬁcial pacemaker code 6-8 or code 6-1, 8-2-1. 8-2-2, or 8-4-1 with a heart rate ≥ 140. To qualify as a Q-wave, the deﬂection should be at least 100 µV (1 mm in amplitude).
void CmcCode::mcCode1()
{
	m_bIsExistCode1=false;
	if(mcCode111()) m_bIsExistCode1=true;	//111LIA
	if(mcCode112()) m_bIsExistCode1=true;	//112LIA
	if(mcCode113L()) m_bIsExistCode1=true;	//	Q duration ≥ 40 ms plus R amplitude ≥ 300 µV in lead aVL.
	if(mcCode114I()) m_bIsExistCode1=true;	//	Q duration ≥ 50 ms in lead III, plus a Q-wave amplitude ≥ 100 µV in the majority of beats in lead aVF.
	if(mcCode115I()) m_bIsExistCode1=true;	//	Q duration ≥ 50 ms in lead aVF.
	if(mcCode116A()) m_bIsExistCode1=true;	//	QS pattern when initial R-wave is present in adjacent lead to the right on the chest,in any of leads V2, V3, V4, V5, V6.
	if(mcCode121()) m_bIsExistCode1=true;	//121LIA
	if(mcCode122()) m_bIsExistCode1=true;	//122LIA
	if(mcCode123()) m_bIsExistCode1=true;	//123LI
	if(mcCode124I()) m_bIsExistCode1=true;	//	Q duration ≥ 40 ms and < 50 ms in lead III, plus a Q-wave ≥ 100 µV amplitude in the majority of beats in aVF.
	if(mcCode125I()) m_bIsExistCode1=true;	//	Q duration ≥ 40 ms and < 50 ms in lead aVF.
	if(mcCode117_27_32A()) m_bIsExistCode1=true;	//117A,127A,132A
	if(mcCode131()) m_bIsExistCode1=true;	//131LIA
	if(mcCode138()) m_bIsExistCode1=true;	//138LA
	if(mcCode133L()) m_bIsExistCode1=true;	//	Q duration ≥ 30 ms and < 40 ms, plus R amplitude ≥ 300 µV in lead aVL.
	if(mcCode134I()) m_bIsExistCode1=true;	//	Q duration ≥ 30 ms and < 40 ms in lead III, plus a Q-wave ≥ 100 µV amplitude in the majority of beats in lead aVF.
	if(mcCode135I()) m_bIsExistCode1=true;	//	Q duration ≥ 30 ms and < 40 ms in lead aVF.
	if(mcCode136_7I()) m_bIsExistCode1=true;	//	136,137
}
//Anterolateral Site (leads I, aVL, V6)
short CmcCode::mcCode111()	//	Q/R amplitude ratio ≥ 1/3, plus Q duration ≥ 30 ms in lead I or V6.
{
	short i,L[]={I,II,V2,V3,V4,V5,V6},n=sizeof(L)/sizeof(short);
	unsigned long  nLeads=0;
	for(i=0;i<n;i++) {
		if(3*uvQd(L[i])>=uvR(L[i]) && msQd(L[i])>=30) {
			nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,1,1);	//1011;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode112()	//	Q duration ≥ 40 ms in lead I or V6.
{
	short i,L[]={I,II,V1,V2,V3,V4,V5,V6},n=sizeof(L)/sizeof(short);
	unsigned long  nLeads=0;
	for(i=0;i<n;i++) {
		if(msQd(L[i])>=40) {
			nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,1,2);	//1012;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode113L()	//	Q duration ≥ 40 ms plus R amplitude ≥ 300 µV in lead aVL.
{
	unsigned long  nLeads=0;
	if(msQd(aVL)>=40 && uvR(aVL)>=300) {
		nLeads|=m_infoLead[aVL].mask;
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,1,3);	//1013;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode121()	//	Q/R amplitude ratio ≥ 1/3, plus Q duration ≥ 20 ms and < 30 ms in lead I or V6.
{
	short i,L[]={I,II,V2,V3,V4,V5,V6},n=sizeof(L)/sizeof(short);
	unsigned long  nLeads=0;
	for(i=0;i<n;i++) {
		if(3*uvQd(L[i])>=uvR(L[i]) && msQd(L[i])>=20 && msQd(L[i])<30) {
			nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,2,1);	//1021;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode122()	//	Q duration ≥ 30 ms and < 40 ms in lead I or V6.
{
	short i,L[]={I,II,V2,V3,V4,V5,V6},n=sizeof(L)/sizeof(short);
	unsigned long  nLeads=0;
	for(i=0;i<n;i++) {
		if(msQd(L[i])>=30 && msQd(L[i])<40) {
			nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,2,2);	//1022;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode123()	//	QS pattern in lead I. Do not code in the presence of 7-1-1.
{
	short i,L[]={I,II},n=sizeof(L)/sizeof(short);
	unsigned long  nLeads=0;
	for(i=0;i<n;i++) {
		if(isQS(L[i])) {
			nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,2,3);	//1023;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode131()	//	Q/R amplitude ratio ≥ 1/5 and < 1/3, plus Q duration ≥ 20 ms and < 30 ms in lead I or V6.
{
	short i,L[]={I,II,V2,V3,V4,V5,V6},n=sizeof(L)/sizeof(short);
	unsigned long  nLeads=0;
	for(i=0;i<n;i++) {
		if(5*uvQd(L[i])>=uvR(L[i]) && 3*uvQd(L[i])<uvR(L[i]) && msQd(L[i])>=20 && msQd(L[i])<30) {
			nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,3,1);	//1031;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode133L()	//	Q duration ≥ 30 ms and < 40 ms, plus R amplitude ≥ 300 µV in lead aVL.
{
	unsigned long  nLeads=0;
	if(msQd(aVL)>=30 && msQd(aVL)<40 && uvR(aVL)>=300) {
		nLeads|=m_infoLead[aVL].mask;
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,3,3);	//1033;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode138()	//  Initial R amplitude decreasing to 200 µV or less in every beat (and absence of codes 3-2, 7-1-1, 7-2-1, or 7-3) between any of leads V2 and V3, V3, and V4, or V4 and V5. (All beats in the lead immediately to the right on the chest must have an initial R >200 µV.)
{
	short i,L[]={V2,V3,V4,V5},n=sizeof(L)/sizeof(short);
	unsigned long  nLeads=0;
	for(i=0;i<n;i++) {
		if(uvR(L[i])>200 && uvR(L[i]+1)<=200) {
			nLeads|=m_infoLead[L[i]+1].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,3,8);	//1038;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
//Posterior (inferior) site (leads II, III, aVF)
//short CmcCode::mcCode111I();	//	Q/R amplitude ratio ≥ 1/3, plus Q duration ≥ 30 ms in lead II.
//short CmcCode::mcCode112I();	//	Q duration ≥ 40 ms in lead II.
short CmcCode::mcCode114I()	//	Q duration ≥ 50 ms in lead III, plus a Q-wave amplitude ≥ 100 µV in the majority of beats in lead aVF.
{
	unsigned long  nLeads=0;
	if(msQd(III)>=50 && uvQd(aVF)<=-100) {
		nLeads|=(m_infoLead[III].mask|m_infoLead[aVF].mask);
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,1,4);	//1014;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode115I()	//	Q duration ≥ 50 ms in lead aVF.
{
	unsigned long  nLeads=0;
	if(msQd(aVF)>=50) {
		nLeads|=m_infoLead[aVF].mask;
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,1,5);	//1015;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
//short CmcCode::mcCode121I();	//	Q/R amplitude ratio ≥ 1/3, plus Q duration ≥ 20 ms and < 30 ms in lead II.
//short CmcCode::mcCode122I();	//	Q duration ≥ 30 ms and < 40 ms in lead II.
//short CmcCode::mcCode123I();	//	QS pattern in lead II. Do not code in the presence of 7-1-1.
short CmcCode::mcCode124I()	//	Q duration ≥ 40 ms and < 50 ms in lead III, plus a Q-wave ≥ 100 µV amplitude in the majority of beats in aVF.
{
	unsigned long  nLeads=0;
	if(msQd(III)>=40 && msQd(III)<50 && uvQd(aVF)<=-100) {
		nLeads|=(m_infoLead[III].mask|m_infoLead[aVF].mask);
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,2,4);	//1024;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode125I()	//	Q duration ≥ 40 ms and < 50 ms in lead aVF.
{
	unsigned long  nLeads=0;
	if(msQd(aVF)>=40 && msQd(aVF)<50) {
		nLeads|=m_infoLead[aVF].mask;
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,2,5);	//1025;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
//short CmcCode::mcCode131I();	//	Q/R amplitude ratio ≥ 1/5 and < 1/3, plus Q duration ≥ 20 ms and < 30 ms in lead II.
short CmcCode::mcCode134I()	//	Q duration ≥ 30 ms and < 40 ms in lead III, plus a Q-wave ≥ 100 µV amplitude in the majority of beats in lead aVF.
{
	unsigned long  nLeads=0;
	if(msQd(III)>=30 && msQd(III)<40 && uvQd(aVF)<=-100) {
		nLeads|=(m_infoLead[III].mask|m_infoLead[aVF].mask);
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,3,4);	//1034;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode135I()	//	Q duration ≥ 30 ms and < 40 ms in lead aVF.
{
	unsigned long  nLeads=0;
	if(msQd(aVF)>=30 && msQd(aVF)<40) {
		nLeads|=m_infoLead[aVF].mask;
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,3,5);	//1035;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode136_7I()	//	QS pattern in each of leads III and aVF. (Do not code in the presence of 7-1-1.)
{
	unsigned short code=mccode(1,3,7);
	unsigned long  nLeads=0;
	if(isQS(aVF)) {
		nLeads=m_infoLead[aVF].mask;
		if(isQS(III)) {
			code=mccode(1,3,6);
			nLeads|=m_infoLead[III].mask;
		}
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
//short CmcCode::mcCode137I();	//   QS pattern in lead aVF only. (Do not code in the presence 7-1-1)
//Anterior Site (leads V1, V2, V3, V4, V5)
//short CmcCode::mcCode111A();	//	Q/R amplitude ratio ≥ 1/3 plus Q duration ≥ 30 ms in any of leads V2, V3, V4, V5.
//short CmcCode::mcCode112A();	//	Q duration ≥ 40 ms in any of leads V1, V2, V3, V4, V5.
short CmcCode::mcCode116A()	//	QS pattern when initial R-wave is present in adjacent lead to the right on the chest,in any of leads V2, V3, V4, V5, V6.
{
	short i,L[]={V2,V3,V4,V5,V6},n=sizeof(L)/sizeof(short);
	unsigned long  nLeads=0;
	for(i=0;i<n;i++) {
		if(isQS(L[i]) && uvR1(L[i]-1)>=25) {
			nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(1,1,6);	//1016;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode117_27_32A()	//	QS pattern in all of leads V1-V4 or V1-V5.
{
	unsigned short code=mccode(1,3,2);
	unsigned long  nLeads=0;
	if(isQS(V1) && isQS(V2)) nLeads=m_infoLead[V1].mask|m_infoLead[V2].mask;
	if(nLeads>0) {
		if(isQS(V3)) {
			nLeads|=m_infoLead[V3].mask;
			if(isQS(V4)) {
				code=mccode(1,1,7);
				nLeads|=m_infoLead[V4].mask;
				if(isQS(V5)) nLeads|=m_infoLead[V5].mask;
				if(isQS(V6)) nLeads|=m_infoLead[V6].mask;
			}
			else code=mccode(1,2,7);
		}
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
//short CmcCode::mcCode121A();	//	Q/R amplitude ratio ≥ 1/3 plus Q duration ≥ 20 ms and < 30 ms, in any of leads V2,V3, V4, V5.
//short CmcCode::mcCode122A();	//	Q duration ≥ 30 ms and < 40 ms in any of leads V2, V3, V4, V5.
//short CmcCode::mcCode127A();	//	QS pattern in all of leads V1, V2, and V3. (Do not code in the presence of 7-1-1.)
//short CmcCode::mcCode131A();	//	Q/R amplitude ratio ≥ 1/5 and < 1/3, plus Q duration ≥ 20 ms and < 30 ms in any of leads V2, V3, V4, V5.
//short CmcCode::mcCode132A();	//	QS pattern in lead V1 and V2. (Do not code in the presence of 3-1 or 7-1-1.)
//short CmcCode::mcCode138A();	//  Initial R amplitude decreasing to 200 µV or less in every beat (and absence of codes 3-2, 7-1-1, 7-2-1, or 7-3) between any of leads V2 and V3, V3, and V4, or V4 and V5. (All beats in the lead immediately to the right on the chest must have an initial R >200 µV.)
//QRS Axis Deviation(2-code)
//Do not code in presence of low-voltage QRS code 9-1, WPW 6-4-1, artiﬁcial pacemaker code 6-8, ventricular conduction defects 7-1-1, 7-2-1, 7-4, or 7-8.
void CmcCode::mcCode2()
{
	if(mcCode910()) return;
	short axis=axisQRS();
	if(axis<-90 && axis>-180) axis+=360;
	unsigned short code=0;
	if(axis<=-30 && axis>=-90) code=mccode(2,1,0);	//2010;
	else if(axis>=120 && axis<=210) code=mccode(2,2,0);	//2020;
	else if(axis>90 && axis<120)	code=mccode(2,3,0);	//2030;
	else if(axis>210) code=mccode(2,4,0);	//2040;
	else if(axis==90) code=mccode(2,5,0);	//2050;
	if(code) SetmcCode(code,0);
}
//
//short CmcCode::mcCode210()	//	Left. QRS axis from –30° through –90° in leads I, II and III. (The algebraic sum of major positive and major negative QRS waves must be zero or positive in I, negative in III, and zero or negative in II.)
//{
//	if(axisQRS()>=-90 && axisQRS()<=-30) {
//		unsigned short code=mccode();	//2010;
//		SetmcCode( code,0);
//		return  code;
//	}
//	return 0;
//}
//short CmcCode::mcCode220()	//	Right. QRS axis from +120° through –150° in leads I, II, and III. (The algebraic sum of major positive and major negative QRS waves must be negative in I, and zero or positive in III, and in I must be one-half or more of that in III.)
//{
//	short axis=axisQRS();
//	if(axis<-150) axis+=360;
//	if(axis>=120 && axis<210) {
//		unsigned short code=mccode();	//2020;
//		SetmcCode( code,0);
//		return  code;
//	}
//	return 0;
//}
//short CmcCode::mcCode230()	//	Right (optional code when 2-2 is not present). QRS axis from +90° through +119° in leads I, II, and III. (The algebraic sum of major positive and major negative QRS waves must be zero or negative in I and positive in II and III.)
//{
//	if(axisQRS()>90 && axisQRS()<120) {
//		unsigned short code=mccode();	//2030;
//		SetmcCode( code,0);
//		return  code;
//	}
//	return 0;
//}
//short CmcCode::mcCode240()	//	Extreme axis deviation (usually S1, S2, S3 pattern). QRS axis from –90° through–149° in leads I, II and III. (The algebraic sum of major positive and major negative QRS waves must be negative in each of leads I, II, and III.)
//{
//	if(axisQRS()>=-150 && axisQRS()<-90) {
//		unsigned short code=mccode();	//2040;
//		SetmcCode( code,0);
//		return  code;
//	}
//	return 0;
//}
//short CmcCode::mcCode250()	//	Indeterminate axis. QRS axis approximately 90° from the frontal plane. (The algebraic sum of major positive and major negative QRS waves is zero in each of leads I, II and III, or the information from these three leads is incongruous.)
//{
//	if(axisQRS()==90) {
//		unsigned short code=mccode();	//2050;
//		SetmcCode( code,0);
//		return  code;
//	}
//	return 0;
//}

//High Amplitude R Waves(3-code)
//Do not code in the presence of codes 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8.
void CmcCode::mcCode3()
{
	short code=0,code320=0;
	code=mcCode310();
	code320=mcCode320();
	if(code==0) code=mcCode330();
	else if(code320>0) code=mcCode340();
}
short CmcCode::mcCode310()	//	Left: R amplitude > 2600 µV in either V5  or V6, or R amplitude > 2000 µV in any of leads I, II, III, aVF, or R amplitude > 1200 µV in lead aVL measured only on second to last complete normal beat.
{
	unsigned long  nLeads=0;
	if(uvR(V5) >2600)	nLeads|=m_infoLead[V5].mask;
	if(uvR(V6) >2600)	nLeads|=m_infoLead[V6].mask;
	if(uvR(I)  >2000)	nLeads|=m_infoLead[I].mask;
	if(uvR(II) >2000)	nLeads|=m_infoLead[II].mask;
	if(uvR(III)>2000)	nLeads|=m_infoLead[III].mask;
	if(uvR(aVF)>2000)	nLeads|=m_infoLead[aVF].mask;
	if(uvR(aVL)>1200)	nLeads|=m_infoLead[aVL].mask;
	if(nLeads>0) {
		unsigned short code=mccode(3,1,0);	//3010;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode320()	//	Right: R amplitude ≥ 500 µV and R amplitude ≥ S amplitude in the majority of beats in lead V1, when S amplitude is > R amplitude somewhere to the left on the chest of V1 (codes 7-3 and 3-2, if criteria for both are present).
{
	short i,L[]={V2,V3,V4,V5,V6},n=sizeof(L)/sizeof(short);
	unsigned long  nLeads=0;
	if(uvR(V1)>500 && uvR(V1)>abs(uvS(V1)))	nLeads|=m_infoLead[V1].mask;
	if(nLeads>0) {
		for(i=0;i<n;i++) {
			if(abs(uvS(L[i]))>uvR(L[i])) {
				nLeads|=m_infoLead[L[i]].mask;	break;
			}
		}
		if(nLeads>m_infoLead[V1].mask) {
			unsigned short code=mccode(3,2,0);	//3020;
			SetmcCode( code,nLeads);
			return  code;
		}
	}
	return 0;
}
short CmcCode::mcCode330()	//	Left (optional code when 3-1 is not present): R amplitude > 1500 µV but ≤ 2000 µV in lead I, or R amplitude in V5 or V6, plus S amplitude in V1 > 3500 µV.
{
	unsigned long  nLeads=0;
	if(uvR(I)>1500 && uvR(I)<=2000)	nLeads|=m_infoLead[I].mask;
	if(uvR(V5)+abs(uvS(V1))>3500)	nLeads|=(m_infoLead[V1].mask|m_infoLead[V5].mask);
	else if(uvR(V6)+abs(uvS(V2))>3500)	nLeads|=(m_infoLead[V2].mask|m_infoLead[V6].mask);
	if(nLeads>0) {
		unsigned short code=mccode(3,3,0);	//3030;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode340()	//	Criteria for 3-1 and 3-2 both present.
{
	unsigned short code310=mccode(3,1,0),code320=mccode(3,2,0);
	CodeList *p310=mc.found(code310);
	CodeList *p320=mc.found(code320);
	if(p310 && p320) {
		unsigned long  nLeads=(p310->code.nLeads|p320->code.nLeads);
		unsigned short code=mccode(3,4,0);	//3040;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}

//ST Junction (J) and Segment Depression(4-code)
//Do not code in the presence of codes 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8. When 4-1, 4-2, or 4-3 is coded, then a 5-code most often must also be assigned except in lead V1.
void CmcCode::mcCode4()
{
	mcCode411L();
	mcCode411I();
	mcCode411A();
	mcCode412L();
	mcCode412I();
	mcCode412A();
	mcCode420L();
	mcCode420I();
	mcCode420A();
	mcCode430L();
	mcCode430I();
	mcCode430A();
	mcCode440L();
	mcCode440I();
	mcCode440A();
}
	
inline short CmcCode::mcCode411(short *L,short n)	//LIA
{
	unsigned long nLeads=0;
	for(short i=0;i<n;i++) {
		if(isDownwardST(L[i]) || isHorizontalST(L[i])) {
			if(uvST40(L[i])<=-200) nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(4,1,1);	//4011;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
inline short CmcCode::mcCode412(short *L,short n)	//LIA
{
	unsigned long nLeads=0;
	for(short i=0;i<n;i++) {
		if(isDownwardST(L[i]) || isHorizontalST(L[i])) {
			if(uvST40(L[i])>-200 && uvST40(L[i])<=-100) nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(4,1,2);	//4012;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
inline short CmcCode::mcCode420(short *L,short n)	//LIA
{
	unsigned long nLeads=0;
	for(short i=0;i<n;i++) {
		if(isDownwardST(L[i]) || isHorizontalST(L[i])) {
			if(uvST40(L[i])>-100 && uvST40(L[i])<=-50) nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(4,2,0);	//4020;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode430(short *L,short n)	//LIA
{
	unsigned long nLeads=0;
	for(short i=0;i<n;i++) {
		if(isDownwardST(L[i]) && uvSTe(L[i])<=-50) {
			if(uvST40(L[i])>-50 && uvST40(L[i])<50) nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(4,3,0);	//4030;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode440(short *L,short n)	//LIA
{
	unsigned long nLeads=0;
	for(short i=0;i<n;i++) {
		if(uvST40(L[i])>-50) {
			if(isUtypeST(L[i])) nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(4,4,0);	//4040;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
////Anterolateral Site (leads I, aVL, V6)
short CmcCode::mcCode411L()	//	STJ depression ≥ 200 µV and ST segment horizontal or downward sloping in any of leads I, aVL, or V6.
{
	short L[]={I,aVL,V6},n=sizeof(L)/sizeof(short);
	return mcCode411(L,n);
}
short CmcCode::mcCode412L()	//	STJ depression ≥ 100 µV but < 200 µV, and ST segment horizontal or downward sloping in any of leads I, aVL, or V6.
{
	short L[]={I,aVL,V6},n=sizeof(L)/sizeof(short);
	return mcCode412(L,n);
}
short CmcCode::mcCode420L()	//	STJ depression ≥ 50 µV but < 100 µV and ST segment horizontal or downward sloping in any of leads I, aVL, or V6.
{
	short L[]={I,aVL,V6},n=sizeof(L)/sizeof(short);
	return mcCode420(L,n);
}
short CmcCode::mcCode430L()	//	No STJ depression as much as 50 µV, but ST segment downward sloping and segment or T-wave nadir ≥ 50 µV below P-R baseline, in any of leads I, aVL, or V6.
{
	short L[]={I,aVL,V6},n=sizeof(L)/sizeof(short);
	return mcCode430(L,n);
}
short CmcCode::mcCode440L()	//	STJ depression ≥ 100 µV and ST segment upward sloping or U-shaped, in any of leads I, aVL, or V6.
{
	short L[]={I,aVL,V6},n=sizeof(L)/sizeof(short);
	return mcCode440(L,n);
}
////Posterior (inferior) Site (leads II, III, aVF)
short CmcCode::mcCode411I()	//	STJ depression ≥ 200 µV and ST segment horizontal or downward sloping in lead II or aVF.
{
	short L[]={II,aVF},n=sizeof(L)/sizeof(short);
	return mcCode411(L,n);
}
short CmcCode::mcCode412I()	//	STJ depression ≥ 100 µV but < 200 µV and ST segment horizontal or downward sloping in lead II or aVF.
{
	short L[]={II,aVF},n=sizeof(L)/sizeof(short);
	return mcCode412(L,n);
}
short CmcCode::mcCode420I()	//	STJ depression ≥ 50 µV but < 100 µV and ST segment horizontal or downward sloping in lead II or aVF.
{
	short L[]={II,aVF},n=sizeof(L)/sizeof(short);
	return mcCode420(L,n);
}
short CmcCode::mcCode430I()	//	No STJ depression as much as 50 µV, but ST segment downward sloping and segment or T-wave nadir ≥ 50 µV below P-R baseline in lead II.
{
	short L[]={II,aVF},n=sizeof(L)/sizeof(short);
	return mcCode430(L,n);
}
short CmcCode::mcCode440I()	//	STJ depression ≥ 100 µV and ST segment upward sloping, or U-shaped, in lead II.
{
	short L[]={II,aVF},n=sizeof(L)/sizeof(short);
	return mcCode440(L,n);
}
////Anterior Site (leads V1, V2, V3, V4, V5)
short CmcCode::mcCode411A()	//	STJ depression ≥ 200 µV and ST segment horizontal or downward sloping in any of leads V1, V2, V3, V4, V5.
{
	short L[]={V1,V2,V3,V4,V5},n=sizeof(L)/sizeof(short);
	return mcCode411(L,n);
}
short CmcCode::mcCode412A()	//	STJ depression ≥ 100 µV but < 200 µV and ST segment horizontal or downward sloping in any of leads V1, V2, V3, V4, V5.
{
	short L[]={V1,V2,V3,V4,V5},n=sizeof(L)/sizeof(short);
	return mcCode412(L,n);
}
short CmcCode::mcCode420A()	//	STJ depression ≥ 50 µV but < 100 µV and ST segment horizontal or downward sloping in any of leads V1, V2, V3, V4, V5.
{
	short L[]={V1,V2,V3,V4,V5},n=sizeof(L)/sizeof(short);
	return mcCode420(L,n);
}
short CmcCode::mcCode430A()	//	No STJ depression as much as 50 µV, but ST segment downward sloping and segment or T-wave nadir ≥ 50 µV below P-R baseline in any of leads V2, V3, V4, V5.
{
	short L[]={V1,V2,V3,V4,V5},n=sizeof(L)/sizeof(short);
	return mcCode430(L,n);
}
short CmcCode::mcCode440A()	//	STJ depression ≥ 100 µV and ST segment upward sloping or U-shaped in any of leads V1, V2, V3, V4, V5.
{
	short L[]={V1,V2,V3,V4,V5},n=sizeof(L)/sizeof(short);
	return mcCode440(L,n);
}

//T-Wave Items(5-code)
//Do not code in the presence of codes 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8.
void CmcCode::mcCode5()
{
	mcCode510L();
	mcCode510I();
	mcCode510A();
	mcCode520L();
	mcCode520I();
	mcCode520A();
	mcCode530L();
	mcCode530I();
	mcCode530A();
	mcCode540L();
	mcCode540I();
	mcCode540A();
}
short CmcCode::mcCode530(short *L,short n)
{
	unsigned long nLeads=0;
	for(short i=0;i<n;i++) {
		if(isFlatT(L[i]) || uvT1(L[i])<=-100) {
			nLeads|=m_infoLead[L[i]].mask; 
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(5,3,0);	//5030;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode540(short *L,short n)
{
	unsigned long nLeads=0;
	for(short i=0;i<n;i++) {
		short Ta=uvT(L[i]),Ra=uvR(L[i]);
		if(Ta>0 && Ra>=1000 && 20*Ta<Ra) nLeads|=m_infoLead[L[i]].mask;
	}
	if(nLeads>0) {
		unsigned short code=mccode(5,4,0);	//5040;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
//Anterolateral Site (leads I, aVL, V6)
short CmcCode::mcCode510L()	//	T amplitude negative 500 µV or more in either of leads I, V6, or in lead aVL when R amplitude is ≥ 500 µV.
{
	short i,L[]={I,V6},n=sizeof(L)/sizeof(short);
	unsigned long nLeads=0;
	for(i=0;i<n;i++) {
		if(uvNegaT(L[i])<=-500) nLeads|=m_infoLead[L[i]].mask; 
	}
	if(uvNegaT(aVL)<=-500 && uvR(aVL)>=500) nLeads|=m_infoLead[aVL].mask; 
	if(nLeads>0) {
		unsigned short code=mccode(5,1,0);	//5010;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode520L()	//	T amplitude negative or diphasic (positive–negative or negative–positive type) with negative phase at least 100 µV but not as deep as 500 µV in lead I or V6, or in lead aVL when R amplitude is ≥ 500 µV.
{
	short i,L[]={I,V6},n=sizeof(L)/sizeof(short);
	unsigned long nLeads=0;
	for(i=0;i<n;i++) {
		if(uvT1(L[i])>-500 && uvT1(L[i])<=-100) {
			nLeads|=m_infoLead[L[i]].mask; 
		}
	}
	if(uvT1(aVL)>-500 && uvT1(aVL)<=-100 && uvR(aVL)>=500) {
		nLeads|=m_infoLead[aVL].mask; 
	}
	if(nLeads>0) {
		unsigned short code=mccode(5,2,0);	//5020;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode530L()	//	T amplitude zero (ﬂat), or negative, or diphasic (negative–positive type only) with less than 100 µV negative phase in lead I or V6, or in lead aVL when R amplitude is ≥ 500 µV.
{
	short L[]={I,V6},n=sizeof(L)/sizeof(short);
	short code0=mcCode530(L,n);
	unsigned short code=mccode(5,3,0);	//5030;
	unsigned long nLeads=0;
	if((isFlatT(aVL) || uvT1(aVL)<=-100) && uvR(aVL)>=500) {
		nLeads|=m_infoLead[aVL].mask; 
		if(nLeads) {
			SetmcCode( code,nLeads);
		}
	}
	if(code0>0 || nLeads>0) return  code;
	return 0;
}
short CmcCode::mcCode540L()	//	T amplitude positive and T/R amplitude ratio < 1/20 in any of leads I, aVL, V6; R wave amplitude must be ≥ 1000 µV.
{
	short L[]={I,aVL,V6},n=sizeof(L)/sizeof(short);
	return mcCode540(L,n);
}
//Posterior (Inferior) Site (leads II, III, aVF)
short CmcCode::mcCode510I()	//	T amplitude negative 500 µV or more in lead II, or in lead aVF when QRS is mainly upright.
{
	short i,L[]={II},n=sizeof(L)/sizeof(short);
	unsigned long nLeads=0;
	for(i=0;i<n;i++) {
		if(uvNegaT(L[i])<=-500) nLeads|=m_infoLead[L[i]].mask; 
	}
	if(uvNegaT(aVF)<=-500 && uvQRSnet(aVF)>0) nLeads|=m_infoLead[aVF].mask; 
	if(nLeads>0) {
		unsigned short code=mccode(5,1,0);	//5010;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode520I()	//	T amplitude negative or diphasic with negative phase (negative–positive or positive– negative type) at least 100 µV but not as deep as 500 µV in lead II, or in lead aVF when QRS is mainly upright.
{
	short i,L[]={II},n=sizeof(L)/sizeof(short);
	unsigned long nLeads=0;
	for(i=0;i<n;i++) {
		if(uvNegaT(L[i])>-500 && uvNegaT(L[i])<=-100) {
			nLeads|=m_infoLead[L[i]].mask; 
		}
	}
	if(uvNegaT(aVF)>-500 && uvNegaT(aVF)<=-100 && uvQRSnet(aVF)>0) {
		nLeads|=m_infoLead[aVF].mask; 
	}
	if(nLeads>0) {
		unsigned short code=mccode(5,2,0);	//5020;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode530I()	//	T amplitude zero (ﬂat), or negative, or diphasic (negative-positive type only) with less than 100 µV negative phase in lead II; not coded in lead aVF.
{
	short L[]={II},n=sizeof(L)/sizeof(short);
	return mcCode530(L,n);
}
short CmcCode::mcCode540I()	//	T amplitude positive and T/R amplitude ratio < 1/20 in lead II; R wave amplitude must be ≥ 1000 µV.
{
	short L[]={II},n=sizeof(L)/sizeof(short);
	return mcCode540(L,n);
}
//Anterior Site (leads V2, V3, V4, V5)
short CmcCode::mcCode510A()	//	T amplitude negative 500 µV or more in any of leads V2, V3, V4, V5.
{
	short i,L[]={V2,V3,V4,V5},n=sizeof(L)/sizeof(short);
	unsigned long nLeads=0;
	for(i=0;i<n;i++) {
		if(uvNegaT(L[i])<=-500) nLeads|=m_infoLead[L[i]].mask;
	}
	if(nLeads>0) {
		unsigned short code=mccode(5,1,0);	//5010;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode520A()	//	T amplitude negative, or diphasic (negative–positive or positive–negative type) with negative phase at least 100 µV but not as deep as 500 µV, in any of leads V2, V3, V4, V5.
{
	short i,L[]={V2,V3,V4,V5},n=sizeof(L)/sizeof(short);
	unsigned long nLeads=0;
	for(i=0;i<n;i++) {
		if(uvNegaT(L[i])>-500 && uvNegaT(L[i])<=-100) {
			nLeads|=m_infoLead[L[i]].mask; 
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(5,2,0);	//5020;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode530A()	//	T amplitude zero (ﬂat), or negative, or diphasic (negative–positive type only) with less than 100 µV negative phase, in any of leads V3, V4, V5.
{
	short L[]={V3,V4,V5},n=sizeof(L)/sizeof(short);
	return mcCode530(L,n);
}
short CmcCode::mcCode540A()	//	T amplitude positive and T/R amplitude ratio < 1/20 in any of leads V3, V4, V5; R wave amplitude must be ≥ 1000 µV.
{
	short L[]={V3,V4,V5},n=sizeof(L)/sizeof(short);
	return mcCode540(L,n);
}

//A-V Conduction Defect(6-code)
void CmcCode::mcCode6()
{
	mcCode62x();
	//mcCode622();
//	mcCode610();
	mcCode630();
//	short code641=mcCode641();
	short code642=mcCode642();
	short code650=mcCode650();
	if(code650==0 && code642==0) mcCode660();
//	mcCode680();
}
short CmcCode::mcCode610()	//	Complete (third degree) A-V block (permanent or intermittent) in any lead. Atrial and ventricular complexes independent, and atrial rate faster than ventricular rate, with ventricular rate < 60.
{
	unsigned short code=0;
	if(GetVrate()<60) {
		if(GetArate()>18*GetVrate()/10) {
			 code=mccode(6,2,2);	//6010;
		}
		else if(GetArate()>7*GetVrate()/5) code=mccode(6,1,0);	//6010;
	}
	if(code>0) SetmcCode( code,0);
	return  code;
}
short CmcCode::mcCode62x()	//	Mobitz Type II (occurrence of P-wave on time with dropped QRS and T).
{
	unsigned short code=0;
	short j621=0,j622=0,j623=0;
	if(GetVrate()<90 && GetArate()>=5*GetVrate()/4) {
		j622=1;
		short i,j=0,beatsnum=GetBeatsNum();
		VH_ECGbeat *beats=GetBeats();
		long hr0=0,hr;
		float hrratio,prratio[2];
		hr0=beats[2].Pos-beats[1].Pos;
		for(i=3;i<beatsnum;i++) {
			hr=beats[i].Pos-beats[i-1].Pos;
			hrratio=(float)hr/hr0;
			if(hrratio>=1.4 && hrratio<=2.1) {
				prratio[0]=(beats[i-1].PR>0)?float(beats[i-2].PR)/beats[i-1].PR:1;
				prratio[1]=(beats[i-2].PR>0)?float(beats[i-1].PR)/beats[i-2].PR:1;
				if(prratio[0]<0.9 && prratio[1]>0.95) j623++;
				else if(prratio[0]>=0.9 && prratio[0]<=1.1 && prratio[1]>=0.9 && prratio[1]<=1.1) j621++;
			}
			hr0=hr;
		}
	}
	if(j623 > j621) code=mccode(6,2,3);
	else if(j621>0) code=mccode(6,2,1);
	else if(j622>0) code=mccode(6,2,2);
	if(code) {
		SetmcCode( code,0);
		return  code;
	}

	return 0;
}
//short CmcCode::mcCode621()	//	Mobitz Type II (occurrence of P-wave on time with dropped QRS and T).
//short CmcCode::mcCode622()	//	Partial (second degree) A-V block in any lead (2:1 or 3:1 block).
//{
//	if(GetVrate()<80 && GetArate()>=5*GetVrate()/4) {
//		unsigned short code=mccode(6,2,2);	//6022;
//		SetmcCode( code,0);
//		return  code;
//	}
//	return 0;
//}
//short CmcCode::mcCode623();	//	Wenckebach’s Phenomenon (P-R interval increasing from beat to beat until QRS and T dropped).
short CmcCode::mcCode630()	//	P-R (P-Q) interval ≥ 220 ms in the majority of beats in any of leads I, II, III, aVL, aVF.
{
	short L[]={I, II, III, aVL, aVF},n=sizeof(L)/sizeof(short);
	//for(i=j=0;i<n;i++) {
	//	if(msPR(L[i])>=220) j++;
	//}
	if(msPRjudge(L,n,220)>1) {
		unsigned short code=mccode(6,3,0);	//6030;
		SetmcCode( code,0);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode641()	//	Wolff-Parkinson-White Pattern (WPW), persistent. Sinus P-wave. P-R interval < 120 ms, plus QRS duration ≥ 120 ms, plus R peak duration ≥ 60 ms, coexisting in the same beat and present in the majority of beats in any of leads I, II, aVL, V4, V5, V6. (6-4-1 suppresses 1-2-3, 1-2-7, 1-3-2, 1-3-6, 1-3-8, all 3, 4, 5, 7, 9-2, 9-4, 9-5 codes.)
{
	unsigned short nLeads=0;
	char wpw=GetWPW();
	unsigned short code=mccode(6,4,1);	//6041;
	if(wpw=='A' || wpw=='a' || wpw=='B' || wpw=='b' || wpw=='W' || wpw=='w') {
		short i,j,L[]={I, II, aVL, V4, V5, V6},n=sizeof(L)/sizeof(short);
		for(i=j=0;i<n;i++) {
			if(msPR(L[i])>0 && msPR(L[i])<120 && msQRS(L[i])>=120 && msRpeak(L[i])>=60) {
				nLeads|=m_infoLead[L[i]].mask;
				j++;
			}
		}
		if(j) {
			SetmcCode( code,0);
			return  code;
		}
		return  code;
	}
	//else {
	//	short i,j,L[]={I, II, aVL, V4, V5, V6},n=sizeof(L)/sizeof(short);
	//	for(i=j=0;i<n;i++) {
	//		if(msPR(L[i])<120 && msQRS(L[i])>=120 && msRpeak(L[i])>=60) {
	//			nLeads|=m_infoLead[L[i]].mask;
	//			j++;
	//		}
	//	}
	//	if(j>2) {
	//		SetmcCode( code,0);
	//		return  code;
	//	}
	//}
	return 0;
}
short CmcCode::mcCode642()	//	WPW Pattern, intermittent. WPW pattern in ≤ 50% of beats in appropriate leads.
{//cannot be detected
	return 0;
}
short CmcCode::mcCode650()	//	Short P-R interval. P-R interval < 120 ms in all beats of any two of leads I, II, III, aVL, aVF.
{
	short i,j,L[]={I, II, III, aVL, aVF},n=sizeof(L)/sizeof(short);
	for(i=j=0;i<n;i++) {
		if(msPR(L[i])>0 && msPR(L[i])<120) j++;
	}
	if(j>2) {
		unsigned short code=mccode(6,5,0);	//6050;
		SetmcCode( code,0);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode660()	//	Intermittent aberrant atrioventricular conduction. P-R > 120 ms (except in presence of 6-5 or heart rate greater than 100), and wide QRS complex > 120 ms, and normal P-wave when most beats are sinus rhythm. (Do not code in the presence of 6-4-2.)
{
	if(bpmHR()>=100) return 0;
	if(msPR()>120 && msQRS()>120) {
		//short i,j,beatsnum=GetBeatsNum();
		//VH_ECGbeats *beats=GetBeats();
		//for(i=0;i<beatsnum;i++) {
		//}
		if(isSinalP()) {
			unsigned short code=mccode(6,6,0);	//6060;
			SetmcCode( code,0);
			return  code;
		}
	}
	return 0;
}
short CmcCode::mcCode680()	//	Electronic pacemaker.
{
	short SpikesN;
	char PaceMaker;
	long *SpikesPos=GetPaceMaker(PaceMaker,SpikesN); //'N': none, 'A': A-Type, 'V': V-type, 'B': Both
	if(PaceMaker!='N' && SpikesN>0 && SpikesPos) {
		unsigned short code=mccode(6,8,0);	//6080;
		SetmcCode( code,0);
		return  code;
	}
	return 0;
}

//Ventricular Conduction Defect(7-code)
void CmcCode::mcCode7()
{
	mcCode711_60();
	mcCode712();
	mcCode72x();
	mcCode740();
	mcCode750();
	mcCode770();
	mcCode780();
	mcCode791_2_3();
	mcCode7100();
}
short CmcCode::mcCode711_60()	//	Complete left bundle branch block (LBBB). (Do not code in presence of 6-1, 6-4-1,6-8, 8-2-1 or 8-2-2.) QRS duration ≥ 120 ms in a majority of beats (of the same QRS pattern) in any of leads I, II, III, aVL, aVF, plus R peak duration ≥ 60 ms in a majority of beats (of the same QRS pattern) in any of leads I, II, aVL, V5, V6. (7-1-1 suppresses 1-2-3, 1-2-7, 1-3-2, 1-3-6, 1-3-7, 1-3-8, all 2, 3, 4, 5, 9-2, 9-4, 9-5 codes. If any other codable Q-wave coexists with the LBBB pattern, code the Q and diminish the 7-1-1 code to a 7-4 code.)
{
	short i,j,L[]={I,II,aVL,V5,V6},n=sizeof(L)/sizeof(short);
	unsigned long nLeads=0;
	for(i=j=0;i<n;i++) {
		if((msR(L[i])>60 || msRpeak(L[i])>=51) && noQ(L[i])/* && uvST40(L[i])<=0 && (uvNegaT(L[i])<0 || isFlatT(L[i]))*/) {
			nLeads|=m_infoLead[L[i]].mask;	j++;
		}
	}
	if(j>1 && noQ(V5) && noQ(V6)) {
		unsigned short code=0;
		short L[]={I, II, III, aVL, aVF},n=sizeof(L)/sizeof(short);
		if(msQRSjudge(L,n,120)>0) code=mccode(7,1,1);
		else if(msQRSjudge(L,n,110)>0) code=mccode(7,6,0);	//code760;
		else return 0;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode712()	//	Intermittent left bundle branch block. Same as 7-1-1 but with presence of normally conducted QRS complexes of different shape than the LBBB pattern.
{//cannot be detected
	return 0;
}
short CmcCode::mcCode72x()	//	Complete right bundle branch block (RBBB). (Do not code in the presence of 6-1,6-4-1, 6-8, 8-2-1 or 8-2-2.) QRS duration ≥ 120 ms in a majority of beats (of the same QRS pattern) in any of leads I, II, III, aVL, aVF, plus: R΄ > R in V1; or QRS mainly upright, plus R peak duration ≥ 60 ms in V1 or V2; or S duration > R duration in all beats in lead I or II. (Suppresses 1-3-8, all 2-, 3-, 4-and 5-codes, 9-2, 9-4, 9-5.)
{
	short i,j,L[]={I,II,V1,V2},n=sizeof(L)/sizeof(short);
	unsigned long nLeads=0;
	bool c1=false,c2=false;//,c3=false;
	if(uvR2(V1)>uvR1(V1) && uvR1(V1)>0) {
		c1=true;
		nLeads|=m_infoLead[V1].mask;
	}
	for(i=2;i<4;i++) {
		if(uvQRSnet(L[i])>=0 && msR(L[i])>60 && msRpeak(L[i])>=60) {
			c2=true;
			nLeads|=m_infoLead[L[i]].mask;
		}
	}
	for(i=j=0;i<2;i++) {
		if(msS(L[i])>msR(L[i]) && uvS(L[i])<-100) {
			j++;
			nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(msS(V6)>msR(V6) && uvS(V6)<-100) j++;
	if(c1 || c2 || j>1) {
		unsigned short code=0;
		short L[]={I, II, III, aVL, aVF},n=sizeof(L)/sizeof(short);
		if(msQRSjudge(L,n,120)>1) code=mccode(7,2,1);	//code721;
		else return mcCode730();	//code720;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
//short CmcCode::mcCode722()	//	Intermittent right bundle branch block. Same as 7-2-1 but with presence of normally conducted QRS complexes of different shape than the RBBB pattern.
//{//cannot be detected
//	return 0;
//}
short CmcCode::mcCode730()	//	Incomplete right bundle branch block. QRS duration < 120 ms in each of leads I, II, III, aVL, aVF, and R΄ > R in either of leads V1, V2 (Code as 3-2 in addition if those criteria are met. 7-3 suppresses code 1-3-8.)
{
//	short L[]={I,II,III,aVL,aVF,V1,V2},n=sizeof(L)/sizeof(short);
	unsigned long nLeads=0;
	if(uvR2(V1)>uvR1(V1) && uvR1(V1)>0)	nLeads|=m_infoLead[V1].mask;
	if(uvR2(V2)>uvR1(V2) && uvR1(V2)>0)	nLeads|=m_infoLead[V2].mask;
	if(nLeads>0) {
		unsigned short code=mccode(7,3,0);
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}

short CmcCode::mcCode740()	//	Intraventricular block. QRS duration ≥ 120 ms in a majority of beats in any of leads I, II, III, aVL, aVF. (7-4 suppresses all 2, 3, 4, 5, 9-2, 9-4, 9-5 codes.)
{
	unsigned short code=mccode(7,4,0);	//7040;
	short L[]={I, II, III, aVL, aVF},n=sizeof(L)/sizeof(short);
	if(msQRSjudge(L,n,120)>1) {
		SetmcCode( code,0);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode750()	//	R-R΄ pattern in either of leads V1, V2 with R΄ amplitude ≤ R.
{
	short i,L[]={V1,V2},n=sizeof(L)/sizeof(short);
	unsigned long nLeads=0;
	for(i=0;i<n;i++) {
		if(uvR2(L[i])>=25 && uvR2(L[i])<=uvR1(L[i])) nLeads|=m_infoLead[L[i]].mask;
	}
	if(nLeads>0) {
		unsigned short code=mccode(7,5,0);	//7050;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
//short CmcCode::mcCode760();	//	Incomplete left bundle branch block. (Do not code in the presence of any codable Q- or QS-wave.) QRS duration ≥ 100 ms and < 120 ms in the majority of beats of each of leads I, aVL, and V5 or V6.
short CmcCode::mcCode770()	//	Left anterior hemiblock (LAH). QRS duration < 120 ms in the majority of beats in leads I, II,III, aVL, aVF, plus Q-wave amplitude ≥ 250 µV and < 30 ms duration in lead I or aVL, plus left axis deviation of –45° or more negative. (In presence of 7-2, code 7-8 if axis is < – 45° and the Q-wave in lead I meets the above criteria.) 
{
	unsigned long nLeads=0;
	short i,L[]={I, II, III, aVL, aVF},n=sizeof(L)/sizeof(short);
	if(msQRSjudge(L,n,120)<2 && axisQRS()<=-45) {
		short L[]={I,aVL},n=sizeof(L)/sizeof(short);
		for(i=0;i<n;i++) {
			if(uvQd(L[i])<=-250 /*&& msQd(L[i])<30*/) nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(nLeads>0) {
		unsigned short code=mccode(7,7,0);	//7070;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode780()	//	Combination of 7-7 and 7-2.
{
	unsigned short code=mccode(7,8,0),code721=mccode(7,2,1),code722=mccode(7,2,2),code770=mccode(7,7,0);
	unsigned long nLeads=0;
	CodeList *p721=mc.found(code721);	if(p721) nLeads|=p721->code.nLeads;
	CodeList *p722=mc.found(code722);	if(p722) nLeads|=p722->code.nLeads;
	CodeList *p770=mc.found(code770);	if(p770) nLeads|=p770->code.nLeads;
	if(p721 && p722 && p770) {
		SetmcCode( code,nLeads);
		mc.remove(code721);	mc.remove(code722);	mc.remove(code770);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode791_2_3()
{
	unsigned short code=0,code791=mccode(7,9,1),code792=mccode(7,9,2),code793=mccode(7,9,3);
	unsigned long nLeads = 0x0000,mask;

	short brugada1 = 0, brugada2 = 0, brugada3 = 0;
	for (short i = V1; i<V3; i++) {
		mask=m_infoLead[i].mask;
		if (uvSTj(i) >= 200) {
			if (isNegativeT(i)) { brugada1++; nLeads |= mask; }
			else if (isDualT(i)) { brugada2++; nLeads |= mask; }
			else if (isPositiveT(i)) {
				if (uvST60(i)<uvSTj(i)) {
					if (uvST60(i)<max(uvT1(i), uvT2(i))) {
						if (uvST60(i) >= 100) {
							if (brugada1>0) { brugada1++; nLeads |= mask; }
							brugada2++; {nLeads |= mask; }
						}
						else if (uvST60(i)>0) { brugada3++; nLeads |= mask; }
					}
					else if (max(uvT1(i), uvT2(i))<50) { brugada1++; nLeads |= mask; }
				}
			}
		}
	}
	if (brugada1>1) code = code791;
	else if (brugada2>1) code = code792;
	else if (brugada3>1) code = code793;

	if ( code>0) {
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
//short CmcCode::mcCode791();	//  Type 1 Brugada pattern convex (coved) ST segment elevation ≥ 200 µV plus T-wave negative with little or no isolelectric (baseline) separation in at least 2 leads of V1-V3.
//short CmcCode::mcCode792();	//  Type 2 Brugada pattern ST segment elevation ≥ 200 µV plus T-wave positive or diphasic that  results in a “saddle-back” shape in at least 2 leads of V1-V3.
//short CmcCode::mcCode793();	//  Type 3 Brugada pattern. 7-2-1 plus ST segment elevation ≤ 100 µV plus a “saddle-back”conﬁguration in at least 2 leads of V1–V3.
short CmcCode::mcCode7100()	//	Fragmented QRS.
{
	short i,j,j1,k,L,z,pe,qb,se,step=short(5.0*m_fs/1000+0.5);	if(step<1) step=1;
	short uv200=short(200/m_uVpb+0.5),uv100=uv200/2,posi=0,nega=0;
	unsigned long nLeads = 0x0000,mask;
	for(i=k=0;i<m_chnum;i++) {
		L=LeadIndexFromChn(i);
		mask=m_infoLead[L].mask;
		pe=Pe(L);	qb=Qb(L);	se=Se(L);
		if(qb>pe) {
			posi=nega=0;
			if(pe>0) z=(pe+qb)/2;	else z=qb;
			if(se>qb) {
				for(j=qb+step+step;j<se-step-step;j+=step) {
					if(m_templ[i][j]-m_templ[i][j-step]>uv200 && m_templ[i][j]-m_templ[i][j+step]>uv200) {
						posi++;
						for(j1=0;j1<step;j1++) {
							if(m_templ[i][j-j1]<m_templ[i][j-j1-1] || m_templ[i][j+j1]<m_templ[i][j+j1+1]) {
								posi--;	break;
							}
						}
					}
					if(m_templ[i][j-step]-m_templ[i][j]>uv200 && m_templ[i][j+step]-m_templ[i][j>uv200]) {
						nega++;
						for(j1=0;j1<step;j1++) {
							if(m_templ[i][j-j1-1]<m_templ[i][j-j1] || m_templ[i][j+j1+1]<m_templ[i][j+j1]) {
								posi--;	break;
							}
						}
					}
				}
				if(posi>0 && uvR1(L)>uv100) posi--;
				if(posi>0 && uvR2(L)>uv100) posi--;
				if(isQS(L)) {
					if(nega>0) nega--;
				}
				else {
					if(nega>0 && uvQ(L)<-uv100) nega--;
					if(nega>0 && uvS(L)<-uv100) nega--;
				}
				if(posi>0 && nega>0) nLeads|=mask;
			}
		}
	}
	if (nLeads>0) {
		unsigned short code=mccode(7,10,0);	//7100;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
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
void CmcCode::mcCode8()
{
	short arr=0;
	if(arr==0) arr=mcCode81x();
	if(arr==0) arr=mcCode814();
	if(arr==0) arr=mcCode815();
//	if(arr==0) arr=mcCode821();
//	if(arr==0) arr=mcCode822();
	if(arr==0) arr=mcCode823_42();
	if(arr==0) arr=mcCode824();
	if(arr==0) arr=mcCode831_2();
	if(arr==0) arr=mcCode833();
	if(arr==0) arr=mcCode834();
//	if(arr==0) arr=mcCode841();
	if(arr==0) arr=mcCode85x();
	if(arr==0) arr=mcCode86x();
	if(arr==0) arr=mcCode870_80();
	if(arr==0) {
		if(isSinalP()) {
			if(bpmHR()>50 && bpmHR()<100) return;
		}
		if(!mc.found(mccode(8,4,1))) arr=mcCode890();
	}
}
short CmcCode::mcCode81x()	//811,812,813//,814,815
{
	unsigned short code=0;
	int i,s=0,v=0;
	char *p=GetBeatsType();
	short beatsnum=GetBeatsNum();
	for(i=0;i<beatsnum;i++) {
		if(p[i]=='S') s++;
		else if(p[i]=='V') v++;
	}
	if(s>0 && v>0) code=mccode(8,1,3);
	else if(s>0) code=mccode(8,1,1);
	else if(v>0) code=mccode(8,1,2);
	else return 0;
	SetmcCode( code,0);
	return  code;
}
//short CmcCode::mcCode811();	//	Presence of any atrial or junctional premature beats.
//short CmcCode::mcCode812();	//	Presence of any ventricular premature beats.
//short CmcCode::mcCode813();	//	Presence of both atrial and/or junctional premature beats and ventricular premature beats.
short CmcCode::mcCode814()	//	Wandering atrial pacemaker.
{//undetected
	return 0;
}
short CmcCode::mcCode815()	//	Presence of 8-1-2 and 8-1-4.
{
	unsigned short code812=mccode(8,1,2),code814=mccode(8,1,4);
	if(mc.found(code812) && mc.found(code814)) {
		unsigned short code=mccode(8,1,5);	//8015;
		SetmcCode( code,0);
		mc.remove(code812);	mc.remove(code814);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode821()	//	Ventricular ﬁbrillation or ventricular asystole.
{
	unsigned short code=0;
	short beatsnum=GetBeatsNum();
	if((GetVrate()<25 && GetArate()<30) || beatsnum<4 || 10.*beatsnum/m_seconds<4) {
		code=mccode(8,2,1);
	}
	else {
		int i,n,p,p0=0,ms=m_fs*3;
		VH_ECGbeat *beat=GetBeats();
		for(i=n=0;i<beatsnum;i++) {
			p=beat[i].Pos;	if(p-p0>=ms) n++;
			p0=p;
		}
		p=m_length;	if(p-p0>=ms) n++;
		if(10.*n/m_seconds>2) code=mccode(8,2,1);
	}
	if(code) SetmcCode( code,0);

	return code;
}
short CmcCode::mcCode822()	//	Persistent ventricular (idioventricular) rhythm.
{
	if(msQRS()>=120) {
		bool ok=false;
		short i,j,pb=Pb(),pe=Pe(),L;
		if(pb>0 && pe>pb) {
			for(i=j=0;i<m_chnum;i++) {
				L=LeadIndexFromChn(i);
				if(abs(uvP(L))>25) j++;
			}
			if(j<1) ok=true;
		}
		else ok=true;
		if(ok) {
			unsigned short code=mccode(8,2,2);	//8022;
			SetmcCode( code,0);
			return  code;
		}
	}
	return 0;
}
short CmcCode::mcCode823_42()	//	Intermittent ventricular tachycardia. Three or more consutive ventricular premature beats occurring at a rate ≥ 100. This includes more persistent ventricular tachycardia.
{
	int i,s=0,v=0;
	double sr=0,vr=0,srmin,vrmin;
	char *p=GetBeatsType();
	VH_ECGbeat *beat=GetBeats();
	short beatsnum=GetBeatsNum();
	bool ok823=false,ok842=false;
	srmin=vrmin=m_fs*2;
	CMean vrm,srm;
	for(i=0;i<beatsnum;i++) {
		if(p[i]=='V') v++;
		else if(p[i]=='S') s++;
		else {
			if(v>0) {
				vr=vrm.mean(beat[i].Pos-beat[i-1].Pos);
				if(vrmin>vr) vrmin=vr;
				if(v>2) ok823=true;
			}
			if(s>0) {
				sr=srm.mean(beat[i].Pos-beat[i-1].Pos);
				if(srmin>sr) srmin=sr;
				if(s>2) ok842=true;
			}
			s=v=0;
		}
	}
	unsigned short code=0;
	if(ok823) {
		vrmin=HRfromSamples((int)vrmin);
		if(vrmin>=100) {
			code=mccode(8,2,3);	//code823;
			SetmcCode( code,0);
		}
	}
	if(ok842) {
		srmin=HRfromSamples((int)srmin);
		if(srmin>=100) {
			code=mccode(8,4,2);	//code842;
			SetmcCode( code,0);
		}
	}
	return  code;
}
short CmcCode::mcCode824()	//	Ventricular parasystole (should not be coded in presence of 8-3-1).
{
	int i,vr=0,vrmin=m_fs*2,vrmax=0;
	char *p=GetBeatsType();
	VH_ECGbeat *beat=GetBeats();
	short beatsnum=GetBeatsNum();
	for(i=1;i<beatsnum;i++) {
		if(p[i-1]=='N' && p[i]=='V') {
			vr=(beat[i].Pos-beat[i-1].Pos);
			if(vrmin>vr) vrmin=vr;
			if(vrmax<vr) vrmax=vr;
		}
	}

	if(msfromSamples(vrmax-vrmin)>=120) {//此处未考虑多形室早，因信息量不够
		unsigned short code=mccode(8,2,4);	//8024;//,code814={8,1,4},code815={8,1,5};
		SetmcCode( code,0);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode831_2()	//	Atrial ﬁbrillation (persistent).
{
	short af=GetAflutAfib();
	unsigned short code=0;
	switch(af) {
		case 1:	code=mccode(8,3,2);	break;//code832;	break;
		case 2:	code=mccode(8,3,1);	break;//code831;	break;
		default:	return 0;
	}
	SetmcCode( code,0);
	return  code;
}
//short CmcCode::mcCode831();	//	Atrial ﬁbrillation (persistent).
//short CmcCode::mcCode832();	//	Atrial ﬂutter (persistent).
short CmcCode::mcCode833()	//	Intermittent atrial ﬁbrillation (code if 3 or more clear-cut, consutive sinus beats are present in any lead).
{//cannot be detected
	return 0;
}
short CmcCode::mcCode834()	//	Intermittent atrial ﬂutter (code if 3 or more clear-cut, consutive sinus beats are present in any lead).
{//cannot be detected
	return 0;
}
short CmcCode::mcCode841()	//	Supraventricular rhythm persistent. QRS duration < 120 ms; and absent P-waves or presence of abnormal P-waves (inverted or ﬂat in II, III and aVF); and regular rhythm.
{
	if(!m_pEcgParm || !m_pEcgLead || !m_pEcgBeat || !m_pEcgInfo) return 0;
	short i,L[]={II,III,aVF},n=sizeof(L)/sizeof(short);
	if(msQRS()<120 && bpmHR()<100) {
		short flat=0,nega=0;
		for(i=0;i<n;i++) {
			if(uvP(L[i])<50) flat++;
			if(uvNegaP(L[i])<0) nega++;
		}
		if(flat>=n && nega>1) {
			unsigned short code=mccode(8,4,1);	//8041;//,code814={8,1,4},code815={8,1,5};
			SetmcCode( code,0);
			return  code;
		}
	}
	return 0;
}
//short CmcCode::mcCode842();	//	Supraventricular tachycardia intermittent. Three consutive atrial or junctional pre- mature beats occurring at a rate ≥ 100.
short CmcCode::mcCode85x()
{
	int i,j=0,k,n=0,j0,j1,j2;
	int rr[3];
	float ratio,ratio0;
	char *p=GetBeatsType();
	VH_ECGbeat *beat=GetBeats();
	unsigned short code=0,code851=mccode(8,5,1),code852=mccode(8,5,2);
	short beatsnum=GetBeatsNum();
	bool find=false;
	for(i=1;i<beatsnum;i++) {
		if(p[i]=='N') {
			rr[j]=beat[i].Pos-beat[i-1].Pos;
			if(n<3) n++;
			if(n>1) {
				j2=j;	j1=j2-1;	j0=j1-1;
				if(j1<0) j1+=3;	if(j0<0) j0+=3;
				ratio=(float)rr[j2]/rr[j1];
				find=false;
				for(k=2;k<4;k++) {
					if(ratio>=k*0.9 && ratio<=k*1.1) {find=true;break;}
				}
				if(find) {
					if(n>2) {
						ratio0=(float)rr[j1]/rr[j0];
						if(ratio0<=0.9) code=code852;
						else code=code851;
					}
					else {
						code=code851;
					}
					break;
				}
			}
			j++;	if(j>=3) j=0;
		}
		else {	j=n=0;	}
	}
	if( code>0) {
		SetmcCode( code,0);
		return  code;
	}
	return 0;
}
//short CmcCode::mcCode851();	//	Sinoatrial arrest. Unexpected absence of P, QRS and T, plus a R-R interval at a ﬁxed multiple of the normal interval, ±10%.
//short CmcCode::mcCode852();	//	Sinoatrial block. Unexpected absence of P, QRS and T, preceded by progressive shortening of P-P intervals. (R-R interval at a ﬁxed multiple of the normal interval,± 10%.)
short CmcCode::mcCode86x()	//861,862,863,864
{
	if(GetArate()>0 && GetArate()<GetVrate()*0.9) {
//		char *p=GetBeatsType();
		VH_ECGbeat *beat=GetBeats();
		unsigned short code=0,code861=8061,code862=8062,code863=8063,code864=8064;
		short beatsnum=GetBeatsNum();
		short i,j=0,rr[2];
		j=1;
		rr[0]=short(beat[1].Pos-beat[0].Pos);
		for(i=2;i<beatsnum;i++) {
			rr[1]=short(beat[i].Pos-beat[i-1].Pos);
			if(rr[1]<0.9*rr[0]) {
				if(beat[i].PR>=120) {j=2; break;}
			}
		}
		if(j==1) {
			if(msQRS()>=120) code=mccode(8,6,1);	else code=mccode(8,6,2);	//code862;
		}
		else if(j==2) {
			if(msQRS()>=120) code=mccode(8,6,3);	else code=mccode(8,6,4);	//code864;
		}
		SetmcCode( code,0);
		return  code;
	}
	return 0;
}
//short CmcCode::mcCode861();	//	A-V dissociation with ventricular pacemaker (without capture). Requires: P-P and R-R occur at variable rates with ventricular rate as fast as or faster than the atrial rate, plus variable P-R intervals, plus no capture beats.
//short CmcCode::mcCode862();	//	A-V dissociation with ventricular pacemaker (with capture).
//short CmcCode::mcCode863();	//	A-V dissociation with atrial pacemaker (without capture).
//short CmcCode::mcCode864();	//	A-V dissociation with atrial pacemaker (with capture).
short CmcCode::mcCode870_80()	//870,880
{
	if(isSinalP()) {
		unsigned short code=0;
		if(bpmHR()>=100) code=mccode(8,7,0);	//code870;
		else if(bpmHR()<=50) code=mccode(8,8,0);	//code880;
		else return 0;
		SetmcCode( code,0);
		return  code;
	}
	return 0;
}
//short CmcCode::mcCode870();	//	Sinus tachycardia (≥100/min).
//short CmcCode::mcCode880();	//	Sinus bradycardia (≤50/min).
short CmcCode::mcCode890()	//	Other arthythmias. Heart rate may be recorded as a continuous variable.
{//if no any arthythmia above, then call this member
	unsigned short code=mccode(8,9,0);	//8090;
	SetmcCode( code,0);
	return  code;
}

//
void CmcCode::mcCode9()
{
	mcCode920();
	mcCode910();
	mcCode930();
	mcCode941();
	mcCode942();
	mcCode950();
	mcCode960();
	mcCode971_2();
	mcCode981();
	mcCode982();
	mcCode983();
	mcCode984();
}
//ST Segment Elevation(9-2-code)
//Do not code in the presence of codes 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8.
//Anterolateral Site (leads I, aVL, V6)
short CmcCode::mcCode920()
{
	short i,j,L[]={I,aVL,V6,II,III,aVF,V1,V2,V3,V4,V5},n=sizeof(L)/sizeof(short);
	unsigned long nLeads = 0x0000;
	short judge=100;
	bool ok=false;
	for(i=j=0;i<n;i++) {
		if(L[i]>=V1 && L[i]<=V4) judge=200;
		else judge=100;
		if(uvST40(L[i])>=judge) {
			ok=true;
			if (isUpwardST(L[i])) {
				if (uvST80(L[i]) - uvST40(L[i]>judge / 2)) ok = false;
			}
			if (ok) {
				j++;
				nLeads |= m_infoLead[L[i]].mask;
			}
		}
	}
	if(j>1) {
		unsigned short code=mccode(9,2,0);	//9020;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
////Anterolateral Site (leads I, aVL, V6)
//short CmcCode::mcCode920L();	//	ST segment elevation ≥ 100 µV in any of leads I, aVL, V6.
////Posterior (Inferior) Site (leads II, III, aVF)
//short CmcCode::mcCode920I();	//	ST segment elevation ≥ 100 µV in any of leads II, III, aVF.
////Anterior site (Leads V1, V2, V3, V4, V5)
//short CmcCode::mcCode920A();	//	ST segment elevation ≥ 100 µV in lead V5 or ST segment elevation ≥ 200 µV in any of leads V1, V2, V3, V4.
//Miscellaneous Items(other 9-code)
short CmcCode::mcCode910()	//	Low QRS amplitude. QRS peak-to-peak amplitude < 500 µV in all beats in each of leads I, II, III, or < 1000 µV in all beats in each of leads V1, V2, V3, V4, V5, V6. (Check calibration before coding.)
{
	short i,L[]={I,II,III,V1,V2,V3,V4,V5,V6},n=sizeof(L)/sizeof(short);
	unsigned long nLeads = 0x0000;
	bool okL=true,okV=true;
	for(i=0;i<n;i++) {
		if(i<3) {
			if(uvQRSpp(L[i])<500) nLeads|=m_infoLead[L[i]].mask;
			else okL=false;
		}
		else {
			if(uvQRSpp(L[i])<1000) nLeads|=m_infoLead[L[i]].mask;
			else okV=false;
		}
	}
	if(okL || okV) {
		unsigned short code=mccode(9,1,0);	//9010;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode930()	//	P-wave amplitude ≥ 250 µV in any of leads II, III, aVF, in a majority of beats.
{
	short i,L[]={II,III,aVF},n=sizeof(L)/sizeof(short);
	unsigned long nLeads = 0x0000;
	for(i=0;i<n;i++) {
		if(uvP(L[i])>=250) nLeads|=m_infoLead[L[i]].mask;
	}
	if(nLeads>0) {
		unsigned short code=mccode(9,3,0);	//9030;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}

short CmcCode::mcCode941()	//	QRS transition zone at V3  or to the right of V3 on the chest.(all beats mainly positive in any lead V1~V3). (Do not code in the presence of 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8.)
{
	short i,j,L[]={V1,V2,V3},n=sizeof(L)/sizeof(short);
	unsigned long nLeads = 0x0000;
	for(i=j=0;i<n;i++) {
		if(uvQRSnet(L[i])>0) {
			j++;
			nLeads|=m_infoLead[L[i]].mask;
		}
	}
	if(j>1) {
		unsigned short code=mccode(9,4,1);	//9041;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode942()	//	QRS transition zone at V4  or to the left of V4 on the chest.(all beats mainly negative in any lead V4~V6 and all leads negative in V1~V3). (Do not code in the presence of 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8.)
{
	short i,L[]={V1,V2,V3,V4,V5,V6},n=sizeof(L)/sizeof(short);
	unsigned long nLeads = 0x0000;
	bool ok=true;
	for(i=0;i<3;i++) {
		if(uvQRSnet(L[i])>0) {ok=false;break;}
	}
	if(ok) {
		for(i=3;i<n;i++) 
			if(uvQRSnet(L[i])<0) nLeads|=m_infoLead[L[i]].mask;
		if(nLeads>0) {
			unsigned short code=mccode(9,4,2);	//9042;
			SetmcCode( code,nLeads);
			return  code;
		}
	}
	return 0;
}
short CmcCode::mcCode950()	//	T-wave amplitude > 1200 µV in any of leads I, II, III, aVL, aVF, V1, V2, V3, V4, V5, V6. (Do not  code in the presence of 6-4-1, 6-8, 7-1-1, 7-2-1, 7-4, or 7-8.)
{
	short i,L[]={I, II, III, aVL, aVF, V1, V2, V3, V4, V5, V6},n=sizeof(L)/sizeof(short);
	unsigned long nLeads = 0x0000;
	for(i=0;i<n;i++) {
		if(uvT(L[i])>1200) nLeads|=m_infoLead[L[i]].mask;
	}
	if(nLeads>0) {
		unsigned short code=mccode(9,5,0);	//9050;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode960()	//  Notched and widened P wave (duration ≥ 120 ms.) in frontal plane (usually lead II), and/or deep negative component to the P wave in lead V1  duration ≥ 40 ms. and depth ≥ 100 µV.
{
	unsigned long nLeads = 0x0000;
	if(msP(II)>=120) {
		if(m_templ) {
			short i,step=20*m_fs/1000;	if(step<1) step=1;
			short pb=Pb(II)+step,pe=Pe(II)-step;
			short ch=m_infoLead[II].chn;
			short uv50=(m_uVpb>0)?short(50/m_uVpb+0.5):50;
			for(i=pb;i<pe;i++) {
				if(m_templ[ch][i-step]-m_templ[ch][i]>uv50 && m_templ[ch][i+step]-m_templ[ch][i]>uv50) {
					nLeads|=m_infoLead[II].mask;
					break;
				}
			}
		}
	}
	if(uvNegaP(V1)<=-100) nLeads|=m_infoLead[V1].mask;
	if(nLeads>0) {
		unsigned short code=mccode(9,6,0);	//9060;
		SetmcCode( code,nLeads);
		return  code;
	}
	return 0;
}
short CmcCode::mcCode971_2()
{
	unsigned long nLeads = 0x0000;
	unsigned short code=0,code971=mccode(9,7,1),code972=mccode(9,7,2);
	for(short i=V3;i<V6;i++) {
		if(uvT(i)>1200) return 0;	//9-5-0
		else if(uvT(i)>=800) {
			if( code!=code971) {
				if(uvSTj(i)>=100) {
					nLeads|=m_infoLead[i].mask;
					 code=code972;
				}
			}
		}
		else if(uvT(i)>=500) {
			bool ok971=false;
			if(uvSTj(i)>=200) {
				ok971=true;
			}
			else if(uvSTj(i)>=100) {
				ok971=true;
			}
			if(ok971) {
				nLeads|=m_infoLead[i].mask;
				 code=code972;
			}
		}
	}
	if(nLeads>0) {
		if(m_templ) {
			bool ok=false;
			for(short i=V3;i<V6;i++) {
				if(nLeads&m_infoLead[i].mask) {
					short step=5*m_fs/1000;	if(step<1) step=1;
					short j=J(i),dms=40*m_fs/1000;
					short ch=m_infoLead[i].chn;
					short uv30=(m_uVpb>0)?short(30/m_uVpb+0.5):30;
					for(short i=j-dms;i<j+dms;i+=step) {
						if(m_templ[ch][i]-m_templ[ch][i-step]>uv30 && m_templ[ch][i]-m_templ[ch][i+step]>uv30) {
							ok=true;
							break;
						}
					}
				}
			}
			if(ok) {
				SetmcCode( code,nLeads);
				return  code;
			}
		}
	}
	return 0;
}
//short CmcCode::mcCode971()	//  Definite Early Repolarization.STJ elevation ≥ 1mm in the majority of beats, T wave amplitude≥ 500 µV, prominent J point, upward concavity of the ST segment, and a distinct notch  or slur on the down-stroke of the R wave in any of V3 –V6, or STJ elevation ≥ 200 µV in the majority of beats and T wave amplitude ≥ 500 µV, prominent J point and upward concavity of the  ST segment in any of V3 –V6.
//short CmcCode::mcCode972();	//  Probable Early Repolarization. STJ elevation ≥ 100 µV in the majority of beats, prominent J point,  and upward concavity of the ST segment in any of V3 –V6 and T wave amplitude ≥ 800 µV in any of the leads V3 –V6.
short CmcCode::mcCode981()	//  Uncorrectable lead reversal.
{
	//code this after VH-code
	return 0;
}
short CmcCode::mcCode982()	//  Poor Quality/Technical problems which interfere with coding.
{//cannot be detected
	return 0;
}
short CmcCode::mcCode983()	//  Correctable lead reversal
{
	//code this after VH-code
	return 0;
}
							//i.  Correctable limb lead connection error RA/LA Reserval
							//ii.  Correctable chest lead connection error in V1-V3 
							//iii. Correctable chest lead connection error in V4-V6
							//iv.  Correctable other chest lead connection error
short CmcCode::mcCode984()	//  Technical problems that do not interfere with coding.
{
	if(!m_templ) {
		unsigned short code=mccode(9,8,4);	//9084;
		SetmcCode( code,0);
		return  code;
	}
	return 0;
}

//end class CmcCode
