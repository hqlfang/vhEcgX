#include "StdAfx.h"
#include "EcgCodeVH.h"
#include "MyMath.h"
#include <stdio.h>

char CvhCode::vhcodebuf[8];

CvhCode::CvhCode(void) : CmcCode()
{
//	MI_L=initialEcgLeadInfo[aVL].mask|initialEcgLeadInfo[I].mask|initialEcgLeadInfo[aVR].mask;
//	MI_I=initialEcgLeadInfo[II].mask|initialEcgLeadInfo[aVF].mask|initialEcgLeadInfo[III].mask;
//	MI_S=initialEcgLeadInfo[V1].mask|initialEcgLeadInfo[V2].mask;
//	MI_A=initialEcgLeadInfo[V3].mask|initialEcgLeadInfo[V3].mask|initialEcgLeadInfo[V5].mask|initialEcgLeadInfo[V6].mask;
	m_stQscore.score=0;		m_stQscore.dwLeads=0;
	m_stSTDscore.score=0;	m_stSTDscore.dwLeads=0;
	m_stSTEscore.score=0;	m_stSTEscore.dwLeads=0;
	m_stTNscore.score=0;	m_stTNscore.dwLeads=0;
	m_stPLMIscore.score=0;	m_stPLMIscore.dwLeads=0;
	m_bPolymorphicPVC=false;
	m_bAf=false;
	vh.reset();
	cv.nCode=0;
	cv.nLeads=0;
	cv.nClass=0;
	cv.nSort=0;
	cv.nIndex=0;
}

CvhCode::~CvhCode(void)
{
}

unsigned short CvhCode::SetvhCode(unsigned short nCode,DWORD dwLeads)
{
	CodeList *p=vh.found(nCode);
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
		vh.add(code);
	}
	return nCode;
}
	
short CvhCode::vhCodeCount()
{
	return (short)vh.getCount();
}
	
short CvhCode::vhCodeGetFirst(char *szLeadName)
{
	CodeList *p=vh.getFirst();
	if(p) {
		if(szLeadName) {
			strcpy(szLeadName,LeadNames(p->code.nLeads));
		}
		return p->code.nCode;
	}
	return 0;
}
	
short CvhCode::vhCodeGetNext(char *szLeadName)
{
	CodeList *p=vh.getNext();
	if(p) {
		if(szLeadName) {
			strcpy(szLeadName,LeadNames(p->code.nLeads));
		}
		return p->code.nCode;
	}
	return 0;
}
	
unsigned short CvhCode::vhCodeFrommcCode(unsigned short mcCode,unsigned short vhCode,DWORD &dwLeads)
{
	unsigned short code=0;
	dwLeads=0;
	CodeList *p=mc.found(mcCode);
	if(p) {
		code=vhCode;
		dwLeads=p->code.nLeads;
		SetvhCode(code,dwLeads);
	}
	return code;
}
	
char *CvhCode::vhCodeString(unsigned short vhcode)
{
	sprintf(vhcodebuf,"%d-%d-%d",vhcode/1000,(vhcode%1000)/100,vhcode%100);
	return vhcodebuf;
}
	
void CvhCode::SetCriticalValue(short cv,DWORD dwLeads)
{
	if(this->cv.nCode<cv) {
		this->cv.nCode=cv;
		this->cv.nLeads=dwLeads;
	}
}
short CvhCode::GetCriticalValue()
{
	return cv.nCode;
}
short CvhCode::GetCriticalValue(DWORD &dwLeads)
{
	dwLeads=cv.nLeads;
	return cv.nCode;
}

//Table 3   Lead group and for Myocardial Infarction
//L(Lateral)	I(Inferior)	S(Septal)	A(Anterior)
//aVL, I, -aVR	II, aVF, III	V1, V2	V3, V4, V5, V6
//
//Table 4    Q scores for ECG leads
//Score	Condition
//注1.
//If Q score < 5 in aVF and Q score ≥ 5 in III modiﬁes Q score in lead III, reducing it by 5 points
//If R amplitude in lead aVL < 300 uV, then Q Score aVL = 0
//For inverted aVR, Q = Ri and R/Q ratio = S/Ri ratio, Codes 3-1-x and 3.3-x (LBBB and IVCD) modify QS score to 0.
//注2. Q score for posterolateraI (PL) MI: 
//For lead V1:
//score = 40 for Ri ≥ 60 ms; score = 30 for Ri ≥ 50 ms and Ri/S ≥ 1.5;and score = 20 for Ri ≥ 30 ms and Ri/S ≥ 1.5. 
//For lead V2:
//score = 30 for Ri ≥ 60 ms; score = 20 for Ri 50 ms (Ri ≥2000 µV and Ri/S ≥ 2); and score = 10 for Ri ≥ 40 ms (Ri ≥ 2000 µV and Ri/S ≥ 2)
//
//Table 5  Scores for ST Depression(STD), T wave Negative(TN) 
//and ST Evevation(STE)( µV)
//Note	[a] T score is not determined for leads V1,V2 and III; aVF only if R/S>1; aVL only for R ≥ 500µV
//[b] or “Flat” T wave with positive phase < 25µV
short CvhCode::Qscore(DWORD &dwLeads)
{
	DWORD dwTmp=0;
	short i,n,lead,Qd,uvQ,score=0;
	dwLeads=0;
//	{
	if(score<=40) {//40	Q ≥ 50ms in I,II,-aVR,V2,V3,V4,V5,V6,//QS wave in V4
//		if(score<40) dwLeads=0;
		short L[]={I,II,aVR,V2,V3,V4,V5,V6};		n=sizeof(L)/sizeof(short);
		for(i=0;i<n;i++) {
			lead=L[i];
			Qd=msQd(lead);	uvQ=abs(uvQd(lead));
			if((Qd>=50 || uvQ>350) && 3*uvQ>=uvRd(lead)) dwTmp|=m_infoLead[lead].mask;
		}
		if(isQS(V4)) dwTmp|=m_infoLead[V4].mask;
		if(dwTmp>0 && score<=40) score=40;
		dwLeads|=dwTmp;	dwTmp=0;
	}
//	{
	if(score<=35) {//35	Q ≥ 50ms in aVF.//Q ≥ 40ms and R/Q < 4 in I,II,-aVR,V3,V4,V5,V6.//Ri < 200µV in V2,V4 and QS in next lead V3,V5
//		if(score<35) dwLeads=0;
		short Qd;
		short L[]={I,II,aVR,V3,V4,V5,V6};		n=sizeof(L)/sizeof(short);
		for(i=0;i<n;i++) {
			lead=L[i];
			Qd=msQd(lead);	uvQ=abs(uvQd(lead));
			if((Qd>=40 || uvQ>250)&& 4*uvQ>=uvRd(lead)) dwTmp|=m_infoLead[lead].mask;
		}
		if(msQd(aVF)>=50 && -4*uvQd(aVF)>=uvRd(aVF)) dwTmp|=m_infoLead[aVF].mask;
		if(uvR1(V2)<200 && isQS(V3)) dwTmp|=m_infoLead[V3].mask;
		if(uvR1(V4)<200 && isQS(V5)) dwTmp|=m_infoLead[V5].mask;
		if(dwTmp>0 && score<=35) score=35;
		dwLeads|=dwTmp;	dwTmp=0;
	}
//	{
	if(score<=30) {//30	Q ≥ 50ms in aVL.//Q ≥ 40ms in I,II,-aVR,V2,V3,V4,V5,V6.//QS in I,II,-aVR,V3,V5
//		if(score<30) dwLeads=0;
		short L[]={I,II,aVR,V2,V3,V4,V5,V6};		n=sizeof(L)/sizeof(short);
		for(i=0;i<n;i++) {
			lead=L[i];
			Qd=msQd(lead);	uvQ=abs(uvQd(lead));
			if((Qd>=40 || uvQ>150)&& 4*uvQ>=uvRd(lead)) dwTmp|=m_infoLead[lead].mask;
		}
		if(uvRd(aVL)>=300 && msQd(aVL)>=50 && -4*uvQd(aVL)>=uvRd(aVL)) dwTmp|=m_infoLead[aVL].mask;
		if(isQS(I)) dwTmp|=m_infoLead[I].mask;
		if(isQS(II)) dwTmp|=m_infoLead[II].mask;
		if(isQS(V3)) dwTmp|=m_infoLead[V3].mask;
		if(isQS(V5)) dwTmp|=m_infoLead[V5].mask;
		if(isR(aVR)) dwTmp|=m_infoLead[aVR].mask;
		if(dwTmp>0 && score<=30) score=30;
		dwLeads|=dwTmp;	dwTmp=0;
	}
//	{
	if(score<=25) {//25	Q ≥ 30ms and R/Q< 4 in I,II,-aVR,V3,V4,V5,V6.//Ri < 200µV in V1,V5 and QS in next lead V2,V6.
//		if(score<25) dwLeads=0;
		short L[]={I,II,aVR,V3,V4,V5,V6};		n=sizeof(L)/sizeof(short);
		for(i=0;i<n;i++) {
			lead=L[i];
			Qd=msQd(lead);	uvQ=abs(uvQd(lead));
			if(Qd>=40 && 4*uvQ>=uvRd(lead)) dwTmp|=m_infoLead[lead].mask;
		}
		if(uvR1(V1)<200 && isQS(V2)) dwTmp|=m_infoLead[V2].mask;
		if(uvR1(V5)<200 && isQS(V6)) dwTmp|=m_infoLead[V6].mask;
		if(dwTmp>0 && score<=25) score=25;
		dwLeads|=dwTmp;	dwTmp=0;
	}
//	{
	if(score<=20) {//20	Q ≥ 40ms in aVL.//Q ≥ 30ms in I,II,-aVR,V1~V6.//QS in V2,V6
//		if(score<20) dwLeads=0;
		short L[]={I,II,aVR,V1,V2,V3,V4,V5,V6};		n=sizeof(L)/sizeof(short);
		for(i=0;i<n;i++) {
			lead=L[i];
			Qd=msQd(lead);	uvQ=abs(uvQd(lead));
			if(Qd>=30 && 4*uvQ>=uvRd(lead)) dwTmp|=m_infoLead[lead].mask;
		}
		if(uvRd(aVL)>=300 && msQd(aVL)>=40 && -4*uvQd(aVL)>uvRd(aVL)) dwTmp|=m_infoLead[aVL].mask;
		if(isQS(V2)) dwTmp|=m_infoLead[V2].mask;
		if(isQS(V6)) dwTmp|=m_infoLead[V6].mask;
		if(dwTmp>0 && score<=20) score=20;
		dwLeads|=dwTmp;	dwTmp=0;
	}
//	{
	if(score<=15) {//15	Q ≥ 40ms in III.//Q ≥ 30ms in aVF.//Q ≥ 20ms and R/Q< 4 in I,II,-aVR,V2,V3,V4,V5,V6.//QS in aVF
//		if(score<15) dwLeads=0;
		short L[]={I,II,aVR,V2,V3,V4,V5,V6};		n=sizeof(L)/sizeof(short);
		for(i=0;i<n;i++) {
			lead=L[i];
			Qd=msQd(lead);	uvQ=abs(uvQd(lead));	
			if(Qd>=20 && 4*uvQ>=uvRd(lead)) dwTmp|=m_infoLead[lead].mask;
		}
		if(msQd(aVF)<20) {
			if(msQd(III)>=50 && -4*uvQd(III)>=uvRd(III)) dwTmp|=m_infoLead[III].mask;
		}
		else {
			if(msQd(aVF)>=30 && -4*uvQd(aVF)>=uvRd(aVF)) dwTmp|=m_infoLead[aVF].mask;
			if(msQd(III)>=40 && -4*uvQd(III)>=uvRd(III)) dwTmp|=m_infoLead[III].mask;
		}
		if(isQS(aVF)) dwTmp|=m_infoLead[aVF].mask;
		if(dwTmp>0 && score<=15) score=15;
		dwLeads|=dwTmp;	dwTmp=0;
	}
//	{
	if(score<=10) {//10	Q ≥ 30ms in aVL.//Q ≥ 20ms in I,II,-aVR,V1~V6.//QS in V1
//		if(score<10) dwLeads=0;
		short L[]={I,II,aVR,V1,V2,V3,V4,V5,V6};		n=sizeof(L)/sizeof(short);
		for(i=0;i<n;i++) {
			lead=L[i];
			Qd=msQd(lead);	uvQ=abs(uvQd(lead));	
			if(Qd>=20 && 5*uvQ>=uvRd(lead)) dwTmp|=m_infoLead[lead].mask;
		}
		if(uvRd(aVL)>=300 && msQd(aVL)>=30 && -5*uvQd(aVL)>=uvRd(aVL)) dwTmp|=m_infoLead[aVL].mask;
		if(isQS(V1)) dwTmp|=m_infoLead[V1].mask;
		if(dwTmp>0 && score<=10) score=10;
		dwLeads|=dwTmp;	dwTmp=0;
	}
//	{
	if(score<=5) {//5	Q ≥ 30ms in III.//Q ≥ 20ms in aVF.//QS in III
//		if(score<5) dwLeads=0;
		if((msQd(III)>=30  && -5*uvQd(III)>=uvRd(III)) || isQS(III)) dwTmp|=m_infoLead[III].mask;
		if(msQd(aVF)>=20 && -5*uvQd(aVF)>=uvRd(aVF)) dwTmp|=m_infoLead[aVF].mask;
		if(dwTmp>0 && score<=5) score=5;
		dwLeads|=dwTmp;	dwTmp=0;
	}
	if(score>0) {//如果仅是aVR、V1,暂不计分
		if((dwLeads|m_infoLead[aVR].mask|m_infoLead[V1].mask)==(m_infoLead[aVR].mask|m_infoLead[V1].mask)) {
			score=0;	dwLeads=0;
		}
	}
	return score;
}
short CvhCode::plMIscore(DWORD &dwLeads)//后侧壁心梗评分
{
	short score=0;
	dwLeads=0;
	if(score<=40) {
		if(score<40) dwLeads=0;
		if(msR1(V1)>=60 && uvR1(V1)>=1000 && 2*uvR1(V1)>=-3*uvS(V1)) {
			score=40;	dwLeads|=m_infoLead[V1].mask;
		}
	}
	else if(score<=30) {
		if(score<30) dwLeads=0;
		if(msR1(V1)>=50 && uvR1(V1)>=1000 && 2*uvR1(V1)>=-3*uvS(V1)) {
			score=30;	dwLeads|=m_infoLead[V1].mask;
		}
		if(msR1(V2)>=60 && uvR1(V2)>=2000 && uvR1(V2)>=-2*uvS(V2)) {
			score=30;	dwLeads|=m_infoLead[V2].mask;
		}
	}
	else if(score<=20) {
		if(score<20) dwLeads=0;
		if(msR1(V1)>=30 && uvR1(V1)>=1000 && 2*uvR1(V1)>=-3*uvS(V1)) {
			score=20;	dwLeads|=m_infoLead[V1].mask;
		}
		if(msR1(V2)>=50 && uvR1(V2)>=2000 && uvR1(V2)>=-2*uvS(V2)) {
			score=20;	dwLeads|=m_infoLead[V2].mask;
		}
	}
	else if(score<=10) {
		if(score<10) dwLeads=0;
		if(msR1(V2)>=40 && uvR1(V2)>=2000 && uvR1(V2)>=-2*uvS(V2)) {
			score=10;	dwLeads|=m_infoLead[V2].mask;
		}
	}
	return score;
}
//Score	STD	TN(L-I-A)[a]	STE(L-I-A)	STE(S)
//50	ST≤-300	T≤-400	T≥400	ST≥500
//40	-300<ST≤-200	-400<T≤-300	400>ST≥300	500>ST≥400
//30	-200<ST≤-100	-300<T≤-200	300>ST≥200	400>ST≥300
//20	-100<ST≤-50	-200<T≤-100	200>ST≥100	300>ST≥200
//10	-50<ST≤-25	-100<T<25[2]	100>ST≥50	200>ST≥100
//0	-25<ST	T≥25	50>ST>-25	100>ST>-25
short CvhCode::STDscore(DWORD &dwLeads)
{
	dwLeads=0;
	short st=0,score=0;;
	for(short i=I;i<V6;i++) {
		if(i!=aVR) st=uvST40(i);	else st=-uvST40(i);
		if(score<=50 && st<=-300) {
			if(score<50) dwLeads=0;
			score=50;dwLeads|=m_infoLead[i].mask;
		}
		else if(score<=40 && st<=-200) {
			if(score<40) dwLeads=0;
			score=40;dwLeads|=m_infoLead[i].mask;
		}
		else if(score<=30 && st<=-100) {
			if(score<30) dwLeads=0;
			score=30;dwLeads|=m_infoLead[i].mask;
		}
		else if(score<=20 && st<=-50)  {
			if(score<20) dwLeads=0;
			score=20;dwLeads|=m_infoLead[i].mask;
		}
		else if(score<=10 && st<=-25)  {
			score=10;dwLeads|=m_infoLead[i].mask;
		}
	}
	return score;
}
short CvhCode::STEscore(DWORD &dwLeads)
{
	dwLeads=0;
	short st=0,score=0;;
	for(short i=I;i<V6;i++) {
		if(i!=aVR) st=uvST40(i);	else st=-uvST40(i);
		if(i==V1 || i==V2) {
			if(score<=50 && st>=500) {
				if(score<50) dwLeads=0;
				score=50;dwLeads|=m_infoLead[i].mask;
			}
			else if(score<=40 && st>=400) {
				if(score<40) dwLeads=0;
				score=40;dwLeads|=m_infoLead[i].mask;
			}
			else if(st>=300) {
				if(score<=30 && score<30) dwLeads=0;
				score=30;dwLeads|=m_infoLead[i].mask;
			}
			else if(st>=200)  {
				if(score<=20 && score<20) dwLeads=0;
				score=20;dwLeads|=m_infoLead[i].mask;
			}
			else if(score<=10 && st>=100)  {
				score=10;dwLeads|=m_infoLead[i].mask;
			}
		}
		else {
			if(score<=50 && st>=400) {
				if(score<50) dwLeads=0;
				score=50;dwLeads|=m_infoLead[i].mask;
			}
			else if(score<=40 && st>=300) {
				if(score<40) dwLeads=0;
				score=40;dwLeads|=m_infoLead[i].mask;
			}
			else if(score<=30 && st>=200) {
				if(score<30) dwLeads=0;
				score=30;dwLeads|=m_infoLead[i].mask;
			}
			else if(score<=20 && st>=100)  {
				if(score<20) dwLeads=0;
				score=20;dwLeads|=m_infoLead[i].mask;
			}
			else if(score<=10 && st>=50)  {
				score=10;dwLeads|=m_infoLead[i].mask;
			}
		}
	}
	return score;
}
short CvhCode::TNscore(DWORD &dwLeads)
{
	dwLeads=0;
	short i,j,t=0,score=0,L[]={I,II,aVR,aVL,aVF,V3,V4,V5,V6},n=sizeof(L)/sizeof(short);
	for( j=0;j<n;j++) {	i=L[j];
		if(i!=aVR) t=uvT(i);	else t=-uvT(i);
		if(i==aVF && uvRd(i)<=-uvS(i)) continue;
		if(i==aVL && uvRd(i)<500) continue;
		if(score<=50 && t<=-400) {
			if(score<50) dwLeads=0;
			score=50;dwLeads|=m_infoLead[i].mask;
		}
		else if(score<=40 && t<=-300) {
			if(score<40) dwLeads=0;
			score=40;dwLeads|=m_infoLead[i].mask;
		}
		else if(score<=30 && t<=-200) {
			if(score<30) dwLeads=0;
			score=30;dwLeads|=m_infoLead[i].mask;
		}
		else if(score<=20 && t<=-100) {
			if(score<20) dwLeads=0;
			score=20;dwLeads|=m_infoLead[i].mask;
		}
		else if(score<=10 && t<75)  {
			score=10;dwLeads|=m_infoLead[i].mask;
		}
	}
	return score;
}
	
bool CvhCode::isPolymorphicPVC()
{
	m_bPolymorphicPVC=false;
	if(!m_data || m_length<m_fs) return m_bPolymorphicPVC;
	short i,k,L[]={II,V1};
	short *src[2],*dst[2];
	short start=m_ms180/2,len=short(360L*m_fs/1000L),ms=short(10*m_fs/1000+0.5);	if(ms<1) ms=1;
	short beatsnum=GetBeatsNum();
	int pos;
	VH_ECGbeat *beats=GetBeats();
	char *p=GetBeatsType();
	CMaxLinearCorrelation mlc[2];
	double r[2];
	for(i=k=0;i<beatsnum;i++) {
		if(p[i]=='V') {
			pos=beats[i].Pos-start;
			if(k==0) {
				if(pos>=0) {
					src[0]=m_data[L[0]]+pos;
					src[1]=m_data[L[1]]+pos;
					k++;

					//HDC hDC=GetDC(NULL);
					//CDC *pDC=CDC::FromHandle(hDC);
					//pDC->MoveTo(0,400);
					//for(int j=0;j<len;j+=5) {
					//	pDC->LineTo(j,400-src[0][j]/5);
					//}
					//ReleaseDC(NULL,hDC);
				}
			}
			else {
				if(pos+len<m_length) {
					dst[0]=m_data[L[0]]+pos;
					dst[1]=m_data[L[1]]+pos;
					r[0]=mlc[0].MaxLinearCorrelation(src[0],dst[0],len);
					r[1]=mlc[1].MaxLinearCorrelation(src[1],dst[1],len);
					if(r[0]<0.9 && r[1]<0.9) {
						m_bPolymorphicPVC=true;
						break;
					}
					if(!m_bPolymorphicPVC) {
						short maxp[2][2],minp[2][2];//[Lead][index]
						for(short j=0;j<2;j++) {//lead
							for(short s=0;s<2;s++) maxp[j][s]=minp[j][s]=0;
							for(short p0=0;p0<len;p0++) {
								if(src[j][maxp[j][0]]<src[j][p0] && src[j][p0]>src[j][p0-ms] && src[j][p0]>src[j][p0+ms]) maxp[j][0]=p0;
								if(src[j][minp[j][0]]>src[j][p0] && src[j][p0]<src[j][p0-ms] && src[j][p0]<src[j][p0+ms]) minp[j][0]=p0;
								if(dst[j][maxp[j][1]]<dst[j][p0] && dst[j][p0]>dst[j][p0-ms] && dst[j][p0]>dst[j][p0+ms]) maxp[j][1]=p0;
								if(dst[j][minp[j][1]]>dst[j][p0] && dst[j][p0]<dst[j][p0-ms] && dst[j][p0]<dst[j][p0+ms]) minp[j][1]=p0;
							}
							if((minp[j][0]-maxp[j][0])*(minp[j][1]-maxp[j][1])<0) {
								m_bPolymorphicPVC=true;	break;
							}
						}
//						CMax max;	CMin min;
						//for(int j=0;j<2;j++) {
							//max.maximum(src[j],len);	maxp[0][j]=max.GetIndex();
							//min.minimum(src[j],len);	minp[0][j]=min.GetIndex();
							//max.maximum(dst[j],len);	maxp[1][j]=max.GetIndex();
							//min.minimum(dst[j],len);	minp[1][j]=min.GetIndex();
						//	if((minp[0][j]-maxp[0][j])*(minp[1][j]-maxp[1][j])<0) {
						//		m_bPolymorphicPVC=true;	break;
						//	}
						//}
					}
				}
			}
		}
	}	

	return m_bPolymorphicPVC;
}

void CvhCode::code()
{
	mcCode();
	vhCode();
}

short CvhCode::vhCode()
{
	vh.reset();
	m_bPolymorphicPVC=false;
	m_bAf=false;
	CheckQuality();
	if(vhCode010()) return vh.getCount();
	if(vhCode173_4()) return vh.getCount();
	if(vhCode0()) return vh.getCount();
	vhCode1();	//Rhythm Codes
	if(bpmHR()>=180) SetCriticalValue(CV_HVR);
	else if(bpmHR()<=45) SetCriticalValue(CV_LHR); 
	if(!m_pEcgParm || !m_pEcgLead || !m_pEcgBeat || !m_pEcgInfo) return (short)vh.getCount();


	m_stQscore.score=Qscore(m_stQscore.dwLeads);
	m_stTNscore.score=TNscore(m_stTNscore.dwLeads);
//	if(!vh.found(vhcode(0,4,0))) {
		m_stSTDscore.score=STDscore(m_stSTDscore.dwLeads);
		m_stSTEscore.score=STEscore(m_stSTEscore.dwLeads);
//	}
	m_stPLMIscore.score=plMIscore(m_stPLMIscore.dwLeads);
	vhCode7();	//Axis deviation
	vhCode2();	//Atrioventricular Conduction Abnormalities
	if(!vh.found(vhcode(0,2,2)) && !vh.found(vhcode(0,2,3))) {
		vhCode6();	//心室肥大与心房扩大
		vhCode3();	//Prolonged Ventricular Excitation and Fascicular Blocks
		vhCode4();	//Repolarization abnormalities
		vhCode5();	//Myocardial Infarction/Ischemia (MI Likelihood)
		vhCode8();	//Other abnormalities
		if(vh.found(vhcode(6,1,1))) {
			vh.remove(vhcode(5,5,0));
			vh.remove(vhcode(5,6,0));
			vh.remove(vhcode(5,6,1));
			vh.remove(vhcode(5,7,0));
			vh.remove(vhcode(5,8,0));
		}
		else {
		}

		vh.remove(vhcode(0,4,0));

		if(vh.found(vhcode(3,1,0)) || vh.found(vhcode(3,1,1)) || vh.found(vhcode(3,2,0)) || vh.found(vhcode(3,2,1))) {
			vh.remove(vhcode(6,1,0));	vh.remove(vhcode(6,1,1));
			vh.remove(vhcode(6,3,1));
		}
	}
	if(vh.found(vhcode(8,5,0))) {
		vh.remove(vhcode(5,5,0));
		vh.remove(vhcode(5,8,0));
	}
	if(vh.found(vhcode(6,1,1))) {
	}
	else {
	}
	if(m_stSTDscore.score>30) {
		int i,n=0;
		for(i=0;i<12;i++) {
			if(m_stSTDscore.dwLeads&(1<<i)) n++;
		}
		if(n>1) SetCriticalValue(CV_STD);
	}
	if(m_stSTEscore.score>30) {
		if(mc.found(mccode(9,5,0))) {
			int i,n=0;
			for(i=0;i<12;i++) {
				if(m_stSTEscore.dwLeads&(1<<i)) n++;
			}
			if(n>1) SetCriticalValue(CV_STE_HT);
		}
		if(m_stTNscore.score>30) {
			SetCriticalValue(CV_STE_IT);
		}
	}
	return vh.getCount();
}
//3.1 Code 0, Baseline EGG Suppression Codes
//If any code 0 is present, no any other code.
bool CvhCode::vhCode0()
{
//	if(vhCode010()) return true;
	if(vhCode021()) return true;
	if(!vhCode022()) {
		if(vhCode031()) return true;
//		if(vhCode032()) return true;
		if(vhCode0321()) return true;
		if(vhCode0322()) return true;
		//if(vhCode0323()) return true;
	}
	if(!vhCode023()) {
		vhCode033_4();
	}
	vhCode040();
	if(vhCode050()) m_bAf=true;// return true;
	if(vhCode061_2()) {
		vh.remove(vhcode(0,4,0));
		return true;
	}
	return false;
}
short CvhCode::vhCode010()	//0-1-0 ECG not available
{
	//C1 = No more than three beats every ten seconds are detected.
	//C2 = The peak to peak amplitude of all leads < 100µV in most of time.
	//Code 0-1-0 = C1 and C2
	short seconds=short(m_length/m_fs);
	short beatsnum=GetBeatsNum();
	short i,j=0;
	if(beatsnum*10<4*seconds) {
		for(i=0;i<VH_EcgMaxLeads;i++) {
			if(m_infoLead[i].quality==3) j++;
		}
		if(j>=m_chnum/2) {
			unsigned short code=vhcode(0,1,0);	//100;
			SetvhCode(code,0);
			SetCriticalValue(CV_VSTOP);
			return code;
		}
	}
	return 0;
}
short CvhCode::vhCode021()	// Inadequate quality
{//undetected
	//C1 = poor quality and is present if either there is a baseline drift > 400 µV (choose three successive P-QRS-T complexes and measure peak- to-peak baseline drift in millimeters in the worst lead); OR > 200 µV perturbation of the isoelectric line (measurement in millimeters from the highest peak-to-peak random [muscle] or 50/60-Hz noise in the worst lead).
	//Code 0-2-1 = C1
	return 0;
}
short CvhCode::vhCode022()	// Missing limb leads
{
	//C1 = The amplitude any of I, II, III no more than 25 µV in most of time (more than 50% of time)
	//Code 0-2-2 = C1
	short i,L[]={I,II,III},n=sizeof(L)/sizeof(short);
	unsigned long  nLeads=0;
	for(i=0;i<n;i++) {
		if(m_infoLead[L[i]].quality==3) nLeads|=m_infoLead[L[i]].mask;
	}
	if(nLeads>0) {
		unsigned short code=vhcode(0,2,2);	//202;
		SetvhCode(code,nLeads);
		return code;
	}
	return 0;
}
short CvhCode::vhCode023()	// Missing chest leads
{
	//C1: The amplitude difference between any chest lead(Vi) and (II+III)/3 no more than 25µV in most of time (more than 50% of time)
	//Code 0-2-3 = C1
	unsigned long  nLeads=0;
	for(short i=V1;i<VH_EcgMaxLeads;i++) {
		if(m_infoLead[i].quality==3) nLeads|=m_infoLead[i].mask;
	}
	if(nLeads>0) {
		unsigned short code=vhcode(0,2,3);	//203;
		SetvhCode(code,nLeads);
		return code;
	}
	return 0;
}
short CvhCode::vhCode031()	//  RA/RL reversal ? (Check limb lead electrodes again) 
{
	//C1: PII < 50 µV, QRSpII < 50 µV and TpII < 50 µV
	//C2: I = -III (P, ORS, and T are mirror images of each other)
	//C3: PaVR > 0, PaVL >0
	//Code 0-3-1 = C1 and C2 and C3
	if(uvP(II)<50 && uvQRSpp(II)<50 && uvTpp(II)<50) {
		if(uvP(aVR)>0 && uvP(aVL)>0) {
			if(m_templen>0 && m_templ) {
				short chI=m_infoLead[II].chn;
				short chIII=m_infoLead[III].chn;
				short step=10*m_fs/1000;	if(step<1) step=1;
				short c2=0;
				for(short i=0;i<m_templen;i+=step) {
					if(m_templ[chI][i]+m_templ[chIII][i]>m_uv100) c2++;
				}
				if(c2==0) {
					unsigned short code=vhcode(0,3,1);	//301;
					SetvhCode(code,0);
					return code;
				}
			}
		}
	}
	return 0;
}
//short CvhCode::vhCode032()	// Other limb lead connection interchanges (correction made and ECG coded)
//{//undetected
//	//Code 0-3-2 = Code 0-3-2(1) or Code 0-3-2(2) or Code 0-3-2(3)
//	return 0;
//}
short CvhCode::vhCode0321()	// RA/LA reversal? (Check limb lead electrodes again)
{
	//C1: PI ≤ 0µV and QRSnI < 0µV
	//C2: PII ≥ 0µV and QRSnII > 0µV
	//C3: PIII ≥ 0µV and QRSnIII > 0µV
	//C4: PaVL ≤ 0µV and QRSnaVL < 0µV
	//C5: QRSn(V4 and V5) ≥ 1mV
	//Code 0-3-2(1) = C1 and (C2 or C3) and C4 and C5
	//Note: see also MC 9-8-3(i)
	bool c1=(uvP(I)<0 && uvQRSnet(I)<0)?true:false;
	bool c2=(uvP(II)>0 && uvQRSnet(II)>0)?true:false;
	bool c3=(uvP(III)>0 && uvQRSnet(III)>0)?true:false;
	bool c4=(uvP(aVL)<0 && uvQRSnet(aVL)<0)?true:false;
	bool c5=(uvQRSnet(V4)>=1000 && uvQRSnet(V5)>=1000)?true:false;
	if(c1 && (c2 || c3) && c4 && c5) {
		unsigned short code=vhcode(0,3,21);	//321;
		SetvhCode(code,0);
		return code;
	}
	return 0;
}
short CvhCode::vhCode0322()	// RA/LL reversal? (Check limb lead electrodes again)
{
	//C1: PI ≤ 0µV and QRSnI < 0µV
	//C2: PII ≤ 0µV and QRSnII < 0µV
	//C3: PIII ≤ 0µV and QRSnIII < 0µV
	//C4: PaVF ≤ 0µV and QRSnaVF < 0µV
	//C5: QRSn(V4 and V5) ≥ 1mV
	//Code 0-3-2(2) = C1 and (C2 or C3) and C4 and C5
	bool c1=(uvP(I)<0 && uvQRSnet(I)<0)?true:false;
	bool c2=(uvP(II)<0 && uvQRSnet(II)<0)?true:false;
	bool c3=(uvP(III)<0 && uvQRSnet(III)<0)?true:false;
	bool c4=(uvP(aVF)<0 && uvQRSnet(aVF)<0)?true:false;
	bool c5=(uvQRSnet(V4)>=1000 && uvQRSnet(V5)>=1000)?true:false;
	if(c1 && (c2 || c3) && c4 && c5) {
		unsigned short code=vhcode(0,3,22);	//322;
		SetvhCode(code,0);
		return code;
	}
	return 0;
}
//short CvhCode::vhCode0323()	// LA/LL reversal? (Check limb lead electrodes again)
//{
//	//C1: PI ≥ 0µV and QRSnI > 0µV
//	//C2: PII ≥ 0µV and QRSnII > 0µV
//	//C3: PIII ≤ 0µV and QRSnIII < 0µV
//	//C4: PaVR ≤ 0µV and QRSnaVR < 0µV
//	//C5: QRSn(V4 and V5) ≥ 1mV
//	//Code 0-3-2(3)=C1 and C2 and C3 and C4 and C5
//	bool c1=(uvP(I)>0 && uvQRSnet(I)>0)?true:false;
//	bool c2=(uvP(II)>0 && uvQRSnet(II)>0)?true:false;
//	bool c3=(uvP(III)<0 && uvQRSnet(III)<0)?true:false;
//	bool c4=(uvP(aVR)<0 && uvQRSnet(aVR)<0)?true:false;
//	bool c5=(uvQRSnet(V4)>=1000 && uvQRSnet(V5)>=1000)?true:false;
//	if(c1 && c2 && c3 && c4 && c5) {
//		unsigned short code=vhcode(0,3,23);	//323;
//		SetvhCode(code,0);
//		return code;
//	}
//	return 0;
//}
	
short CvhCode::vhCode033_4()	// 判断胸导R波高度变化规律
{
	bool peak=false,ok=true;
	int i,j=0,k=0;
	for(i=V1;i<V6;i++) {
		if(!peak) {
			if(uvR(i+1)+100>uvR(i)) j++;
			else peak=true;
		}
		else {
			if(uvR(i)+100>uvR(i+1)) k++;
			else {ok=false;	break;}
		}
	}
	short code=0;
	if(!ok) {
		code=vhCode033();
		if(code==0) code=vhCode033();
		if(code==0) code=vhCode034();
//		if(code==0) code=vhCode035();
	}
	return code;
}

short CvhCode::vhCode033()	// Correctable chest lead in V1-V3 connection error? (ECG codable)
{
	//C1: RV2 < RV1 < RV3 and SV2 < SV1 < SV3
	//C2: RV3 < RV2 < RV1 and SV3 < SV2 < SV1
	//C3: RV1 < RV3 < RV2 and SV1 < SV3 < SV2
	//C4: RV3 < RV1 < RV2 and SV3 < SV1 < SV2
	//C5: RV2 < RV3 < RV1 and SV2 < SV3 < SV1
	//Code 0-3-3 = C1 or C2 or C3 or C4 or C5
	//Note: see also MC 9-8-3(ii)
	bool c1=(uvR(V2)+100<uvR(V1) && uvR(V1)<uvR(V3) && -uvS(V2)+100<-uvS(V1) && -uvS(V1)<-uvS(V3))?true:false;
	bool c2=(uvR(V3)+100<uvR(V2) && uvR(V2)+100<uvR(V1) && -uvS(V3)+100<-uvS(V2) && -uvS(V2)+100<-uvS(V1))?true:false;
	bool c3=(uvR(V1)<uvR(V3) && uvR(V3)+100<uvR(V2) && -uvS(V1)<-uvS(V3) && -uvS(V3)+100<-uvS(V2))?true:false;	if(c3) if(uvR(V4)<=uvR(V3)) c3=false;
	bool c4=(uvR(V3)+100<uvR(V1) && uvR(V1)<uvR(V2) && -uvS(V3)+100<-uvS(V1) && -uvS(V1)<-uvS(V2))?true:false;
	bool c5=(uvR(V2)<uvR(V3) && uvR(V3)+100<uvR(V1) && -uvS(V2)<-uvS(V3) && -uvS(V3)+100<-uvS(V1))?true:false;
	if(c1 || c2 || c3 || c4 || c5) {
		unsigned short code=vhcode(0,3,3);	//303;
		SetvhCode(code,0);
		return code;
	}
	return 0;
}
short CvhCode::vhCode034()	// Correctable chest lead in V4-V6 connection error? (ECG codable)
{
	//C1: RV6 < RV4 < RV5 and SV6 < SV4 < SV5
	//C2: RV4 < RV5 < RV6 and SV4 < SV5 < SV6
	//C3: RV5 < RV6 < RV4 and SV5 < SV6 < SV4
	//C4: RV5 < RV4 < RV6 and SV5 < SV4 < SV6
	//C5: RV4 < RV6 < RV5 and SV4 < SV6 < SV5
	//Code 0-3-4 = C1 or C2 or C3 or C4 or C5
	//Note: see also MC 9-8-3(iii)
	bool c1=(uvR(V6)<uvR(V4) && uvR(V4)<uvR(V5) && -uvS(V6)<-uvS(V4) && -uvS(V4)<-uvS(V5))?true:false;
	bool c2=(uvR(V4)<uvR(V5) && uvR(V5)<uvR(V6) && -uvS(V4)<-uvS(V5) && -uvS(V5)<-uvS(V6))?true:false;
	bool c3=(uvR(V5)<uvR(V6) && uvR(V6)<uvR(V4) && -uvS(V5)<-uvS(V6) && -uvS(V6)<-uvS(V4))?true:false;
	bool c4=(uvR(V5)<uvR(V4) && uvR(V4)<uvR(V6) && -uvS(V5)<-uvS(V4) && -uvS(V4)<-uvS(V6))?true:false;
	bool c5=(uvR(V4)<uvR(V6) && uvR(V6)<uvR(V5) && -uvS(V4)<-uvS(V6) && -uvS(V6)<-uvS(V5))?true:false;
	if(uvR(V4)>uvR(V3) && uvR(V4)<uvR(V5)) c1=false;
	if(c1 || c2 || c3 || c4 || c5) {
		unsigned short code=vhcode(0,3,4);	//304;
		SetvhCode(code,0);
		return code;
	}
	return 0;
}
short CvhCode::vhCode035()	// Correctable other chest lead connection error? (ECG codable)
{
	//C1: RV3 > RV2 and RV3 > RV4 and SV3 < SV2 and SV3 < SV4
	//C2: RV4 > RV3 and RV5 > RV4 and SV3 < SV4 and SV5 > SV4
	//C3: R/S(V2) < R/S(V3) < R/S(V4) and RV2 > RV4 and SV2 < SV4
	//C4: R/S(V2) < R/S(V3) < R/S(V4) and RV4 > RV5 and SV2 < SV4 < SV5
	//Code 0-3-5 = C1 or C2 or C3 or C4
	//Note: see also MC 9-8-3(iv)
	bool c1=(uvR(V3)>uvR(V2) && uvR(V3)>uvR(V4) && -uvS(V3)<-uvS(V2) && -uvS(V3)<-uvS(V4))?true:false;
	bool c2=(uvR(V4)>uvR(V3) && uvR(V5)>uvR(V4) && -uvS(V3)<-uvS(V4) && -uvS(V5)>-uvS(V4))?true:false;
	bool c3=(uvR(V2)>uvR(V4) && -uvS(V2)<-uvS(V4))?true:false;
	bool c4=(uvR(V4)>uvR(V5) && -uvS(V2)<-uvS(V4) && -uvS(V4)<-uvS(V5))?true:false;
	if(uvS(V2)!=0 && uvS(V3)!=0 && uvS(V4)!=0) {
		if(!(ratioRS(V2)<ratioRS(V3) && ratioRS(V3)<ratioRS(V4))) c3=c4=false;
	}
	if(c1 || c2 || c3 || c4) {
		unsigned short code=vhcode(0,3,5);	//305;
		SetvhCode(code,0);
		return code;
	}
	return 0;
}
short CvhCode::vhCode040()	// Any conditions with ORS duration ≥ 120 ms
{
	//C1: Any of codes 2-4-x, 3-1-x, 3-2-x present
	//Note 1: Code 0-4-0 suppresses ST-T scores.
	//Note 2: Needn’t output this code in the diagnostic report.
	if(msQRS()>=120) {
		unsigned short code=vhcode(0,4,0);	//400;
		SetvhCode(code,0);
		return code;
	}
	return 0;
}
short CvhCode::vhCode050()	// Atrial ﬁbrillation or ﬂutter
{
	//C1: Rhythm codes 1-5-1 or 1-5-2 (atrial ﬂutter type 1 or type 2) or 1-5-3 (atrial ﬁbril- lation) present
	//Code 0-5-0 = C1
	//Note 1: Code 0-5-0 suppresses code 2, code 7, and code 9. Atrial ﬂutter suppresses ST-T scores and the corresponding prevalent and incident ischemic codes.
	//Note 2: Needn’t output the code 0-5-0 in the diagnostic report, use rhythm codes 1-5-1 or 1-5-2 or 1-5-3
	if(mc.found(mccode(8,3,1))) SetvhCode(vhcode(1,5,3),0);
	else if(mc.found(mccode(8,3,2))) SetvhCode(vhcode(1,5,1),0);
	else return 0;
	unsigned short code=vhcode(0,5,0);	//500;
	if(vhCode153x()) {
		vh.remove(vhcode(1,5,1));
		vh.remove(vhcode(1,5,3));
	}
	return code;
}
short CvhCode::vhCode061_2()
{
	char Type;
	short SpikesN;
	long *p=GetPaceMaker(Type,SpikesN);// { //'N': none, 'A': A-Type, 'V': V-type, 'B': Both
	switch(Type) {
		case 'V':	case 'B':	{unsigned short code=vhcode(0,6,1);	SetvhCode(vhcode(1,6,1),0);return code;}break;
		case 'A':				{unsigned short code=vhcode(0,6,2);	SetvhCode(vhcode(1,6,2),0);return code;}break;
	}
	return 0;
}
//short CvhCode::vhCode061()	// Ventricular or dual-chamber electronic pacemaker
//{
//	//C1: Code 1-6-1 is present
//	//Note 1: Except demand pacemaker with more than two adequate-quality nonpaced complexes is available for coding from all lead groups.
//	//Note 2: Code 0-6-1 suppresses all codes (except code 1-6-2).
//	//Note 3: Needn’t output the code 0-6-1 in the diagnostic report. use code 1-6-1
//	return 0;
//}
//short CvhCode::vhCode062()	// Atrial electronic pacemaker
//{
//	//C1: Code 1-6-2 is present
//	//Note 1: Except demand pacemaker with more than two adequate-quality nonpaced complexes is available for coding from all lead groups.
//	//Note 2: Code 0-6-2 suppresses code 3, code 7, and code 9.
//	//Note 3: Needn’t output the code 0-6-2 in the diagnostic report. use code 1-6-2.
//	return 0;
//}
//3.2.	Code 1, Rhythm Codes
void CvhCode::vhCode1()
{
	if(vh.found(vhcode(1,5,1)) || vh.found(vhcode(1,5,2)) || vh.found(vhcode(1,5,3)) || vh.found(vhcode(1,5,31)) || vh.found(vhcode(1,5,32)) || vh.found(vhcode(1,5,33))) return;
	bool code1=false;
	short code=0;
	code=vhCode100_4();	if(!code1 && code) code1=true;
	code=vhCode101x();	if(!code1 && code) code1=true;
	code=vhCode103x();	if(!code1 && code) code1=true;
	if(!code) code=vhCode1020();	if(!code1 && code) code1=true;
	code=vhCode1040();	if(!code1 && code) code1=true;
	code=vhCode1050();	if(!code1 && code) code1=true;
	code=vhCode1060();	if(!code1 && code) code1=true;
	if(!code1)	code=vhCode110();	if(!code1 && code) code1=true;
	if(!code1)	code=vhCode12x();	if(!code1 && code) code1=true;
	if(!code1)	code=vhCode13x();	if(!code1 && code) code1=true;
	if(!code1)	code=vhCode134();	if(!code1 && code) code1=true;
	if(!code1)	code=vhCode14x();	if(!code1 && code) code1=true;
	//code=vhCode150();	if(!code1 && code) code1=true;
	//code=vhCode151();	if(!code1 && code) code1=true;
	//code=vhCode152();	if(!code1 && code) code1=true;
	//code=vhCode153();	if(!code1 && code) code1=true;
	//code=vhCode153x();	if(!code1 && code) code1=true;
	//code=vhCode154();	if(!code1 && code) code1=true;
	//code=vhCode16x();	if(!code1 && code) code1=true;
	code=vhCode170_2();	if(!code1 && code) code1=true;
	//code=vhCode173_4();	if(!code1 && code) code1=true;
	if(!code1)	code=vhCode181();	if(!code1 && code) code1=true;
	if(!code1)	code=vhCode182();	if(!code1 && code) code1=true;
	if(!code1) code=vhCode190();
}
short CvhCode::vhCode100_4()
{
	bool sinal=(uvP1(II)>0 && uvP2(II)==0)?true:false;
	if(!sinal) sinal=(uvP(II)>=0 && uvP1(I)>0 && uvP2(I)==0)?true:false;      
	if(sinal && uvP(aVR)<=0) {//1(aVR)<=0  && uvP2(aVR)==0) {
		unsigned short code=0,code101=vhcode(1,0,1),code102=vhcode(1,0,2),code103=vhcode(1,0,3),code104=vhcode(1,0,4);
		if(m_ageY<1) {
			if(bpmHR()<=100) code=code102;
			else if(bpmHR()>=140) code=code103;
		}
		else if(m_ageY<6) {
			if(bpmHR()<=80) code=code102;
			else if(bpmHR()>=120) code=code103;
		}
		//else if(m_ageY<14) {
		//}
		else {
			if(bpmHR()<60) code=code102;
			else if(bpmHR()>=100) code=code103;
		}
		if(code==0) {
			int i,n=GetBeatsNum();
			VH_ECGbeat *p=GetBeats();
			char *t=GetBeatsType();
			short jp=0,jv=0,rr,rr0=0,avgrr=msRR()*m_fs/1000;
			for(i=1;i<n;i++) {
				rr=short(p[i].Pos-p[i-1].Pos);
				if(t[i]=='N') {
					if(100*abs(rr-avgrr)/avgrr>=10) jp++;
					if(rr0>0) {
						if(abs(rr-rr0)>=m_ms120) jv++;
					}
					rr0=rr;
				}
			}
			if(m_ageY<14) {
				if(3*jv>n) code=code104;
			}
			else {
				if(3*jp>n) code=code104;
			}
			if(code==0) code=code101;
		}
		SetvhCode(code,0);
		return code;
	}
	return 0;
}
//short CvhCode::vhCode100()	// Sinus rhythm (SR) 
//{
//	//C1: PII ≥ 0 µV and PaVR ≤ 0 µV 
//	//C2: P amplitude variation < 100 µV
//	//C3: Presence of three or more P-QRS-T complexes meeting criteria C1 and C2
//	//Code 1-0-0 = C1 and C2 and C3
//	//Note 1: C1 implies P axis ≤ 120° and ≥ -30°.
//	//Note 2: Usually sinus rhythm is associated with ﬁxed coupling between the normal P waves and the following QRS complexes, with P-R interval variation <20% from the median PR. However, in Mobitz type 1 ﬁrst degree AV block (Code 2-2-1), the PR interval varies.
//	if(uvP(II)>0 && uvP(aVR)<0) {
//		unsigned short code=vhcode();	//1,0,0};
//		SetvhCode(code,0);
//		return code;
//	}
//	return 0;
//}
//short CvhCode::vhCode101()	// Normal sinus rhythm(NSR)
//{
//	//C1: Rhythm sinus (code 1-0)
//	//C2: No code 1-0-2,1-0-3,1-0-4
//	//Code 1-0-1 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode102()	// Sinus bradycardia (SB) 
//{
//	//C1: Rhythm sinus (code 1-0) 
//	//C2: Ventricular rate ≤ 50 bpm for adults
//	//C3: Ventricular rate ≤ lower limits for children. the lower limits for different age as follows,
//	//100 bpm for < 1y, 80 bpm for 1 to 6y, 60 bpm for 6 to 14y
//	//Code 1-0-2 = C1 and (C2 or C3)
//	return 0;
//}
//short CvhCode::vhCode103()	// Sinus tachycardia (ST) 
//{
//	//C1: Basic rhythm sinus (code 1-0-0) 
//	//C2: Ventricular rate ≥ 95 bpm for adults
//	//C3: Ventricular rate ≥ upper limits for children. the upper limits for different age as follows,
//	//140 bpm for < 1y, 120 bpm for 1 to 6y, 100 bpm for 6 to 14y
//	//Code 1-0-3 = C1 and (C2 or C3)
//	return 0;
//}
//short CvhCode::vhCode104()	// Sinus arrhythmia (SAR) 
//{
//	//C1: Basic rhythm sinus (code 1-0-0) 
//	//C2: Ventricular rate variance ratio ≥ 10% for adults
//	//C3: Difference of ventricular rate ≥ 120 ms for children
//	//Code 1-0-4 = C1 and (C2 or C3)
//	//Supplementary Codes to Sinus Rhythm(1-0-10 to 1-0-60)
//	return 0;
//}
short CvhCode::vhCode101x()//1010,1011,1012,1013,1016
{
	if(vh.found(vhcode(1,0,1)) || vh.found(vhcode(1,0,2)) || vh.found(vhcode(1,0,3))) {
		short i,j,n[10]={0,0,0,0,0,0,0,0,0,0};
		short step[10]={1,7,7,5,4,3,4,5,6,1};	//the length of each jV
		short beatsnum=GetBeatsNum();
		char *jA[10]={"S","NNSNNSN","NSSNSSN","NSNSNS","NSSN","SSS","SSSS","SSSSS","SSSSSS","1"};
		for(i=0;i<10;i++) {
			j=step[i];
			char *p=GetBeatsType();
			while(p && j<=beatsnum) {
				p=strstr(p,jA[i]);
				if(p) {
					n[i]++;
					p+=step[i];	j+=step[i];
				}
			}
			if(n[0]==0) break;
		}
		unsigned short code=0;
		if(n[0]>0) {
			if(n[1]>0 || n[2]>0) code=vhcode(1,0,12);	//1012;
			else if(n[3]>0) {
				code=vhcode(1,0,11);	//1011;
			}
			else if(n[4]>0) code=vhcode(1,0,13);	//1013;
			else if(n[8]>0) code=vhcode(1,0,18);	//Runs of PAC>5
			else if(n[5]>0 || n[6]>0 || n[7]>0) code=vhcode(1,0,17);	//Runs of PAC 3-5
			else {
				if(n[0]>1) {
					if(m_seconds>0) {
						if(n[0]*10>m_seconds) code=vhcode(1,0,16);	//1016;
					}
					else code=vhcode(1,0,16);	//1016;
				}
				if(code==0) code=vhcode(1,0,10);	//1010;
			}
			SetvhCode(code,0);
			return code;
		}
	}
	return 0;
}
//short CvhCode::vhCode1010()	// Ectopic (ectosinal) supraventricular complexes (ESVC)
//{
//	//C1: Code 1-0-1 or Code 1-0-2 (NSR or SB)
//	//C2: QRS morphology matches sinal QRS complexes (QRS duration within 20 ms and QRS wave labeling same as for sinal QRS)
//	//C3: P amplitude differs by more than 100 µV from sinal P waves, or retrograde P or no P wave discernible
//	//C4: PR ≥ 40 ms shorter than PR of sinal QRS complexes
//	//C5: R-R of the early complex 200 ms shorter than the preceding R-R and ≥ 240 ms shorter than the R-R following the ectopic complex
//	//Code 1-0-10 = (C1 and C2) and (C3 or C4 or C5)
//	//Note: C5 reﬂects the minimum expected combined effect of the ectopic complex on the prematurely (R-R preceding) and SA node suppression (R-R following).
//
//	return 0;
//}
//short CvhCode::vhCode1011()	// Atrial bigeminy (ABG) 
//{
//	//C1: Code 1-0-10
//	//C2: Ectopic complex follows every sinal QRS complex
//	//Code 1-0-11 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode1012()	// Atrial trigeminy (ATG) 
//{
//	//C1: Code 1-0-10
//	//C2: Ectopic complex after every pair of sinal QRS complexes
//	//C3: A pair of ectopic complex after every sinal QRS complexes
//	//Code 1-0-12 = C1 and (C2 or C3)
//	return 0;
//}
//short CvhCode::vhCode1013()	//  A doublet of ectopic atrial complexes (DEAC)
//{
//	//C1: Code 1-0-10
//	//C2: Two EACs in succession within one R-R interval of sinal QRS complexes
//	//Code 1-0-13 = C1 and C2
//	return 0;
//}
short CvhCode::vhCode1020()	// Aberrant supraventricular complexes (ASVC)
{
	//C1: Code 1-0-1 or Code 1-0-2 (NSR or SB)
	//C2: QRS ≥ 20 ms longer than normally conducted sinal QRS 
	//C3: P wave precedes wide QRS complex
	//C4: RSR2 in V1, with R2 > R
	//C5: QRSd ≤ 140 ms
	//Code 1-0-20 = C1 and C2 and [C3 or (C4 and C5)]
	//cannot be detected
//	if(vh.found(vhcode(1,0,10))) return 0;
//	bool c1=vh.found(vhcode(1,0,1)) || vh.found(vhcode(1,0,2))?true:false;
//	bool c2=false,c3=false;
//	bool c4=(uvR2(V1)>uvR1(V1))?true:false;
//	bool c5=false;//(msQRS()<=140)?true:false;
//	if(c1) {
//		VH_ECGbeat *beats=GetBeats();
//		int i,n=GetBeatsNum();
//		for(i=0;i<n;i++) {
//			if((beats[i].QRSw-msQRS()>=25 && beats[i].SubQRSw-msQRS()>=25) && !beats[i].Status) {
//				c2=true;
//				if(beats[i].Pnum>0) c3=true;
//				if(beats[i].QRSw<140 && beats[i].SubQRSw<140) c5=true;
//			}
//			if(c2 && c3) break;
//		}
//		if(c2 && c3 && c5) {
////		if(c2 && (c3 || (c4 && c5))) {
//			unsigned short code=vhcode(1,0,20);	//1020;
//			SetvhCode(code,0);
//			return code;
//		}
//	}
	return 0;
}
short CvhCode::vhCode103x()	//1030,1031,1032,1033,1034,1035,1036
{
	short i,j,n[11]={0,0,0,0,0,0,0,0,0,0,0};
	short step[11]={1,8,7,5,4,3,4,5,6,1,2};	//the length of each jV
	short beatsnum=GetBeatsNum();
	char *jA[11]={"V","VNNVNNVN","NVVNVVN","NVNVN","NVVN","VVV","VVVV","VVVVV","VVVVVV","N","VV"};
	for(i=0;i<11;i++) {
		char *p=GetBeatsType();
		j=step[i];
		while(p && j<=beatsnum) {
			p=strstr(p,jA[i]);
			if(p) {
				n[i]++;
				p+=step[i];	j+=step[i];
			}
		}
		if(n[0]==0) break;
	}
	if(n[0]==0) return 0;
	if(n[0]>1) {
		if(isPolymorphicPVC()) SetCriticalValue(CV_MPVC);
		short value=(QRS>=120)?JTI:QTI;
		if(value>116) {
			SetCriticalValue(CV_FPVC);
		}
	}

	unsigned short code=0;
	VH_ECGbeat *beats=GetBeats();
	if(n[9]>0) {
		if(n[1]>0 || n[2]>0) code=vhcode(1,0,35);	//1035;//三联律
		else if(n[3]>0 || n[4]>0) {
			if(n[3]>0) {code=vhcode(1,0,34);	SetvhCode(code,0);}//1034;//二联律
			if(n[4]>0) code=vhcode(1,0,31);	//1031;//成对
		}
		else if(n[8]>0) {
			code=vhcode(1,0,38);	//Runs of PVC>5
			SetCriticalValue(CV_VT);
		}
		else if(n[5]>0 || n[6]>0 || n[7]>0) {
			code=vhcode(1,0,37);	//Runs of PVC 3-5
			SetCriticalValue(CV_VT);
		}
		else {
			i=0;
			char *p=GetBeatsType();
			while(p && i<beatsnum) {
				if(*p=='V') {
					if(i>0) {
						short rr=short(1000*(beats[i].Pos-beats[i-1].Pos)/m_fs);
						if(rr<msQT()-15) {
							code=vhcode(1,0,32);	//1032;//RonT
							SetCriticalValue(CV_MPVC);
							break;
						}
					}
				}
				p++;	i++;
			}
			if(n[0]>1) {
				//1-0-33 undetected	多形室早
				if(m_bPolymorphicPVC) {
					code=vhcode(1,0,33);
					SetCriticalValue(CV_MPVC);
				}
				else {//if no 1-0-33 else
					if(m_seconds>0) {
						if(n[0]*11>m_seconds) code=vhcode(1,0,36);	//1036;
					}
					else code=vhcode(1,0,36);	//1036;
				}
			}
			if(code==0)	code=vhcode(1,0,30);	//1030;//室早
		}
		SetvhCode(code,0);
		return code;
	}
	return 0;
}
//short CvhCode::vhCode1030()	// Ectopic ventricular complexes (EVC) 
//{
//	//C1: QRS ≥ 120 ms for adults or 100ms for children or QRS ≥ 20 ms longer than normally conducted QRS complexes.
//	//C2: Criteria for Code 1-0-20 not met
//	//Code 1-0-30 = C1 and C2
//	//Note 1: Code 1-0 S 30 includes interpolated ectopic ventricular complexes
//	//Note 2: QRS fusion complexes (preceding sinal P occurring with normal timing in the P wave train) are counted as ectopic ventricular complexes.
//
//	return 0;
//}
//short CvhCode::vhCode1031()	// A doublet of ectopic ventricular complexes (DEVC)
//{
//	//C1: Code 1-0-30
//	//C2: Two EVCs in succession within one R-R interval of sinal QRS complexes
//	//Code 1-0-31 = C1 and C2
//	//Note: A triplet of EVCs is coded under Code 1-7-0 (VT).
//	return 0;
//}
//short CvhCode::vhCode1032()	// Coalescent ventricular ectopic complexes (CEVC) 
//{
//	//C1: Ectopic ventricular complex overlaps the ST-T of the preceding QRS-T complex
//	//Code 1-0-32 = C1
//	return 0;
//}
//short CvhCode::vhCode1033()	// Polymorphic ectopic ventricular complexes (PEVC)
//{
//	//C1: QRSa (net QRS amplitudes) of EVCs differ by ≥ 50% 
//	//C2: QRS durations of EVCs differ by ≥ 20 ms
//	//Code 1-0-33 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode1034()	// Ventricular bigeminy (VBG) 
//{
//	//C1: Code 1-0-30
//	//C2: Ectopic complex follows every sinal QRS complex
//	//Code 1-0-34 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode1035()	// Ventricular trigeminy (VTG) 
//{
//	//C1: Code 1-0-30
//	//C2: Ectopic complex after every pair of sinal QRS complexes
//	//C3: A pair of ectopic complex after every sinal QRS complexes
//	//Code 1-0-35 = C1 and (C2 or C3)
//	return 0;
//}
//short CvhCode::vhCode1036()	// Frequent ventricular ectopic complexes 
//{
//	//C1: Code 1-0-30
//	//C2: more than one ectopic complex per 10 seconds
//	//Code 1-0-36 = C1 and C2
//	return 0;
//}
short CvhCode::vhCode1040()	// Pause (possible sinoatrial arrest or block) 
{
	//C1: Code 1-0-0
	//C2: P-P interval containing pause prolonged ≥ 90% compared with median P-P of sinal P waves
	//C3: No P wave in the prolonged P-P interval until the next PQRS complex
	//C4: Preceding PQRS complex not an ectopic supraventricular or ventricular complex
	//Code 1-0-40 = C1 and C2 and C3 and C4
	if(mc.found(mccode(8,5,1)) || mc.found(mccode(8,5,2))) {
		unsigned short code=vhcode(1,0,40);	//1040;
		SetvhCode(code,0);
		int i,beatsnum=GetBeatsNum(),s2=2*m_fs,rr;
		VH_ECGbeat *beats=GetBeats();
		for(i=1;i<beatsnum;i++)  {
			rr=beats[i].Pos-beats[i-1].Pos;
			if(rr>=s2) {
				SetCriticalValue(CV_PAUSE);	break;
			}
		}
		return code;
	}
	return 0;
}
short CvhCode::vhCode1050()	// Reduced heart rate variability (RHRV) 
{
	//C1: Code 1-0-1 (normal sinus rhythm)
	//C2: R-R interval variation range of normally conducted QRS complexes <40 ms, excluding complexes following ectopic (ventricular or supraventricular) complexes (interpolated or with compensatory pause)
	//Code 1-0-50 = C1 and C2
	//Note: With computer measurements of all R-R intervals of normally conducted QRS complexes (N-N intervals) available, Code 1-0-50 = SDNN ≤ 5 ms, where SDNN is the standard deviation of N-N intervals.
	if(vh.found(vhcode(1,0,1))) {
		if(m_SDNN>=0 && m_SDNN<=3) {
			unsigned short code=vhcode(1,0,50);	//1050;
			SetvhCode(code,0);
			return code;
		}
	}
	return 0;
}
short CvhCode::vhCode1060()	// Increased heart rate variability (IHRV)
{
	//C1: Code 1-0-1 (normal sinus rhythm)
	//C2: Largest successive difference of R-R intervals of normally conducted QRS complexes > 100 ms excluding complexes following ectopic (ventricular or supraventricular) complexes (interpolated or with compensatory pause)
	//Code 1-0-60 = C1 and C2
	//Note 1: With computer measurements of all R-R intervals of normally conducted QRS complexes (N-N intervals) available, 1-0-60 = SDNN ≥30 ms, where SDNN is the standard deviation of N-N intervals.
	//Note 2: if code 1-0-4 Sinus arrhythmia (SAR) is present, code 1-0-60 not reported.
	if(vh.found(vhcode(1,0,1))) {
		if(vh.found(vhcode(1,0,4))==NULL) {
			if(m_SDNN>=50) {
				unsigned short code=vhcode(1,0,60);	//1060;
				SetvhCode(code,0);
				return code;
			}
		}
	}
	return 0;
}

short CvhCode::vhCode110()	// Wandering atrial pacemaker (WAP) 
{
	//C1: Presence of more than one P wave trains with three or more P waves in each and with P amplitudes changing by 100 µV or more
	//Code 1-1-0 = C1
	//Note: Sinus rhythm with segments of transient ectopic atrial or junctional rhythm (deﬁned below) is  coded as WAP
	DWORD dwLeads=0;
	return vhCodeFrommcCode(mccode(8,1,4),vhcode(1,1,0),dwLeads);
}
short CvhCode::vhCode12x()	//120,121,122,123
{
	short bpm=bpmHR();
	if(uvP(aVR)>0 && uvP(I)>=0 && bpm<90) {
		unsigned short code=vhcode(1,2,0);	//1200;
		if(bpm<45) code=vhcode(1,2,2);	//1202;
		else if(bpm<65) code=vhcode(1,2,1);	//1201;
		else if(bpm<90) code=vhcode(1,2,3);	//1203;
		SetvhCode(code,0);
		return code;
	}
	return 0;
}
//short CvhCode::vhCode120()	// Junctional rhythm (JR) 
//{
//	//C1: PaVR > 0 µV and PI ≥ 0 µV
//	//C2: PR≤ 120 ms and PR variation < 20 ms
//	//C3: Retrograde P waves
//	//C4: No P waves identiﬁable and no atrial ﬂutter or ﬁbrillation waves
//	//C5: R-Rmax - R-R min ≤ 40 ms
//	//Code 1-2-0 = (C1 and C2) or ([C3 or C4] and C5)
//	//Note 1: Criterion C1 implies a P axis from -60° to -90°.
//	//Note 2: C1 with PR > 120 ms (possible coronary sinus rhythm or JR with delayed antegrade conduction) is coded under code 1-3 (EAR).
//	//Note 3: For C5, exclude possible competing ectopic or sinus complexes.
//	//Note 4: Atrial ﬁbrillation with junctional rhythm is coded under Code 1-5-3 (atrial ﬁbrillation).
//	return 0;
//}
//short CvhCode::vhCode121()	// Junctional rhythm, rate 45-64 bpm
//{
//	//C1: Code 1-2-0
//	//C2: Ventricular rate 45-64 bpm
//	//Code 1-2-1 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode122()	//  Junctional bradycardia(JBR) 
//{
//	//C1: Code 1-2-0
//	//C2: Ventricular rate < 45 bpm
//	//Code 1-2-2 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode123()	//  Accelerated junctional rhythm (AJR) 
//{
//	//C1: Code 1-2-0
//	//C2: Ventricular rate from 65 bpm and < 89 bpm
//	//Code 1-2-3 = C1 and C2
//	//Note: Code as narrow QRS tachycardia (Code 1-4) if rate ≥ 90 bpm.
//	return 0;
//}
short CvhCode::vhCode13x()	// 130,131,132,133,1311,134
{
	unsigned short code=0;
	if(vh.found(vhcode(1,2,0)) || vh.found(vhcode(1,2,1)) || vh.found(vhcode(1,2,2)) || vh.found(vhcode(1,2,3))) return 0;
	short bpm=bpmHR();
	if(uvP(II)<0 && uvP(aVR)>0) {
		code=vhcode(1,3,0);	//1300;
		if(uvP(V1)>0 && uvNegaP(V1)==0 && (uvP(V5)<0 || uvP(V6)<0)) code=vhcode(1,3,11);	//1311;
		if(bpm<50) code=vhcode(1,3,2);	//1302;
		else if(bpm<90) code=vhcode(1,3,1);	//1301;
		else if(bpm>=90) code=vhcode(1,3,3);	//1303;
		SetvhCode(code,0);
		return code;
	}
	return 0;
}
//short CvhCode::vhCode130()	// Ectopic atrial rhythm (EAR) 
//{
//	//C1: PII < 0 µV
//	//C2: PaVR > 0 µV
//	//C3: No Code 1-2 (JR)
//	//Code 1-3 = (C1 or C2) and C3
//	//Note: Criterion C1 or C2 implies a P axis < -30° or > 120° .
//	return 0;
//}
//short CvhCode::vhCode131()	// Ectopic atrial rhythm, rate 50-90 bpm
//{
//	//C1: Code 1-3-0
//	//C2: Ventricular rate 50-89 bpm
//	//Code 1-3-1 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode132()	// Ectopic atrial bradycardia (EABR) 
//{
//	//C1: Code 1-3-0
//	//C2: Ventricular rate < 50 bpm
//	//Code 1-3-2 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode133()	// Ectopic atrial tachycardia (EAT) 
//{
//	//C1: Code 1-3-0
//	//C2: Ventricular rate ≥ 90 bpm
//	//Code 1-3-3 = C1 and C2
//	//Supplementary Codes to Ectopic Atrial Rhythm(1-3-11)
//	return 0;
//}
//short CvhCode::vhCode1311()	//  Probably left atrial ectopic focus
//{
//	//C1: Code 1-3-0
//	//C2: PI < 0
//	//C3: PV1 > 0 and PPV1 ≥ 0
//	//Code 1-3-11 = C1 and C2 and C3
//	//Note: C3 implies that PV1 is positive of biﬁd.
//	return 0;
//}
short CvhCode::vhCode134()	// Coronary Sinus Rhythm (CSR) 
{
	//C1: MC-code 8-4-1
	//Code 1-3-4 = C1
	DWORD dwLeads=0;
	unsigned short code=vhCodeFrommcCode(mccode(8,4,1),vhcode(1,3,4),dwLeads);
	return code;
}
short CvhCode::vhCode14x()	//140,141,142,143
{
	VH_ECGbeat *beats=GetBeats();
	char *p=GetBeatsType();
	short beatsnum=GetBeatsNum();
	int i=0,s=0,rr=0;
	double srr=0,srrmin=32767;
	bool svt=false;
	CMean mean;
	while(p && i<beatsnum) {
		if(*p=='S') {
			if(s>0) {
				srr=mean.mean(beats[i].Pos-beats[i-1].Pos);
			}
		}
		else {
			if(s>5) {
				if(srrmin>srr) srrmin=srr;
				svt=true;
			}
			s=0;	mean.init();
		}
		p++;	i++;
	}
	if(svt) {
		unsigned short code=0;
		int hr=(int)HRfromSamples((int)srrmin);
		if(m_ageY<4) {
			if(hr>=230) code=vhcode(1,4,3);	//1403;
		}
		else if(m_ageY<14) {
			if(hr>=180) code=vhcode(1,4,3);	//1403;
		}
		else {
			if(hr>=130) code=vhcode(1,4,2);	//1402;
			else if(hr>=95) code=vhcode(1,4,1);	//1401;
		}
		if(code>0) {
			SetvhCode(code,0);
			return code;
		}
	}
	return 0;
}
//short CvhCode::vhCode140()	// Supraventricular (SVT) or narrow QRS tachycardia
//{
//	//Criteria(adults)
//	//C1: Six or more successive supraventricular ectopic complexes, with QRS < 120 ms
//	//C2: Ventricular rate ≥ 95 bpm
//	//Code 1-4-0 = C1 and C2
//	//Note: If fewer than six ectopic complexes, include them in supraventricular ectopic complex count.
//	return 0;
//}
//short CvhCode::vhCode141()	// Supraventricular tachycardia, rate < 130 bpm
//{
//	//Criteria(adults)
//	//C1: Code 1-4-0
//	//C2: Ventricular rate during episode < 130 bpm
//	//Code 1-4-1 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode142()	// Supraventricular tachycardia, rate ≥ 130 bpm
//{
//	//Criteria(adults)
//	//C1: Code 1-4-0
//	//C2: Ventricular rate during episode ≥ 130 bpm
//	//Code 1-4-2 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode143()	// Supraventricular tachycardia
//{
//	//Criteria(children)
//	//C1: Code 1-4-0
//	//C2: Ventricular rate during episode ≥ 180 bpm for ≥ 4 years of age
//	//C3: Ventricular rate during episode ≥ 230 bpm for < 4 years of age
//	//Code 1-4-3 = C1 and (C2 or C3)
//	return 0;
//}
short CvhCode::vhCode150()	//  Atrial ﬂutter or ﬁbrillation (AFLF) 
{//see code 0-5-0
	//Screening criteria
	//C1: No P waves present
	//C2: Flutter (F) waves ≥ 100 µV peak to peak with repetitive, regular morphology, present in lead II, aVF or V1
	//C3: Fibrillation (f) waves or F waves with irregular cycle intervals or amplitudes,
	//in II, III, or aVF
	//C4: R-R intervals irregular (fewer than three R-R within 40-ms class interval) 
	//Code 1-5-0 = C1 and (C2 or C3 or C4 or C5)
	return 0;
}
short CvhCode::vhCode151()	//  Atrial ﬂutter type 1 (AFL1) 
{//see code 0-5-0
	//C1: Five or more R-R intervals, each with F wave amplitudes ≥ 100 µV peak to peak
	//C2: F ≤ 333 bpm (F cycle interval ≥ 180 ms) (i.e., ≥ 450 µV at 2500 µV/second) 
	//C3: At least partial regularity of R-R intervals, with three or more R-R intervals within 40 ms of each other (i.e., within two adjacent class intervals of 20 ms); if ventricular rate ≥ 100 bpm, four or fewer R-R intervals are required to be within 40 ms of each other
	//Code 1-5-1 = C1 and C2 and C3
	return 0;
}
short CvhCode::vhCode152()	//  Atrial ﬂutter type 2 (AFL2) 
{//see code 0-5-0
	//C1: F waves ≥ 100 µV peak to peak sustained for 5 or more R-R intervals
	//C2: F > 333 bpm and < 430 bpm (F cycle interval 141–179 ms, i.e., 3.6-4.400 µV at 2500 µV/second
	//Code 1-5-2 = C1 and C2
	//Note: In type 2 atrial ﬂutter at F > 350 bpm, F wave morphology tends to become irregular in amplitude and in cycle length, and ventricular rate often does not meet the partial regularity criteria characteristics of atrial ﬂutter type 1.
	return 0;
}
short CvhCode::vhCode153()	//  Atrial ﬁbrillation (AF) 
{//see code 0-5-0
	//C1: Code 1-5-0
	//C2: Criteria for codes 1.5.1 and 1.5.2 not met
	//Code 1-5-3 = C1 and C2
	//Note: Supplementary  code  for  atrial  ﬁbrillation  with  probable  junctional rhythm is deﬁned under Code 1-5-30 – 1-5-31.
	//Supplementary Codes to Atrial Fibrillation(1-5-30 to 1-5-32)
	return 0;
}
short CvhCode::vhCode153x()	// 1530,1531,1532
{
	if(vh.found(vhcode(1,5,1)) || vh.found(vhcode(1,5,3))) {
		int i,beatsnum=GetBeatsNum();
		VH_ECGbeat *beats=GetBeats();
		char *t=GetBeatsType();
		short RR0=0,RR=0,difRR=0,difRRmax=0;
		double meanbpm=0;
		CMean mean;
		for(i=1;i<beatsnum;i++) {
			if((t[i]=='N' || t[i]=='S') && (t[i-1]=='N' || t[i-1]=='S')) {
				RR=short(beats[i].Pos-beats[i-1].Pos);
				meanbpm=mean.mean(RR);
				if(RR0>0) {
					difRR=RR-RR0;	if(difRRmax<difRR) difRRmax=difRR;
				}
				RR0=RR;
			}
		}
		if(difRRmax>0 && meanbpm>0) {
			difRRmax=msfromSamples(difRRmax);
			meanbpm=HRfromSamples(short(meanbpm));

			unsigned short code=0;
			if(difRRmax<40) {
				code=vhcode(1,5,33);	//1533;
			}
			if(meanbpm<50) code=vhcode(1,5,31);	//1531;
			else if(meanbpm>95) code=vhcode(1,5,32);	//1532;
			else if(code!=0) {
				SetvhCode(code,0);	return code;
			}
		}
	}
	return 0;
}
//short CvhCode::vhCode1530()	// Atrial ﬁbrillation with AV dissociation and junctional rhythm
//{
//	//C1: Code 1-5-3
//	//C2: R-R interval variation range < 40 ms
//	//Code 1-5-30 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode1531()	// Atrial ﬁbrillationwith slow ventricular response
//{
//	//C1: Code 1-5-3
//	//C2: Ventricular rate < 50 bpm
//	//Code 1-5-31 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode1532()	// Atrial ﬁbrillation with rapid ventricular response
//{
//	//C1: Code 1-5-3
//	//C2: Ventricular rate > 95
//	//Code 1-5-32 = C1 and C2
//	return 0;
//}
short CvhCode::vhCode154()	// Atrial ﬁbrillation/ﬂutter with possible dominant AV conduction
{//	undetected
	//C1: Fibrillation waves or four or less R-R intervals, each with F waves ≥ 100µV peak to peak
	//C2: Possible dominant AV conduction present, with R-R within two adjacent class intervals of 20 ms (i.e., within 40 ms from each other). If mean R-R≤ 600 ms, four or more R-R intervals within 40 ms required to deﬁne possible dominant conduction.
	//Code 1-5-4 = C1 and C2
	return 0;
}
short CvhCode::vhCode16x()	// 160,161,162
{
	if(mc.found(mccode(6,8,0))) {
		short SpikesN;
		char PaceMaker;
		long *SpikesPos=GetPaceMaker(PaceMaker,SpikesN); //'N': none, 'A': A-Type, 'V': V-type, 'B': Both
		unsigned short code=0;
		switch(PaceMaker) {
			case 'A':	code=vhcode(1,6,1);	//1601;
			case 'V':	case 'B':	code=vhcode(1,6,2);	//1602;
		}
		if(code>0) {
			SetvhCode(code,0);	return code;
		}
	}
	return 0;
}
//short CvhCode::vhCode160()	// Electronic pacemaker (PM)
//{
//	return 0;
//}
//short CvhCode::vhCode161()	// Ventricular pacemaker (VPM) or dual chamber pacemaker (CPM) 
//{
//	//C1: Coupled pacemaker spikes with spike–spike interval ≥ 80 ms
//	//C2: Single pacemaker spikes
//	//C3: QRS ≥ 120 ms
//	//Code 1-6-1 = (C1 and C3) or (C2 and C3)
//	return 0;
//}
//short CvhCode::vhCode162()	// Atrial electronic pacemaker (APM) 
//{
//	//C1: Single pacemaker spikes >80 ms before QRS complex, preceding P waves
//	//C2: QRS < 120 ms
//	//Code 1-6-2 = C1 and C2
//	return 0;
//}
short CvhCode::vhCode170_2()	//170,171,172
{
	VH_ECGbeat *beats=GetBeats();
	char *p=GetBeatsType();
	int i=0,v=0,rr=0;
	double vrr=0,vrrmin=32767;
	short beatsnum=GetBeatsNum();
	short vt=0;
	CMean mean;
	while(p && i<beatsnum) {
		if(*p=='V' || (*p=='N' && beats[i].Pnum<1 && beats[i].QRSw>=120)) {
			if(v>0) {
				vrr=mean.mean(beats[i].Pos-beats[i-1].Pos);
			}
		}
		else {
			if(v>2) {
				if(vrrmin>vrr) vrrmin=vrr;
				vt++;
			}
			v=0;	mean.init();
		}
		p++;	i++;
	}
	if(vt*3>beatsnum*2) {
		unsigned short code=0;
		int hr=HRfromSamples((int)vrrmin);
		if(hr<=40) code=vhcode(1,7,1);	//1701;
		else if(hr<130) code=vhcode(1,7,2);	//1702;
		else {
			code=vhcode(1,7,0);	//1700;
			SetCriticalValue(CV_VT);
		}
		if(code>0) {
			SetvhCode(code,0);
			return code;
		}
	}
	return 0;
}
//short CvhCode::vhCode170()	// Ventricular tachycardia (VT) 
//{
//	//C1: Three or more successive ventricular ectopic complexes with rate ≥ 130 bpm and < 250 bpm
//	//Code 1-7-0 = C1
//	//Note: Other ventricular ectopic complexes counted separately (under continuous measurements and counts).
//	return 0;
//}
//short CvhCode::vhCode171()	// Ventricular escape rhythm (VER) 
//{
//	//C1: P wave has no accompany QRS complex
//	//C2: QRS > 110 ms
//	//C3: Ventricular rate ≤ 40 bpm
//	//Code 1-7-1 = C1 and C2 and C3
//	//Note: Code 2-3-x, Third-degree (complete) AV block (AVB3) suppress code 1-7-1.
//	return 0;
//}
//short CvhCode::vhCode172()	// Accelerated idioventricular rhythm (AIVR) 
//{
//	//C1: P wave cannot be detected.
//	//C2: QRS ≥ 120 ms
//	//C3: Ventricular rate > 40 bpm and < 130 bpm
//	//Code 1-7-2 = C1 and C2 and C3
//	return 0;
//}
short CvhCode::vhCode173_4()	//173,174
{
	unsigned short code=0;
	if(msQRS()>=120 && bpmHR()>=250) code=vhcode(1,7,3);	//1703;
	if(mc.found(mccode(8,2,1))) code=vhcode(1,7,4);
	if(code==0) {
		short i,VF=0,Vf=0,nos=0;
		for(i=0;i<m_chnum;i++) {
			switch(m_infoLead[i].quality) {
				case 1:	VF++;	break;
				case 2: Vf++;	break;
				case 3: nos++;	break;
			}
		}
		if(VF>0) {
			code=vhcode(1,7,3);	//1703;
		}
		else if(Vf>0) {
			code=vhcode(1,7,4);	//1704;
		}
	}
	if(code>0) {
		SetvhCode(code,0);
		SetCriticalValue(CV_VF);
		return code;
	}
	return code;
}
//short CvhCode::vhCode173()	// Ventricular Flutter (VF) 
//{
//	//C1: P wave cannot be detected.
//	//C2: QRS ≥ 120 ms and T wave cannot be distinguish from QRS complex
//	//C3: Ventricular rate ≥ 250 bpm and ≤ 350 bpm
//	//Code 1-7-3 = C1 and C2 and C3
//	//Note: If ventricular rate < 250, then say 1-7-0, VT.
//	return 0;
//}
//short CvhCode::vhCode174()	// Ventricular Fibrillation (Vf) 
//{
//	//C1: P wave cannot be detected.
//	//C2: QRS complex cannot be detected clearly
//	//C3: Ventricular rate is disorder
//	//C4: The peak to peak amplitude more than one lead ≥ 100 µV
//	//Code 1-7-4 = C1 and C2 and C3
//	return 0;
//}
short CvhCode::vhCode181()//1-8-1 Extreme Tachycardia(ExT)
{
//Criteria
//C1: Ventricular rate ≥ 120 bpm for age > 18 or
//Ventricular rate ≥ 140 bpm for age > 11 or
//Ventricular rate ≥ 160 bpm for other age
//C2: No other rhythm rrhythmia
//Code 1-8-1 = C1 and C2
	unsigned short code=0;
	if(m_ageY>18) {
		if(bpmHR()>=120) code=vhcode(1,8,1);
	}
	else if(m_ageY>11) {
		if(bpmHR()>=140) code=vhcode(1,8,1);
	}
	else {
		if(bpmHR()>=160) code=vhcode(1,8,1);
	}
	if(code) SetvhCode(code,0);
	return code;
}
short CvhCode::vhCode182()//1-8-2 Extreme Bradycardia(ExB)
{
//Criteria
//C1: Ventricular rate ≤ 40 bpm for age > 11 or
//Ventricular rate ≤ 45 bpm for other age
//C2: No other rhythm rrhythmia
//Code 1-8-2 = C1 and C2
	unsigned short code=0;
	if(m_ageY>11) {
		if(bpmHR()<=40) code=vhcode(1,8,2);
	}
	else {
		if(bpmHR()<=45) code=vhcode(1,8,2);
	}
	if(code) SetvhCode(code,0);
	return code;
}
short CvhCode::vhCode190()	// Other abnormal rhythm classiﬁcation
{
	//C1: No other rhythm code
	//Code 1-9-0 = C1
//	if no other 1-x-x then
	unsigned short code=vhcode(1,9,0);	//1900;
	SetvhCode(code,0);
	return code;
}

//3.3.	Code 2, Atrioventricular Conduction Abnormalities
void CvhCode::vhCode2()
{
	if(vhCode240()) return;
	if(!vhCode231x()) {
		if(!vhCode220() && !vhCode221() && !vhCode222() && !vhCode223()) vhCode210();
	}
	vhCode250();
}
short CvhCode::vhCode210()	//  First-degree AV block (AVB1) 
{
	//C1: PR ≥ 220 ms for adults or ≥ 180 ms for children
	//C2: P-R intervals within 40-ms class interval (excluding ectopic complexes)
	//Code 2-1-0 = C1 and C2
	if(vh.found(vhcode(1,8,1))) return 0;
	unsigned short code=0;
	short PR=msPR();
	if(m_ageY>=14) {
		if(PR>=210) code=vhcode(2,1,0);	//2100;
	}
	else {
		if(PR>=180) code=vhcode(2,1,0);	//2100;
	}
	if(code>0) SetvhCode(code,0);
	return code;
}

short CvhCode::vhCode220()	//  Second-degree AV block (AVB2) 
{
	//C1: Occurrence of one or more blocked P waves within the R-R interval of conducted P waves
	//Code 2-2 = C1
	return 0;
}
short CvhCode::vhCode221()	//  Second-degree AV block type Wenckebach or Mobitz 1 (AVB2W) 
{
	//C1: Repetitive cycles of progressive prolongation of PR followed by a blocked P wave
	//Code 2-2-1 = C1
	DWORD dwLeads=0;
	return vhCodeFrommcCode(mccode(6,2,3),vhcode(2,2,1),dwLeads);
}
short CvhCode::vhCode222()	//  Second-degree singular AV block or type Mobitz 2 (AVB2S) 
{
	//C1: Singular blocked P at variable or ﬁxed (1:2, 1:3, etc.) block ratio
	//Code 2-2-2 = C1
	//Note 1: Singular blocked P wave equal to no more than one blocked P wave within any R-R interval. Blocked P is in a “regular” P wave train (rather than an early premature P within the refractory period of AV node).
	//Note 2: Block ratio for the second degree singular AV block is the ratio of blocked P waves to conducted P waves. Block ratio = 1:x, where x is the number of conducted P waves following the blocked P wave. Second-degree singular AV block with 1:1 block ratio is often con- fusingly called 2:1 AV block, and with a 1:2 block ratio, it is called 3:1 AV block.
	//Note 3: Second-degree singular AV block with 1:1 block ratio may be a Wenck- ebach block that is revealed only if the block ratio changes to 1:2 or 1:3. A true Mobitz 2 second-degree AV block is commonly associated with a complete bundle branch block or with bifascicular block.
	short code=0;
	if(mc.found(mccode(6,2,1)) || mc.found(mccode(6,2,2))) {
		code=vhcode(2,2,2);
		SetvhCode(code,0);
		SetCriticalValue(CV_AVB);
	}
	return code;
}
short CvhCode::vhCode223()	//  Second-degree multiple AV block (AVB2M) 
{
	//C1: Two or more blocked P waves within R-R interval of conducted atrial complexes
	//Code 2-2-3 = C1
	DWORD dwLeads=0;
	if(vhCodeFrommcCode(mccode(6,2,1),vhcode(2,2,3),dwLeads)) {
		SetCriticalValue(CV_AVB);
		return vhcode(2,2,3);
	}
	else return 0;
}

short CvhCode::vhCode231x()	//231,2330,2331,2332
{
	unsigned short code=0;
	if(mc.found(mccode(6,1,0))) {
		if(msQRS()<120) {
			code=vhcode(2,3,30);	//2330;
		}
		else {
			if(bpmHR()<=45) code=vhcode(2,3,32);	//2332;
			else code=vhcode(2,3,31);	//2331;
		}
		SetvhCode(code,0);
		SetCriticalValue(CV_AVB);
	}
	return code;
}
//short CvhCode::vhCode231()	//  Third-degree (complete) AV block (AVB3) 
//{
//	//C1: P wave train and QRS wave train independent (with no consistent relationship) for the majority of the complexes
//	//C2: R-R intervals of the independent QRS complexes regular within 100 ms, with QRS duration varying less than 10 ms
//	//Code 2-3-0 = C1 and C2
//	//Note 1: Second-degree AV blocks can also be considered to present a form of AV dissociation.
//	//Note 2: AV dissociation with atrial ﬂutter is coded under Code 1-5-1.
//	//C1: Criteria for Code 2-3-0 persist throughout the record
//	//Code 2-3-1 = C1
//	//Note 1: Ventricular rate is usually slower than atrial rate.
//	//Note 2: If several types of AV blocks coexist in the same record, code the highest block.
//	//Supplementary Codes to AV Dissociation(2-3-30 to 2-3-32)
//}
//short CvhCode::vhCode2330()	// Third-degree (complete) AV block with narrow QRS complex
//{
//	//C1: Code 2-3-1
//	//C2: QRS < 120 ms (in the majority group) 
//	//Code 2-3-30 = C1 and C2
//	//Note: Code 2-3-30 indicates that the likely block site is the AV node or His bundle.
//	return 0;
//}
//short CvhCode::vhCode2331()	// Third-degree (complete) AV block with wide QRS complex
//{
//	//C1: Code 2-3-1
//	//C2: QRS ≥ 120 ms (in the majority group) 
//	//Code 2-3-31 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode2332()	// Third-degree (complete) AV block with wide QRS complex and slow ventricular rate
//{
//	//C1: Code 2-3-31
//	//C2: Ventricular rate ≤ 45 bpm
//	//Code 2-3-32 = C1 and C2
//	return 0;
//}
short CvhCode::vhCode240()	//  Wolff-Parkinson-White syndrome (WPW) 
{
	//C1:	PR < 120 ms
	//C2:	P axis from 1° to 90° 
	//C3:	QRS complex ≥ 120 ms for adults, or ≥ 100 ms for children ≥ 4 years of age, or ≥ 80 ms for infants < 4 years of age
	//C4:	Delta wave present
	//Code 2-4-0 = C1 and C2 and C3 and C4
	unsigned short code=0,code240=vhcode(2,4,0);
	if(m_ageY>=14) {
		if(mc.found(mccode(6,4,1))) code=code240;	//2400;
	}
	else {
		if(msPR()<120) {
			if(axisP()>=1 && axisP()<=90) {
				char wpw=GetWPW();
				if(wpw=='A' || wpw=='a' || wpw=='B' || wpw=='b' || wpw=='W' || wpw=='w') {
					if(m_ageY>=4) {
						if(msQRS()>=100) code=code240;	//2400;
					}
					else {
						if(msQRS()>=80) code=code240;	//2400;
					}
				}
			}
		}
	}
	if(code>0) SetvhCode(code,0);
	return code;
}
short CvhCode::vhCode250()	//  Short PR interval
{
	//Code 2-5-0 = MC 6-5
	unsigned short code=0;
	if(mc.found(mccode(6,5,0)) && vh.found(vhcode(2,4,0))==NULL) {
		code=SetvhCode(vhcode(2,5,0));
	}
	return code;
}
//CONTINUE...
//3.4.	Code 3, Conduction Disturbances
//VH code 3 includes NC-3 and NC-10.
//3.4.1.	Prolonged Ventricular Excitation(3-1-x to 3-4-x)
void CvhCode::vhCode3()
{
	vhCode31x();
	vhCode32x();
	vhCode33x();
	vhCode341();
	vhCode342();
//If QRS axis > –45° and QRS axis ≤ 90°, no code 3-5-x(NC-10,Fascicular block).
	short QRSaxis=axisQRS();
//	if(!(QRSaxis>-45 && QRSaxis<=90)) {
		vhCode350_1();
		vhCode352();
//	}
	if(vhCode371()==0) {
		if(vhCode372()==0) vhCode373();
	}
	vhCode380();	// Fragmented QRS
}

short CvhCode::vhCode31x()	//310,311
{
	unsigned short code=0;
	DWORD dwLeads=0;
	if(m_ageY>=14) {
		code=vhCodeFrommcCode(mccode(7,1,1),vhcode(3,1,0),dwLeads);
	}
	else {
		if(vh.found(vhcode(2,4,0))==NULL) {
			bool age=false;
			if(m_ageY>=4) {
				if(msQRS()>100) age=true;
			}
			else {
				if(msQRS()>90) age=true;
			}
			if(age) {
				short i,lead,L[]={I,II,aVL,V5,V6},n=sizeof(L)/sizeof(short);
				for(i=0;i<n;i++) {
					lead=L[i];
					if(msR(lead)>60 && msRpeak(lead)>=60) dwLeads|=m_infoLead[lead].mask;
				}
				if(dwLeads>0) code=SetvhCode(vhcode(3,1,0),dwLeads);
			}
		}
	}
	if(code>0) {
		if(m_stQscore.score>=25) {
			dwLeads|=m_stQscore.dwLeads;
			code=vh.replace(vhcode(3,1,0),vhcode(3,1,1),dwLeads);
		}
	}
	return code;
}
//short CvhCode::vhCode310()	// Left bundle branch block (LBBB) 
//{
//	//C1: WPW absent (no Code 2-4-0)
//	//C2: QRS ≥ 120 ms for adults; QRS > 100 ms for children between 4 to 14 years of age; QRS > 90 ms for children less than 4 years of age
//	//C3: R peak time or R2 peak time ≥ 60 ms in leads V5 or V6 or I or aVL or II
//	//Code 3-1 = C1 and C2 and C3.
//	//Note: See also MC-7-1-1.
//}
//short CvhCode::vhCode311()	// LBBB with possible MI
//{
//	//C1: Code 3-1-0
//	//C2: Q score ≥ 25
//	//Code 3-1-1 = C1 and C2.
//	unsigned short code=vhcode();	//0;
//	DWORD dwLeads=0;
//	CodeList *p=vh.found(vhcode(3,1,0));
//	if(p) {
//		if(Qscore(dwLeads)>=25) {
//			dwLeads|=p->code.nLeads;
//			vh.remove(3,1,0));
//			code=vhcode();	//SetvhCode(3,1,1),dwLeads);
//		}
//	}
//	return code;
//}
short CvhCode::vhCode32x()
{
	if(vh.found(vhcode(2,4,0))) return 0;
	DWORD dwLeads=0;
	unsigned short code=vhCodeFrommcCode(mccode(7,2,1),vhcode(3,2,0),dwLeads);
	if(code>0) return code;
	bool age=false;
	if(m_ageY>=14) {
		if(msQRS()>=120) age=true;
	}
	else if(m_ageY>=4) {
		if(msQRS()>=100) age=true;
	}
	else {
		if(msQRS()>=90) age=true;
	}
	if(age) {
		if(uvR2(V1)>uvR1(V1) && uvR1(V1)>0) dwLeads=m_infoLead[V1].mask;
		if(uvR1(V1)>0 && uvR2(V1)>0 && msR(V1)>60 && msRpeak(V1)>=60)	dwLeads|=m_infoLead[V1].mask;
		if(uvR1(V2)>0 && uvR2(V2)>0 && msR(V1)>60 && msRpeak(V2)>=60)	dwLeads|=m_infoLead[V2].mask;
		{
			int i=0;
			DWORD dwS=0;
			if(msS(I)>msR(I) && msS(I)>50 && msR(I)>0 && uvS(I)<-100)		{	i++;	dwS|=m_infoLead[I].mask;	}
			if(msS(II)>msR(II) && msS(II)>50 && msR(II)>0 && uvS(II)<-100)	{	i++;	dwS|=m_infoLead[II].mask;	}
			if(msS(V6)>msR(V6) && msS(V6)>50 && msR(V6)>0 && uvS(V6)<-100)	{	i++;	dwS|=m_infoLead[V6].mask;	}
			if(i>1 && uvR(V1)>100) dwLeads|=dwS;
		}
		if(dwLeads>0) code=SetvhCode(vhcode(3,2,0),dwLeads);
	}
	if(code>0) {
		if(m_stQscore.score>=25 && (dwLeads&m_stQscore.dwLeads)) {
			dwLeads|=m_stQscore.dwLeads;
			code=vh.replace(vhcode(3,2,0),vhcode(3,2,1),dwLeads);
		}
	}
	return code;
}
//short CvhCode::vhCode320()	// Right bundle branch block (RBBB) 
//{
//	//NC-Criteria
//	//C1: WPW absent (no Code 2-4-0)
//	//C2: QRS ≥ 120 ms; QRS > 100 ms for children between 4 to 16 years of age; QRS > 90 ms for children less than 4 years of age
//	//C3: R peak time or R2 peak time ≥ 60 ms in leads V1 or V2
//	//C4: S duration ≥ R duration in I or V6
//	//Code 3-2-0 = C1 and C2 and (C3 or C4)
//	//MC-Criteria(MC-7-2-1).
//	//C1: No MC code 6-1,6-4-1, 6-8, 8-2-1 or 8-2-2
//	//C2: QRS ≥ 120 ms(in any lead of I, II, III, aVL, aVF)
//	//C3: R2 > R in V1
//	//C4: QRSn > 0(QRS mainly upright) and R peak time ≥ 60 ms in V1 or V2
//	//C5: S duration > R duration in I or II
//	//Code 3-2-0 = C1 and C2 and (C3 or C4 or C5)
//}
//short CvhCode::vhCode321()	// RBBB with possible MI 
//{
//	//C1: Code 3-2-0
//	//C2: Q score ≥ 25
//	//Code 3-2-1 = C1 and C2
//	return 0;
//}
short CvhCode::vhCode33x()	// 330,331
{
	unsigned short code=0;
	if(!vh.found(vhcode(2,4,0)) && !vh.found(vhcode(3,1,0)) && !vh.found(vhcode(3,1,1)) && !vh.found(vhcode(3,2,0)) && !vh.found(vhcode(3,2,1))) {
		short L[]={I, II, III, aVL, aVF},n=sizeof(L)/sizeof(short);
		if(msQRSjudge(L,n,120)>1) {
			if(m_stQscore.score>=25) code=vhcode(3,3,1);	//3301;
			else code=vhcode(3,3,0);	//3300;
		}
		if(code) SetvhCode(code,m_stQscore.dwLeads);
	}
	return code;
}
//short CvhCode::vhCode330()	// Indeterminate ventricular conduction delay (IVCD) 
//{
//	//Criteria(MC-7-4)
//	//C1: WPW absent (no Code 2-4-0) 
//	//C2: No code 3-1-x or 3-2-x
//	//C3: QRS ≥ 120 ms (in any of I, II, III, aVL, aVF)
//	//Code 3-3-0 = C1 and C2 and C3
//	//Note: Code 3-3-0 includes LBBB pattern with QRS 120–124 ms.
//	return 0;
//}
//short CvhCode::vhCode331()	//  IVCD with possible MI
//{
//	//C1: Code 3-3-0
//	//C2: Q score ≥ 25
//	//Code 3-3-1 = C1 and C2
//	return 0;
//}
short CvhCode::vhCode341()	//  Incomplete right bundle branch block (IRBBB) 
{
	//Criteria(Maquette)
	//C1: QRS > 90 ms and ≤ 120 ms for adults; QRS ≥ 90 ms and ≤ 100 ms for children of age 8 to 16 years; QRS > 85 ms and ≤ 90 ms for children of age less than 8 years
	//C2: S > 40 ms in any two of I,aVL,V4,V5,V6
	//C3: in V1 or V2, R ≥ 30 ms and R > 100 µV and no S wave is present
	//C4: in V1 or V2, R2 ≥ 30 ms and R2 > 100 µV and no S2 wave is present
	//C5: R2 > R in V1 or V2
	//Code 3-4-1 = C1 and C2 and (C3 or C4)
	//Code 3-4-1 = C1 and C5
	//Note: refer to MC-7-3
	if(vh.found(vhcode(3,2,0)) || vh.found(vhcode(3,2,1))) return 0;
	unsigned short code=0;
	DWORD dwLeads=0;
	if(m_ageY>=16) {
		code=vhCodeFrommcCode(mccode(7,3,0),vhcode(3,4,1),dwLeads);
		if(code>0) return code;
	}
	bool c1=false,c2=false,c3=false,c4=false,c5=false;
	if(m_ageY>=16) {
		if(msQRS()>90 && msQRS()<=120) c1=true;
	}
	else if(m_ageY>=8) {
		if(msQRS()>=90 && msQRS()<=100) c1=true;
	}
	else {
		if(msQRS()>85 && msQRS()<=90) c1=true;
	}
	if(!c1) return 0;

	short i,j,lead,L[]={I,aVL,V4,V5,V6},n=sizeof(L)/sizeof(short);
	for(i=j=0;i<n;i++) {
		lead=L[i];	
		if(msS(lead)>40) {
			j++;	dwLeads|=m_infoLead[lead].mask;
		}
	}
	if(j>1) c2=true;

	if(msR1(V1)>=30 && uvR1(V1)>100 && noS1(V1)) {c3=true;dwLeads|=m_infoLead[V1].mask;}
	if(msR1(V2)>=30 && uvR1(V2)>100 && noS1(V2)) {c3=true;dwLeads|=m_infoLead[V2].mask;}

	if(msR2(V1)>=30 && uvR2(V1)>100 && noS2(V1)) {c4=true;dwLeads|=m_infoLead[V1].mask;}
	if(msR2(V2)>=30 && uvR2(V2)>100 && noS2(V2)) {c4=true;dwLeads|=m_infoLead[V2].mask;}

	if(uvR2(V1)>uvR1(V1) && uvR1(V1)>0) {c5=true;dwLeads|=m_infoLead[V1].mask;}
	if(uvR2(V2)>uvR1(V2) && uvR1(V2)>0) {c5=true;dwLeads|=m_infoLead[V2].mask;}

	if((c2 && (c3 || c4)) || c5) {
		code=vhcode(3,4,1);	//3401;
		SetvhCode(code,dwLeads);
	}
	return code;
}
short CvhCode::vhCode342()	// Incomplete left bundle branch block(ILBBB)
{
	//Criteria(Maquette)
	//C1: QRS ≥ 105 ms and ≤ 120 ms for adults; QRS ≥ 90 ms and ≤ 100 ms for children of age 8 to 16 years; QRS ≥ 80 ms and ≤ 90 ms for children of age less than 8 years
	//C2: QRSn ≤ 0 in V1 and V2
	//C3: Q or S ≥ 80 ms in V1 and V2
	//C4: no Q wave is present in any two of I,V5,V6
	//C5: R > 60 ms in any two of I,aVL,V5,V6
	//Code 3-4-2 = C1 and C2 and C3 and C4 and C5
	//Note: refer to MC-7-6
	if(vh.found(vhcode(3,1,0)) || vh.found(vhcode(3,1,1))) return 0;
	unsigned short code=0;
	DWORD dwLeads=0;
	if(m_ageY>=16) {
		code=vhCodeFrommcCode(mccode(7,6,0),vhcode(3,4,2),dwLeads);
		if(code>0) return code;
	}
	bool c1=false,c2=false,c3=false,c4=false,c5=false;
	if(m_ageY>=16) {
		if(msQRS()>105 && msQRS()<=120) c1=true;
	}
	else if(m_ageY>=8) {
		if(msQRS()>=90 && msQRS()<=100) c1=true;
	}
	else {
		if(msQRS()>=80 && msQRS()<=90) c1=true;
	}
	if(!c1) return 0;
	if(uvQRSnet(V1)<=0 && uvQRSnet(V2)<=0) {c2=true;dwLeads|=(m_infoLead[V1].mask|m_infoLead[V2].mask);}
	if(!c2) return 0;
	if((msQd(V1)>=80 || msS(V1)>=80) && (msQd(V2)>=80 || msS(V2)>=80)) {c3=true;dwLeads|=(m_infoLead[V1].mask|m_infoLead[V2].mask);}
	if(!c3) return 0;

	short i,j,k,lead,L[]={I,aVL,V5,V6},n=sizeof(L)/sizeof(short);
	for(i=j=k=0;i<n;i++) {
		lead=L[i];
		if(msR(lead)>60) {
			k++;	dwLeads|=m_infoLead[lead].mask;
		}
		if(lead==aVL) continue;
		if(noQ(lead) && isDownwardST(lead)) {
			j++;	dwLeads|=m_infoLead[lead].mask;
		}
	}
	if(j>2) c4=true;
	if(k>1) c5=true;
	if(c4 && c5) {
		code=vhcode(3,4,2);	//3402;
		SetvhCode(code,dwLeads);
	}
	return code;
}
//3.4.2.	Fascicular Blocks (3-5-x)
//If QRS axis > –45° and QRS axis ≤ 90°, no code 3-5-x(NC-10,Fascicular block).
short CvhCode::vhCode350_1()
{
	unsigned short code=0;
	DWORD dwLeads=0;
	code=vhCodeFrommcCode(mccode(7,7,0),vhcode(3,5,0),dwLeads);
	if(code>0) return code;
	bool c1=(msQRS()<120)?true:false;
	bool c2=(uvQRSnet(II)<0 && uvQRSnet(III)<0)?true:false;	if(c2) dwLeads|=(m_infoLead[II].mask|m_infoLead[III].mask);
	bool c3=(uvR1(II)>=25 && uvR1(II)<1000 && uvR1(II)<-uvS(II) && uvR2(II)==0)?true:false;	if(c3) dwLeads|=m_infoLead[II].mask;
	bool c4=(uvQd(aVL)<=-25 && uvQd(aVL)>=-200)?true:false;	if(c4) dwLeads|=m_infoLead[aVL].mask;
	bool c5=(uvR(aVL)>=200 && msR(aVL)>40)?true:false;	if(c5) dwLeads|=m_infoLead[aVL].mask;
	if(c1 && c3 && c4 && c5) {
		if(c2) {
			code=vhcode(3,5,0);	//3500;
			SetvhCode(code,dwLeads);
		}
		else {
			if(axisQRS()>=-45 && axisQRS()<=-30) {
				code=vhcode(3,5,1);	//3501;
				SetvhCode(code,dwLeads);
			}
		}
	}
	if(code==0) {
		c3=(axisQRS()>=-90 && axisQRS()<-45)?true:false;
		c5=(uvS(III)<uvS(II)) && (uvR(aVL)>uvR(I))?true:false;	if(c5) dwLeads|=(m_infoLead[I].mask|m_infoLead[II].mask|m_infoLead[aVL].mask);
		if(c3 && c5) {
			code=vhcode(3,5,1);	//3501;
			SetvhCode(code,dwLeads);
		}
	}
	return code;
}
//short CvhCode::vhCode350()	// Left anterior fascicular block (LAFB) 
//{
//	//C1: QRS < 120 ms
//	//C2: QRSII < 0 µV and QRSIII < 0 µV
//	//C3: RiS patterns in lead II, with initial Ra < 200 µV 
//	//C4: Q 25-100 µV in lead aVL
//	//C5: R ≥ 200 µV and R peak time ≥ 40 ms in lead aVL 
//	//C6: MC-7-7: QRS(I,II,III,aVL,aVF) < 120ms and Q(I or aVL) ≥ 25µV and < 30 ms and QRS axis ≤ –45°(In presence of MC-7-2, code MC-7-8 if axis is < – 45° and the Q-wave in lead I meets the above criteria. i.e. no MC-7-7)
//	//Code 3-5-0 = (C1 and C2 and C3 and C4 and C5) or C6
//	//Note: Criterion 2 (net QRS amplitude < 0 µV in leads II and III) implies QRS axis from -31° to -119°.
//}
//short CvhCode::vhCode351()	// Probably Left anterior fascicular block (PLAFB) 
//{
//	//C1: No code 3-5-0
//	//C2: -45°≤ QRS axis ≤ -30°
//	//C3: -90°≤ QRS axis ≤ -45°
//	//C4: Other criteria except C2 of code 3-5-1
//	//C5: SV(III) > SV(II) and RV(aVL) > RV(I)
//	//Code 3-5-1 = C1 and [(C2 and C4) or (C3 and C5)]
//	return 0;
//}
short CvhCode::vhCode352()	// Left posterior fascicular block (LPFB) 
{
	//C1: QRS < 120 ms
	//C2: For adults, QRSI < 0 µV and QRSaVF > 0 µV
	//C3: For children, right-axis deviation present, see 7-3-0
	//C4: No code 6-3-1 (no RVH)
	//C5: Q from 25 µV to 99 µV and R ≥ 100 µV in III and aVF 
	//C6: Q < 40 ms in III and aVF
	//Code 3-5-2 = C1 and (C2 or C3) and C4 and C5 and C6
	//Note: Criterion C2 implies QRS axis > 90° and < 180°.
	unsigned short code=0,dwLeads=0;
	bool c1=(msQRS()<120)?true:false;
	bool c2=false,c3=false,c4=true,c5=false;
	if(m_ageY>=16) {
		if(uvQRSnet(I)<0 && uvQRSnet(aVF)>0) c2=true;
	}
	else {
		if(vh.found(vhcode(7,3,0))) c3=true;
	}
//	if(vh.found(vhcode(6,3,1))) c4=false;
	if(uvQd(III)<100 && uvR(III)>=100 && msQd(III)<40) {
		dwLeads|=m_infoLead[III].mask;
		if(uvQd(aVF)<100 && uvR(aVF)>=100 && msQd(aVF)<40) {
			dwLeads|=m_infoLead[aVF].mask;
			c5=true;
		}
	}
	bool c6=(uvR(V1)<-uvS(V1) && uvR(V2)<-uvS(V2) && (uvR(V5)<-uvS(V5) || uvR(V6)<-uvS(V6)))?true:false;
	if(c1 && (c2 || c3) && c4 && c5 && c6) {
		code=vhcode(3,5,2);	//3502;
		SetvhCode(code,dwLeads);
	}
	return code;
}
//short CvhCode::vhCode370()	//  Brugada syndrome
//{
//	return 0;
//}
short CvhCode::vhCode371()	// Brugada syndrome, Type1
{
	//Code 3-7-1 = MC 7-9-1
	DWORD dwLeads=0;
	return vhCodeFrommcCode(mccode(7,9,1),vhcode(3,7,1),dwLeads);
}
short CvhCode::vhCode372()	// Brugada syndrome, Type2
{
	//Code 3-7-2 = MC 7-9-2
	DWORD dwLeads=0;
	return vhCodeFrommcCode(mccode(7,9,2),vhcode(3,7,2),dwLeads);
}
short CvhCode::vhCode373()	// Brugada syndrome, Type3
{
	//Code 3-7-3 = MC 7-9-3
	DWORD dwLeads=0;
	return vhCodeFrommcCode(mccode(7,9,3),vhcode(3,7,3),dwLeads);
}
short CvhCode::vhCode380()	// Fragmented QRS
{
	//Code 3-8-0 = MC 7-10
	DWORD dwLeads=0;
	return vhCodeFrommcCode(mccode(7,10,0),vhcode(3,8,0),dwLeads);
}

//3.5.	Code 4, Repolarization abnormalities
void CvhCode::vhCode4()
{
	if(m_bAf) return;
	vhCode41x();
	if(vhCode421()==0) vhCode422();
}
//If QTI < 112%, no code 4-1-x(NC 4-1-x,Prolonged Ventricular Repolarization).
//No MC code for prolonged ventricular Repolarization.
short CvhCode::vhCode41x()	// 410,411,412
{
	if(vh.found(vhcode(1,8,1))) return 0;
	unsigned short code=0;
	short value=(QRS>=120)?JTI:QTI;
	if(value>116) code=vhcode(4,1,2);	//4102;
	else if(value>=112) code=vhcode(4,1,1);	//4101;
	if(code>0) SetvhCode(code,0);
	return code;
}
//short CvhCode::vhCode410()	// Prolonged ventricular repolarization
//{
//	//C1: QTI ≥ 112%. Using JTI instead of QTI if QRS ≥ 120 ms
//	//Code 4-1-0 = C1
//	return 0;
//}
//short CvhCode::vhCode411()	// Marginal prolongation of ventricular repolarization
//{
//	//C1: QTI 112% to 116%. Using JTI instead of QTI if QRS ≥ 120 ms
//	//Code 4-1-1 = C1
//	return 0;
//}
//short CvhCode::vhCode412()	// Deﬁnite prolongation of ventricular repolarization
//{
//	//C1: QTI > 116%. Using JTI instead of QTI if QRS ≥ 120 ms
//	//Code 4-1-2 = C1
//	//Note 1: QTI (%) = (QT/656) × (HR + 100). QT is in ms. At heart rate 60 bpm, QTI 112% corresponds to a QT of 460 ms.
//	//Note 2: It is essential to use JTI rather than QTI for coding prolonged repolarization if QRS ≥ 120 ms. JT prolongation index JTI (%) = (JT/518) × (HR + 100) where JT = QT – QRS.
//	//Note 3: It should be recognized that QTI includes not only the period of ventricular repolarization but also ventricular excitation. The inclusion of separate terms for ventricular repolarization (JT) and excitation (QRS) may be warranted.
//	return 0;
//}
short CvhCode::vhCode421()	//  Early Repolarzation(ER)
{
	//Code 4-2-1 = MC 9-7-1
	DWORD dwLeads=0;
	return vhCodeFrommcCode(mccode(9,7,1),vhcode(4,2,1),dwLeads);
}
short CvhCode::vhCode422()	//  Probable early repolarization(PER)
{
	//Code 4-2-2 = MC 9-7-2
	DWORD dwLeads=0;
	return vhCodeFrommcCode(mccode(9,7,2),vhcode(4,2,2),dwLeads);
}
//3.6.	Code 5, Myocardial Infarction/Ischemia (MI Likelihood)
//
void CvhCode::vhCode5()
{
	bool MI=false;
	bool ST=false;
	short code=vhCode510();
	if(code) MI=true;
	else {
		code=vhCode520_30();
		if(code) {
			MI=true;
			if(code==vhcode(5,2,0)) ST=true;
		}
		else {
			code=vhCode540();
			if(code) {
				MI=true;
				if(code==vhcode(5,4,0)) ST=true;
			}
		}
	}
	if(!MI) {
		vhCode501();
		if(!vhCode502()) vhCode570();
		if(!ST) {
			if(vhCode550_6x()) ST=true;
		}
	}
//	if(vhCode551()) ST=true;
	if(!ST) vhCode580();
}
short CvhCode::vhCode501()	// Poor R progression
{
	//C1: No code 5-1-x to 5-4-x
	//C2: MC-1-3-8
	//C3: RV3≤300µV and RV3≤RV2
	//C4: RV1>RV2>RV3>RV4
	//Code 5-0-1 = C1 and (C2 or C3 or C4)
	unsigned short code=0;
	DWORD dwLeads=0;
	code=vhCodeFrommcCode(mccode(1,3,8),vhcode(5,0,1),dwLeads);
	if(code==0) {
		if(uvR(V3)<=300 && uvR(V3)<=uvR(V2)) {code=vhcode(5,0,1);	dwLeads|=m_infoLead[V3].mask;}
	}
	if(code==0) {
		if(uvR(V1)>uvR(V2) && uvR(V2)>uvR(V3) && uvR(V3)>uvR(V4)) {code=vhcode(5,0,1);	dwLeads|=(m_infoLead[V2].mask|m_infoLead[V3].mask|m_infoLead[V4].mask);}
	}
	if(code>0) SetvhCode(code,dwLeads);
	return code;
}
short CvhCode::vhCode502()	// Abnormal Q wave
{
	//C1: no MI and MC-1.x.x for any two leads
	//Code 5-0-2 = C1
	//Note: Even if MI(5-1-x to 5-4-x) is present for persons below age of 18 years, 5-0-2 is reported.
	unsigned short code=0,codes[]={
		mccode(1,1,1),mccode(1,1,2),mccode(1,1,3),mccode(1,1,4),mccode(1,1,5),mccode(1,1,6),mccode(1,1,7),//1170,
		mccode(1,2,1),mccode(1,2,2),mccode(1,2,3),mccode(1,2,4),mccode(1,2,5),mccode(1,2,7),
		mccode(1,3,1),mccode(1,3,2),mccode(1,3,3),mccode(1,3,4),mccode(1,3,5),mccode(1,3,6),mccode(1,3,7)
	};
	short i,j,n=sizeof(codes)/sizeof(short);
	DWORD dwLeads=0;
	for(i=j=0;i<n;i++) {
		CodeList *p=mc.found(codes[i]);
		if(p) {
			dwLeads|=p->code.nLeads;	j++;
		}
	}
	if(j>0) {
		for(i=j=0;i<18;i++) {
			if((dwLeads&(1<<i))) j++;
		}
		if(j>1) {
			code=vhcode(5,0,2);	//5002;
			SetvhCode(code,dwLeads);
		}
	}
	return code;
}
short CvhCode::vhCode510()	// Q wave MI with major Q waves 
{
	//High Likelihood of Q Wave MI
	//Criteria(adults)
	//C1: Q score ≥ 35 in any lead group
	//Code 5-1-0 = C1
	unsigned short code=0;	//5100;
	if(m_stQscore.score>=35) {
		if(m_ageY>=18) {
			code=vhcode(5,1,0);
			if(m_stQscore.dwLeads&MI_L) SetvhCode(vhcode(5,1,1),m_stQscore.dwLeads&MI_L);
			if(m_stQscore.dwLeads&MI_I) SetvhCode(vhcode(5,1,2),m_stQscore.dwLeads&MI_I);
			if(m_stQscore.dwLeads&MI_S) SetvhCode(vhcode(5,1,3),m_stQscore.dwLeads&MI_S);
			if(m_stQscore.dwLeads&MI_A) SetvhCode(vhcode(5,1,4),m_stQscore.dwLeads&MI_A);
		}
		else {
			code=vhcode(5,0,2);	//5002;
			SetvhCode(code,m_stQscore.dwLeads);
		}
	}
	if(m_ageY>=18 && m_stPLMIscore.score>=30) {
		SetvhCode(vhcode(5,1,5),m_stPLMIscore.dwLeads);
	}
	return code;
}
//short CvhCode::vhCode511()	// Lateral
//short CvhCode::vhCode512()	// Inferior
//short CvhCode::vhCode513()	// Septal
//short CvhCode::vhCode514()	// Anterior
short CvhCode::vhCode520_30()	//520,530
{
	unsigned short code=0;	//5002;
	if(m_stQscore.score>=25) {
		if(m_ageY>=18) {
			if(m_stSTDscore.score>=20 || m_stTNscore.score>=20) {
				code=vhcode(5,2,0);	//5200;
				if(m_stQscore.dwLeads&MI_L && ((m_stSTDscore.dwLeads&MI_L) || (m_stTNscore.dwLeads&MI_L))) SetvhCode(vhcode(5,2,1),(m_stQscore.dwLeads|m_stSTDscore.dwLeads|m_stTNscore.dwLeads)&MI_L);
				if(m_stQscore.dwLeads&MI_I && ((m_stSTDscore.dwLeads&MI_I) || (m_stTNscore.dwLeads&MI_I))) SetvhCode(vhcode(5,2,2),(m_stQscore.dwLeads|m_stSTDscore.dwLeads|m_stTNscore.dwLeads)&MI_I);
				if(m_stQscore.dwLeads&MI_S && ((m_stSTDscore.dwLeads&MI_S) || (m_stTNscore.dwLeads&MI_S))) SetvhCode(vhcode(5,2,3),(m_stQscore.dwLeads|m_stSTDscore.dwLeads|m_stTNscore.dwLeads)&MI_S);
				if(m_stQscore.dwLeads&MI_A && ((m_stSTDscore.dwLeads&MI_A) || (m_stTNscore.dwLeads&MI_A))) SetvhCode(vhcode(5,2,4),(m_stQscore.dwLeads|m_stSTDscore.dwLeads|m_stTNscore.dwLeads)&MI_A);
			}
			else {
				code=vhcode(5,3,0);	//5300;
				if(m_stQscore.dwLeads&MI_L) SetvhCode(vhcode(5,3,1),m_stQscore.dwLeads);
				if(m_stQscore.dwLeads&MI_I) SetvhCode(vhcode(5,3,2),m_stQscore.dwLeads);
				if(m_stQscore.dwLeads&MI_S) SetvhCode(vhcode(5,3,3),m_stQscore.dwLeads);
				if(m_stQscore.dwLeads&MI_A) SetvhCode(vhcode(5,3,4),m_stQscore.dwLeads);
			}
		}
		else {
			code=vhcode(5,0,2);	//5002;
			SetvhCode(code,m_stQscore.dwLeads);
//			SetvhCode(5501,m_stSTDscore.dwLeads|m_stTNscore.dwLeads);
		}
	}
	if(m_ageY>=18 && m_stPLMIscore.score>=20 && m_stPLMIscore.score<30) {
		if(m_stSTDscore.score>=20 || m_stTNscore.score>=20) {
			SetvhCode(vhcode(5,2,5),m_stPLMIscore.dwLeads);
		}
		else {
			SetvhCode(vhcode(5,3,5),m_stPLMIscore.dwLeads);
		}
	}
	return code;
}
//short CvhCode::vhCode520()	// Q wave MI with moderate Q waves and with ST-T abnormalities
//{
//	//Moderate Likelihood of MI
//	//Criteria(probable) (adults)
//	//C1: Q score ≥ 25 in any lead group
//	//C2: ST-segment depression (STD) or T wave negativity (TN) score 20 or higher in any lead group
//	//Code 5-2-0 = C1 and C2
//}
//short CvhCode::vhCode521()	// Lateral
//short CvhCode::vhCode522()	// Inferior
//short CvhCode::vhCode523()	// Septal
//short CvhCode::vhCode524()	// Anterior
//bool CvhCode::vhCode530()	// Possible Q wave MI with moderate Q waves and without ST-T abnormalities
//{
//	//Criteria(probable) (adults)
//	//C1: Q score ≥ 25 in any lead
//	//C2: STD and TN score < 20 in all lead groups
//	//Code 5-3-0 = C1 and C2
//	unsigned short code=vhcode();	//5,2,0);
//	return code;
//}
//short CvhCode::vhCode531()	// Lateral
//short CvhCode::vhCode532()	// Inferior
//short CvhCode::vhCode533()	// Septal
//short CvhCode::vhCode534()	// Anterior
short CvhCode::vhCode540()	// Possible Q wave MI with minor Q waves and with ST-T abnormalities
{
	//Criteria(possible) (adults)
	//C1: Q score ≥ 15 in any lead
	//C2: STD or TN score ≥ 20 in any lead group
	//Code 5-4-0 = C1 and C2
	//Isolated Ischemic Abnormalities
	unsigned short code=0;	//5002;
	if(m_stQscore.score>=15) {
		if(m_ageY>=18) {
			if(m_stSTDscore.score>=20 || m_stTNscore.score>=20) {
				code=vhcode(5,4,0);	//5400;
				if((m_stQscore.dwLeads&MI_L) && ((m_stSTDscore.dwLeads&MI_L) || (m_stTNscore.dwLeads&MI_L))) SetvhCode(vhcode(5,4,1),(m_stQscore.dwLeads|m_stSTDscore.dwLeads|m_stTNscore.dwLeads)&MI_L);
				if((m_stQscore.dwLeads&MI_I) && ((m_stSTDscore.dwLeads&MI_I) || (m_stTNscore.dwLeads&MI_I))) SetvhCode(vhcode(5,4,2),(m_stQscore.dwLeads|m_stSTDscore.dwLeads|m_stTNscore.dwLeads)&MI_I);
				if((m_stQscore.dwLeads&MI_S) && ((m_stSTDscore.dwLeads&MI_S) || (m_stTNscore.dwLeads&MI_S))) SetvhCode(vhcode(5,4,3),(m_stQscore.dwLeads|m_stSTDscore.dwLeads|m_stTNscore.dwLeads)&MI_S);
				if((m_stQscore.dwLeads&MI_A) && ((m_stSTDscore.dwLeads&MI_A) || (m_stTNscore.dwLeads&MI_A))) SetvhCode(vhcode(5,4,4),(m_stQscore.dwLeads|m_stSTDscore.dwLeads|m_stTNscore.dwLeads)&MI_A);
			}
		}
		else {
			code=vhcode(5,0,2);	//5002;
			SetvhCode(code,m_stQscore.dwLeads);
//			SetvhCode(5501,m_stSTDscore.dwLeads|m_stTNscore.dwLeads);
		}
	}
	if(m_ageY>=18 && m_stPLMIscore.score>=10 && m_stPLMIscore.score<20) {
		SetvhCode(vhcode(5,4,5),m_stPLMIscore.dwLeads);
	}
	return code;
}
//short CvhCode::vhCode541()	// Lateral
//short CvhCode::vhCode542()	// Inferior
//short CvhCode::vhCode543()	// Septal
//short CvhCode::vhCode544()	// Anterior
short CvhCode::vhCode550_6x()
{
	if(m_bAf) return 0;
	unsigned short code=0;
//	if(m_stQscore.score<15) {
		if(m_stSTDscore.score>=20) {
			code=vhcode(5,5,0);	//5500;
			SetvhCode(code,m_stSTDscore.dwLeads);
		}
		if(m_stTNscore.score>=20) {
			code=vhcode(5,6,0);	//5600;
			SetvhCode(code,m_stTNscore.dwLeads);
		}
		else if(m_stTNscore.score==10) {
			code=vhcode(5,6,1);	//5601;
			SetvhCode(code,m_stTNscore.dwLeads);
		}
//	}
//	else code=vhcode();	//0;
	return code;
}
//short CvhCode::vhCode550()	// ST abnormalities without Q waves
//{
//	//C1: STD score ≥ 20 in any lead group
//	//C2: Q score < 15 in all leads
//	//Code 5-5-0 = C1 and C2
//	return 0;
//}
//short CvhCode::vhCode551()	// ST abnormalities
//{
//	//C1: STD score ≥ 20 in any lead group
//	//Code 5-5-0 = C1
//	//do nothing
//	return 0;
//}
//short CvhCode::vhCode560()	// T wave inversion without Q waves
//{
//	//C1: TN score ≥ 20 in any lead group
//	//C2: Q score < 15 in all leads
//	//Code 5-6-0 = C1 and C2
//	//Minor Q Wave or ST- T Abnormalities
//	return 0;
//}
//short CvhCode::vhCode561()	// T wave flat without Q waves
//{
//	//C1: TN score = 10 in any lead group
//	//C2: Q score < 15 in all leads
//	//Code 5-6-1 = C1 and C2
//	//Minor Q Wave or ST- T Abnormalities
//	return 0;
//}
short CvhCode::vhCode570()	// Minor Q waves without ST-T abnormalities
{
	//C1: Q score ≥ 15 in any lead
	//C2: STD and TN score < 20 in all lead groups
	//Code 5-7-0 = C1 and C2
	unsigned short code=0;
	if(m_stQscore.score>=15 && m_stSTDscore.score<20 && m_stTNscore.score<20) {
		code=vhcode(5,7,0);	//5700;
		SetvhCode(code,m_stQscore.dwLeads);
	}
	return code;
}
short CvhCode::vhCode580()	// Minor ST-T abnormalities
{
	//C1: STD or TN score ≥ 10 in any lead group
	//Code 5-8-0 = C1
	//Note 1: Code 0-1 (ECG not available) and codes 0-2-1 and 0-2-3 (inadequate quality or missing leads, including RA/RL reversal) interfere with morphologic codes 0-3 (ventricular pacemaker), 0-4 (complete bundle branch block and Wolff-Parkinson-White pattern), 0-6 (electronic pacemaker) and suppress all code 5 items.
	//Note 2: if no code 4-1-0 (no left ventricular hypertrophy) is recommended as an additional condition for codes 5-5 and 5-6.
	unsigned short code=0;
	//if(m_stSTDscore.score>=10 || m_stTNscore.score>=10) {
	//	code=vhcode(5,8,0);	//5800;
	//	SetvhCode(code,m_stSTDscore.dwLeads|m_stTNscore.dwLeads);
	//}
	return code;
}
//3.7.	Code 6, Ventricular Hypertrophy and Atrial Enlargement
//VH code 6 includes NC-6,7,8,9.
void CvhCode::vhCode6()
{
	vhCode61x();	// 610,611x
	vhCode621();	// Left atrial enlargement (NC 7-1)
	vhCode631();	// Right ventricular hypertrophy (RVH) (NC 8-1)
	vhCode641();	// Right atrial enlargement (NC 9-1)
}
short CvhCode::vhCode61x()	//610,611
//short CvhCode::vhCode610()	// Left ventricular hypertrophy (LVH) (NC 6-1)
{
	unsigned short code=0;
	DWORD dwLeads=0;
	if(m_ageY>=12) {
		//Adults or children for no less than 12 years of age
		//NC-{(Cornell Voltage {)
		//C1: RaVL + SV3 ≥ 2,800 µV in men 
		//C2: RaVL + SV3 ≥ 2,200 µV in women
		if(uvR(aVL)+uvR(V3)>=((m_sex=='M')?2800:2200)) dwLeads|=(m_infoLead[aVL].mask|m_infoLead[V3].mask);
		//Code 6-1-0 = C1 or C2
		//MC-Criteria(MC-3-1: Sokolow-Lyon Voltage {)
		//C1: RV >= 2600 µV in V5 or V6)
		//C2: RV >= 2000 µV in any of I, II, III, aVF
		//C3: RaVL >= 1200 µV
		//C4: R(V5 or V6)+SV1 >= 3500µV
		//Code 6-1-0 = C1 or C2 or C3 or C4
		if(uvR(V5)>=2600) dwLeads|=m_infoLead[V5].mask;
		if(uvR(V6)>=2600) dwLeads|=m_infoLead[V6].mask;
		{
			short l=0;
			if(uvR(I)>=2000) {l++;dwLeads|=m_infoLead[I].mask;}
			if(uvR(II)>=2000) {l++;dwLeads|=m_infoLead[II].mask;}
			if(uvR(III)>=2000) {l++;dwLeads|=m_infoLead[III].mask;}
			if(uvR(aVF)>=2100) {l++;dwLeads|=m_infoLead[aVF].mask;}
			if(uvR(aVL)>=1200) {l++;dwLeads|=m_infoLead[aVL].mask;}
			if(l<2) dwLeads=0;
		}
		short RVjudge=3500;
		if(m_ageY<=30) RVjudge=5000;
		else if(m_ageY<=40) RVjudge=4000;
		if(uvR(V5)-uvS(V1)>=RVjudge) dwLeads|=(m_infoLead[V5].mask|m_infoLead[V1].mask);
		if(uvR(V6)-uvS(V2)>=RVjudge) dwLeads|=(m_infoLead[V6].mask|m_infoLead[V2].mask);
		//if(uvS(V1)+uvS(V2)+uvS(V3)<-3800 && uvR(I)+uvR(II)+uvR(aVL)>2300) dwLeads|=(m_infoLead[V1].mask|m_infoLead[V2].mask|m_infoLead[V3].mask|m_infoLead[I].mask|m_infoLead[II].mask|m_infoLead[aVL].mask);
		//int uvQRS=0,maxl=I;
		//for(short l=I;l<V6;l++) {
		//	uvQRS+=uvQRSpp(l);
		//	if(uvQRSpp(l)>uvQRSpp(maxl)) maxl=l;
		//}
		//if(uvQRS>17500) dwLeads|=m_infoLead[maxl].mask;
	}
	else {
	//Criteria(children for less than 12 years of age)
	//C1: RV5 or RV6 >= 3000 µV for < 3 years of age or 3500 µV for ≥ 3 years of age
	//C2: RV5 + SV1 >= 4500 µV for < 3 years of age or 5000 µV for ≥ 3 years of age
	//C3: SV1 or SV2 >= 2000µV
	//C4: RII + RIII >= 4500 µV
	//C5: RI + RIII >= 3000 µV
	//C6: RaVL >= 2000µV
	//C7: RaVF >= 2500µV
	//Code 6-1-0 = C1 or C2 or C3 or C4 or C5 or C6 or C7
		if(uvR(V5)>=((m_ageY>=3)?3500:3000)) dwLeads|=m_infoLead[V5].mask;
		if(uvR(V6)>=((m_ageY>=3)?3500:3000)) dwLeads|=m_infoLead[V6].mask;
		if(uvR(V5)-uvS(V1)>=((m_ageY>=3)?5000:4500)) dwLeads|=(m_infoLead[V5].mask|m_infoLead[V1].mask);
		if(uvS(V1)<=-2000) dwLeads|=m_infoLead[V1].mask;
		if(uvS(V2)<=-2000) dwLeads|=m_infoLead[V2].mask;
		if(uvR(II)+uvR(III)>=4500) dwLeads|=(m_infoLead[II].mask|m_infoLead[III].mask);
		if(uvR(I)+uvR(III)>=4500) dwLeads|=(m_infoLead[II].mask|m_infoLead[III].mask);
		if(uvR(aVL)>=2000) dwLeads|=m_infoLead[aVL].mask;
		if(uvR(aVF)>=2500) dwLeads|=m_infoLead[aVF].mask;
	}
	if(dwLeads) {
		if(m_stSTDscore.dwLeads>=20 || m_stTNscore.score>=20) code=vhcode(6,1,1);	//6101;
		else code=vhcode(6,1,0);	//6100;
		SetvhCode(code,dwLeads|m_stSTDscore.dwLeads|m_stTNscore.dwLeads);
	}
	return code;
}
//short CvhCode::vhCode611()	// LVH with ST-T abnormalities (NC 6-1-1)
//{
//	//C1: Code 6-1-0
//	//C2: STD or TN score ≥ 20
//	//Code 6-1-1 = C1 and C2
//	return 0;
//}
short CvhCode::vhCode621()	// Left atrial enlargement (NC 7-1)
{
	//C1: In any two of II,III,aVF, P ≥ 120 ms for adults or ≥ 100 ms for children
	//C2: (P2aV1 x P2dV1) ≥ -4000 µVms  (Negative P ≥ 40 ms and ≥ 100µV in V1)
	//Code 6-2-1 = C1 and C2. Equivalent to MC-9-6
	//Note: Use C2 alone, if available, for higher speciﬁcity.
	unsigned short code=0;
	DWORD dwLeads=0;
	short i,j,L[]={II,III,aVF},n=sizeof(L)/sizeof(short);
	short p=((m_ageY<14)?100:120);
	for(i=j=0;i<n;i++) {
		if(msP(L[i])>=p && uvP(L[i])>=100) {j++;dwLeads|=m_infoLead[L[i]].mask;}
	}
	if(j>1 && uvNegaP(V1)<-90) {
		code=vhcode(6,2,1);
		dwLeads|=m_infoLead[V1].mask;
	}
/*	if(j>=n-1 && uvNegaP(V1)<=-90) {
		code=vhcode(6,2,1);
		dwLeads|=m_infoLead[V1].mask;
	}
//	if(uvNegaP(V1)*msP(V1)/2<-4000 || 
	if(uvP2(V1)<=-90 && int(uvNegaP(V1))*msNegaP(V1)<=-4000) {
		dwLeads|=m_infoLead[V1].mask;
		code=vhcode(6,2,1);
	}
*/
	if(code) {
		SetvhCode(code,dwLeads);
	}
	return code;
}
short CvhCode::vhCode631()	// Right ventricular hypertrophy (RVH) (NC 8-1)
{
	unsigned short code=0;
	DWORD dwLeads=0;
	bool c1=false,c2=false,c3=false,c4=false,c5=false;
	if(m_ageY>=12) {
	//Criteria(adults or children for no less than 12 years of age)
	//C1: QRSnaVR ≥ 0 µV and QRSnaVL ≤ 0 µV 
	//C2: (R/S) I ≤ 1 and (R/S) II ≤ 1 and (R/S) III ≤1
	//C3: (R/S) (V5 or V6) ≤ 1
	//C4: RV1 >= 1000µV or RV1 + SV5 > 1200µV
	//C5: MC-3-2: RV1 ≥ 500 µV and (R/S)V1 ≥ 1 and R/S < 1 in any of V2~V6 and no MC-7-3 is present
	//Code 6-3-1 = [ (C1 or C2) and C3] or C4 or C5
	//Note: Criterion 1 (net QRS amplitude in aVR ≥ 0 µV and in aVL ≤ 0 µV) implies QRS axis from 120° to 240° (right axis deviation).
		if(uvR(aVR)>500 && uvQRSnet(aVR)>=0 && uvQRSnet(aVL)<=0)	{c1=true;dwLeads|=(m_infoLead[aVR].mask|m_infoLead[aVL].mask);}
		if(uvR(I)<=-uvS(I) && uvR(II)<=-uvS(II) && uvR(III)<=-uvS(III))	{c2=true;dwLeads|=(m_infoLead[I].mask|m_infoLead[II].mask|m_infoLead[III].mask);}
		if(uvR(V5)<=-uvS(I))	{c3=true;dwLeads|=m_infoLead[V5].mask;}
		if(uvR(V6)<=-uvS(I))	{c3=true;dwLeads|=m_infoLead[V6].mask;}
		if(uvR(V1)>=500 && uvR(V1)>=-uvS(V1)) {
			if(uvR(V1)>=1000)		{c4=true;dwLeads|=m_infoLead[V1].mask;}
			if(uvR(V1)-uvS(V5)>=1200)	{c4=true;dwLeads|=(m_infoLead[V1].mask|m_infoLead[V5].mask);}
			dwLeads|=m_infoLead[V1].mask;
			short i,j=0,L[]={V2,V3,V4,V5,V6},n=sizeof(L)/sizeof(short);
			for(i=j=0;i<n;i++) {
				if(uvR(L[i])<-uvS(L[i])) {j++;dwLeads|=m_infoLead[L[i]].mask;}
			}
			if(j) c5=true;
		}
		if(((c1 || c2) && c3) || c4 || c5) code=vhcode(6,3,1);	//6301;
	}
	else {
	//Criteria(children for less than 12 years of age)
	//C1: Age ≥ 3 months, RV1 >= 1700µV; or V1 in rSR’ pattern and R2 > 1500µV
	//C2: R/S in V1 is abnormal, the abnormal is as follows:
	//3~6mo>7.2, 6~12mo > 5.1, 1~2y > 4.6, 2~3y > 2.6, 3~5y > 1.9, 5~10y > 1.4, 10~10y > 1
	//C3: (R/S) (V5 or V6) ≤ 1 for ≥ 3 months of age
	//C4: SV5 > 1500µV for ≥ 3 years of age or > 900µV for < 3 years of age
	//Code 6-3-1 = C1 or C2 or (C3 and C4)
		if(m_ageM>=3) {
			if(uvR(V1)>=1700) {c1=true;dwLeads|=m_infoLead[V1].mask;}
			if(uvR2(V1)>uvR1(V1) && uvR2(V1)>=1500) {c1=true;dwLeads|=m_infoLead[V1].mask;}
			double ratio=ratioRS(V1);//double(uvR(V1))/-uvS(V1);
			if(ratio>1) {
				if(m_ageM<3) {}
				else if(m_ageM>=3 && m_ageM<6) {
					if(ratio>7.2) c2=true;
				}
				else if(m_ageM>=6 && m_ageM<12) {
					if(ratio>5.1) c2=true;
				}
				else if(m_ageY>=1 && m_ageY<2) {
					if(ratio>4.6) c2=true;
				}
				else if(m_ageY>=2 && m_ageY<3) {
					if(ratio>2.6) c2=true;
				}
				else if(m_ageY>=3 && m_ageY<5) {
					if(ratio>1.9) c2=true;
				}
				else if(m_ageY>=5 && m_ageY<10) {
					if(ratio>1.4) c2=true;
				}
				else {
					c2=true;
				}
				if(c2) dwLeads|=m_infoLead[V1].mask;
			}
			if(uvR(V5)<=-uvS(V5)) {c3=true;dwLeads|=m_infoLead[V5].mask;}
			if(uvR(V6)<=-uvS(V6)) {c3=true;dwLeads|=m_infoLead[V6].mask;}
		}
		if(uvS(V5)<((m_ageY<3)?900:1500)) {c4=true;dwLeads|=m_infoLead[V5].mask;}
		if(c1 || c2 || (c3 && c4)) code=vhcode(6,3,1);	//6301;
	}
	if(code>0) SetvhCode(code,dwLeads);
	return code;
}
short CvhCode::vhCode641()	// Right atrial enlargement (NC 9-1)
{
	//Criteria(adults)
	//C1: MC-9-3: P > 250 µV in any of II,III,aVF
	//Code 6-4-1 = C1
	//Criteria(children)
	//C1: P > 200 µV in any of II,III,aVF
	//Code 6-4-1 = C1
	unsigned short code=0;
	DWORD dwLeads=0;
	short p=(m_ageY>=14)?250:200;
	short i,L[]={II,III,aVF},n=sizeof(L)/sizeof(short);
	for(i=0;i<n;i++) {
		if(uvP(L[i])>p) {dwLeads|=m_infoLead[L[i]].mask;}
	}

	if(dwLeads>0) {
		code=vhcode(6,4,1);	//6401;
		SetvhCode(code,dwLeads);
	}
	return code;
}
//3.8.	Code 7, Axis deviation
void CvhCode::vhCode7()
{
	short code=vhCode710();	//vhCode710();	// Left QRS axis deviation
	if(code==0)	vhCode720();	// Marked right QRS axis deviation
	if(code==0)	vhCode730();	// Right QRS axis deviation
	if(code==0)	vhCode740();	// Extreme right QRS axis deviation
	if(code==0)	vhCode750();	// Indeterminate QRS axis
	if(code==0)	vhCode76x();	// 761，762
}
short CvhCode::vhCode710()	// Left QRS axis deviation
{
	//Criteria(adults)
	//Code 7-1-0 = MC 2-1(QRS axis < -30° and > -90°)
	//Criteria(children)
	//C1: Age from 5y to 16 y and QRS axis < 0° and > -90°
	//C2: Age from 1mo to 5y and QRS axis < 10° and > -90°
	//C3: Age < 1mo and QRS axis < 30° and > -90°
	//Code 7-1-0 = C1 or C2 or C3
	unsigned short code=0,code710=vhcode(7,1,0);
	short axis=axisQRS();	if(axis<=-90) axis+=360;
	if(axis>-90) {
		if(m_ageY>=16) {
			if(axis<-30) code=code710;	//7100;
		}
		else if(m_ageY>=5) {
			if(axis<0) code=code710;	//7100;
		}
		else if(m_ageY>=1) {
			if(axis<5) code=code710;	//7100;
		}
		else if(m_ageM>=1) {
			if(axis<10) code=code710;	//7100;
		}
		else {
			if(axis<30) code=code710;	//7100;
		}
		if(code>0) SetvhCode(code,0);
	}
	return code;
}
short CvhCode::vhCode720()	// Marked right QRS axis deviation
{
	//Criteria(adults)
	//Code 7-2-0 = MC 2-2(QRS axis ≥ 120° and ≤ 180°, QRS axis ≥ -180° and ≤ -150°)
	unsigned short code=0;
	short axis=axisQRS();	if(axis<=-90) axis+=360;
	if(m_ageY>=16) {
		if(axis>=120 && axis<=210) code=vhcode(7,2,0);	//7200;
		if(code>0) SetvhCode(code,0);
	}
	return code;
}
short CvhCode::vhCode730()	// Right QRS axis deviation
{
	//Criteria(adults)
	//Code 7-3-0 = MC 2-3(QRS axis > 90° and < 120°)
	//Criteria(children)
	//C1: Age from 8y to 16 y and QRS axis > 120°
	//C2: Age from 5y to 8 y and QRS axis > 140°
	//C3: Age from 1y to 5y and QRS axis > 100°
	//C4: Age from 1mo to 1y and QRS axis > 120°
	//Code 7-3-0 = C1 or C2 or C3 or C4
	unsigned short code=0,code730=vhcode(7,3,0);
	short axis=axisQRS();	if(axis<=-90) axis+=360;
	if(m_ageY>=16) {
		if(axis>90 && axis<120) code=code730;	//7300;
	}
	else if(m_ageY>=5) {
		if(axis>140)  code=code730;	//7300;
	}
	else if(m_ageY>=1) {
		if(axis>100)  code=code730;	//7300;
	}
	else if(m_ageM>=1) {
		if(axis>120)  code=code730;	//7300;
	}
	if(code>0) SetvhCode(code,0);
	return code;
}
short CvhCode::vhCode740()	// Extreme right QRS axis deviation
{
	//Criteria(adults)
	//Code 7-4-0 = MC 2-4(QRS axis > -150° and < -90°)
	//Criteria(children)
	//C1: Age < 1mo and QRS axis < -90° and > -170°
	//Code 7-4-0 = C1
	unsigned short code=0;
	short axis=axisQRS();	if(axis<=-90) axis+=360;
	if(m_ageY>=16) {
		if(axis>=210 && axis<270) code=vhcode(7,4,0);	//7400;
	}
	else if(m_ageM<1) {
		if(axis>=190 && axis<270) code=vhcode(7,4,0);	//7400;
	}
	if(code>0) SetvhCode(code,0);
	return code;
}
short CvhCode::vhCode750()	// Indeterminate QRS axis
{
	//Criteria(adults)
	//Code 7-5-0 = MC 2-5(QRS axis = 90°)
	unsigned short code=0;
	if(axisQRS()==90) {
		code=vhcode(7,5,0);	//7500;
	}
	if(code>0) SetvhCode(code,0);
	return code;
}
short CvhCode::vhCode76x()
{
	if(m_bAf) return 0;
	short QRS=axisQRS();	if(QRS<0) QRS+=360;
	short T=axisT();		if(T<0) T+=360;
	short QRST=QRS-T;		if(QRST<0) QRST=-QRST;
	if(QRST>180) QRST=360-QRST;
	unsigned short code=0;
	if(QRST>=90 && QRST<135) code=vhcode(7,6,1);	//7601;
	else if(QRST>=135) code=vhcode(7,6,2);	//7602;
	if(code>0) SetvhCode(code,0);
	return code;
}
//short CvhCode::vhCode761()	// Wide QRS-T angle
//{
//	//Criteria(adults)
//	//C1: QRS-T angle ≥ 90° and < 135°
//	//Code 7-6-1 = C1
//}
//short CvhCode::vhCode762()	// Very wide QRS-T angle
//{
//	//Criteria(adults)
//	//C1: QRS-T angle ≥ 135°
//	//Code 7-6-2 = C1
//	return 0;
//}
//3.9.	 Code 8, Other abnormalities
void CvhCode::vhCode8()
{
	vhCode810();	// Dexiocardia(Check electrode again?)
	vhCode82x();	// 820,821,822
	vhCode831();	// Counter clock wise rotation(CCW)
	vhCode832();	// Clock wise rotation(CW)
	vhCode840();	// High T wave(HT)
	vhCode850();	// ST Segment Elevation (STE)
}

short CvhCode::vhCode810()	// Dexiocardia(Check electrode again?)
{
	//C1: PI ≤ 0µV and QRSnI < 0µV
	//C2: QRSn < 1mV in V4 and V5
	//Code 8-1-0 = C1 and C2
	unsigned short code=0;
	DWORD dwLeads=0;
	if(uvP(I)>=0 && uvQRSnet(I)<0 && uvT(I)<0) {
		dwLeads|=m_infoLead[I].mask;
		if(uvR(V1)>-uvS(V1) && uvQRSnet(V4)<1000 && uvQRSnet(V5)<1000) {
			code=vhcode(8,1,0);	//8100;
			SetvhCode(code,dwLeads);
		}
	}
	return code;
}
short CvhCode::vhCode82x()	//820,821,822
{
	int i,l=0,c=0;
	for(i=I;i<V6;i++) {
		if(i<V1) {
			if(uvQRSpp(i)>=500) l++;
		}
		else {
			if(uvQRSpp(i)>=1000) c++;
		}
	}

	unsigned short code=0;
	if(l==0 && c==0) code=vhcode(8,2,0);	//8200;
	else {
		if(l==0) code=vhcode(8,2,1);	//8201;
		else if(c==0) code=vhcode(8,2,2);	//8202;
	}
	if(code>0) {
		SetvhCode(code,0);
	}
	return code;
}
//short CvhCode::vhCode820()	// Low voltage QRS(LOW)
//{
//	//C1: MC 9-1 for both limb and chest leads
//	//Code 8-2-0 = C1
//	return 0;
//}
//short CvhCode::vhCode821()	// Limb leads low voltage QRS(LLW)
//{
//	//C1: MC 9-1 for limb leads only but not chest leads
//	//Code 8-2-1 = C1
//	return 0;
//}
//short CvhCode::vhCode822()	// Chest leads low voltage QRS(CLW)
//{
//	//C1: MC 9-1 for chest leads only but not limb leads
//	//Code 8-2-2 = C1
//	return 0;
//}
short CvhCode::vhCode831()	// Counter clock wise rotation(CCW)
{
	//Code 8-3-1 = MC 9-4-1
	DWORD dwLeads=0;
	return vhCodeFrommcCode(mccode(9,4,1),vhcode(8,3,1),dwLeads);
}
short CvhCode::vhCode832()	// Clock wise rotation(CW)
{
	//Code 8-3-2 = MC 9-4-2
	DWORD dwLeads=0;
	return vhCodeFrommcCode(mccode(9,4,2),vhcode(8,3,2),dwLeads);
}
short CvhCode::vhCode840()	// High T wave(HT)
{
	//Code 8-4-0 = MC 9-5
	DWORD dwLeads=0;
	return vhCodeFrommcCode(mccode(9,5,0),vhcode(8,4,0),dwLeads);
}
short CvhCode::vhCode850()	// ST Segment Elevation (STE)
{
	//Code 8-5-0 = MC 9-2
	if(m_bAf) return 0;
	DWORD dwLeads=0;
	return vhCodeFrommcCode(mccode(9,2,0),vhcode(8,5,0),dwLeads);
}
