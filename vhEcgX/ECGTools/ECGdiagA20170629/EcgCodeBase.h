#pragma once
#include "EcgDiagDefines.h"
#include "CodeEx2.h"

#define	max3(a,b,c)		(max(a,max(b,c)))
#define	min3(a,b,c)		(min(a,min(b,c)))
#define square(a)		((a)*(a))

class CMean {
	int i;
	double m;
public:
	CMean();
	virtual ~CMean();
	void init();
	double mean(double x);
};

class CEcgCodeBase {
public:
	short			m_fs,m_chnum;
	short			m_templpos,m_templen,**m_templ;
	short			**m_data,m_seconds;
	long			m_length;
	double			m_uVpb;
	VH_EcgLeadInfo	m_infoLead[VH_EcgMaxLeads];
	VH_ECGparm		*m_pEcgParm;
	VH_ECGlead		*m_pEcgLead;
	VH_ECGbeat		*m_pEcgBeat;
	VH_ECGinfo		*m_pEcgInfo;
	char			m_szLeadName[64];
	char	*m_pBeatsType;		//心搏类型。目前只有N:正常，V:室性，B:边界无法判定，P:起搏
//	long	m_nBeatsNum;
	char	m_sex;	//'M','F'
	short	m_ageD,m_ageM,m_ageY;
	double	m_meanNN,m_SDNN;
	short	JT,JTI,QT,QTI,QRS,HR;
protected:
	inline char* LeadNames(DWORD dwLeads);
	inline short LeadIndexFromChn(short chn);
	inline char* LeadNameFromChn(short chn);
	inline short HRfromSamples(int samples);
	inline short msfromSamples(int samples);
protected:
	inline int   maxnpos(short *data,int n);
	inline int   minnpos(short *data,int n);
	inline short ppvalue(short *data,int n);
protected:
	inline short Pb(),Pb(short lead);
	inline short Pe(),Pe(short lead);
	inline short Qb(),Qb(short lead);
	inline short Se(),Se(short lead);
	inline short Tb(),Tb(short lead);
	inline short Te(),Te(short lead);
	inline short J(),J(short lead);
protected:
	inline short msRR(),bpmHR();
	inline short msPd(),msPR(),msQRS(),msQT(),msQTc();
	inline char  WPW();  //return: A/a B/b W/w  A型, B型, W型, 小写: 可疑, 其它: 无WPW
	inline short msQTdis(),msQTmax(),msQTmin();
	inline char* QTmaxLead();
	inline char* QTminLead();
//	QTdiscretion QTd;
	inline short axisP(),axisQRS(),axisT();  //return Degree,  for ChNumber==12 only
	inline short uvRV(short lead),uvSV(short lead);
	inline short uvRV5();   //return unit: uV,         for ChNumber==12 only
	inline short uvRV6();   //return unit: uV,         for ChNumber==12 only
	inline short uvSV1();   //return unit: uV,         for ChNumber==12 only
	inline short uvSV2();   //return unit: uV,         for ChNumber==12 only
	inline short uvRV1();
	inline short uvSV5();
protected:
	inline short msP(short lead),msNegaP(short lead);
	inline short msQ(short lead),msQd(short lead);	//Qd for -aVR，msQd用于心梗评分
	inline short msR1(short lead),msR2(short lead),msR(short lead);
	inline short msS1(short lead),msS2(short lead);
	inline short msS(short lead);
	inline short msT(short lead);
	inline short msPR(short lead);
	inline short msQT(short lead);
	inline short msQRS(short lead);
	inline short msRpeak(short lead);
	inline short msQRSjudge(DWORD dwLeads,short msjudge=120);	//QRS间期大于msjudge的导联数
	inline short msQRSjudge(short *Lead,short n,short msjudge=120);	//同上
	inline short msPRjudge(short *Lead,short n,short msjudge=120);	//PR间期大于msjudge的导联数
	inline short uvP1(short lead),uvP2(short lead),uvP(short lead);//绝对值较大者
	inline short uvQ(short lead),uvQd(short lead);	//Qd for -aVR
	inline short uvR1(short lead),uvR2(short lead),uvR(short lead),uvRd(short);	//Qd for -aVR
	inline short uvS1(short lead),uvS2(short lead),uvS(short lead);
	inline short uvT1(short lead),uvT2(short lead),uvT(short lead);//绝对值较大者
	inline short uvTpp(short lead);	//峰峰值
	inline short uvQS(short lead);
	inline short uvST(short lead,short msJ=0);
	inline short uvSTj(short lead);
	inline short uvST1(short lead);
	inline short uvST2(short lead);
	inline short uvST3(short lead);
	inline short uvST20(short lead);
	inline short uvST40(short lead);
	inline short uvST60(short lead);
	inline short uvST80(short lead);
	inline short uvSTe(short lead);	//ST段终点或T波起点
	inline short uvNegaP(short lead);
	inline short uvPosiP(short lead);
	inline short uvNegaT(short lead);
	inline short uvPosiT(short lead);
	inline short uvQRSpp(short lead);
	inline short uvQRSnet(short lead);
	inline short bpmHR(short beat);
protected:
//	inline unsigned short vhcode(unsigned short codeGroup,unsigned short codeItem,unsigned short codeSub);
protected:
	bool	isR(short lead);
	bool	isQS(short lead);
	bool	isDownwardST(short lead);
	bool	isUpwardST(short lead);
	bool	isHorizontalST(short lead);
	bool	isUtypeST(short lead);
	bool	isNegativeP(short lead);
	bool	isPositiveP(short lead),notPositiveP(short lead);
	bool	isDualT(short lead);
	bool	isFlatT(short lead);
	bool	isNegativeT(short lead);
	bool	isPositiveT(short lead);
	bool	isSinalP();//,isSinalP(short beat);
	bool	noQ(short lead);
	bool	noS(short lead);
	bool	noS1(short lead);
	bool	noS2(short lead);
protected:
	float	ratioRS(short lead);
public:
	CEcgCodeBase(void);
	virtual ~CEcgCodeBase(void);
	void	SetEcgDataInfo(short fs,short chnum,double uVpb);
	void	SetEcgData(short **data,long length);
	void	SetEcgTempl(short templpos,short templen,short **templ);//call first
	void	SetEcgParm(VH_ECGparm *pEcgParm);
	void	SetEcglead(VH_ECGlead *pEcgLead);
	void	SetEcgInfo(VH_ECGinfo *pEcgInfo,char *pBeatsType);
	void	SetEcgLeadInfo(short leadidx,short chnidx);
	void	SetPatientInfo(char sex,short age,short ageYmd=0);
//	void	SetBeats(long nBeatsNum,char *pBeatsType);
	void	CheckQuality();
private:
	short	CheckQuality(short lead);
public:
	char	GetWPW()		{return (m_pEcgParm)?m_pEcgParm->WPW:' ';	}
	short	GetBeatsNum()	{return (m_pEcgInfo)?m_pEcgInfo->BeatsNum:0;}
	VH_ECGbeat *GetBeats()	{return m_pEcgBeat;	}
	short	GetArate()		{return (m_pEcgInfo)?m_pEcgInfo->Arate:0;	}
	short	GetVrate()		{return (m_pEcgInfo)?m_pEcgInfo->Vrate:0;	}
	short	GetAflutAfib()	{return (m_pEcgInfo)?m_pEcgInfo->AflutAfib:0;}	//0:none, 1:Aflut(房扑), 2:Afib(房颤)
	long*	GetPaceMaker(char &Type,short &SpikesN) { //'N': none, 'A': A-Type, 'V': V-type, 'B': Both
		if(m_pEcgInfo) {
			Type=m_pEcgInfo->PaceMaker;
			SpikesN=m_pEcgInfo->SpikesN;
			return m_pEcgInfo->SpikesPos;
			
		}
		Type='N';	SpikesN=0;
		return NULL;
	}
	char*	GetBeatsType()	{return m_pBeatsType;}
protected:
	short m_uv25,m_uv50,m_uv100,m_uv200,m_uv500;
	short m_ms20,m_ms120,m_ms180;
};

	
inline char* CEcgCodeBase::LeadNames(DWORD dwLeads)
{
	strcpy(m_szLeadName,"\0");
	for(short i=0;i<VH_EcgMaxLeads;i++) {
		if(dwLeads&m_infoLead[i].mask) {
			if(strlen(m_szLeadName)>0) strcat(m_szLeadName,",");
			strcat(m_szLeadName,m_infoLead[i].name);
		}
	}
	return m_szLeadName;
}
	
inline short CEcgCodeBase::LeadIndexFromChn(short chn)
{
	short lead=-1;
	for(short i=0;i<VH_EcgMaxLeads;i++) {
		if(chn==m_infoLead[i].chn) {
			lead=i;	break;
		}
	}
	return lead;
}
		
inline char* CEcgCodeBase::LeadNameFromChn(short chn)
{
	short lead=-1;
	for(short i=0;i<VH_EcgMaxLeads;i++) {
		if(chn==m_infoLead[i].chn) {
			return (char *)m_infoLead[i].name;
		}
	}
	return "";
}
inline short CEcgCodeBase::HRfromSamples(int samples)
{
	return 60*m_fs/samples;
}
inline short CEcgCodeBase::msfromSamples(int samples)
{
	return 1000*samples/m_fs;
}
inline int CEcgCodeBase::maxnpos(short *data,int n)
{
	int pos=0;
	for(int i=1;i<n;i++) {
		if(data[i]>data[pos]) pos=i;
	}
	return pos;
}
inline int CEcgCodeBase::minnpos(short *data,int n)
{
	int pos=0;
	for(int i=1;i<n;i++) {
		if(data[i]<data[pos]) pos=i;
	}
	return pos;
}
inline short CEcgCodeBase::ppvalue(short *data,int n)
{
	int pmax=0,pmin=0;
	for(int i=1;i<n;i++) {
		if(data[i]>data[pmax]) pmax=i;
		if(data[i]<data[pmin]) pmin=i;
	}
	return data[pmax]-data[pmin];
}

inline short CEcgCodeBase::Pb()				{return (m_pEcgParm)?m_pEcgParm->OnOff[0]:0;}
inline short CEcgCodeBase::Pb(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].OnOff[0]:0;}
inline short CEcgCodeBase::Pe()				{return (m_pEcgParm)?m_pEcgParm->OnOff[1]:0;}
inline short CEcgCodeBase::Pe(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].OnOff[1]:0;}
inline short CEcgCodeBase::Qb()				{return (m_pEcgParm)?m_pEcgParm->OnOff[2]:0;}
inline short CEcgCodeBase::Qb(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].OnOff[2]:0;}
inline short CEcgCodeBase::Se()				{return (m_pEcgParm)?m_pEcgParm->OnOff[3]:0;}
inline short CEcgCodeBase::Se(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].OnOff[3]:0;}
inline short CEcgCodeBase::Tb()				{return (m_pEcgParm)?m_pEcgParm->OnOff[4]:0;}
inline short CEcgCodeBase::Tb(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].OnOff[4]:0;}
inline short CEcgCodeBase::Te()				{return (m_pEcgParm)?m_pEcgParm->OnOff[5]:0;}
inline short CEcgCodeBase::Te(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].OnOff[5]:0;}
inline short CEcgCodeBase::J()				{return (m_pEcgParm)?m_pEcgParm->OnOff[3]:0;}
inline short CEcgCodeBase::J(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].OnOff[3]:0;}

inline short CEcgCodeBase::msRR()		{return (m_pEcgParm)?m_pEcgParm->RR:0;}
inline short CEcgCodeBase::bpmHR()		{return (m_pEcgParm)?m_pEcgParm->HR:0;}
inline short CEcgCodeBase::msPd()		{return (m_pEcgParm)?m_pEcgParm->Pd:0;}
inline short CEcgCodeBase::msPR()		{return (m_pEcgParm)?m_pEcgParm->PR:0;}
inline short CEcgCodeBase::msQRS()		{return (m_pEcgParm)?m_pEcgParm->QRS:0;}
inline short CEcgCodeBase::msQT()		{return (m_pEcgParm)?m_pEcgParm->QT:0;}
inline short CEcgCodeBase::msQTc()		{return (m_pEcgParm)?m_pEcgParm->QTC:0;}
inline char  CEcgCodeBase::WPW()		{return (m_pEcgParm)?m_pEcgParm->WPW:0;} //return: A/a B/b W/w  A型, B型, W型, 小写: 可疑, 其它: 无WPW
inline short CEcgCodeBase::msQTdis()	{return (m_pEcgParm)?m_pEcgParm->QTdis:0;}
inline short CEcgCodeBase::msQTmax()	{return (m_pEcgParm)?m_pEcgParm->QTmax:0;}
inline short CEcgCodeBase::msQTmin()	{return (m_pEcgParm)?m_pEcgParm->QTmin:0;}
inline char* CEcgCodeBase::QTmaxLead()	{return (m_pEcgParm)?LeadNameFromChn(m_pEcgParm->QTmaxLead):0;}
inline char* CEcgCodeBase::QTminLead()	{return (m_pEcgParm)?LeadNameFromChn(m_pEcgParm->QTminLead):0;}
inline short CEcgCodeBase::axisP()		{return (m_pEcgParm)?m_pEcgParm->axisP:0;}
inline short CEcgCodeBase::axisQRS()	{return (m_pEcgParm)?m_pEcgParm->axisQRS:0;}
inline short CEcgCodeBase::axisT()		{return (m_pEcgParm)?m_pEcgParm->axisT:0;}  //return Degree,  for ChNumber==12 only
inline short CEcgCodeBase::uvRV5()		{
//	return (m_pEcgParm)?m_pEcgParm->uvRV5:0;
//	return (m_pEcgLead)?max(m_pEcgLead[m_infoLead[V5].chn].Ra1,m_pEcgLead[m_infoLead[V5].chn].Ra2):0;
	return uvRV(V5);
}   //return unit: uV,         for ChNumber==12 only
inline short CEcgCodeBase::uvRV6()		{
//	return (m_pEcgParm)?m_pEcgParm->uvRV6:0;
//	return (m_pEcgLead)?max(m_pEcgLead[m_infoLead[V6].chn].Ra1,m_pEcgLead[m_infoLead[V6].chn].Ra2):0;
	return uvRV(V6);
}   //return unit: uV,         for ChNumber==12 only
inline short CEcgCodeBase::uvRV(short lead)
{
	return (m_pEcgLead)?max(m_pEcgLead[m_infoLead[lead].chn].Ra1,m_pEcgLead[m_infoLead[lead].chn].Ra2):0;
}
inline short CEcgCodeBase::uvSV(short lead)
{
	short SV=0;
	if(m_pEcgLead) {
		short SV=min(m_pEcgLead[m_infoLead[lead].chn].Sa1,m_pEcgLead[m_infoLead[lead].chn].Sa2);
		if(uvRV(lead)<=0) SV=min(SV,m_pEcgLead[m_infoLead[lead].chn].Qa);
	}
	return SV;
}
inline short CEcgCodeBase::uvSV1()		{
//	return (m_pEcgParm)?m_pEcgParm->uvSV1:0;
//	return (m_pEcgLead)?min(m_pEcgLead[m_infoLead[V1].chn].Sa1,m_pEcgLead[m_infoLead[V1].chn].Sa2):0;
	return uvSV(V1);
}   //return unit: uV,         for ChNumber==12 only
inline short CEcgCodeBase::uvSV2()		{
//	return (m_pEcgParm)?m_pEcgParm->uvSV2:0;
//	return (m_pEcgLead)?min(m_pEcgLead[m_infoLead[V2].chn].Sa1,m_pEcgLead[m_infoLead[V2].chn].Sa2):0;
	return uvSV(V2);
}   //return unit: uV,         for ChNumber==12 only
inline short CEcgCodeBase::uvRV1()		{
//	return (m_pEcgParm)?m_pEcgParm->uvRV1:0;
//	return (m_pEcgLead)?max(m_pEcgLead[m_infoLead[V1].chn].Ra1,m_pEcgLead[m_infoLead[V1].chn].Ra2):0;
	return uvRV(V1);
}
inline short CEcgCodeBase::uvSV5()		{
//	return (m_pEcgParm)?m_pEcgParm->uvSV5:0;
//	return (m_pEcgLead)?min(m_pEcgLead[m_infoLead[V5].chn].Sa1,m_pEcgLead[m_infoLead[V5].chn].Sa2):0;
	return uvSV(V5);
}

inline short CEcgCodeBase::msP(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Pd:0;}
inline short CEcgCodeBase::msNegaP(short lead) {//近似求-P ms
	if(m_pEcgLead) {
		short P1=abs(uvP1(lead));
		short P2=abs(uvP2(lead));
		if(uvP1(lead)<=0 && uvP2(lead)<=0) {
			return m_pEcgLead[m_infoLead[lead].chn].Pd;
		}
		else if(uvP1(lead)<0) {
			return m_pEcgLead[m_infoLead[lead].chn].Pd*P1/(P1+P2);
		}
		else if(uvP2(lead)<0) {
			return m_pEcgLead[m_infoLead[lead].chn].Pd*P2/(P1+P2);
		}
	}
	return 0;
}
inline short CEcgCodeBase::msQ(short lead)	
{
	if(m_pEcgLead) {
		short chn=m_infoLead[lead].chn;
		if(isQS(lead)) return max(m_pEcgLead[chn].Qd,m_pEcgLead[chn].Sd1+m_pEcgLead[chn].Sd2);
		else return m_pEcgLead[chn].Qd;
	}
	return 0;
}
inline short CEcgCodeBase::msQd(short lead)	//for MI
{
	if(m_pEcgLead) {
		short chn=m_infoLead[lead].chn;
		if(lead!=aVR) {
			if(isQS(lead)) return max(m_pEcgLead[chn].Qd,m_pEcgLead[chn].Sd1+m_pEcgLead[chn].Sd2);
			else return m_pEcgLead[chn].Qd;
		}
		else {
			short Qd=0;
			if(uvR2(lead)>0) Qd=msR1(lead);
			else {
				if(uvQ(lead)<0) Qd=0;	//QS or QR or QRS
				else Qd=msR1(lead);		//R or RS
			}
			return Qd;
		}
	}
	return 0;
}
inline short CEcgCodeBase::msR1(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Rd1:0;}
inline short CEcgCodeBase::msR2(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Rd2:0;}
inline short CEcgCodeBase::msR(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Rd1+m_pEcgLead[m_infoLead[lead].chn].Rd2:0;}
inline short CEcgCodeBase::msS1(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Sd1:0;}
inline short CEcgCodeBase::msS2(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Sd2:0;}
inline short CEcgCodeBase::msS(short lead)
{
	if(m_pEcgLead) {
		short chn=m_infoLead[lead].chn;
		if(isQS(lead)) return max(m_pEcgLead[chn].Qd,m_pEcgLead[chn].Sd1+m_pEcgLead[chn].Sd2);
		else return m_pEcgLead[chn].Sd1+m_pEcgLead[chn].Sd2;
	}
	return 0;
}
inline short CEcgCodeBase::msT(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Td:0;}
inline short CEcgCodeBase::msPR(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].PR:0;}
inline short CEcgCodeBase::msQT(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].QT:0;}
inline short CEcgCodeBase::msQRS(short lead){return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].QRS:0;}
inline short CEcgCodeBase::msRpeak(short lead) {
	if(m_templ) {
		short qb=Qb(lead),se=Se(lead)-2,rp=qb;
		short start=0;
		if(se>qb) {
			short *templ=m_templ[m_infoLead[lead].chn];
			for(short i=qb+2;i<se;i++) {
				if(start) {
					 if(templ[i]>templ[rp]) rp=i;
				}
				else {
					if((templ[i]-templ[qb])>m_uv50) start=i-1;
//					if(templ[i-1]-templ[qb]<0 && templ[i]>templ[i-1]  && templ[i+1]>templ[i] && templ[i-1]<templ[i-2]) start=i-1;
				}
			}
		}
		return (start>0 && rp>start)?1000*(rp-start)/m_fs:0;
	}
	return 0;
}

inline short CEcgCodeBase::msQRSjudge(DWORD dwLeads,short msjudge)	//QRS间期大于msjudge的导联数
{
	short count=0;
	if(m_pEcgLead) {
		for(short i=0;i<VH_EcgMaxLeads;i++) {
			if(dwLeads && m_infoLead[i].mask) {
				if(msQRS(i)>=msjudge) count++;
			}
		}
	}
	return count;
}

inline short CEcgCodeBase::msPRjudge(short *Lead,short n,short msjudge)	//同上
{
	short count=0;
	if(m_pEcgLead) {
		for(short i=0;i<n;i++) {
			if(msPR(Lead[i])>=msjudge) count++;
		}
	}
	return count;
}
inline short CEcgCodeBase::msQRSjudge(short *Lead,short n,short msjudge)	//同上
{
	short count=0;
	if(m_pEcgLead) {
		for(short i=0;i<n;i++) {
			if(msQRS(Lead[i])>=msjudge) count++;
		}
	}
	return count;
}

inline short CEcgCodeBase::uvP1(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Pa1:0;}
inline short CEcgCodeBase::uvP2(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Pa2:0;}
inline short CEcgCodeBase::uvP(short lead)
{
	if(m_pEcgLead) {
		if(uvPosiP(lead)>=-uvNegaP(lead)) return uvPosiP(lead);
		else return uvNegaP(lead);
	}
	return 0;
}
inline short CEcgCodeBase::uvQ(short lead)	
{
	if(m_pEcgLead) {
		short chn=m_infoLead[lead].chn;
		if(isQS(lead)) return min3(m_pEcgLead[chn].Qa,m_pEcgLead[chn].Sa1,m_pEcgLead[chn].Sa2);
		else return m_pEcgLead[chn].Qa;
	}
	return 0;
}
inline short CEcgCodeBase::uvQd(short lead)	//for MI
{
	if(m_pEcgLead) {
		short chn=m_infoLead[lead].chn;
		if(lead!=aVR) {
			if(isQS(lead)) return min3(m_pEcgLead[chn].Qa,m_pEcgLead[chn].Sa1,m_pEcgLead[chn].Sa2);
			else return m_pEcgLead[chn].Qa;
		}
		else {
			short uvQd=0;
			if(uvR2(lead)>0) uvQd=-uvR1(lead);
			else {
				if(uvQ(lead)<0) uvQd=0;	//QS or QR or QRS
				else uvQd=-uvR1(lead);		//R or RS
			}
			return uvQd;
		}
	}
	return 0;
}
inline short CEcgCodeBase::uvR1(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Ra1:0;}
inline short CEcgCodeBase::uvR2(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Ra2:0;}
inline short CEcgCodeBase::uvR(short lead)	{return max(uvR1(lead),uvR2(lead));}
inline short CEcgCodeBase::uvRd(short lead)
{
	if(lead!=aVR) {
		return uvR(lead);
	}
	else {
		return -min(uvQ(lead),uvS(lead));
	}
}
inline short CEcgCodeBase::uvS1(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Sa1:0;}
inline short CEcgCodeBase::uvS2(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Sa2:0;}
inline short CEcgCodeBase::uvS(short lead)
{
	short uv=0;
	if(m_pEcgLead) {
		short chn=m_infoLead[lead].chn;
		if(isQS(lead)) uv=min3(m_pEcgLead[chn].Qa,m_pEcgLead[chn].Sa1,m_pEcgLead[chn].Sa2);
		else uv=min(m_pEcgLead[chn].Sa1,m_pEcgLead[chn].Sa2);
	}
	return uv;
}
inline short CEcgCodeBase::uvT1(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Ta1:0;}
inline short CEcgCodeBase::uvT2(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].Ta2:0;}
inline short CEcgCodeBase::uvT(short lead)	
{
	if(m_pEcgLead) {
		if(uvPosiT(lead)>=-uvNegaT(lead)) return uvPosiT(lead);
		else return uvNegaT(lead);
	}
	return 0;
}
inline short CEcgCodeBase::uvTpp(short lead)	//峰峰值
{
	return uvPosiT(lead)-uvNegaT(lead);
}
inline short CEcgCodeBase::uvQS(short lead) 
{
	if(m_pEcgLead) {
		short chn=m_infoLead[lead].chn;
		return min3(m_pEcgLead[chn].Qa,m_pEcgLead[chn].Sa1,m_pEcgLead[chn].Sa2);
	}
	return 0;
}

inline short CEcgCodeBase::uvST(short lead,short msJ)
{
	if(m_templ) {
		short i=m_infoLead[lead].chn;
		short j=J(lead)+msJ*m_fs/1000;
		if(j>0 && j<m_templen) {
			short z=-1;
			if(Pe()>0 && Qb()>Pe()) z=(Pe()+Qb())/2;
			else if(Qb()>0) z=Qb()-20*m_fs/1000;
			if(z>0) return short((m_templ[i][j]-m_templ[i][z])*m_uVpb);
		}
	}
	return 0;
}
inline short CEcgCodeBase::uvSTj(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].ST[0]:0;}
inline short CEcgCodeBase::uvST1(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].ST[1]:0;}
inline short CEcgCodeBase::uvST2(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].ST[2]:0;}
inline short CEcgCodeBase::uvST3(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].ST[3]:0;}
inline short CEcgCodeBase::uvST20(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].ST[4]:0;}
inline short CEcgCodeBase::uvST40(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].ST[5]:0;}
inline short CEcgCodeBase::uvST60(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].ST[6]:0;}
inline short CEcgCodeBase::uvST80(short lead)	{return (m_pEcgLead)?m_pEcgLead[m_infoLead[lead].chn].ST[7]:0;}
inline short CEcgCodeBase::uvSTe(short lead)	//ST段终点或T波起点
{
	if(m_templ) {
		short i=m_infoLead[lead].chn;
		short j=Tb(lead);
		if(j>0 && j<m_templen) {
			short z=-1;
			if(Pe()>0 && Qb()>Pe()) z=(Pe()+Qb())/2;
			else if(Qb()>0) z=Qb()-20*m_fs/1000;
			if(z>0) return short((m_templ[i][j]-m_templ[i][z])*m_uVpb);
		}
	}
	return 0;
}
inline short CEcgCodeBase::uvNegaP(short lead)	{short Pmin=min(uvP1(lead),uvP2(lead));	return (Pmin<0)?Pmin:0;}
inline short CEcgCodeBase::uvPosiP(short lead)	{short Pmax=max(uvP1(lead),uvP2(lead));	return (Pmax>0)?Pmax:0;}
inline short CEcgCodeBase::uvNegaT(short lead)	{short Tmin=min(uvT1(lead),uvT2(lead));	return (Tmin<0)?Tmin:0;}
inline short CEcgCodeBase::uvPosiT(short lead)	{short Tmax=max(uvT1(lead),uvT2(lead));	return (Tmax>0)?Tmax:0;}
inline short CEcgCodeBase::uvQRSpp(short lead)	{return uvR(lead)-min(uvQ(lead),uvS(lead));}
inline short CEcgCodeBase::uvQRSnet(short lead)	{return uvR(lead)+min(uvQ(lead),uvS(lead));}
inline short CEcgCodeBase::bpmHR(short beat)	{
	if(!m_pEcgBeat || beat<1 || beat>=GetBeatsNum()) return 0;
	return short(60*(m_pEcgBeat[beat].Pos-m_pEcgBeat[beat-1].Pos)/m_fs);
}
inline bool	CEcgCodeBase::noQ(short lead) {
	return (msQ(lead)>0)?false:true;
}
inline bool	CEcgCodeBase::noS(short lead)
{
	return (msS(lead)>0)?false:true;
}
inline bool	CEcgCodeBase::noS1(short lead)
{
	return (msS1(lead)>0)?false:true;
}
inline bool	CEcgCodeBase::noS2(short lead)
{
	return (msS2(lead)>0)?false:true;
}
inline float CEcgCodeBase::ratioRS(short lead)
{
	if(m_pEcgLead) {
		short ch=m_infoLead[lead].chn;
		if(ch<0 || ch>=m_chnum) return -1;
		float R=float(uvR(lead)),S=float(abs(uvS(lead)));
		if(S>0) return R/S;
	}
	return -1;
}
//inline unsigned short CEcgCodeBase::vhcode(unsigned short codeGroup,unsigned short codeItem,unsigned short codeSub)
//{
//	VHCODE c;
//	c.codeGroup=codeGroup;
//	c.codeItem=codeItem;
//	c.codeSub=codeSub;
//	return c.nCode;
//}
