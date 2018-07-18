#include "stdafx.h"
#include "EcgCodeBase.h"
#include <math.h>

CMean::CMean()
{
	init();
}
CMean::~CMean()
{
}
void CMean::init()
{
	i=0;	m=0;
}
double CMean::mean(double x)
{
	i++;
	m=x/i+(i-1.0)/i*m;
	return m;
}

///////////////////////////////////////////////////////////////////////
//begin class CEcgCodeBase
CEcgCodeBase::CEcgCodeBase(void)
	: m_fs(0)
	, m_chnum(0)
	, m_length(0)
	, m_seconds(0)
	, m_data(NULL)
	, m_templpos(0)
	, m_templen(0)
	, m_templ(NULL)
	, m_sex('M')	//'M','F'
	, m_ageD(0),m_ageM(0),m_ageY(35)
	, m_pEcgParm(NULL)
	, m_pEcgLead(NULL)
	, m_pEcgBeat(NULL)
	, m_pEcgInfo(NULL)
	, m_pBeatsType(NULL)		//心搏类型。目前只有N:正常，V:室性，B:边界无法判定，P:起搏
//	, m_nBeatsNum(0)
	, m_meanNN(0)
	, m_SDNN(0)
{
	for(short i=0;i<VH_EcgMaxLeads;i++) m_infoLead[i]=initialEcgLeadInfo[i];
	strcpy(m_szLeadName,"\0");
}

CEcgCodeBase::~CEcgCodeBase(void)
{
	//if(m_pEcgParm)	delete m_pEcgParm;	m_pEcgParm=NULL;
	//if(m_pEcgLead)	delete[]m_pEcgLead;	m_pEcgLead=NULL;
	//if(m_pEcgBeat)	delete[]m_pEcgBeat;	m_pEcgBeat=NULL;
	//if(m_pEcgInfo)	delete[]m_pEcgInfo;	m_pEcgInfo=NULL;
}

void CEcgCodeBase::SetEcgDataInfo(short fs,short chnum,double uVpb)
{
	m_fs=fs;
	m_chnum=chnum;
	m_uVpb=uVpb;

	m_uv25=short(25/m_uVpb+0.5);
	m_uv50=short(50/m_uVpb+0.5);
	m_uv100=short(100/m_uVpb+0.5);
	m_uv200=short(200/m_uVpb+0.5);
	m_uv500=short(500/m_uVpb+0.5);

	m_ms20=20*m_fs/1000;
	m_ms120=120*m_fs/1000;
	m_ms180=180*m_fs/1000;
}

void CEcgCodeBase::SetEcgData(short **data,long length)
{
	m_data=data;
	m_length=length;
	m_seconds=short(m_length/m_fs);
}

void CEcgCodeBase::SetEcgTempl(short templpos,short templen,short **templ)
{
	m_templpos=templpos;
	m_templen=templen;
	m_templ=templ;
}
	
void CEcgCodeBase::SetEcgParm(VH_ECGparm *pEcgParm)
{
	m_pEcgParm=pEcgParm;
}
	
void CEcgCodeBase::SetEcglead(VH_ECGlead *pEcgLead)
{
	m_pEcgLead=pEcgLead;
}
	
void CEcgCodeBase::SetEcgInfo(VH_ECGinfo *pEcgInfo,char *pBeatsType)
{
	m_pEcgInfo=pEcgInfo;
	m_pEcgBeat=m_pEcgInfo->Beats;
	m_pBeatsType=pBeatsType;

	m_meanNN=0;
	m_SDNN=0;
	int i=0,j=0,n=m_pEcgInfo->BeatsNum;
	double NN=0,preNN=0;
	for(i=1;i<n;i++) {
		if(m_pBeatsType[i]=='N' && m_pBeatsType[i-1]=='N') {
			NN=m_pEcgBeat[i].Pos-m_pEcgBeat[i-1].Pos;
			if(preNN>0 && (NN>=0.75*preNN && NN<=1.25*preNN)) {
				j++;
				m_meanNN=NN/j+m_meanNN*(j-1.0)/j;
			}
			preNN=NN;
		}
	}
	j=0;	preNN=0;
	for(i=1;i<n;i++) {
		if(m_pBeatsType[i]=='N' && m_pBeatsType[i-1]=='N') {
			NN=m_pEcgBeat[i].Pos-m_pEcgBeat[i-1].Pos;
			if(preNN>0 && (NN>=0.75*preNN && NN<=1.25*preNN)) {
				j++;
				m_SDNN=square(NN-m_meanNN)/j+m_SDNN*(j-1.)/j;
			}
			preNN=NN;
		}
	}
	if(j>n/2) m_SDNN=sqrt(j*m_SDNN/(j-1));	else m_SDNN=-1;
	m_meanNN=1000*m_meanNN/m_fs;
	m_SDNN=1000*m_SDNN/m_fs;

	QT=msQT();	QRS=msQRS();	HR=bpmHR();
	JT=QT-QRS;
	QTI=(HR+100)*QT/656;
	JTI=(HR+100)*JT/518;
}

void CEcgCodeBase::SetEcgLeadInfo(short leadidx,short chnidx)
{
	m_infoLead[leadidx].chn=chnidx;
}

///////////////////////////////////////////////////////////
/*
功能：		设置病人信息
输入参数：	sex		病人性别	字符：'F':女，其他:男
			age		病人年龄	整型
			ageYmd	年龄单位	整型，0:岁，1:月, 2:天
输出参数：	无
*/
//////////////////////////////////////////////////////////
void CEcgCodeBase::SetPatientInfo(char sex,short age,short ageYmd)
{
	if(sex!='M' && sex!='F') sex='M';
	m_sex=sex;
	if(age<0) {
		age=35;	ageYmd=0;
	}
	switch(ageYmd) {
		case 1:	case 'M':	case 'm':	m_ageD=age*30;	m_ageM=age;	m_ageY=age/12;	break;
		case 2:	case 'D':	case 'd':	m_ageD=age;	m_ageM=age/30;	m_ageY=age/365;	break;
		default:	m_ageD=age*365;	m_ageM=age/12;	m_ageY=age;	break;
	}
}

void CEcgCodeBase::CheckQuality()
{
	for(short i=0;i<VH_EcgMaxLeads;i++) {
		if(m_infoLead[i].quality>=0) continue;
		m_infoLead[i].quality=CheckQuality(i);
	}
}
short CEcgCodeBase::CheckQuality(short lead)
{
	if(m_length<=0) return -1;

	short ch=m_infoLead[lead].chn;
	if(ch<0 || ch>=m_chnum) return -2;

	short *data=m_data[ch];
	int seconds=m_length/m_fs;
	int i=1,j=0,k=0;
	int w=m_ms180,w1=w/2,length=m_length-w1,w0=w/10;	if(w0<1) w0=1;
	int high=0,total=0;
	for(i=length/4;i<length;i++) {
		if(data[i]<-m_uv100 || data[i]>m_uv100) high++;
	}
	if(high<length/1000) return 3;//no signal
	if(lead>=V1) {
		high=0;total=0;
		short *dataII =m_data[m_infoLead[II].chn];
		short *dataIII=m_data[m_infoLead[III].chn];
		for(i=length/4;i<length;i++) {
			if(abs(dataII[i])>m_uv50 && abs(dataIII[i])>m_uv50) {
				total++;
				if(abs(data[i]-(dataII[i]+dataIII[i])/3)>m_uv25) high++;
			}
		}
		if(high<total/20) return 3;//no signal
	}
	if(m_templ && m_templen>0) {
		data=m_templ[ch];
		length=m_templen;
		high=0;
		for(i=0;i<length;i++) {
			if(data[i]<-m_uv25 || data[i]>m_uv25) high++;
		}
		if(high<m_ms20) return 3;//no signal
		if(lead>=V1) {
			high=0;
			short *dataII =m_data[m_infoLead[II].chn];
			short *dataIII=m_data[m_infoLead[III].chn];
			for(i=0;i<length;i++) {
				if(abs(data[i]-(dataII[i]+dataIII[i])/3)>m_uv25) high++;
			}
			if(high<m_ms20) return 3;//no signal
		}
	}
	else {
		int maxp0=0,minp0=0,maxp=0,minp=0;
		int VF=0,Vf=0;
		for(i=length/4;i<length;i++) {
			if(data[i]-data[i-w1]>m_uv200 && data[i]-data[i+w1]>m_uv200 && data[i]>data[i-w0] && data[i]>data[i+w0] && data[i]>m_uv50) {
				if(maxp0==0) maxp0=i;
				else if(i>maxp0) {
					maxp=i;
					k=16*(maxp-maxp0);
					if(k>=2*m_fs && k<=8*m_fs) {//2Hz-8Hz(480bpm-120bpm)
						minp=maxp0;
						for(j=maxp0+1;j<maxp-1;j++) {
							if(data[j]-data[j-w1]<-m_uv200 && data[j]-data[j+w1]<-m_uv200 && data[j]<data[j-w0] && data[j]<data[j+w0] && data[j]<-m_uv50) {
								minp=j;	break;
							}
						}
						k=(maxp-maxp0)/3;
						if(minp-maxp0>k && maxp-minp>k) VF++;
					}
					maxp0=maxp;
				}
				i+=w1;
			}
		}
		if(VF/seconds>0) {
			high=total=0;
			for(i=length/4;i<length;i++) {
				if(abs(data[i]>m_uv200)) {
					total++;
					if(abs(data[i]-data[i-1])>m_uv100) high++;
				}
			}
			if(total<length/20 || high<total/20) return 2;//Vf
			else return 1;//VF
		}
	}
	return -1;
}
//
//void CEcgCodeBase::SetBeats(long nBeatsNum,char *pBeatsType)
//{
//	m_nBeatsNum=nBeatsNum;
//	m_pBeatsType=pBeatsType;
//}

bool	CEcgCodeBase::isR(short lead)
{
	if(m_pEcgLead) {
		short chn=m_infoLead[lead].chn;
		if(uvR(lead)>=100 && uvQ(lead)>-100 && uvS(lead)>-100) return true;
	}
	return false;
}

bool	CEcgCodeBase::isQS(short lead)	
{
	if(m_pEcgLead) {
		short chn=m_infoLead[lead].chn;
		short Ra=max(m_pEcgLead[chn].Ra1,m_pEcgLead[chn].Ra2);
		short Qa=m_pEcgLead[chn].Qa;
		short Sa=min(m_pEcgLead[chn].Sa1,m_pEcgLead[chn].Sa2);
		short msR1=m_pEcgLead[chn].Rd1;
		short msR2=m_pEcgLead[chn].Rd2;
		if(Ra>=50) return false;
		if(Ra>=25 && (msR1>=18 || msR2>=18)) return false;
		if(Qa<=-100 || Sa<=-100) return true;
	}
	return false;
}
bool	CEcgCodeBase::isDownwardST(short lead)
{
	if(!isUtypeST(lead)) {
		if(m_pEcgLead) {
			if(uvST1(lead)-uvST2(lead)>25) return true;
		}
	}
	return false;
}
bool	CEcgCodeBase::isUpwardST(short lead)
{
	if(m_pEcgLead) {
		if(uvST2(lead)-uvST1(lead)>25) return true;
	}
	return false;
}
bool	CEcgCodeBase::isHorizontalST(short lead)
{
	if(!isUtypeST(lead)) {
		if(m_pEcgLead) {
			if(abs(uvST2(lead)-uvST1(lead))<=25) return true;
		}
	}
	return false;
}
bool	CEcgCodeBase::isUtypeST(short lead)
{
	if(m_templ) {
		short i=m_infoLead[lead].chn;
		short j=Se(lead),k=Tb(lead);
		if(k<j) k=j+200*m_fs/1000;
		short b=(j+k)/2;
		if(m_templ[i][j]-m_templ[i][b]>50 && m_templ[i][k]-m_templ[i][b]>50) return true;
	}
	return 0;
}
bool	CEcgCodeBase::isNegativeT(short lead)
{
	if(m_pEcgLead) {
		if(uvPosiT(lead)==0 && uvNegaT(lead)<0) return true;
	}
	return false;
}
bool	CEcgCodeBase::isPositiveT(short lead)
{
	if(m_pEcgLead) {
		if(uvNegaT(lead)==0 && uvPosiT(lead)>0) return true;
	}
	return false;
}
bool	CEcgCodeBase::isDualT(short lead)
{
	if(m_pEcgLead) {
		if(uvT1(lead)*uvT2(lead)<0) return true;
	}
	return false;
}
bool	CEcgCodeBase::isFlatT(short lead)
{
	if(m_pEcgLead) {
		if(abs(uvT1(lead))<100 && abs(uvT2(lead))<100) return true;
	}
	return false;
}
bool	CEcgCodeBase::isNegativeP(short lead)
{
	if(m_pEcgLead) {
		if(uvPosiP(lead)==0 && uvNegaP(lead)<0) return true;
	}
	return false;
}
bool	CEcgCodeBase::isPositiveP(short lead)
{
	if(m_pEcgLead) {
		if(uvNegaP(lead)==0 && uvPosiP(lead)>0) return true;
	}
	return false;
}
bool	CEcgCodeBase::notPositiveP(short lead)
{
	if(m_pEcgLead) {
		if(uvPosiP(lead)==0 && uvNegaP(lead)<=0) return true;
	}
	return false;
}
bool	CEcgCodeBase::isSinalP()
{
	if(isPositiveP(II) && notPositiveP(aVR)) return true;
	return false;
}
//bool	CEcgCodeBase::isSinalP(short beat)
//{
//	short beatsnum=GetBeatsNum();
//	VH_ECGbeats *beats=GetBeats();
//}

//end class CEcgCodeBase
