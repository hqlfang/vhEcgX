// HspEcgDiagEx2.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"

#include "ECGpropEx.h"
#include "EcgCodeVH.h"
#include "vhEcgDiag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(disable:4819)	// warning C4819: 该文件包含不能在当前代码页(936)中表示的字符。请将该文件保存为 Unicode 格式以防止数据丢失

ECGprop		*g_pEcgProp=NULL;
CvhCode		*g_vhCode=NULL;
//CodeList	*rCodeList=NULL;
//MultiLead_Templates	*g_pEcgTemplates=NULL;

CvhEcgDiag::CvhEcgDiag()
{
	g_pEcgProp=NULL;
	g_vhCode=new CvhCode;
//	m_nSex=m_nAge=0;
	for(int i=0;i<12;i++)	m_nVindex[i]=i+6;
	m_pEcgParm=new VH_ECGparm;
	m_pEcgLead=new VH_ECGlead[VH_EcgMaxLeads];
	m_pEcgInfo=new VH_ECGinfo;
	//m_pTemplate=new VH_Template;
	m_pEcgBeat=NULL;
//	m_pBeatsType=NULL;
}

CvhEcgDiag::~CvhEcgDiag()
{
	if(m_pEcgParm)	delete m_pEcgParm;	m_pEcgParm=NULL;	//VH_ECGparm
	if(m_pEcgLead)	delete[]m_pEcgLead;	m_pEcgLead=NULL;
	if(m_pEcgBeat)	delete[]m_pEcgBeat;	m_pEcgBeat=NULL;
	if(m_pEcgInfo)	delete m_pEcgInfo;	m_pEcgInfo=NULL;
	//if(m_pTemplate)	delete m_pTemplate;	m_pTemplate=NULL;

	if(g_pEcgProp) delete g_pEcgProp;	g_pEcgProp=NULL;
	if(g_vhCode) delete g_vhCode;	g_vhCode=NULL;
}

void CvhEcgDiag::SetVindex(Vindex index, short chn)
{
	if(index<V1 || index>V9) return;
	if(chn<6 || chn>=18) chn=-1;
	m_nVindex[index]=chn;
}

BOOL CvhEcgDiag::CreateEcgDiag(short ChNumber,short **DataIn,short Seconds,short Samplerate,double uVperbit)
{
	if(g_pEcgProp) delete g_pEcgProp;	g_pEcgProp=NULL;
	g_pEcgProp=new ECGprop(ChNumber,DataIn,Seconds,Samplerate,uVperbit);
	short **data=g_pEcgProp->m_pDataIn;
	long length=Seconds*Samplerate;
	g_vhCode->SetEcgDataInfo(Samplerate,ChNumber,uVperbit);
	g_vhCode->SetEcgData(data,length);
	if(g_pEcgProp && g_pEcgProp->AutoProcess()) {
		short fs=Samplerate,chnum=ChNumber;
		double uVpb=uVperbit;
		short templpos=g_pEcgProp->Temp.Pos;
		short templen=g_pEcgProp->Temp.Length;
		short **templ=g_pEcgProp->Temp.Data;
		g_vhCode->SetEcgTempl( templpos, templen, templ);//call first
		//short ChN,SampleRate;//通道数，采样频率。********新增
		//double Uvperbit;	//每位微伏数
		//short Length;  //length of template data
		//short Pos;     //叠加位置。***********************新增
		//short **Data;	//template data (8通道)
		//short Left,Right;  //analysis range is from Left to Right
		//short SpikeA,SpikeV; // 房室起搏钉位置

		SetParameters();

		return TRUE;
	}
	else {
		if(g_pEcgProp) delete g_pEcgProp;	g_pEcgProp=NULL;
		return FALSE;
	}
}
void CvhEcgDiag::SetParameters()
{
	if(g_pEcgProp) {
		short templchn=g_pEcgProp->GetTemplChN();
		for(short i=0;i<templchn;i++) {
			for(short j=0;j<6;j++) m_pEcgLead[i].OnOff[j]=g_pEcgProp->Lead[i].OnOff[j];	//Pb,Pe,QRSb,QRSe,Tb,Te;	//特征点位置
			m_pEcgLead[i].Pstatus=g_pEcgProp->Lead[i].Pstatus;   //0: none, 1: +, 2: -; 3: +-; 4: -+
			m_pEcgLead[i].Tstatus=g_pEcgProp->Lead[i].Tstatus;
			m_pEcgLead[i].Pd	=g_pEcgProp->Lead[i].Pd;
			m_pEcgLead[i].Qd	=g_pEcgProp->Lead[i].Qd;
			m_pEcgLead[i].Rd1	=g_pEcgProp->Lead[i].Rd1;
			m_pEcgLead[i].Rd2	=g_pEcgProp->Lead[i].Rd2;
			m_pEcgLead[i].Sd1	=g_pEcgProp->Lead[i].Sd1;
			m_pEcgLead[i].Sd2	=g_pEcgProp->Lead[i].Sd2;
			m_pEcgLead[i].Td	=g_pEcgProp->Lead[i].Td;
			m_pEcgLead[i].PR	=g_pEcgProp->Lead[i].PR;
			m_pEcgLead[i].QT	=g_pEcgProp->Lead[i].QT;
			m_pEcgLead[i].QRS	=g_pEcgProp->Lead[i].QRS;
			m_pEcgLead[i].Pa1	=g_pEcgProp->Lead[i].Pa1;
			m_pEcgLead[i].Pa2	=g_pEcgProp->Lead[i].Pa2;
			m_pEcgLead[i].Qa	=g_pEcgProp->Lead[i].Qa;
			m_pEcgLead[i].Ra1	=g_pEcgProp->Lead[i].Ra1;
			m_pEcgLead[i].Ra2	=g_pEcgProp->Lead[i].Ra2;
			m_pEcgLead[i].Sa1	=g_pEcgProp->Lead[i].Sa1;
			m_pEcgLead[i].Sa2	=g_pEcgProp->Lead[i].Sa2;
			m_pEcgLead[i].Ta1	=g_pEcgProp->Lead[i].Ta1;
			m_pEcgLead[i].Ta2	=g_pEcgProp->Lead[i].Ta2;
			m_pEcgLead[i].Rnotch=g_pEcgProp->Lead[i].Rnotch;	//0,1,2,3: none,上升边，下降边，两边
			for(short j=0;j<8;j++) m_pEcgLead[i].ST[j]=g_pEcgProp->Lead[i].ST[j];	//STj,ST1,ST2,ST3,ST20,ST40,ST60,ST80
			for(short j=0;j<4;j++) m_pEcgLead[i].STslope[j]=g_pEcgProp->Lead[i].STslope[j];
			strcpy(m_pEcgLead[i].morpho,g_pEcgProp->Lead[i].morpho);
		}
		g_vhCode->SetEcglead(m_pEcgLead);

		m_pEcgParm->RR	=g_pEcgProp->RR();
		m_pEcgParm->HR	=g_pEcgProp->HR();
		m_pEcgParm->Pd	=g_pEcgProp->Pd_ms();
		m_pEcgParm->PR	=g_pEcgProp->PR();
		m_pEcgParm->QRS	=g_pEcgProp->QRS();
		m_pEcgParm->QT	=g_pEcgProp->QT();
		m_pEcgParm->QTC	=g_pEcgProp->QTc();
		m_pEcgParm->WPW	=g_pEcgProp->WPW();  //return: A/a B/b W/w  A型, B型, W型, 小写: 可疑, 其它: 无WPW
		m_pEcgParm->empty=' ';
		m_pEcgParm->QTdis=g_pEcgProp->QTdis();
		m_pEcgParm->QTmax=g_pEcgProp->QTmax();
		m_pEcgParm->QTmin=g_pEcgProp->QTmin();
		m_pEcgParm->QTmaxLead=g_pEcgProp->QTmaxLead();
		m_pEcgParm->QTminLead=g_pEcgProp->QTminLead();
//	QTdiscretion QTd;
		m_pEcgParm->axisP	=g_pEcgProp->Paxis();
		m_pEcgParm->axisQRS	=g_pEcgProp->QRSaxis();
		m_pEcgParm->axisT	=g_pEcgProp->Taxis();  //return Degree,  for ChNumber==12 only
		m_pEcgParm->uvRV1=RV1();
		m_pEcgParm->uvRV5=RV5();
		m_pEcgParm->uvRV6=RV6();
		m_pEcgParm->uvSV1=SV1();
		m_pEcgParm->uvSV2=SV2();
		m_pEcgParm->uvSV5=SV5();
		//m_pEcgParm->uvRV5	=max(m_pEcgLead[m_nVindex[V5]].Ra1,m_pEcgLead[m_nVindex[V5]].Ra2);//g_pEcgProp->RV5();   //return unit: uV,         for ChNumber==12 only
		//m_pEcgParm->uvRV6	=max(m_pEcgLead[m_nVindex[V6]].Ra1,m_pEcgLead[m_nVindex[V6]].Ra2);//g_pEcgProp->RV6();   //return unit: uV,         for ChNumber==12 only
		//m_pEcgParm->uvSV1	=min(m_pEcgLead[m_nVindex[V1]].Sa1,m_pEcgLead[m_nVindex[V1]].Sa2);//g_pEcgProp->SV1();   //return unit: uV,         for ChNumber==12 only
		//m_pEcgParm->uvSV2	=min(m_pEcgLead[m_nVindex[V2]].Sa1,m_pEcgLead[m_nVindex[V2]].Sa2);//g_pEcgProp->SV2();   //return unit: uV,         for ChNumber==12 only
		//m_pEcgParm->uvRV1	=max(m_pEcgLead[m_nVindex[V1]].Ra1,m_pEcgLead[m_nVindex[V1]].Ra2);//g_pEcgProp->RV1();
		//m_pEcgParm->uvSV5	=min(m_pEcgLead[m_nVindex[V5]].Sa1,m_pEcgLead[m_nVindex[V5]].Sa2);//g_pEcgProp->SV5();
		//if(isQS(m_nVindex[V1])) m_pEcgParm->uvSV1=min(m_pEcgParm->uvSV1,m_pEcgLead[m_nVindex[V1]].Qa);
		//if(isQS(m_nVindex[V2])) m_pEcgParm->uvSV2=min(m_pEcgParm->uvSV2,m_pEcgLead[m_nVindex[V2]].Qa);
		//if(isQS(m_nVindex[V5])) m_pEcgParm->uvSV5=min(m_pEcgParm->uvSV5,m_pEcgLead[m_nVindex[V5]].Qa);
		g_vhCode->SetEcgParm(m_pEcgParm);
		for(short i=0;i<6;i++) m_pEcgParm->OnOff[i]=g_pEcgProp->Parm->OnOff[i];

		if(g_pEcgProp->m_pOutPut) {
			long BeatsNum=g_pEcgProp->m_pOutPut->BeatsNum;
			if(m_pEcgBeat) delete[]m_pEcgBeat;	m_pEcgBeat=NULL;
			if(BeatsNum>1) {
				m_pEcgBeat=new VH_ECGbeat[BeatsNum];
				m_pEcgInfo->Status		=(g_pEcgProp->m_pOutPut->Status==PROC_OK)?true:false;	//模板是否有效
				m_pEcgInfo->AflutAfib	=g_pEcgProp->m_pOutPut->AflutAfib;	//0:none, 1:Aflut(房扑), 2:Afib(房颤)
				m_pEcgInfo->LeadNo		=g_pEcgProp->m_pOutPut->LeadNo;
				m_pEcgInfo->SubLeadNo	=g_pEcgProp->m_pOutPut->SubLeadNo;  //
				m_pEcgInfo->Vrate		=g_pEcgProp->m_pOutPut->Vrate;
				m_pEcgInfo->Arate		=g_pEcgProp->m_pOutPut->Arate;
				m_pEcgInfo->BeatsNum=g_pEcgProp->m_pOutPut->BeatsNum;  //Number of Beats
				for(short i=0;i<BeatsNum;i++) {
					m_pEcgBeat[i].Status	=(g_pEcgProp->m_pOutPut->Beats[i].Status==OK)?true:false;  //0,1 是否叠加
					m_pEcgBeat[i].QRSonset=g_pEcgProp->m_pOutPut->Beats[i].QRSonset;
					m_pEcgBeat[i].Pos		=g_pEcgProp->m_pOutPut->Beats[i].Pos;
					m_pEcgBeat[i].QRSw	=g_pEcgProp->m_pOutPut->Beats[i].QRSw;
					m_pEcgBeat[i].PR		=g_pEcgProp->m_pOutPut->Beats[i].PR;
					m_pEcgBeat[i].QT		=g_pEcgProp->m_pOutPut->Beats[i].QT;    //mS
					m_pEcgBeat[i].Pdir	=g_pEcgProp->m_pOutPut->Beats[i].Pdir;
					m_pEcgBeat[i].QRSdir	=g_pEcgProp->m_pOutPut->Beats[i].QRSdir;
					m_pEcgBeat[i].Tdir	=g_pEcgProp->m_pOutPut->Beats[i].Tdir;
					m_pEcgBeat[i].Udir	=g_pEcgProp->m_pOutPut->Beats[i].Udir;  //0: none, 1: +, -1: -, 2: +- (+>-), -2: +- (->+)
					m_pEcgBeat[i].Pnum	=g_pEcgProp->m_pOutPut->Beats[i].Pnum;    //0, 1, 2 (P0,P1)
					m_pEcgBeat[i].SubQRSw	=g_pEcgProp->m_pOutPut->Beats[i].SubQRSw;
					m_pEcgBeat[i].SubQRSdir=g_pEcgProp->m_pOutPut->Beats[i].SubQRSdir;
				}
				m_pEcgInfo->Beats		=m_pEcgBeat;
				m_pEcgInfo->PaceMaker	=g_pEcgProp->m_pOutPut->PaceMaker; //'N': none, 'A': A-Type, 'V': V-type, 'B': Both
				m_pEcgInfo->SpikesN		=g_pEcgProp->m_pOutPut->SpikesN;
				m_pEcgInfo->SpikesPos	=g_pEcgProp->m_pOutPut->SpikesPos;
				char *pBeatsType=g_pEcgProp->Beats;
				g_vhCode->SetEcgInfo(m_pEcgInfo,pBeatsType);
			}
		}
	}
	else {
		g_vhCode->SetEcglead(NULL);
		g_vhCode->SetEcgParm(NULL);
		g_vhCode->SetEcgInfo(NULL,NULL);
	}
}
//
//BOOL CvhEcgDiag::EcgAutoProcess(short TemplChnNumber)
//{
//	return g_pEcgProp->AutoProcess(TemplChnNumber);
//}

BOOL CvhEcgDiag::EcgCode(char bySex,short age,short ageYmd)
{
//	if(!g_pEcgProp || GetTemplChNumber()<6) return FALSE;


	g_vhCode->SetPatientInfo(bySex,age,ageYmd);
	g_vhCode->code();

	return TRUE;
}

BOOL CvhEcgDiag::CommenManual(short *OnOff)
{
	if(!g_pEcgProp) return FALSE;

	for(short j=0;j<6;j++) g_pEcgProp->m_classTemplates->OnOff[j]=g_pEcgProp->Parm->OnOff[j]=OnOff[j];

	g_pEcgProp->m_classTemplates->CommenManual();
	g_pEcgProp->LeadParameters();
	g_pEcgProp->CommParameters();
	SetParameters();

	if(GetTemplChNumber()>=12) {
		g_vhCode->code();
	}

	return TRUE;
}

BOOL CvhEcgDiag::ECGparmManual(VH_ECGparm *parm)
{
	if(!g_pEcgProp) return FALSE;

	*m_pEcgParm=*parm;
	{
		g_pEcgProp->Parm->RR	=m_pEcgParm->RR;//	=RR();
		g_pEcgProp->Parm->HR	=m_pEcgParm->HR;//	=g_pEcgProp->HR();
		g_pEcgProp->Parm->Pd	=m_pEcgParm->Pd;//	=g_pEcgProp->Pd_ms();
		g_pEcgProp->Parm->PR	=m_pEcgParm->PR;//	=g_pEcgProp->PR();
		g_pEcgProp->Parm->QRS	=m_pEcgParm->QRS;//	=g_pEcgProp->QRS();
		g_pEcgProp->Parm->QT	=m_pEcgParm->QT;//	=g_pEcgProp->QT();
		g_pEcgProp->Parm->QTC	=m_pEcgParm->QTC;//	=g_pEcgProp->QTc();
		g_pEcgProp->Parm->WPW	=m_pEcgParm->WPW;//	=g_pEcgProp->WPW();  //return: A/a B/b W/w  A型, B型, W型, 小写: 可疑, 其它: 无WPW
		g_pEcgProp->Parm->empty	=m_pEcgParm->empty;//=' ';
		g_pEcgProp->Parm->msQTdis	=m_pEcgParm->QTdis;//=g_pEcgProp->QTdis();
		g_pEcgProp->Parm->msMax	=m_pEcgParm->QTmax;//=g_pEcgProp->QTmax();
		g_pEcgProp->Parm->msMin	=m_pEcgParm->QTmin;//=g_pEcgProp->QTmin();
		g_pEcgProp->Parm->MaxLeadNo	=m_pEcgParm->QTmaxLead;//=g_pEcgProp->QTmaxLead();
		g_pEcgProp->Parm->MinLeadNo	=m_pEcgParm->QTminLead;//=g_pEcgProp->QTminLead();
//	QTdiscretion QTd;
		g_pEcgProp->Parm->axisP	=m_pEcgParm->axisP;//	=g_pEcgProp->Paxis();
		g_pEcgProp->Parm->axisQRS	=m_pEcgParm->axisQRS;//	=g_pEcgProp->QRSaxis();
		g_pEcgProp->Parm->axisT	=m_pEcgParm->axisT;//	=g_pEcgProp->Taxis();  //return Degree,  for ChNumber==12 only
		g_pEcgProp->Parm->uvRV5	=m_pEcgParm->uvRV5;//	=g_pEcgProp->RV5();   //return unit: uV,         for ChNumber==12 only
		g_pEcgProp->Parm->uvRV6	=m_pEcgParm->uvRV6;//	=g_pEcgProp->RV6();   //return unit: uV,         for ChNumber==12 only
		g_pEcgProp->Parm->uvSV1	=m_pEcgParm->uvSV1;//	=g_pEcgProp->SV1();   //return unit: uV,         for ChNumber==12 only
		g_pEcgProp->Parm->uvSV2	=m_pEcgParm->uvSV2;//	=g_pEcgProp->SV2();   //return unit: uV,         for ChNumber==12 only
		for(short i=0;i<6;i++) g_pEcgProp->Parm->OnOff[i]	=m_pEcgParm->OnOff[i];//=g_pEcgProp->Parm->OnOff[i];
		g_pEcgProp->Parm->uvRV1	=m_pEcgParm->uvRV1;//	=g_pEcgProp->RV1();
		g_pEcgProp->Parm->uvSV5	=m_pEcgParm->uvSV5;//	=g_pEcgProp->SV5();
	}
	if(GetTemplChNumber()>=12) {
		g_vhCode->code();
	}

	return TRUE;
}

BOOL CvhEcgDiag::IndividualManual(short **OnOffs)
{
	if(!g_pEcgProp) return FALSE;

	short i,j,n=GetTemplChNumber();
	for(i=0;i<n;i++) {
		for(j=0;j<6;j++) g_pEcgProp->m_classTemplates->Lead[i]->OnOff[j]=g_pEcgProp->Lead[i].OnOff[j]=OnOffs[i][j];
	}
	g_pEcgProp->m_classTemplates->IndividualManual();
	g_pEcgProp->LeadParameters();
	g_pEcgProp->CommParameters();
	SetParameters();

	if(GetTemplChNumber()>=12) {
		g_vhCode->code();
	}

	return TRUE;
}
	
void CvhEcgDiag::SomeManual(BOOL bOnOff,BOOL bOnOffs)
{
	if(!g_pEcgProp) return;
	if(bOnOffs) g_pEcgProp->m_classTemplates->IndividualManual();
	if(bOnOff)  g_pEcgProp->m_classTemplates->CommenManual();
	g_pEcgProp->LeadParameters();
	g_pEcgProp->CommParameters();
	SetParameters();

	if(GetTemplChNumber()>=12) {
		g_vhCode->code();
	}
}

short CvhEcgDiag::GetFirstMcode(char *szLeadName)
{
	return g_vhCode->mcCodeGetFirst(szLeadName);
}
	
short CvhEcgDiag::GetNextMcode(char *szLeadName)
{
	return g_vhCode->mcCodeGetNext(szLeadName);
}

short CvhEcgDiag::GetFirstRcode(char *szLeadName)
{
	return g_vhCode->vhCodeGetFirst(szLeadName);
}
	
short CvhEcgDiag::GetNextRcode(char *szLeadName)
{
	return g_vhCode->vhCodeGetNext(szLeadName);
}
	
short CvhEcgDiag::GetMcCodeCount()
{
	return g_vhCode->mcCodeCount();
}
	
short CvhEcgDiag::GetVhCodeCount()
{
	return g_vhCode->vhCodeCount();
}

char *CvhEcgDiag::mcCode(short code)
{
	return CvhCode::mcCodeString((unsigned short) (code));
}
	
char *CvhEcgDiag::vhCode(short code)
{
	return CvhCode::vhCodeString((unsigned short) (code));
}

//	
//char *CvhEcgDiag::CodeLeads(unsigned short code)
//{
//	return g_vhCode->leads(code);
//}
//	
//char *CvhEcgDiag::GetCseCode(unsigned short rCode)
//{
//	return g_vhCode->GetCseCode(rCode);
//}
short CvhEcgDiag::GetCriticalValue()
{
	return g_vhCode->GetCriticalValue();
}

void CvhEcgDiag::SetPrematurePpercent(short percentPAC,short percentPVC)
{
	if (g_pEcgProp) g_pEcgProp->SetPrematurePpercent(percentPAC, percentPVC);
}

short CvhEcgDiag::GetTemplChNumber()
{
	return (g_pEcgProp)?g_pEcgProp->GetTemplChN():0;
}
	
VH_ECGinfo *CvhEcgDiag::GetEcgInfo()	{return m_pEcgInfo;}	//逐拍分析结果
VH_ECGbeat *CvhEcgDiag::GetEcgBeats()	{return m_pEcgBeat;}//逐拍分析结果
//VH_Template *CvhEcgDiag::GetTemplate()			{return &(g_pEcgProp->Temp);}		//12导模板数据，不同于pOutPut中8导模板(对12导分析)
short	CvhEcgDiag::GetTemplLength()			{ return (g_pEcgProp) ? g_pEcgProp->Temp.Length:0; }
short	**CvhEcgDiag::GetTemplData()			{ return (g_pEcgProp) ? g_pEcgProp->Temp.Data:0; }
short	CvhEcgDiag::GetTemplPos()				{ return (g_pEcgProp) ? g_pEcgProp->Temp.Pos : 0; }
long	CvhEcgDiag::GetBeatNum()	{ return (g_pEcgProp) ? g_pEcgProp->m_nBeats:0; }
char	*CvhEcgDiag::GetBeats(long &BeatNum)	{ BeatNum = g_pEcgProp->m_nBeats;	return (g_pEcgProp) ? g_pEcgProp->Beats:NULL; }		//心搏类型。目前只有N:正常，V:室性，B:边界无法判定，P:起搏
BYTE	*CvhEcgDiag::GetBeatAdd(long &BeatNum)	{ BeatNum = g_pEcgProp->m_nBeats;	return (g_pEcgProp) ? g_pEcgProp->BeatAdd:NULL; }	//心搏是否参与叠加，只有0,1两种状态

VH_ECGparm	*CvhEcgDiag::GetECGparm()	{return m_pEcgParm;}	//公共参数
VH_ECGlead *CvhEcgDiag::GetECGlead()	{return m_pEcgLead;}	//各导参数

BOOL	CvhEcgDiag::IsPacedECG()	{ return (g_pEcgProp) ? g_pEcgProp->IsPacedECG():FALSE; }  		//是否起博心电图

BOOL	CvhEcgDiag::TemplIsOk()	{if(g_pEcgProp) return g_pEcgProp->TemplIsOk();	else return FALSE;}
	
int CvhEcgDiag::uvSTvalue(short lead, int MSor123)	{ return (g_pEcgProp) ? g_pEcgProp->m_classTemplates->Lead[lead]->uvSTvalue(MSor123):0; }    //ms=0 STj Value, 1,2,3 -- ST1,ST2,ST3, otherwise ms
float CvhEcgDiag::STslope(short lead, int msStep)	{ return (g_pEcgProp) ? g_pEcgProp->m_classTemplates->Lead[lead]->STslope(msStep):0; }   //return uV/ms, msStep -- ms to j_point 10ms to 100ms
		
long CvhEcgDiag::Samples2ms(long Samples)	{ return (g_pEcgProp) ? Samples * 1000 / g_pEcgProp->GetSampleRate():0; }
long CvhEcgDiag::ms2Samples(long ms)		{ return	(g_pEcgProp) ? ms*g_pEcgProp->GetSampleRate() / 1000:0; }

BOOL CvhEcgDiag::isPositiveQRS(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->isPositiveQRS(lead):FALSE; }//QRS波为正?
BOOL CvhEcgDiag::isNegativeQRS(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->isNegativeQRS(lead) : FALSE; }//QRS波为负?
BOOL CvhEcgDiag::isPositiveP(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->isPositiveP(lead) : FALSE; }
BOOL CvhEcgDiag::isNegativeP(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->isNegativeP(lead) : FALSE; }
BOOL CvhEcgDiag::isDualP(short lead)		{ return (g_pEcgProp) ? g_pEcgProp->isDualP(lead) : FALSE; }
BOOL CvhEcgDiag::isPositiveT(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->isPositiveT(lead) : FALSE; }
BOOL CvhEcgDiag::isNegativeT(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->isNegativeT(lead) : FALSE; }
BOOL CvhEcgDiag::isDualT(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->isDualT(lead) : FALSE; }
BOOL CvhEcgDiag::isFlatT(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->isFlatT(lead) : FALSE; }
BOOL CvhEcgDiag::isQS(short lead)		{ return (g_pEcgProp) ? g_pEcgProp->isQS(lead) : FALSE; }//该导联是QS型吗?
BOOL CvhEcgDiag::isQr(short lead)		{ return (g_pEcgProp) ? g_pEcgProp->isQr(lead) : FALSE; }//该导联是Qr型吗?
BOOL CvhEcgDiag::isrsR(short lead)		{ return (g_pEcgProp) ? g_pEcgProp->isrsR(lead) : FALSE; }//该导联是rsR'型吗?
BOOL CvhEcgDiag::isrsr(short lead)		{ return (g_pEcgProp) ? g_pEcgProp->isrsr(lead) : FALSE; }//该导联是rsr'型吗?
BOOL CvhEcgDiag::isRSrs(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->isRSrs(lead):FALSE; }//是R(r)S(s)?

//get individual parameters
char* CvhEcgDiag::QRSmorpho(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->QRSmorpho(lead):NULL; }//形态编码
char  CvhEcgDiag::GetPaceMaker()			{ return (g_pEcgProp) ? g_pEcgProp->GetPaceMaker():' '; }
short CvhEcgDiag::GetAflutAfib()			{ return (g_pEcgProp) ? g_pEcgProp->GetAflutAfib() : 0; }
//uV
//	short Pa1,Pa2,Qa,Ra1,Ra2,Sa1,Sa2,Ta1,Ta2;
short	CvhEcgDiag::uvPa1(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->uvPa1(lead) : 0; }
short	CvhEcgDiag::uvPa2(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->uvPa2(lead) : 0; }
short	CvhEcgDiag::uvQa(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->uvQa(lead) : 0; }
short	CvhEcgDiag::uvRa1(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->uvRa1(lead) : 0; }
short	CvhEcgDiag::uvRa2(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->uvRa2(lead) : 0; }
short	CvhEcgDiag::uvSa1(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->uvSa1(lead) : 0; }
short	CvhEcgDiag::uvSa2(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->uvSa2(lead) : 0; }
short	CvhEcgDiag::uvTa1(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->uvTa1(lead) : 0; }
short	CvhEcgDiag::uvTa2(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->uvTa2(lead) : 0; }

short	CvhEcgDiag::uvQRSa(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->uvQRSa(lead) : 0; }//QRS振幅
short	CvhEcgDiag::uvRa(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->uvRa(lead) : 0; }
short	CvhEcgDiag::uvSa(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->uvSa(lead) : 0; }

//ms
//	short Pd,Qd,Rd1,Rd2,Sd1,Sd2,Td,PRd,QTd,QRSd;
short	CvhEcgDiag::msPd(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->msPd(lead) : 0; }
short	CvhEcgDiag::msQd(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->msQd(lead) : 0; }
short	CvhEcgDiag::msRd1(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->msRd1(lead) : 0; }
short	CvhEcgDiag::msRd2(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->msRd2(lead) : 0; }
short	CvhEcgDiag::msSd1(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->msSd1(lead) : 0; }
short	CvhEcgDiag::msSd2(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->msSd2(lead) : 0; }
short	CvhEcgDiag::msTd(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->msTd(lead) : 0; }

short	CvhEcgDiag::msPR(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->msPR(lead) : 0; }
short	CvhEcgDiag::msQT(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->msQT(lead) : 0; }
short	CvhEcgDiag::msQRS(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->msQRS(lead) : 0; }

short	CvhEcgDiag::msSd(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->msSd(lead) : 0; }
short	CvhEcgDiag::msRd(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->msRd(lead) : 0; }
//uV
short	CvhEcgDiag::STj(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->STj(lead) : 0; }
short	CvhEcgDiag::ST1(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->ST1(lead) : 0; }
short	CvhEcgDiag::ST2(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->ST2(lead) : 0; }
short	CvhEcgDiag::ST3(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->ST3(lead) : 0; }
short	CvhEcgDiag::ST20(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->ST20(lead) : 0; }
short	CvhEcgDiag::ST40(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->ST40(lead) : 0; }
short	CvhEcgDiag::ST60(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->ST60(lead) : 0; }
short	CvhEcgDiag::ST80(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->ST80(lead) : 0; }
short	CvhEcgDiag::Rnotch(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->Rnotch(lead) : 0; }
//derived
short	CvhEcgDiag::positivePa(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->positivePa(lead) : 0; }//返回正P波幅值uV,没有返回0
short	CvhEcgDiag::negativePa(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->negativePa(lead) : 0; }//返回负P波幅值uV,没有返回0
short	CvhEcgDiag::positiveTa(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->positiveTa(lead) : 0; }//返回正T波幅值uV,没有返回0
short	CvhEcgDiag::negativeTa(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->negativeTa(lead) : 0; }//返回负T波幅值uV,没有返回0
short	CvhEcgDiag::positivePd(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->positivePd(lead) : 0; }//如果是正P波,返回间期ms,否则返回0
short	CvhEcgDiag::negativePd(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->negativePd(lead) : 0; }//返回负P波间期ms,没有返回0
short	CvhEcgDiag::positiveTd(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->positiveTd(lead) : 0; }//返回正T波间期ms,没有返回0
short	CvhEcgDiag::negativeTd(short lead)	{ return (g_pEcgProp) ? g_pEcgProp->negativeTd(lead) : 0; }//返回负T波间期ms,没有返回0
//
//get common parameters
	//ms
short	CvhEcgDiag::HR(short msRR)				{return ECGprop::HR(msRR);}
short	CvhEcgDiag::QTc(short msQT,short msRR) {return ECGprop::QTc(msQT,msRR);}	

short	CvhEcgDiag::HR()		{ return (g_pEcgProp) ? g_pEcgProp->HR() : 0; }
short	CvhEcgDiag::RR()		{ return (g_pEcgProp) ? g_pEcgProp->RR() : 0; }
short	CvhEcgDiag::Pd()	{ return (g_pEcgProp) ? g_pEcgProp->Pd_ms() : 0; }
short	CvhEcgDiag::PR()		{ return (g_pEcgProp) ? g_pEcgProp->PR() : 0; }
short	CvhEcgDiag::QRS()		{ return (g_pEcgProp) ? g_pEcgProp->QRS() : 0; }
short	CvhEcgDiag::QT()		{ return (g_pEcgProp) ? g_pEcgProp->QT() : 0; }
short	CvhEcgDiag::QTc()		{ return (g_pEcgProp) ? g_pEcgProp->QTc() : 0; }
short	CvhEcgDiag::QTdis()	{ return (g_pEcgProp) ? g_pEcgProp->QTdis() : 0; }
short	CvhEcgDiag::QTmax()	{ return (g_pEcgProp) ? g_pEcgProp->QTmax() : 0; }
short	CvhEcgDiag::QTmin()	{ return (g_pEcgProp) ? g_pEcgProp->QTmin() : 0; }
short	CvhEcgDiag::QTmaxLead()	{ return (g_pEcgProp) ? g_pEcgProp->QTmaxLead() : 0; }
short	CvhEcgDiag::QTminLead()	{ return (g_pEcgProp) ? g_pEcgProp->QTminLead():0; }
	//uV	
short	CvhEcgDiag::RV5()	{
//	if(m_nVindex[V5]==10)	return g_pEcgProp->RV5();
//	else {
	if (!g_pEcgProp || m_nVindex[V5]<0 || m_nVindex[V5] >= g_pEcgProp->GetTemplChN()) return 0;
		ECGlead *lead=g_pEcgProp->Lead;
		return max(lead[m_nVindex[V5]].Ra1,lead[m_nVindex[V5]].Ra2);
//	}
}
short	CvhEcgDiag::RV6()	{
//	if(m_nVindex[V6]==11) return g_pEcgProp->RV6();
//	else {
	if (!g_pEcgProp || m_nVindex[V6]<0 || m_nVindex[V5] >= g_pEcgProp->GetTemplChN()) return 0;
		ECGlead *lead=g_pEcgProp->Lead;
		return max(lead[m_nVindex[V6]].Ra1,lead[m_nVindex[V6]].Ra2);
//	}
}
short	CvhEcgDiag::SV1()	{
//	if(m_nVindex[V1]==6) return g_pEcgProp->SV1();
//	else {
	short uvSV=0,L = m_nVindex[V1];
	if (!g_pEcgProp || L<0 || m_nVindex[V5] >= g_pEcgProp->GetTemplChN()) return 0;
		ECGlead *lead=g_pEcgProp->Lead;
		uvSV= min(lead[L].Sa1,lead[L].Sa2);
		if (isQS(L)) uvSV = min(uvSV, lead[L].Qa);
		return uvSV;
//	}
}
short	CvhEcgDiag::SV2()	{
//	if(m_nVindex[V2]==7) return g_pEcgProp->SV2();
//	else {
	short uvSV = 0, L = m_nVindex[V2];
	if (!g_pEcgProp || L<0 || m_nVindex[V5] >= g_pEcgProp->GetTemplChN()) return 0;
	ECGlead *lead = g_pEcgProp->Lead;
	uvSV = min(lead[L].Sa1, lead[L].Sa2);
	if (isQS(L)) uvSV = min(uvSV, lead[L].Qa);
	return uvSV;
	//	}
}
short	CvhEcgDiag::SV5()	{
	short uvSV = 0, L = m_nVindex[V5];
	if (!g_pEcgProp || L<0 || m_nVindex[V5] >= g_pEcgProp->GetTemplChN()) return 0;
	ECGlead *lead = g_pEcgProp->Lead;
	uvSV = min(lead[L].Sa1, lead[L].Sa2);
	if (isQS(L)) uvSV = min(uvSV, lead[L].Qa);
	return uvSV;
}
short	CvhEcgDiag::SV6()	{
	short uvSV = 0, L = m_nVindex[V6];
	if (!g_pEcgProp || L<0 || m_nVindex[V5] >= g_pEcgProp->GetTemplChN()) return 0;
	ECGlead *lead = g_pEcgProp->Lead;
	uvSV = min(lead[L].Sa1, lead[L].Sa2);
	if (isQS(L)) uvSV = min(uvSV, lead[L].Qa);
	return uvSV;
}
short	CvhEcgDiag::RV1()	{
	if (!g_pEcgProp || m_nVindex[V1]<0 || m_nVindex[V5] >= g_pEcgProp->GetTemplChN()) return 0;
	ECGlead *lead=g_pEcgProp->Lead;
	return max(lead[m_nVindex[V1]].Ra1,lead[m_nVindex[V1]].Ra2);
}
short	CvhEcgDiag::RV2()	{
	if (!g_pEcgProp || m_nVindex[V2]<0 || m_nVindex[V5] >= g_pEcgProp->GetTemplChN()) return 0;
	ECGlead *lead=g_pEcgProp->Lead;
	return max(lead[m_nVindex[V2]].Ra1,lead[m_nVindex[V2]].Ra2);
}
	//度
short	CvhEcgDiag::Paxis()	{ return (g_pEcgProp) ? g_pEcgProp->Paxis():0; }
short	CvhEcgDiag::QRSaxis()	{ return (g_pEcgProp) ? g_pEcgProp->QRSaxis():0; }
short	CvhEcgDiag::Taxis()	{ return (g_pEcgProp) ? g_pEcgProp->Taxis():0; }
//	short*	Axis()	  {return Parm->Axis;}
char	CvhEcgDiag::WPW()		{ return (g_pEcgProp) ? g_pEcgProp->WPW():' '; }//return: A/a B/b W/w  A型, B型, W型, 小写: 可疑, 其它: 无WPW

//get Beat 	parameters
short	CvhEcgDiag::RR(int index)			{ return (g_pEcgProp) ? g_pEcgProp->RR(index):0; }	//RR(i) in ms
short	CvhEcgDiag::beatPnum(int index)	{ return (g_pEcgProp) ? g_pEcgProp->beatPnum(index):0; } //P波个数

