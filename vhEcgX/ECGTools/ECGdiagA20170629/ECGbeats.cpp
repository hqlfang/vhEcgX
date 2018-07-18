/*/////////////////////////////////////////////////////////////
Du Xiaodong, VH Medical
01-July-2009 start, 26-Oct-2009 again,  27-Nov-2009 1st Ver. 
2013.02.08 更新函数
	void KinetEnergy(short *in0,short *in1,short *in2,short *out,int length,int q)
2013.05.09
	MultiLead_ECG::MultiLead_ECG(int ChNumber,short **DataIn,int Seconds,int Samplerate,double Uvperbit)
	对输入数据进行扩充以适应不同情况。
2013.12.25 memory error for large noise signal
	ECGbeats.cpp
		void MultiLead_ECG::FinalProcess()
		for(int i=0;i<QRSsN;i++) {
		  for(int k=0;k<3;k++) Beats[i].P[k].Dir=Beats[i].T.Dir=Beats[i].U.Dir=0;
		 }

		void MultiLead_ECG::AntiDrift(short *data)
		break;
2014.06.12 error for fs=250Hz
*//////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ECGbeats.h"

///////////////////////////////////////////////////////////////////////////////////
MultiLead_ECG::MultiLead_ECG(int ChNumber,short **DataIn,int Seconds,int Samplerate,double Uvperbit)
{	if(Seconds>90) Seconds=90;
	ChN=ChNumber;
	SampleRate=Samplerate;
	Length=SampleRate*(Seconds);
	uVperBit=Uvperbit;   //1
//	Ori_uVperBit=Uvperbit;
	MinWave=short(30/uVperBit);
//	fLength=0;

	DataOut=new short *[ChN];
	for(int i=0;i<ChN;i++) DataOut[i]=DataIn[i];

//Test for uV_per_Bit
//	for(int i=0;i<ChN;i++) for(int k=0;k<Length;k++) DataOut[i][k]=short(DataOut[i][k]*Ori_uVperBit);


//	DataOut[1]=DataIn[0];  //DUXIAODONG
//	for(int i=2;i<ChN;i++) DataOut[i]=DataIn[2];

	if(ChN==1) Data[0]=Data[1]=Data[2]=DataOut[0];
	else if(ChN==2) {
		Data[0]=Data[2]=DataOut[0];
		Data[1]=DataOut[1];
	}
	else if(ChN<8) {  //ChN==3 (X,Y,Z)  or 3<ChN<8
		Data[0]=DataOut[0];
		Data[1]=DataOut[1];
		Data[2]=DataOut[2];
	}
	else {//if(ChN==8) { // ChN==8 //(II, V1, V5)	--  I,II,V1,V2,V3,V4,V5,V6
		short LeadsNo[3];
		SelectLeads(DataOut,LeadsNo);
		Data[0]=DataOut[LeadsNo[0]];    //  II   1,2,6
		Data[1]=DataOut[LeadsNo[1]];    // V1
		Data[2]=DataOut[LeadsNo[2]];   // V5
//		Data[0]=DataOut[1];    //  II   1,2,6
//		Data[1]=DataOut[2];    // V1
//		Data[2]=DataOut[6];   // V5
	}
	//else if(ChN<=12) { // ChN>8 //(II, V1, V5)	--  I,II,III,aVR,aVL,aVF,V1,V2,V3,V4,V5,V6(全导联)
	//	Data[0]=DataOut[1];   
	//	Data[1]=DataOut[ChN-6];
	//	Data[2]=DataOut[ChN-2];
	//}
	//else { // ChN>8 //(II, V1, V5)	--  I,II,III,aVR,aVL,aVF,V1,V2,V3,V4,V5,V6,...(全导联+附加导联)
	//	Data[0]=DataOut[1];
	//	Data[1]=DataOut[6];
	//	Data[2]=DataOut[10];
	//}

	if(ChN>2) {    // check noise
		if(MuchNoise(Data[1])) Data[1]=DataOut[0];
		if(MuchNoise(Data[2])) Data[2]=DataOut[0];
	}

	//死机解决 Y2015  如加CSE数据库55,62错判
/*	int w=50*SampleRate/1000;   //50ms
	short NoiseV=short(8000/uVperBit);
	for(int k=0;k<3;k++) {
		for(int i=0;i<Length-w;i+=w) {
			BOOL isNoise=FALSE;
			for(int j=i;j<i+w;j++) if(abs(Data[k][j])>NoiseV) isNoise=TRUE; 
			if(isNoise) for(int j=i;j<i+w;j++) Data[k][j]=0;
		}
	}
*/
	SpikesN=QRSsN=0;
	AnalysisDone=TemplateDone=FALSE;

	OutPut.PaceMaker='N';
 	OutPut.Vrate=OutPut.Arate=0;
	OutPut.AflutAfib=0;  
//	OutPut.VfibVTech=0;

	ProcStatus=NO_TEMPL;

	Spikes=NULL;
	SpikesV=NULL;
	Beats=NULL;

	sData=NULL;
	OutPut.Beats=NULL;
	OutPut.Temp.Data=NULL;
}

MultiLead_ECG::~MultiLead_ECG()
{
	int i;
	delete []DataOut;
	if(Spikes)	delete []Spikes;
	if(SpikesV)	{
		for(i=0;i<SpikesN;i++) if(SpikesV[i]) delete []SpikesV[i];
		delete []SpikesV;
	}
	if(Beats) delete []Beats;
	if(sData)	delete []sData;
	if(OutPut.Beats)	delete []OutPut.Beats;
	if(OutPut.Temp.Data) {
		for(i=0;i<ChN;i++) if(OutPut.Temp.Data[i]) delete []OutPut.Temp.Data[i];
		delete []OutPut.Temp.Data;
	}
//	delete []vData;
}

void MultiLead_ECG::SelectLeads(short **LeadsData,short *LeadNo)  //2018
{
	int i,j,k;
	int w=90*SampleRate/1000;  //ms  wide of QRS
	short maxV,minV;
	short *maxD=new short[ChN];
	short *leadN=new short[ChN];
	for(i=0;i<ChN;i++) {
		leadN[i]=i;
		maxD[i]=0;
		for(j=w/2;j<Length-w-w/2;j++) {
			maxV=minV=LeadsData[i][j];
			for(k=1;k<w;k++) {
				if(LeadsData[i][j+k]>maxV) maxV=LeadsData[i][j+k];
				if(LeadsData[i][j+k]<minV) minV=LeadsData[i][j+k];
			}
			if(maxV-minV>maxD[i]) maxD[i]=maxV-minV;
		}
	}
	BubbleSort(maxD,leadN,ChN);
	LeadNo[0]=leadN[ChN/2+1];
	LeadNo[1]=leadN[ChN/2+2];
	LeadNo[2]=leadN[ChN/2+3];
	delete []maxD;
	delete []leadN;
}

bool MultiLead_ECG::MuchNoise(short *data)
{
	int s,i,k;
	int segment=2*Length/(5*SampleRate);  
	int step=Length/segment;    // 2.5 seconds
	short maxV, minV;
	short MaxDiff=0;
	short MinDiff=short(5000/uVperBit);
	for(s=0;s<Length-step;s+=(step/3)) {
		maxV=minV=data[s];
		for(i=0;i<step;i++) {
			if(data[s+i]>maxV) maxV=data[s+i];
			if(data[s+i]<minV) minV=data[s+i];
		}
		if(maxV-minV>MaxDiff) MaxDiff=maxV-minV;
		if(maxV-minV<MinDiff) MinDiff=maxV-minV;
	}
	if(MaxDiff>2*MinDiff) return TRUE;

	step=10*SampleRate/1000;
	short SteepV=short(3000/uVperBit);
	bool steep=FALSE;
	for(i=2*step;i<Length-4*step;i++) { 
		k=0;
		if(data[i]-data[i-step]>SteepV) for(k=i;k<i+4*step;k++) if(data[k]-data[i-step]<SteepV/2) break;
		if(k==i+4*step) break;
		k=0;
		if(data[i-step]-data[i]>SteepV) for(k=i;k<i+4*step;k++) if(data[i-step]-data[k]<SteepV/2) break;
		if(k==i+4*step) break;
	}
	if(MaxDiff>1.5*MinDiff && i<Length-4*step) return TRUE;
	return FALSE;
}

BOOL MultiLead_ECG::ProcessSucceed()
{
	ProcStatus=NO_TEMPL;//hspecg
	OutPut.Temp.ChN=ChN;
	OutPut.Temp.SampleRate=SampleRate;
	OutPut.Temp.Uvperbit=uVperBit;

	if(ChN<=0||ChN>18) return FALSE;
	if(SampleRate<200||SampleRate>1000) return FALSE;
	if(Length/SampleRate<9||Length/SampleRate>90) return FALSE;
	if(uVperBit>10||uVperBit<0.2) return FALSE;

	ProcStatus=PROC_OK;

	OutPut.Status=PreProcess(); 
	if(OutPut.Status==1) FinalProcess(); 
	else {
		OutPut.LeadNo=0;
		TemplateDone=TRUE;
		if(OutPut.Status==-3||OutPut.Status==0) SingleTemplate(Length/2);
		else SingleTemplate(Beats[QRSsN/2].QRS.Pos);
		switch(OutPut.Status) {//hspecg
			case 0:
			case -1:	ProcStatus=FEW_QRS;		break;
			case -2:	ProcStatus=MUCH_QRS;	break;
			case -3:	ProcStatus=MUSH_NOISE;	break;
		}
	}
	return TRUE;
}

short MultiLead_ECG::PreProcess()
{ 
	SpikesN=0;
	SpikeDetect(); 
//	if(SpikesN>=(Length/SampleRate)*4) return -3;  //spikes noise
	if(SpikesN>=(Length/SampleRate)*6) {
		if(Spikes) delete[]Spikes;	Spikes=NULL;
		if(SpikesV) {
			for(int i=0;i<SpikesN;i++) {if(SpikesV[i]) delete[]SpikesV[i];SpikesV[i]=NULL;}
			delete[]SpikesV;	SpikesV=NULL;
		}
		SpikesN=0;  //Y2015 临时
	}
	if(SpikesN>0&&SpikesN<(Length/SampleRate)/3) NoiseRemoval();//set SpiksN=0 and delete []Spikes
	if(SpikesN>0) SpikeRemoval();

	for(int ch=0;ch<ChN;ch++) Average(DataOut[ch],Length,4);
	QRSsDetect();  
	if(QRSsN<3) return 0; 
 	if(QRSsN<(Length/SampleRate)/5) return -1; //no qrs or signal too small
	else if(QRSsN>(Length/SampleRate)*5) return -2;  // noise  
	else return 1;
}

void MultiLead_ECG::FinalProcess()
{ 
	short **OriData; 
	OriData=new short*[ChN];
	for(int ch=0;ch<ChN;ch++) OriData[ch]=new short[Length];
	for(int ch=0;ch<ChN;ch++) for(int i=0;i<Length;i++) OriData[ch][i]=DataOut[ch][i];

	for(int ch=0;ch<ChN;ch++) AntiDrift(DataOut[ch]);//,QRSsN,QRSsPos);

	OutPut.LeadNo=LeadSelectDependsOnQRS();  //2017-1
	if(OutPut.LeadNo<0)	OutPut.LeadNo=LeadSelectDependsOnP();   //sData be selected (DataOut[Paras.LeadNo]) //2017 

	AnalysisDone=TRUE;
	OutPut.BeatsNum=QRSsN;                  
	OutPut.Beats=new BeatParameters[QRSsN];

	for(int i=0;i<QRSsN;i++) {
		for(int k=0;k<3;k++) Beats[i].P[k].Dir=0;
		Beats[i].T.Dir=Beats[i].U.Dir=0;
	}
	OutPut.SubLeadNo=SubLeadSelect();
	for(int i=0;i<QRSsN;i++) SubQRSanalysis(i,DataOut[OutPut.SubLeadNo]);
//	SubLeadTs();
	ECG_Analysis();

	if(ChN>1&&!GoodSelectPs()) {
		OutPut.LeadNo=LeadSelectDependsOnT(OutPut.LeadNo); //sData be selected (DataOut[Paras.LeadNo])
		OutPut.SubLeadNo=SubLeadSelect();
		for(int i=0;i<QRSsN;i++) SubQRSanalysis(i,DataOut[OutPut.SubLeadNo]);
//		SubLeadTs();
		ECG_Analysis();
	}

	CorrectPTs();

	if(SpikesN>0) {
		OutPut.PaceMaker=PaceMaker_Analysis();
		OutPut.SpikesN=SpikesN;
		OutPut.SpikesPos=Spikes;
	}
	if(OutPut.PaceMaker=='V') 
		for(int i=1;i<QRSsN;i++) for(int k=0;k<3;k++) Beats[i].P[k].Dir=0;

	ParametersCalculation();  //StatusConfirm inside

	for(int ch=0;ch<ChN;ch++) for(int i=0;i<Length;i++) DataOut[ch][i]=OriData[ch][i];
	for(int ch=0;ch<ChN;ch++) AntiDrift2(DataOut[ch]);  //QRSsN, middle of P_offset and QRS_onset
	for(int ch=0;ch<ChN;ch++) delete []OriData[ch];
	delete []OriData;

	if(OutPut.PaceMaker=='N') OutPut.AflutAfib=F_Analysis();

	TemplateDone=TRUE;
	AverageTemplate();

	OutPut.Temp.SpikeA=OutPut.Temp.SpikeV=0;  
	if(OutPut.PaceMaker!='N') SpikeReset();  
}

void MultiLead_ECG::SingleTemplate(long Pos)
{
	int TempLength=1200*SampleRate/1000;
	int left=Pos-2*TempLength/5;  // 480ms + 720ms
	if(left<TempLength) left=TempLength;
	if(left+2*TempLength>Length) left=Length-2*TempLength;

	OutPut.Temp.Pos=2*TempLength/5;
	OutPut.Temp.Left=100*SampleRate/1000;
	OutPut.Temp.Right=TempLength-100*SampleRate/1000;
	OutPut.Temp.Length=TempLength;
	OutPut.Temp.Data=new short*[ChN];
	for(int i=0;i<ChN;i++) OutPut.Temp.Data[i]=new short[TempLength];
	for(int ch=0;ch<ChN;ch++) 
		for(int i=0;i<TempLength;i++) OutPut.Temp.Data[ch][i]=DataOut[ch][left+i];
}
/*
void MultiLead_ECG::AverageTemplate()
{
	int i;
	int Left,Right,maxLeft,maxRight;
	int w0=30*SampleRate/1000;
	Left=maxLeft=480*SampleRate/1000;    // 2/5
	Right=maxRight=720*SampleRate/1000;  //max length =1200mS

	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Status==OK) {
			if(Beats[i].QRS.Pos-(Beats[i-1].T.Offset-w0)<Left)
				Left=Beats[i].QRS.Pos-(Beats[i-1].T.Offset-w0);
			if(Beats[i].QRS.Pos-Left<Beats[i-1].QRS.Offset+w0)
				Left=Beats[i].QRS.Pos-(Beats[i-1].QRS.Offset+w0);
			if((Beats[i+1].P[0].Onset+w0)-Beats[i].QRS.Pos<Right)
				Right=(Beats[i+1].P[0].Onset+w0)-Beats[i].QRS.Pos;
			if(Beats[i].QRS.Pos+Right>Beats[i+1].QRS.Onset-w0)
				Right=(Beats[i+1].QRS.Onset-w0)-Beats[i].QRS.Pos;
		}
	}

	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Status==OK) {
			if(Beats[i].QRS.Pos-Beats[i-1].QRS.Offset<maxLeft)
				maxLeft=Beats[i].QRS.Pos-Beats[i-1].QRS.Offset;
			if(Beats[i+1].QRS.Onset-Beats[i].QRS.Pos<maxRight)
				maxRight=Beats[i+1].QRS.Onset-Beats[i].QRS.Pos;
		}
	}
	
	OutPut.Temp.Pos=maxLeft;
	if(Left>maxLeft-w0/2) Left=maxLeft-w0/2;
	if(Right>maxRight-w0/2) Right=maxRight-w0/2;
	OutPut.Temp.Left=maxLeft-Left;
	OutPut.Temp.Right=maxLeft+Right;

	OutPut.Temp.Length=maxLeft+maxRight;
	OutPut.Temp.Data=new short*[ChN];
	for(i=0;i<ChN;i++) OutPut.Temp.Data[i]=new short[OutPut.Temp.Length];

	int bt,ch,OKcount=0;
	long temp;
	for(bt=1;bt<QRSsN-1;bt++) if(OutPut.Beats[bt].Status==OK) OKcount++;
	for(ch=0;ch<ChN;ch++) {
		for(i=0;i<OutPut.Temp.Length;i++) {
			temp=0;
			for(bt=1;bt<QRSsN-1;bt++) {
				if(OutPut.Beats[bt].Status==OK) 
					temp+=DataOut[ch][Beats[bt].QRS.Pos-maxLeft+i];
			}  //for(bt
			OutPut.Temp.Data[ch][i]=short(temp/OKcount);
		}  // for(i
	}  // for(ch 
}
*/
void MultiLead_ECG::AverageTemplate()
{
	int i;
//	int maxLeft=0;
//	int maxRight=0;
	int L1=0;
	int R1=0;
	int Left=480l*SampleRate/1000;  //0
	int Right=720l*SampleRate/1000;  //0
//	int L2=Left;
//	int R2=Right;
	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Status==OK) {
			if(Beats[i].QRS.Pos-Beats[i-1].QRS.Offset<Left)
				Left=Beats[i].QRS.Pos-Beats[i-1].QRS.Offset;
			if(Beats[i+1].QRS.Onset-Beats[i].QRS.Pos<Right)
				Right=Beats[i+1].QRS.Onset-Beats[i].QRS.Pos;

			if(Beats[i].P[0].Dir!=0&&Beats[i].QRS.Pos-Beats[i].P[0].Onset>L1)
				L1=Beats[i].QRS.Pos-Beats[i].P[0].Onset;
//			if(Beats[i].U.Dir!=0) { //not consider of U
//				if(Beats[i].U.Offset-Beats[i].QRS.Pos>R1)
//					R1=Beats[i].U.Offset-Beats[i].QRS.Pos;
//			}
//			else {
//				if(Beats[i].T.Dir!=0&&Beats[i].T.Offset-Beats[i].QRS.Pos>R1)
//					R1=Beats[i].T.Offset-Beats[i].QRS.Pos;
//			}
			if(Beats[i].T.Dir!=0&&Beats[i].T.Offset-Beats[i].QRS.Pos>R1)
				R1=Beats[i].T.Offset-Beats[i].QRS.Pos;
		}
	}
//following deal with if Left or Right too small
	if(Left<100L*SampleRate/1000L) Left=100L*SampleRate/1000L; 
	if(Right<120L*SampleRate/1000L) Right=120L*SampleRate/1000L; 

//	Left+=w0;
//	Right+=w0;

//	if(L2>L1) L1=L2;
//	if(R2>R1) R1=R2;
//    int Pwidth=int(0.27*m_nSampleRate*sqrt((75./60.)*fabs((double)nRR)/m_nSampleRate));    //P width
//    int Twidth=int(0.385*m_nSampleRate*sqrt((75./60.)*fabs((float)nRR)/m_nSampleRate));    //T width

//followin deal with if L1 or R1 ==0
	int w0=60L*SampleRate/1000L;
	if(OutPut.TemplateRR<1000L) w0=int((OutPut.TemplateRR/20.+10.)*SampleRate/1000.);
	int wT=w0+12L*SampleRate/1000L;
	int wP=w0-8L*SampleRate/1000L;
	if(L1==0) L1=int(0.27*SampleRate*sqrt((75./60.)*fabs(OutPut.TemplateRR/1000.)));
	L1+=wP;
//	if(L1>Left-w0||OutPut.Vrate>150) L1=Left-w0;
	if(R1==0) R1=int(0.385*SampleRate*sqrt((75./60.)*fabs(OutPut.TemplateRR/1000.)));
	R1+=wT;  
//	if(R1>Right-w0||OutPut.Vrate>150) R1=Right-w0/3;

	if(OutPut.Vrate<150) {
		if(L1>Left-w0) L1=Left-w0;
		else L1+=(w0/3);   //20ms
		if(R1>Right-w0) R1=Right-w0;
		else R1+=(w0/3);   //20ms
	}
	else {
		L1=Left-w0/2;
		R1=Right-w0/3;
	}
//Left-=40*SampleRate/1000L;
	OutPut.Temp.Pos=Left;
	OutPut.Temp.Left=Left-L1;
	OutPut.Temp.Right=Left+R1;

	OutPut.Temp.Length=Left+Right;

//  Du 2011-5-6 重写。考虑到去除任一导联有干扰或无信号的心博。
	struct SumLable {
		BOOL *Ok;
		int Count;
	};
	int ch,bt;

	SumLable *Lable;
	Lable=new SumLable[ChN];
	for(ch=0;ch<ChN;ch++) Lable[ch].Ok=new BOOL[QRSsN];
	for(ch=0;ch<ChN;ch++) {
		Lable[ch].Count=0;
		for(bt=1;bt<QRSsN-1;bt++) {
			if(OutPut.Beats[bt].Status==OK&&Addible(&DataOut[ch][Beats[bt].QRS.Pos-Left],OutPut.Temp.Length)) {
				Lable[ch].Count++;
				Lable[ch].Ok[bt]=TRUE;
			}
			else Lable[ch].Ok[bt]=FALSE;
		}
	}

	OutPut.Temp.Data=new short*[ChN];
	for(ch=0;ch<ChN;ch++) OutPut.Temp.Data[ch]=new short[OutPut.Temp.Length];
	for(ch=0;ch<ChN;ch++) for(i=0;i<OutPut.Temp.Length;i++) OutPut.Temp.Data[ch][i]=0;		

	long temp;
	for(ch=0;ch<ChN;ch++) {
		if(Lable[ch].Count>0) {
			for(i=0;i<OutPut.Temp.Length;i++) {
				temp=0;
				for(bt=1;bt<QRSsN-1;bt++) 
					if(Lable[ch].Ok[bt]) temp+=DataOut[ch][Beats[bt].QRS.Pos-Left+i];  
				OutPut.Temp.Data[ch][i]=short(temp/Lable[ch].Count);  
			}  // for(i
		}
	}  // for(ch 

	for(ch=0;ch<ChN;ch++) {delete[]Lable[ch].Ok;Lable[ch].Ok=NULL;}
	delete[]Lable;	Lable=NULL;

//	for(ch=0;ch<ChN;ch++) {   //NEW
//		for(i=0;i<OutPut.Temp.Left;i++) OutPut.Temp.Data[ch][i]=0;
//		for(i=OutPut.Temp.Right;i<OutPut.Temp.Length;i++) OutPut.Temp.Data[ch][i]=0;
//	}

//	for(ch=0;ch<ChN;ch++) for(i=0;i<OutPut.Temp.Length;i++) OutPut.Temp.Data[ch][i]=short(OutPut.Temp.Data[ch][i]/Ori_uVperBit);
}

BOOL MultiLead_ECG::Addible(short *data,int len)
{
	int lowV=int(50/uVperBit);
	int highV=int(8000/uVperBit);

	int i,maxV,minV;
	maxV=minV=data[0];
	for(i=1;i<len;i++) {
		if(data[i]>maxV) maxV=data[i];
		if(data[i]<minV) minV=data[i];
	}
	if(maxV-minV<lowV) return FALSE;
	if(maxV-minV>highV) return FALSE;
	return TRUE;
}

/*
void MultiLead_ECG::AverageTemplate()
{
	int i;
	int w0=30*SampleRate/1000;

	int maxLeft=0;
	int maxRight=0;
	int Left=480*SampleRate/1000;  //0
	int Right=720*SampleRate/1000;  //0
	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Status==OK) {
			if(Beats[i].QRS.Pos-Beats[i-1].QRS.Offset>maxLeft)
				maxLeft=Beats[i].QRS.Pos-Beats[i-1].QRS.Offset;
			if(Beats[i+1].QRS.Onset-Beats[i].QRS.Pos>maxRight)
				maxRight=Beats[i+1].QRS.Onset-Beats[i].QRS.Pos;
			if(Beats[i].QRS.Pos-Beats[i-1].QRS.Offset<Left)
				Left=Beats[i].QRS.Pos-Beats[i-1].QRS.Offset;
			if(Beats[i+1].QRS.Onset-Beats[i].QRS.Pos<Right)
				Right=Beats[i+1].QRS.Onset-Beats[i].QRS.Pos;
//			if(Beats[i].QRS.Pos-(Beats[i-1].T.Offset-w0)>Left)
//				Left=Beats[i].QRS.Pos-(Beats[i-1].T.Offset-w0);
//			if((Beats[i+1].P[0].Onset+w0)-Beats[i].QRS.Pos>Right)
//				Right=(Beats[i+1].P[0].Onset+w0)-Beats[i].QRS.Pos;
		}
	}
	if(maxLeft>480*SampleRate/1000) maxLeft=480*SampleRate/1000;
	if(maxRight>720*SampleRate/1000) maxRight=720*SampleRate/1000;
	if(Left>maxLeft-w0) Left=maxLeft-w0;
	if(Right>maxRight-w0) Right=maxRight-w0;

	OutPut.Temp.Pos=maxLeft;
	OutPut.Temp.Left=maxLeft-Left;
	OutPut.Temp.Right=maxLeft+Right;

	OutPut.Temp.Length=maxLeft+maxRight;
	OutPut.Temp.Data=new short*[ChN];
	for(i=0;i<ChN;i++) OutPut.Temp.Data[i]=new short[OutPut.Temp.Length];

	int bt,ch,OKcount=0;
	long temp;
	for(bt=1;bt<QRSsN-1;bt++) if(OutPut.Beats[bt].Status==OK) OKcount++;
	for(ch=0;ch<ChN;ch++) {
		for(i=0;i<OutPut.Temp.Length;i++) {
			temp=0;
			for(bt=1;bt<QRSsN-1;bt++) {
				if(OutPut.Beats[bt].Status==OK) 
					temp+=DataOut[ch][Beats[bt].QRS.Pos-maxLeft+i];
			}  //for(bt
			OutPut.Temp.Data[ch][i]=short(temp/OKcount);
		}  // for(i
	}  // for(ch 
}
*/
void MultiLead_ECG::SpikeDetect()
{
	long i,j;
	int w=8*SampleRate/1000;  //P
	long *Momentum;
	Momentum=new long[Length];
	KinetEnergy(Data[0],Data[1],Data[2],Momentum,Length,w);
//	for(i=0;i<Length;i++) Momentum[i]=0;
	i=w;
	while(i<Length-w) {
		j=0;
		while(Momentum[i+j]>4000/uVperBit&&i+j<Length-w) j++; 
		if(j>1) {
			Momentum[i+j/2-1]=-999;
			SpikesN++;
		}
		i+=(4*j+1);
	}
	if(SpikesN>1) {
		if(Spikes) delete[]Spikes;
		Spikes=new long[SpikesN];
		j=0;
		for(i=w;i<Length-w;i++) if(Momentum[i]==-999) Spikes[j++]=i; 
		if(SpikesV) {
			for(i=0;i<SpikesN;i++) if(SpikesV[i]) delete[]SpikesV[i];
			delete[]SpikesV;
		}
		SpikesV=new SpikeV *[SpikesN];
		for(i=0;i<SpikesN;i++) SpikesV[i]=new SpikeV[ChN];
	}
	else SpikesN=0;
	delete []Momentum; 
}

void MultiLead_ECG::NoiseRemoval()
{
	int ch,spk;
	long i,j;
	int w=20*SampleRate/1000;
	for(spk=0;spk<SpikesN;spk++) {
		i=Spikes[spk];
		if (i < w) continue;
		for(ch=0;ch<ChN;ch++) {
			for(j=i-w;j<i+w;j++) DataOut[ch][j]
				=short(DataOut[ch][i-w]+(j-i+w)*(DataOut[ch][i+w]-DataOut[ch][i-w])/(2*w));
		}
	}
	if(Spikes) delete []Spikes;	Spikes=NULL;
	for(i=0;i<SpikesN;i++) {if(SpikesV[i]) delete []SpikesV[i];SpikesV[i]=NULL;}
	delete []SpikesV;	SpikesV=NULL;
	SpikesN=0;
}

void MultiLead_ECG::SpikeRemoval()
{
	int ch,spk;
	long i,j,maxP,minP;
	short Zero;
	int w=10*SampleRate/1000;
	for(spk=0;spk<SpikesN;spk++) {
		i=Spikes[spk];
		for(ch=0;ch<ChN;ch++) {
			maxP=minP=i;
			for(j=i-w;j<i+w;j++) {
				if(DataOut[ch][j]>DataOut[ch][maxP]) maxP=j;
				if(DataOut[ch][j]<DataOut[ch][minP]) minP=j;
			}
			Zero=(DataOut[ch][i-w+2]+DataOut[ch][i+w-2])/2;
			if(maxP<minP) {
				SpikesV[spk][ch].V1=DataOut[ch][maxP]-Zero;
				SpikesV[spk][ch].V2=DataOut[ch][minP]-Zero;
			}
			else {
				SpikesV[spk][ch].V2=DataOut[ch][maxP]-Zero;
				SpikesV[spk][ch].V1=DataOut[ch][minP]-Zero;
			}
			for(j=i-w;j<i+w;j++) DataOut[ch][j]
				=short(DataOut[ch][i-w]+(j-i+w)*(DataOut[ch][i+w]-DataOut[ch][i-w])/(2*w));
		}
	}
}

void MultiLead_ECG::SpikeReset()
{
	int p,ch,spk;
	for(spk=0;spk<SpikesN;spk++) {  
		for(ch=0;ch<ChN;ch++) {
			DataOut[ch][Spikes[spk]-1]+=SpikesV[spk][ch].V1;
			DataOut[ch][Spikes[spk]]+=SpikesV[spk][ch].V2;
		}
	}  

//for template 
	int OKcount=0;
	for(p=1;p<QRSsN-1;p++) if(OutPut.Beats[p].Status==OK) OKcount++;
	OKcount=OKcount/2+1;
	for(p=1;p<QRSsN-1;p++) {
		if(OutPut.Beats[p].Status==OK) OKcount--;
		if(OKcount==0) break;
	}
//	for(p=2;p<QRSsN-1;p++) if(OutPut.Beats[p].Status==OK) break;
	int p0,p1;
	p0=p1=0;
	for(spk=0;spk<SpikesN;spk++) if(Spikes[spk]>Beats[p-1].QRS.Pos) break;
	if(Spikes[spk]<Beats[p].QRS.Pos) p0=OutPut.Temp.Pos-(Beats[p].QRS.Pos-Spikes[spk]);
	if(Spikes[spk+1]<Beats[p].QRS.Pos) p1=OutPut.Temp.Pos-(Beats[p].QRS.Pos-Spikes[spk+1]);
	for(ch=0;ch<ChN;ch++) {
		if(p0>0) {
			OutPut.Temp.Data[ch][p0-1]+=SpikesV[spk][ch].V1;
			OutPut.Temp.Data[ch][p0]+=SpikesV[spk][ch].V2;
		}
		if(p1>0) {
			OutPut.Temp.Data[ch][p1-1]+=SpikesV[spk+1][ch].V1;
			OutPut.Temp.Data[ch][p1]+=SpikesV[spk+1][ch].V2;
		}
	}
	OutPut.Temp.SpikeA=p0;
	OutPut.Temp.SpikeV=p1;  
}

char MultiLead_ECG::PaceMaker_Analysis()
{
	int Acount,Vcount,Si,QRSi;
	int w=160*SampleRate/1000;
	Acount=Vcount=0;
	Si=0;
	for(QRSi=1;QRSi<QRSsN;QRSi++) {
		while(Si<SpikesN&&Spikes[Si]<Beats[QRSi-1].QRS.Pos) Si++;
		if(Si<SpikesN&&Spikes[Si]<Beats[QRSi].QRS.Pos) {
			if(Beats[QRSi].QRS.Pos-Spikes[Si]>w) {
				Acount++;
				Si++;
			}
			if(Si<SpikesN&&Spikes[Si]<Beats[QRSi].QRS.Pos) {
				Vcount++;
				Si++;
			}
		}
	}
	if(Acount>QRSsN/4&&Vcount>QRSsN/4) return 'B';
	if(Acount>QRSsN/4) return 'A';
	if(Vcount>QRSsN/4) return 'V';
	return 'N';
}
/*2010.12.16注释掉，用后面的替代
void MultiLead_ECG::QRSsDetect()
{
	short *temp=new short[Length];
	short *tp=new short[Length];
	int w=66*SampleRate/1000;  //possible width of QRS  72?
	long i;

	KinetEnergy(Data[0],Data[1],Data[2],temp,Length,w);
//	ModifiedKinetEnergy(Data[0],Data[1],Data[2],temp,Length,SampleRate); //Du 2010-11-8
	for(i=0;i<Length;i++) tp[i]=temp[i];
    Average(temp,Length,20);

	int minV=temp[w];
	for(i=w;i<Length-w;i++) if(temp[i]<minV) minV=temp[i];
	for(i=0;i<w;i++) temp[i]=temp[Length-i-1]=0;
	for(i=w;i<Length-w;i++) temp[i]-=minV;

	int maxV=temp[w];
	fotrater(i=w;i<Length-w;i++) if(temp[i]>maxV) maxV=temp[i];
	if(maxV<2*MinWave) return;

	double average=0;
	for(i=w;i<Length-w;i++) {
		if(temp[i]<maxV/4) average+=temp[i];
		else average+=(maxV/4);
	}
	int hold=int(2*average/(Length-2*w));//int(4*average/(Length-2*w));
	if(hold>maxV/2) hold=maxV/2;
	for(i=w;i<Length-w;i++) if(temp[i]>hold) temp[i]=hold;

	i=w;
	long p=i-1;  //i-1;
	while(i<Length-w) {  //平双波
		while(temp[i]<hold&&i<Length-w) i++;
		if(i-p<w&&temp[p]==hold&&temp[i]==hold) while(p<i) temp[p++]=hold; 
		while(temp[i]==hold&&i<Length-w) i++;
		p=i-1;
	}

	int w0=100*SampleRate/1000;
	int w45=45*SampleRate/1000;

	char *QRSs=new char[Length];
	for(i=0;i<Length;i++) QRSs[i]='\0';

	long l,r,l0,r0;
	i=w0+w45;
	while(i<Length-(w0+w45)) {
		if(temp[i]==hold) {
			l0=i;
			l=TurnPoint(temp,l0-w0,l0,-1);
			while(temp[l]>temp[l-1]&&l>l0-w0) l--;
			while(temp[i]==hold&&i<Length-w0) i++;
			r0=i;
			r=TurnPoint(temp,r0,r0+w0,-1);
			while(temp[r]>temp[r+1]&&r<r0+w0) r++;
			if(r0-l0>w/2&&r0-l0<4*w&&r-l>w45) {  //w45 for QRSanalysis
				QRSs[l]='L';  
				QRSs[r]='R';
				QRSsN++; 
			}
			i=r+w0;
		}  //temp[i]==hold
		i++;
	}

	long maxP;
	if(QRSsN>=3) {
		Beats=new BeatFeature[QRSsN];	
		int j=0;
		for(i=0;i<Length;i++) {
			if(QRSs[i]=='L') Beats[j].QRS.Start=i;
			if(QRSs[i]=='R') Beats[j++].QRS.End=i;
		}
		for(j=0;j<QRSsN;j++) {
			maxP=Beats[j].QRS.Start;
			for(i=Beats[j].QRS.Start+1;i<Beats[j].QRS.End;i++) if(tp[maxP]<tp[i]) maxP=i;
			Beats[j].QRS.Pos=maxP;
		}
	}

	delete []temp;
	delete []tp;
	delete []QRSs;
}
*/
//替代2010.12.16注释掉的成员函数
void MultiLead_ECG::QRSsDetect()
{
//	short *temp=new short[Length];
//	short *tp=new short[Length];
	long *temp=new long[Length];
	long *tp=new long[Length];
	int w=66*SampleRate/1000;  //possible width of QRS  66?
	long i;
	KinetEnergy(Data[0],Data[1],Data[2],temp,Length,w/2);   //?????????????????????
	EnergyCorrect(temp,Length,SampleRate/10);   //step=100ms
	for(i=0;i<Length;i++) tp[i]=temp[i];
    Average(temp,Length,20);  //20 is translated in ms in Average
	long minV=temp[w];                                                
	for(i=w;i<Length-w;i++) if(temp[i]<minV) minV=temp[i];
	for(i=0;i<w;i++) temp[i]=temp[Length-i-1]=0;
	for(i=w;i<Length-w;i++) temp[i]-=minV;

	long maxV=temp[w];
	for(i=w;i<Length-w;i++) if(temp[i]>maxV) maxV=temp[i];
	if(maxV<long(250/uVperBit)) {delete[]temp; delete[]tp; return;  }   //2*
/*
	long maxV1,minV1,maxV2,minV2,maxV3,minV3;
	maxV1=minV1=Data[0][w];
	maxV2=minV2=Data[1][w];
	maxV3=minV3=Data[2][w];
	int count=0;
	for(i=w;i<Length-w;i++) {
		if(5*temp[i]>2*maxV) count++;
		if(Data[0][i]>maxV1) maxV1=Data[0][i];
		if(Data[0][i]<minV1) minV1=Data[0][i];
		if(Data[1][i]>maxV2) maxV2=Data[1][i];
		if(Data[1][i]<minV2) minV2=Data[1][i];
		if(Data[2][i]>maxV3) maxV3=Data[2][i];
		if(Data[2][i]<minV3) minV3=Data[2][i];
	}
	if(3*maxV<Max3(maxV1-minV1,maxV2-minV2,maxV3-minV3) && 3*count>Length-2*w && maxV<long(500/uVperBit)) {delete[]temp; delete[]tp; return;  }   //2*
*/
//	if(5*count>Length)  {delete[]temp; delete[]tp; return;} 

	double average=0;
	for(i=w;i<Length-w;i++) {
		if(temp[i]<maxV/4) average+=double(temp[i]);
		else average+=double(maxV/4);
	}
	long hold=long(3.8*average/(Length-2*w));//int(3*average/(Length-2*w));  2016-6-12
	if(hold>maxV/2) hold=maxV/2;
	for(i=w;i<Length-w;i++) if(temp[i]>hold) temp[i]=hold;

	i=w;
	long p=i-1;  //i-1;
	while(i<Length-w) {  //平双波
		while(temp[i]<hold&&i<Length-w) i++;
		if(i-p<w&&temp[p]==hold&&temp[i]==hold) while(p<i) temp[p++]=hold; 
		while(temp[i]==hold&&i<Length-w) i++;
		p=i-1;
		i++;	//added by hspecg. dead loop
	}

	int w0=100*SampleRate/1000;
	int w45=45*SampleRate/1000;

	char *QRSs=new char[Length];
	for(i=0;i<Length;i++) QRSs[i]='\0';

	long l,r,l0,r0;
	i=w0+w45;
	while(i<Length-(w0+w45)) {
		if(temp[i]==hold) {
			l0=i;
			l=TurnPoint(temp,l0-w0,l0,-1);
			while(temp[l]>temp[l-1]&&l>l0-w0) l--;
			while(temp[i]==hold&&i<Length-w0) i++;
			r0=i;
			r=TurnPoint(temp,r0,r0+w0,-1);
			while(temp[r]>temp[r+1]&&r<r0+w0) r++;
			if(r0-l0>w/2&&r0-l0<4*w&&r-l>w45) {  //w45 for QRSanalysis      
				if((CestQRS(&Data[0][l],r-l)&&CestQRS(&Data[1][l],r-l)) || 
					(CestQRS(&Data[0][l],r-l)&&CestQRS(&Data[2][l],r-l)) || 
					(CestQRS(&Data[1][l],r-l)&&CestQRS(&Data[2][l],r-l))) {  //同时两个导联即可，Y2015改（郝玉华）
					QRSs[l]='L';  
					QRSs[r]='R';
					QRSsN++;
				}
			}
			i=r+w0;
		}  //temp[i]==hold
		i++;
	}

	if(QRSsN<2) QRSsN=2;
	Beats=new BeatFeature[QRSsN];	
	if(QRSsN<3) {
		Beats[0].QRS.Start=2*w;
		Beats[0].QRS.Pos=3*w;
		Beats[0].QRS.End=4*w;
		Beats[1].QRS.Start=Length-4*w;
		Beats[1].QRS.Pos=Length-3*w;
		Beats[1].QRS.End=Length-2*w;
	}
	else {
		long maxP;        //Old part
		int j=0;
		for(i=0;i<Length;i++) {
			if(QRSs[i]=='L') Beats[j].QRS.Start=i;
			if(QRSs[i]=='R') Beats[j++].QRS.End=i;
		}
		for(j=0;j<QRSsN;j++) {
			maxP=Beats[j].QRS.Start;
			for(i=Beats[j].QRS.Start+1;i<Beats[j].QRS.End;i++) if(tp[maxP]<tp[i]) maxP=i;
			Beats[j].QRS.Pos=maxP;
		}
	}

	delete []temp;
	delete []tp;
	delete []QRSs;
}

BOOL MultiLead_ECG::CestQRS(short *data,int len)
{
	if(len<40*SampleRate/1000) return FALSE;
	int stdV=short(600/uVperBit);//2013.08.21从800改为550。海纳医信一个窄QRS波计算不对

	int maxV=data[0];
	int minV=data[0];
	for(int k=1;k<len-1;k++) {
		if(data[k]>maxV) maxV=data[k];
		if(data[k]<minV) minV=data[k];
	}
	if((maxV-minV>stdV)&&(abs(data[0]-data[len-1])>3*(maxV-minV)/4)) return FALSE;
	return TRUE;
}

int MultiLead_ECG::MaxPowerValue(short *data,int len)
{
//	int pW=45*SampleRate/1000;
	int w0=40*SampleRate/1000;
	int *power;
	power=new int[len];
//	KinetEnergy(data,power,len,pW);
	SumPower(data,power,len,w0);
	int maxV=0;
	for(int i=w0;i<len-w0;i++) if(power[i]>maxV) maxV=power[i];
//	for(int i=w0;i<len-w0;i++) maxV+=power[i];
	delete []power;
	return maxV;
}

void MultiLead_ECG::SumPower(short *data,int *power,int len,int w0)
{
	int i,k;
	int midV;
	for(i=w0;i<len-w0;i++) {
		power[i]=0;
		midV=data[i-w0];//(data[i-w0]+data[i+w0-1])/2;
		for(k=-w0;k<w0;k++) power[i]+=abs(data[i+k]-midV);
	}
}	

int MultiLead_ECG::MaxTsAverage(short *data)
{
	int Ave=0;
	for(int sn=0;sn<QRSsN-1;sn++) {
		int Tw=(Beats[sn+1].QRS.Start-Beats[sn].QRS.End)/2 < (300*SampleRate/1000)?
			(Beats[sn+1].QRS.Start-Beats[sn].QRS.End)/2 : 300*SampleRate/1000;
		int maxV=0;
		for(int j=Beats[sn].QRS.End;j<Beats[sn].QRS.End+Tw;j++)	if(data[j]>maxV) maxV=data[j];
		Ave+=maxV;
	}
	return Ave;
}

/*  //2016-03-28 to be rewrite
short MultiLead_ECG::LeadSelectDependsOnP()
{
	int sn,ch;
	int *RRd;
	RRd=new int[QRSsN-1];
	for(sn=0;sn<QRSsN-1;sn++) RRd[sn]=Beats[sn+1].QRS.Pos-Beats[sn].QRS.Pos;
	BubbleSort(RRd,QRSsN-1);
	int minRR=550*SampleRate/1000;
	int maxRRp=1;
	for(sn=1;sn<QRSsN-1;sn++) if(RRd[sn]-RRd[sn-1]>RRd[maxRRp]-RRd[maxRRp-1]) maxRRp=sn;
	if(RRd[maxRRp-1]<minRR&&RRd[maxRRp]-RRd[maxRRp-1]>RRd[maxRRp-1]/4) minRR=RRd[maxRRp-1];
	else minRR=240*SampleRate/1000;  //240ms
	delete []RRd;
	int *Diff;
	Diff=new int[ChN];
	for(ch=0;ch<ChN;ch++) Diff[ch]=MaxPsAverage(DataOut[ch],minRR);
	if(ChN>=8) {
		Diff[1]=int(Diff[1]*1.2);    // II
		Diff[0]=int(Diff[0]*1.1);   // I Corrected After HU
		Diff[2]=int(Diff[2]*1.1);   // V1
	}
	int maxP1=0;
	for(ch=0;ch<ChN;ch++) if(Diff[ch]>Diff[maxP1] && Diff[ch]*10<Diff[maxP1]) maxP1=ch; 
	delete []Diff;
	return maxP1;  //ch;
}
*/

/*   //2016-03-28 to be rewrite
int MultiLead_ECG::MaxPsAverage(short *data,int midRR)
{
	long i,j;
	int maxV,Pw; 
	int w0=45*SampleRate/1000;   //possible p width
	int Ave=0;
	int P0,minP1,minP2,minT,maxT;
	for(int sn=1;sn<QRSsN;sn++) {
		if(Beats[sn].QRS.Pos-Beats[sn-1].QRS.Pos<midRR) continue;
		Pw=(Beats[sn].QRS.Start-Beats[sn-1].QRS.End)/2 < (200*SampleRate/1000)?
			(Beats[sn].QRS.Start-Beats[sn-1].QRS.End)/2 : 200*SampleRate/1000;
		P0=minP1=minP2=Beats[sn].QRS.Start-Pw;
		for(i=P0-1;i>P0-w0;i--) if(abs(data[i])<abs(data[minP1])) minP1=i;
		for(i=P0+1;i<P0+w0;i++) if(abs(data[i])<abs(data[minP2])) minP2=i;
		if(minP2-P0<P0-minP1) Pw=Beats[sn].QRS.Start-minP2;
		else Pw=Beats[sn].QRS.Start-minP1;
		P0=Beats[sn].QRS.Start-Pw;
		maxV=0;
		for(j=0;j<Pw-w0;j+=(w0/5)) {
			minT=maxT=0;
			for(i=1;i<w0;i++) {
				if(data[P0+j+i]>data[P0+j+maxT]) maxT=i;
				if(data[P0+j+i]<data[P0+j+minT]) minT=i;
			}
			if(data[P0+j+maxT]-data[P0+j+minT]>maxV) maxV=data[P0+j+maxT]-data[P0+j+minT];
		}
		Ave+=maxV;
	}
	return Ave;
}
*/

//2016-03-28 to be rewrite
short MultiLead_ECG::LeadSelectDependsOnP()
{
	if(ChN<8) return 0;
	int *RRd=new int[QRSsN-1];
	for(int sn=0;sn<QRSsN-1;sn++) RRd[sn]=Beats[sn+1].QRS.Pos-Beats[sn].QRS.Pos;
	BubbleSort(RRd,QRSsN-1);
	int smallRR=RRd[(QRSsN-1)/4];
	int largeRR=RRd[3*(QRSsN-1)/4];
	delete []RRd;

//	double *Diff;
//	Diff=new double[4];
	double Diff[4];
	for(int ch=0;ch<4;ch++) Diff[ch]=MaxPsAverage(DataOut[ch],smallRR,largeRR);
	Diff[0]*=1.2;    // I Corrected After HU
	Diff[1]*=1.5;    // II
	Diff[2]*=1.3;    // V1
//	Diff[3]=int(Diff[0]);    // V2
	int maxCH=1;
	for(int ch=0;ch<4;ch++) if(Diff[ch]>Diff[maxCH] && Diff[ch]<500*Diff[maxCH]) maxCH=ch; 
//	delete []Diff;
	if(maxCH!=1 && Diff[maxCH]>2.2*Diff[1]) maxCH=1;   //2017-1 // 临时for M02-093 房扑
	return maxCH;  //ch;
}

//select the biggest difference of maxV-minV
short MultiLead_ECG::LeadSelectDependsOnQRS()
{
	if(ChN<8) return -1;
//	unsigned short Diff[4];
	int LeadNo=-1;
	unsigned short maxDiff=0;
	short maxV,minV;
	int *QRSh=new int[QRSsN];
	for(int ch=0;ch<ChN;ch++) {
		for(int sn=0;sn<QRSsN;sn++) {
			maxV=minV=DataOut[ch][Beats[sn].QRS.Pos];
			for(int i=Beats[sn].QRS.Start;i<Beats[sn].QRS.End;i++) {
				if(DataOut[ch][i]>maxV) maxV=DataOut[ch][i];
				if(DataOut[ch][i]<minV) minV=DataOut[ch][i];
			}
			QRSh[sn]=maxV-minV;
		}
		BubbleSort(QRSh,QRSsN);
		int maxDp=1;
		for(int sn=2;sn<QRSsN;sn++) if(QRSh[sn]-QRSh[sn-1]>QRSh[maxDp]-QRSh[maxDp-1]) maxDp=sn;
		if(QRSh[maxDp]-QRSh[maxDp-1]>maxDiff && 
			4*maxDp>QRSsN && 4*maxDp<3*QRSsN && QRSh[maxDp]>1.5*QRSh[maxDp-1] && 
			QRSh[maxDp]*uVperBit>2000 && QRSh[maxDp-1]*uVperBit>1000) {
			maxDiff=QRSh[maxDp]-QRSh[maxDp-1];
			LeadNo=ch;
		}
	}
	delete []QRSh;
	return LeadNo; // not found right one
}

//2016-03-28 rewrite
int MultiLead_ECG::MaxPsAverage(short *data,int smallRR,int largeRR)
{
	int Ave=0;
	int errorCount=0;
	for(int sn=0;sn<QRSsN-1;sn++) {
		if(Beats[sn+1].QRS.Pos-Beats[sn].QRS.Pos>=smallRR && Beats[sn+1].QRS.Pos-Beats[sn].QRS.Pos<=largeRR) {
			int zeroV=0;
			int w=12*SampleRate/1000;
			for(int i=0;i<w;i++) zeroV+=data[Beats[sn+1].QRS.Start+i];
			zeroV/=w;
			int dPR=200*SampleRate/1000;
			if(dPR<2*(Beats[sn+1].QRS.Start-Beats[sn].QRS.End)/5) dPR=2*(Beats[sn+1].QRS.Start-Beats[sn].QRS.End)/5;
			if(Beats[sn].QRS.End+dPR>Beats[sn+1].QRS.Start) dPR=(Beats[sn+1].QRS.Start-Beats[sn].QRS.End)/2;
			w*=3;   //36ms window for P_wave
			int maxV=0;
			for(int i=Beats[sn+1].QRS.Start-dPR;i<Beats[sn+1].QRS.Start-w;i++) {
				int tV=0;			
				for(int k=0;k<w;k++) {
					if(abs(data[i+k]-zeroV)>1000/uVperBit) errorCount++;
					else tV+=abs(data[i+k]-zeroV);
				}
				if(tV>maxV) maxV=tV;
			}
			Ave+=maxV;
		}
	}
	if(errorCount<2*QRSsN) return Ave;
	else return 0;
}

short MultiLead_ECG::LeadSelectDependsOnT(short LeadNo)
{
	short sCh=(LeadNo+1)%ChN;
	int maxV=0;
	for(short ch=0;ch<ChN;ch++) {
		if(ch!=LeadNo) {
			int t=MaxTsAverage(DataOut[ch]);
			if(t>maxV) {
				maxV=t;
				sCh=ch;
			}
		}
	}
	return sCh;
}

BOOL MultiLead_ECG::GoodSelectPs()
{
	int i,k;
	int mainPn=0;
	int subPn=0;
	for(i=1;i<QRSsN;i++) {
		if(Beats[i].P[0].Dir!=0) mainPn++;
		if(Beats[i].P[1].Dir!=0) subPn++;
		if(Beats[i].P[2].Dir!=0) subPn++;
	}
	if(mainPn<3||4*mainPn<QRSsN) return FALSE;
	int PsN=mainPn+subPn;
	int *PsD=new int[PsN];
	int maxV=0;
	int s=0;
	for(i=1;i<QRSsN;i++) {
		for(k=2;k>=0;k--) {
			if(Beats[i].P[k].Dir!=0) {
				PsD[s++]=Beats[i].P[k].Onset;
				if(Beats[i].P[k].Dir>0&&Beats[i].P[k].pV>maxV) maxV=Beats[i].P[k].pV;
			}
		}
		if(Beats[i].P[0].Dir<0&&Beats[i].P[0].nV>maxV) maxV=Beats[i].P[0].nV;
	}
	int maxD,minD;
	maxD=minD=PsD[1]-PsD[0];
	for(s=1;s<PsN-1;s++) {
		if(PsD[s+1]-PsD[s]>maxD) maxD=PsD[s+1]-PsD[s];
		if(PsD[s+1]-PsD[s]<minD) minD=PsD[s+1]-PsD[s];
	}
	delete []PsD; 
	if(3.3*minD<maxD&&maxV<2.2*MinWave) return FALSE;
	if(7*minD<maxD) return FALSE;
	return TRUE;
}

short MultiLead_ECG::SubLeadSelect()
{  //to be improved
	if(ChN>=8) {
		if(OutPut.LeadNo!=2) return 2;  //V1 
		else return 1;  //II
	}
	if(ChN>1) return (OutPut.LeadNo+1)%ChN;
	return OutPut.LeadNo;
}

void MultiLead_ECG::SubQRSanalysis(int QRSi,short *data)
{
	int step=10*SampleRate/1000;
	int w0=40*SampleRate/1000;
	int QRSw=Beats[QRSi].QRS.End-Beats[QRSi].QRS.Start;
	short *QRSdata=&data[Beats[QRSi].QRS.Start];

	int i,s,maxP,minP,left,right;//minQ,maxQ;
	BOOL HILL,VALE;

	HILL=VALE=FALSE;
	int sum=0;
	for(i=0;i<step;i++) sum+=QRSdata[i];
	short zero=short(sum/step);
	s=maxP=minP=2*step;
	int range=QRSw-w0-2*step>4*step? QRSw-w0-2*step:4*step;
	while(s<range) {
		for(i=0;i<w0;i++) {
			if(QRSdata[s+i]>QRSdata[maxP]) maxP=s+i;
			if(QRSdata[s+i]<QRSdata[minP]) minP=s+i;
		}
		if(Hill(QRSdata,maxP,2*step,zero)) {
			HILL=TRUE;
			break;
		}
		if(Vale(QRSdata,minP,2*step,zero)) {
			VALE=TRUE;
			break;
		}
		s+=step;
	}
	if(!HILL&&!VALE) left=2*step;
	else if(HILL) left=TurnPoint(QRSdata,step,maxP,-1);
	else left=TurnPoint(QRSdata,step,minP,1);  

	HILL=VALE=FALSE;
	sum=0;
	for(i=0;i<step;i++) sum+=QRSdata[QRSw-step-i];
	zero=short(sum/step);
	s=maxP=minP=QRSw-2*step;
	range=w0+2*step<QRSw-4*step? w0+2*step:QRSw-4*step;
	while(s>range) {
		for(i=w0;i>0;i--) {
			if(QRSdata[s-i]>QRSdata[maxP]) maxP=s-i;
			if(QRSdata[s-i]<QRSdata[minP]) minP=s-i;
		}
		if(Hill(QRSdata,maxP,2*step,zero)) {
			HILL=TRUE;
			break;
		}
		if(Vale(QRSdata,minP,2*step,zero)) {
			VALE=TRUE;
			break;
		}
		s-=step;
	}
	if(!HILL&&!VALE) right=QRSw-2*step;
	else if(HILL) right=TurnPoint(QRSdata,maxP,QRSw-step,-1);
	else right=TurnPoint(QRSdata,minP,QRSw-step,1);
	if(right<=left) right=left+step;     //necesseray?

//	OutPut.Beats[QRSi].SubQRSon=Beats[QRSi].QRS.Start+left;  //for test
	OutPut.Beats[QRSi].SubQRSw=(right-left)*1000/SampleRate;

	maxP=minP=left;
	for(i=left;i<right;i++) {
		if(QRSdata[i]>QRSdata[maxP]) maxP=i;
		if(QRSdata[i]<QRSdata[minP]) minP=i;
	}
	if(QRSdata[maxP]<2*MinWave||!Hill(QRSdata,maxP,2*step,0)) maxP=-1;
	if(QRSdata[minP]>-2*MinWave||!Vale(QRSdata,minP,2*step,0)) minP=-1;

	if(maxP==-1&&minP==-1) OutPut.Beats[QRSi].SubQRSdir=0;
	else if(minP==-1) OutPut.Beats[QRSi].SubQRSdir=1;
	else if(maxP==-1) OutPut.Beats[QRSi].SubQRSdir=-1;
	else {
		if(QRSdata[maxP]>-QRSdata[minP]) {
			if(QRSdata[minP]<-3*MinWave) OutPut.Beats[QRSi].SubQRSdir=2;
			else OutPut.Beats[QRSi].SubQRSdir=1;
		}
		else {
			if(QRSdata[maxP]>3*MinWave) OutPut.Beats[QRSi].SubQRSdir=-2; 
			else OutPut.Beats[QRSi].SubQRSdir=-1;
		}
	}
}
/*
void MultiLead_ECG::SubLeadTs()
{  //detect if Ts exist in (V3) 
	int Lead=0;
	if(ChN>=8) {
		if(OutPut.LeadNo!=4) Lead=4;  //V3
		else Lead=1;  //II
	}
	else if(ChN>1) Lead=(OutPut.LeadNo+1)%ChN;

	int w=20*SampleRate/1000;
	for(int i=0;i<QRSsN-1;i++) {
		int len=3*(Beats[i+1].QRS.Start-Beats[i].QRS.End)/5;
		short *data=new short[len+2*w];
		for(int k=0;k<len+2*w;k++) data[k]=DataOut[Lead][Beats[i].QRS.End-w+k];
		Average(data,len+2*w,30);
		int SubT=SubLeadT(&data[w],len);
		if(SubT>0) Beats[i].SubT=SubT+Beats[i].QRS.End;
		else Beats[i].SubT=-1;
		delete []data;
	}
}

int MultiLead_ECG::SubLeadT(short *data,int len)
{  //return T position (Hill or Vale P) 
	int i,k;
	short stdTv=short(80/uVperBit);
	int step=80*SampleRate/1000;
	int hillP,valeP;
	hillP=valeP=len-step;
	for(i=step;i<len-step;i++) {
		if(data[i]-data[i-step]>stdTv&&data[i]-data[i+step]>stdTv) {
			hillP=i;
			for(k=i-step/2;k<i+step/2;k++) if(data[k]>data[hillP]) hillP=k;
			break;
		}
	}
	for(i=step;i<len-step;i++) {
		if(data[i-step]-data[i]>stdTv&&data[i+step]-data[i]>stdTv) {
			valeP=i;
			for(k=i-step/2;k<i+step/2;k++) if(data[k]<data[valeP]) valeP=k;
			break;
		}
	}
	if(hillP==valeP) return -1;
	else if(hillP<valeP) return hillP;
	else return valeP;
}
*/
void MultiLead_ECG::ECG_Analysis()
{
	long i,sn;
	if(!sData) sData=new short[Length];
	for(i=0;i<Length;i++) sData[i]=DataOut[OutPut.LeadNo][i];
	Average(sData,Length,20);

	for(sn=0;sn<QRSsN;sn++) QRSanalysis(sn);
	for(sn=0;sn<QRSsN-1;sn++) PTUanalysis(sn);
}

void MultiLead_ECG::CorrectPTs()   //Y2015-7
{
	int i,CountP=0;
	for(i=1;i<QRSsN;i++) if(Beats[i].P[0].Dir!=0) CountP++;
	int *PRds=new int[CountP];
	int k=0;
	for(i=0;i<QRSsN;i++) if(Beats[i].P[0].Dir!=0) PRds[k++]=Beats[i].QRS.Pos-Beats[i].P[0].Onset;
	BubbleSort(PRds,CountP);
	int AvgPR=PRds[CountP/4];
	delete []PRds;
	int AvgRR=(Beats[QRSsN-1].QRS.Pos-Beats[0].QRS.Pos)/(QRSsN-1);
	short stTv=short(200/uVperBit);
	if(CountP>4) {
		if(2*AvgPR>AvgRR) {
			for(i=1;i<QRSsN;i++) 
				if(Beats[i].P[0].Dir!=0 &&
					2*(Beats[i].QRS.Pos-Beats[i].P[0].Onset)>(Beats[i].QRS.Pos-Beats[i-1].QRS.Pos) &&
					Beats[i-1].T.Dir!=0 && (Beats[i-1].T.pV>stTv || Beats[i-1].T.nV>stTv))
						Beats[i].P[0].Dir=Beats[i].P[1].Dir=Beats[i].P[2].Dir=0;
		}
	}
}

void MultiLead_ECG::ParametersCalculation()
{ 
	for(int i=0;i<QRSsN;i++) {
	//      QRS parameters		
		OutPut.Beats[i].QRSonset=Beats[i].QRS.Onset;
		if(Beats[i].QRS.MaxP==-1&&Beats[i].QRS.MinP==-1) OutPut.Beats[i].QRSdir=0;
		else if(Beats[i].QRS.MinP==-1) OutPut.Beats[i].QRSdir=1;
		else if(Beats[i].QRS.MaxP==-1) OutPut.Beats[i].QRSdir=-1;
		else {
			if(sData[Beats[i].QRS.MaxP]>-sData[Beats[i].QRS.MinP]) OutPut.Beats[i].QRSdir=2;
			else OutPut.Beats[i].QRSdir=-2; 
		}
		OutPut.Beats[i].QRSw=short((Beats[i].QRS.Offset-Beats[i].QRS.Onset)*1000L/SampleRate);
		OutPut.Beats[i].Pos=Beats[i].QRS.Pos;

//QRS-hight: New parameter for output //2017-1 added
		OutPut.Beats[i].QRSh=Beats[i].QRS.Range;

	//	PR and P parameters 
//		if(i==0) OutPut.Beats[i].Pdir=0;
		if(i>0) {
			OutPut.Beats[i].Pdir=Beats[i].P[0].Dir;
			OutPut.Beats[i].Pwide=Beats[i].P[0].Wide;
			if(Beats[i].P[2].Dir!=0) OutPut.Beats[i].Pnum=3;
			else if(Beats[i].P[1].Dir!=0) OutPut.Beats[i].Pnum=2;
			else if(Beats[i].P[0].Dir!=0) OutPut.Beats[i].Pnum=1;
			else OutPut.Beats[i].Pnum=0;
			if(Beats[i].P[0].Dir!=0) {
				OutPut.Beats[i].PR=short((Beats[i].QRS.Onset-Beats[i].P[0].Onset)*1000L/SampleRate); 
				OutPut.Beats[i].PtoR=short((Beats[i].QRS.Pos-Beats[i].P[0].Onset)*1000L/SampleRate); 
			}
			else {
				OutPut.Beats[i].PR=0;
				OutPut.Beats[i].PtoR=0;
			}
			if(Beats[i].P[1].Dir!=0) 
				OutPut.Beats[i].PR1=short((Beats[i].QRS.Onset-Beats[i].P[1].Onset)*1000L/SampleRate); 
			else OutPut.Beats[i].PR1=0;
			if(Beats[i].P[2].Dir!=0) 
				OutPut.Beats[i].PR2=short((Beats[i].QRS.Onset-Beats[i].P[2].Onset)*1000L/SampleRate); 
			else OutPut.Beats[i].PR2=0;
		}
	//	QT and T/U parameters
//		if(i==QRSsN-1) OutPut.Beats[i].Tdir=OutPut.Beats[i].Udir=0;
		if(i<QRSsN-1) {
			OutPut.Beats[i].Tdir=Beats[i].T.Dir;
			OutPut.Beats[i].Udir=Beats[i].U.Dir;
			if(Beats[i].T.Dir!=0) 
				OutPut.Beats[i].QT=short((Beats[i].T.Offset-Beats[i].QRS.Onset)*1000L/SampleRate);
		}
	}

	StatusConfirm();

// V-Rate	
	int RR=(Beats[QRSsN-1].QRS.Pos-Beats[0].QRS.Pos)/(QRSsN-1);
	OutPut.AverageRR=short(RR*1000L/SampleRate);
	OutPut.Vrate=60*SampleRate/RR;

// TemplateRR  Y2015/7/8
	int TemplateRR=0;
	int okCount=0;
	for(int i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Status==OK) {
			TemplateRR+=((Beats[i+1].QRS.Pos-Beats[i-1].QRS.Pos)/2);
			okCount++;
		}
	}
	if(okCount>0) OutPut.TemplateRR=TemplateRR/okCount;
	else OutPut.TemplateRR=RR;
	OutPut.TemplateRR=short(OutPut.TemplateRR*1000L/SampleRate);  // ms

//  max and min HR   //2015/5/12
	int maxRR,minRR;
	maxRR=minRR=RR;
	for(int i=0;i<QRSsN-1;i++) {
		if(Beats[i+1].QRS.Pos-Beats[i].QRS.Pos>maxRR) maxRR=Beats[i+1].QRS.Pos-Beats[i].QRS.Pos;
		if(Beats[i+1].QRS.Pos-Beats[i].QRS.Pos<minRR) minRR=Beats[i+1].QRS.Pos-Beats[i].QRS.Pos;
	}
	OutPut.MaxRR=short(maxRR*1000L/SampleRate);
	OutPut.MinRR=short(minRR*1000L/SampleRate);

// A-Rate
	OutPut.Arate=ArateDetecte();
	
//	int tN=1;
//	for(int i=0;i<QRSsN-1;i++) if(Beats[i].T.Dir!=0) tN++;
//	if(Length!=0) OutPut.Trate=short(60*tN*SampleRate/double(Length));
}

void MultiLead_ECG::StatusConfirm()
{ 
	int i;
	OutPut.Beats[0].Status=OutPut.Beats[QRSsN-1].Status=BORDER;
	for(i=1;i<QRSsN-1;i++) OutPut.Beats[i].Status=OK;

//Y2015/5/29
    BEAT_STATUS_TYPE *TempStatus=new BEAT_STATUS_TYPE[QRSsN-1];
	for(i=1;i<QRSsN-1;i++) TempStatus[i]=OK;

//0th step: QRS range //added 2017-1
	int *QRSh=new int[QRSsN];
	for(i=0;i<QRSsN;i++) QRSh[i]=Beats[i].QRS.Range;
	BubbleSort(QRSh,QRSsN);
	int maxDi=1;
	for(i=1;i<QRSsN;i++) if(QRSh[i]-QRSh[i-1]>QRSh[maxDi]-QRSh[maxDi-1]) maxDi=i;
	if(4*maxDi>QRSsN && 4*maxDi<3*QRSsN && QRSh[maxDi]>1.7*QRSh[maxDi-1] && QRSh[maxDi-1]>500 && QRSh[maxDi]>2000)   //1000 uV
		for(i=1;i<QRSsN-1;i++) if(Beats[i].QRS.Range>=QRSh[maxDi]) OutPut.Beats[i].Status=QRS_Range;
	delete []QRSh;

// 1st step: HR
	int *RRd=new int[QRSsN-1];
	for(i=0;i<QRSsN-1;i++) RRd[i]=Beats[i+1].QRS.Pos-Beats[i].QRS.Pos;
	BubbleSort(RRd,QRSsN-1);

	int minRR=600*SampleRate/1000;   //550  //2017-1 change to 600
	int maxRRp=1;
	for(i=1;i<QRSsN-1;i++) if(RRd[i]-RRd[i-1]>RRd[maxRRp]-RRd[maxRRp-1]) maxRRp=i;
	if(RRd[maxRRp-1]<minRR&&RRd[maxRRp]-RRd[maxRRp-1]>RRd[maxRRp-1]/4) {
		for(i=1;i<QRSsN-1;i++)
			if(Beats[i].QRS.Pos-Beats[i-1].QRS.Pos<=RRd[maxRRp-1]||
				Beats[i+1].QRS.Pos-Beats[i].QRS.Pos<=RRd[maxRRp-1])
//				OutPut.Beats[i].Status=RR_Int;
				if(OutPut.Beats[i].Status==OK) TempStatus[i]=RR_Int;    //Y2015/5/29
	}

	minRR=450*SampleRate/1000;   //450
	if(RRd[0]<minRR&&RRd[QRSsN-2]-RRd[0]>RRd[maxRRp-1]/2) {
		for(i=1;i<QRSsN-1;i++)
			if(Beats[i].QRS.Pos-Beats[i-1].QRS.Pos<=minRR||
				Beats[i+1].QRS.Pos-Beats[i].QRS.Pos<=minRR)
//				OutPut.Beats[i].Status=RR_Int;
				if(OutPut.Beats[i].Status==OK) TempStatus[i]=RR_Int;   //Y2015/5/29
	}
	delete []RRd;

// 2nd step: QRS width (main)
	int *QRSsW=new int[QRSsN];    
	for(i=0;i<QRSsN;i++) QRSsW[i]=OutPut.Beats[i].QRSw;  //mS
	BubbleSort(QRSsW,QRSsN);
	int maxDp=1;  
	for(i=1;i<QRSsN;i++) if(QRSsW[i]-QRSsW[i-1]>QRSsW[maxDp]-QRSsW[maxDp-1]) maxDp=i;
	if(QRSsW[maxDp]-QRSsW[maxDp-1]>3*QRSsW[maxDp-1]/5) {
		if(maxDp>(QRSsN-2)/2) { 
			for(i=1;i<QRSsN-1;i++) 
				if(OutPut.Beats[i].QRSw>=QRSsW[maxDp]&&OutPut.Beats[i].Status==OK) 
					OutPut.Beats[i].Status=QRS_W;
		}
		else {
			for(i=1;i<QRSsN-1;i++) 
				if(OutPut.Beats[i].QRSw<QRSsW[maxDp]&&OutPut.Beats[i].Status==OK) 
					OutPut.Beats[i].Status=QRS_W;
		}
	}

//QRSw (sub)
	for(i=0;i<QRSsN;i++) QRSsW[i]=OutPut.Beats[i].SubQRSw;  //mS
	BubbleSort(QRSsW,QRSsN);
	maxDp=1;  
	for(i=1;i<QRSsN;i++) if(QRSsW[i]-QRSsW[i-1]>QRSsW[maxDp]-QRSsW[maxDp-1]) maxDp=i;
	if(QRSsW[maxDp]-QRSsW[maxDp-1]>3*QRSsW[maxDp-1]/5) {
		if(maxDp>(QRSsN-2)/2) { 
			for(i=1;i<QRSsN-1;i++) 
				if(OutPut.Beats[i].SubQRSw>=QRSsW[maxDp]&&OutPut.Beats[i].Status==OK) 
					OutPut.Beats[i].Status=SubQRS_W;
		}
		else {
			for(i=1;i<QRSsN-1;i++) 
				if(OutPut.Beats[i].SubQRSw<QRSsW[maxDp]&&OutPut.Beats[i].Status==OK) 
					OutPut.Beats[i].Status=SubQRS_W;
		}
	}
	delete []QRSsW;

// 3rd step: QRS direction (main)
	int pC,nC,zC;
	pC=nC=zC=0;
	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].QRSdir>0&&OutPut.Beats[i].Status==OK) pC++;
		if(OutPut.Beats[i].QRSdir<0&&OutPut.Beats[i].Status==OK) nC++;
		if(OutPut.Beats[i].QRSdir==0&&OutPut.Beats[i].Status==OK) zC++;
	}
	if(pC>=nC&&pC>=zC) {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].QRSdir<=0)
			OutPut.Beats[i].Status=QRS_Dir;
	}
	else if(nC>zC) {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].QRSdir>=0)
			OutPut.Beats[i].Status=QRS_Dir;
	}
	else {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].QRSdir!=0)
			OutPut.Beats[i].Status=QRS_Dir;
	}
//QRS direction (sub)
	pC=nC=zC=0;
	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].SubQRSdir>0&&OutPut.Beats[i].Status==OK) pC++;
		if(OutPut.Beats[i].SubQRSdir<0&&OutPut.Beats[i].Status==OK) nC++;
		if(OutPut.Beats[i].SubQRSdir==0&&OutPut.Beats[i].Status==OK) zC++;
	}
	if(pC>=nC&&pC>=zC) {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].SubQRSdir<=0)
			OutPut.Beats[i].Status=SubQRS_Dir;
	}
	else if(nC>zC) {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].SubQRSdir>=0)
			OutPut.Beats[i].Status=SubQRS_Dir;
	}
	else {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].SubQRSdir!=0)
			OutPut.Beats[i].Status=SubQRS_Dir;
	}

// 4th step: T direction
	pC=nC=zC=0;
	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Tdir==0&&OutPut.Beats[i].Status==OK) zC++;
		if(OutPut.Beats[i].Tdir==1&&OutPut.Beats[i].Status==OK) pC++;
		if(OutPut.Beats[i].Tdir==-1&&OutPut.Beats[i].Status==OK) nC++;
		if(OutPut.Beats[i].Tdir==2&&OutPut.Beats[i].Status==OK) pC++;
		if(OutPut.Beats[i].Tdir==-2&&OutPut.Beats[i].Status==OK) {
			if(Beats[i].T.nV<3*MinWave&&Beats[i].T.pV>3*Beats[i].T.nV/5) pC++;
			else nC++;
		}
	}
	if(pC>=nC&&pC>=zC) {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].Tdir<=0)
			OutPut.Beats[i].Status=T_Dir;
	}
	else if(nC>zC) {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].Tdir>=0)
			OutPut.Beats[i].Status=T_Dir;
	}
	else {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].Tdir!=0)
			OutPut.Beats[i].Status=T_Dir;
	}

// 5th step: P direction (see M01-006
/*	pC=nC=zC=0;
	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Pdir==0&&OutPut.Beats[i].Status==OK) zC++;
		if(OutPut.Beats[i].Pdir==1&&OutPut.Beats[i].Status==OK) pC++;
		if(OutPut.Beats[i].Pdir==-1&&OutPut.Beats[i].Status==OK) nC++;
		if(OutPut.Beats[i].Pdir==2&&OutPut.Beats[i].Status==OK) pC++;
		if(OutPut.Beats[i].Pdir==-2&&OutPut.Beats[i].Status==OK) {
//			if(Beats[i].T.nV<2*MinWave&&Beats[i].T.pV>3*Beats[i].T.nV/5) pC++;
			if(Beats[i].T.pV>Beats[i].T.nV/2) pC++;
			else nC++;
		}
	}
	if(pC>=nC&&pC>=zC/2) {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].Pdir<=0)
			OutPut.Beats[i].Status=P_Dir;
	}
	else if(nC>zC/2) {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].Pdir>=0)
			OutPut.Beats[i].Status=P_Dir;
	}
	else {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].Pdir!=0)
			OutPut.Beats[i].Status=P_Dir;
	}
*/

	pC=nC=zC=0;
	int bC=0;
	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Pdir==0&&OutPut.Beats[i].Status==OK) zC++;
		if(OutPut.Beats[i].Pdir==1&&OutPut.Beats[i].Status==OK) pC++;
		if(OutPut.Beats[i].Pdir==-1&&OutPut.Beats[i].Status==OK) nC++;
		if(abs(OutPut.Beats[i].Pdir)==2&&OutPut.Beats[i].Status==OK) bC++;
	}
	if(pC>=nC&&pC>=zC&&pC>=bC) {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].Pdir!=1)
			OutPut.Beats[i].Status=P_Dir;
	}
	else if(nC>zC&&nC>=bC) {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].Pdir!=-1)
			OutPut.Beats[i].Status=P_Dir;
	}
	else if(bC>=zC) {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&abs(OutPut.Beats[i].Pdir)!=2)
			OutPut.Beats[i].Status=P_Dir;
	}
	else {
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK&&OutPut.Beats[i].Pdir!=0)
			OutPut.Beats[i].Status=P_Dir;
	}

//deal with pairs  
	int RangeCount=0;
	for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==QRS_Range) RangeCount++;
	if(5*RangeCount<QRSsN-2) {  //to solve 117 problem 
		if((OutPut.Beats[1].Status==OK && OutPut.Beats[2].Status!=OK)||
			(OutPut.Beats[1].Status!=OK && OutPut.Beats[2].Status==OK)) {
			for(i=1;i<QRSsN-3;i++) if(OutPut.Beats[i].Status!=OutPut.Beats[i+2].Status) break;
				if(i==QRSsN-3) {
				if(OutPut.Beats[1].SubQRSw<OutPut.Beats[2].SubQRSw) {
					for(i=1;i<QRSsN-2;i+=2) {
						OutPut.Beats[i].Status=OK;
						OutPut.Beats[i+1].Status=SubQRS_W;
					}
				}
				else {
					for(i=1;i<QRSsN-2;i+=2) {
						OutPut.Beats[i+1].Status=OK;
						OutPut.Beats[i].Status=SubQRS_W;
					}
				}
			}
		}
	}

//Y2015/5/29 考做RR_int与OK的交换：如果RR二倍于OK
	int RRcount=0;
	int OKcount=0;
	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Status==OK && TempStatus[i]==OK) OKcount++;
		if(OutPut.Beats[i].Status==OK && TempStatus[i]==RR_Int) RRcount++;
	}
	if(2*OKcount<RRcount) {
		for(i=1;i<QRSsN-1;i++) 
			if(OutPut.Beats[i].Status==OK && TempStatus[i]==OK) OutPut.Beats[i].Status=fOK; 
	}
	else {
		for(i=1;i<QRSsN-1;i++) 
			if(TempStatus[i]==RR_Int) OutPut.Beats[i].Status=RR_Int; 
	}
	delete []TempStatus;

// if no OK
	for(i=1;i<QRSsN;i++) if(OutPut.Beats[i].Status==OK) break;
	if(i==QRSsN) {
		if(QRSsN==3) OutPut.Beats[1].Status=OK;
		int Rp=1;
		for(i=2;i<QRSsN-1;i++) 
			if(Beats[i+1].QRS.Pos-Beats[i-1].QRS.Pos>Beats[Rp+1].QRS.Pos-Beats[Rp-1].QRS.Pos)
				Rp=i;
		OutPut.Beats[Rp].Status=OK;
	}

	ComplementPs();

	OK_StatusCorrect();
}

void MultiLead_ECG::ComplementPs()
{
	int i;
	int Pdir=0;
	for(i=1;i<QRSsN;i++) if(abs(Beats[i].P[0].Dir)==1) Pdir=Beats[i].P[0].Dir;
	if(Pdir==0) return;
	for(i=1;i<QRSsN;i++) {
		if(Beats[i].P[0].Dir!=0&&Beats[i].P[0].Dir!=Pdir) return;
		if(Beats[i].P[1].Dir!=0) return;
		if(Beats[i].P[0].Dir>0&&Beats[i].P[0].pV>MinWave) return;
		if(Beats[i].P[0].Dir<0&&Beats[i].P[0].nV>MinWave) return;
	}
	for(i=2;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Status==P_Dir&&
			OutPut.Beats[i-1].Status==OK&&OutPut.Beats[i+1].Status==OK) {
				OutPut.Beats[i].Status=OK;
				OutPut.Beats[i].Pdir=OutPut.Beats[i-1].Pdir;
				OutPut.Beats[i].PR=(OutPut.Beats[i-1].PR+OutPut.Beats[i+1].PR)/2;
				OutPut.Beats[i].Pnum=1;
				Beats[i].P[0].Dir=OutPut.Beats[i].Pdir;
		}
	}
}

void MultiLead_ECG::OK_StatusCorrect()
{  
	int i;
	int OkCount=0;
	for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK) OkCount++;
	if(OkCount<3) return;  
	int *OkRR=new int[OkCount];  
	int k=0;
	for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK) OkRR[k++]=OutPut.Beats[i].Pos-OutPut.Beats[i-1].Pos;
	BubbleSort(OkRR,OkCount);
	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Status==OK && OutPut.Beats[i].Pos-OutPut.Beats[i-1].Pos<4*OkRR[OkCount/2]/7)	OutPut.Beats[i].Status=RR_Int;
		if(OutPut.Beats[i].Status==OK && OutPut.Beats[i].Pos-OutPut.Beats[i-1].Pos>7*OkRR[OkCount/2]/4)	OutPut.Beats[i].Status=RR_Int;
	}
	delete []OkRR;

	OkCount=0;
	for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK) OkCount++;
	if(OkCount<3) return;
	OkRR=new int[OkCount];
	k=0;
	for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK) OkRR[k++]=OutPut.Beats[i+1].Pos-OutPut.Beats[i].Pos;
	BubbleSort(OkRR,OkCount);
	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Status==OK && OutPut.Beats[i+1].Pos-OutPut.Beats[i].Pos<4*OkRR[OkCount/2]/7)	OutPut.Beats[i].Status=RR_Int;
		if(OutPut.Beats[i].Status==OK && OutPut.Beats[i+1].Pos-OutPut.Beats[i].Pos>7*OkRR[OkCount/2]/4)	OutPut.Beats[i].Status=RR_Int;
	}
	delete []OkRR;
/*
// deal with multi-P
	OkCount=0;
	int mPcount=0;
	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Status==OK) {
			OkCount++;
			if(OutPut.Beats[i].PR1>0) mPcount++;
		}
	}
	if(OkCount<2 || mPcount==0) return;
	if(OkCount>2*mPcount) for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Status==OK && OutPut.Beats[i].PR1>0) OutPut.Beats[i].Status=m_P;
	}
	else {  //未完成
		mPcount=0;
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK && OutPut.Beats[i].PR>0) mPcount++;
		int *OkP=new int[mPcount];
		k=0;
		for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK && OutPut.Beats[i].PR>0) OkP[k++]=OutPut.Beats[i].PR;
		BubbleSort(OkP,mPcount);
		for(i=1;i<QRSsN-1;i++)
			if(OutPut.Beats[i].Status==OK && OutPut.Beats[i].PR>0 && OutPut.Beats[i].PR!=OkP[mPcount/2]) 
				OutPut.Beats[i].Status=m_P;
		delete []OkP;
	}
*/

// deal with different PR_durations. replace above "deal with multi-P 2016-7
	OkCount=0;
	int Pcount=0;
	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Status==OK) {
			OkCount++;
			if(OutPut.Beats[i].Pdir!=0) Pcount++;
		}
	}
	if(OkCount<2 || Pcount<2) return;  //normally, it is made sure before that OKcount==Pcount
	int *OkPR=new int[Pcount];
	for(k=0,i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK && OutPut.Beats[i].Pdir!=0) OkPR[k++]=OutPut.Beats[i].PtoR; 
	BubbleSort(OkPR,Pcount);
	int PRm=OkPR[Pcount/2];  //PR, PtoR, Pwide they are all in ms 
	delete []OkPR;
	int Pw=0;
	for(i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Status==OK && OutPut.Beats[i].Pdir!=0) Pw+=OutPut.Beats[i].Pwide;
	Pw/=Pcount;
	for(i=1;i<QRSsN-1;i++) {
		if(OutPut.Beats[i].Status==OK && OutPut.Beats[i].Pdir!=0 && OutPut.Beats[i].PtoR!=PRm) {
			if(OutPut.Beats[i].PtoR>PRm+Pw/5 || OutPut.Beats[i].PtoR<PRm-Pw/5) OutPut.Beats[i].Status=PR_Int;
		}
	}
}

short MultiLead_ECG::ArateDetecte()
{
	int i;
	int mainPn=0;
	int subPn=0;
	for(i=1;i<QRSsN;i++) {
		if(Beats[i].P[0].Dir!=0) mainPn++;
		if(Beats[i].P[1].Dir!=0) subPn++;
		if(Beats[i].P[2].Dir!=0) subPn++;
	}
	if(mainPn==QRSsN-1&&subPn==0) return OutPut.Vrate;
	if(mainPn<3||mainPn<QRSsN/3) return 0;

	int PsN=mainPn+subPn;
	int *PsD=new int[PsN];
	int s=0;
	int maxV=0;
	for(i=1;i<QRSsN;i++) {
		for(int k=2;k>=0;k--) {
			if(Beats[i].P[k].Dir!=0) {
				PsD[s++]=Beats[i].P[k].Onset;
				if(Beats[i].P[k].Dir>0&&Beats[i].P[k].pV>maxV) maxV=Beats[i].P[k].pV;
			}
			if(Beats[i].P[k].Dir<0&&Beats[i].P[k].nV>maxV) maxV=Beats[i].P[k].nV;
		}
	}
	int RR=PsD[PsN-1]-PsD[0];
	for(s=0;s<PsN-1;s++) PsD[s]=PsD[s+1]-PsD[s];
	BubbleSort(PsD,PsN-1);
	int maxP=0;
	for(s=0;s<PsN-2;s++) if(PsD[s+1]-PsD[s]>PsD[maxP+1]-PsD[maxP]) maxP=s;
	int pN=0;
	if(3.5*PsD[0]<PsD[PsN-2]&&maxV<2*MinWave) {
		for(i=1;i<QRSsN;i++) {
			OutPut.Beats[i].Pdir=0;
			OutPut.Beats[i].Pnum=0;
			Beats[i].P[0].Dir=Beats[i].P[1].Dir=Beats[i].P[2].Dir=0;
		}  //pN=0
	}
	else {
		if(mainPn==QRSsN-1&&maxP>0&&maxP<PsN-2&&2*PsD[maxP+1]/3>PsD[maxP]&&
			PsD[0]>3*PsD[maxP]/5&&PsD[maxP+1]>3*PsD[PsN-2]/5) pN=PsN+(PsN-1-(maxP+1))-1;
		else pN=PsN-1;
	}

	if(pN==PsN-1&&5*PsN/3>=QRSsN-1&&maxP>1&&maxP<PsN-2&&2.5*PsD[maxP-1]<PsD[maxP+1]&&
		PsD[0]>3*PsD[maxP-1]/5&&PsD[maxP+1]>3*PsD[PsN-2]/5) pN=PsN+2*(PsN-1-(maxP+1))-1;

	delete []PsD;
	if(RR!=0) return 60*pN*SampleRate/RR;
	return 0;
}

void MultiLead_ECG::QRSanalysis(int QRSi)
{
	int step=10*SampleRate/1000;
	int w0=40*SampleRate/1000;
	int QRSw=Beats[QRSi].QRS.End-Beats[QRSi].QRS.Start;
	short *QRSdata=&sData[Beats[QRSi].QRS.Start];

	int i,s,maxP,minP,left,right;//minQ,maxQ;
	BOOL HILL,VALE;

	HILL=VALE=FALSE;
	int sum=0;
	for(i=0;i<step;i++) sum+=QRSdata[i];
	short zero=short(sum/step);
	s=maxP=minP=2*step;
	int range=QRSw-w0-2*step>4*step? QRSw-w0-2*step:4*step;
	while(s<range) {
		for(i=0;i<w0;i++) {
			if(QRSdata[s+i]>QRSdata[maxP]) maxP=s+i;
			if(QRSdata[s+i]<QRSdata[minP]) minP=s+i;
		}
		if(Hill(QRSdata,maxP,2*step,zero)) {
			HILL=TRUE;
			break;
		}
		if(Vale(QRSdata,minP,2*step,zero)) {
			VALE=TRUE;
			break;
		}
		s+=step;
	}
	if(!HILL&&!VALE) left=2*step;
	else if(HILL) left=TurnPoint(QRSdata,step,maxP,-1);
	else left=TurnPoint(QRSdata,step,minP,1);  
	Beats[QRSi].QRS.Onset=Beats[QRSi].QRS.Start+left;
	                //we got QRS.Onset in [QRSw+step, QRSw-2*step]

	HILL=VALE=FALSE;
	sum=0;
	for(i=0;i<step;i++) sum+=QRSdata[QRSw-step-i];
	zero=short(sum/step);
	s=maxP=minP=QRSw-2*step;
	range=w0+2*step<QRSw-4*step? w0+2*step:QRSw-4*step;
	while(s>range) {
		for(i=w0;i>0;i--) {
			if(QRSdata[s-i]>QRSdata[maxP]) maxP=s-i;
			if(QRSdata[s-i]<QRSdata[minP]) minP=s-i;
		}
		if(Hill(QRSdata,maxP,2*step,zero)) {
			HILL=TRUE;
			break;
		}
		if(Vale(QRSdata,minP,2*step,zero)) {
			VALE=TRUE;
			break;
		}
		s-=step;
	}
	if(!HILL&&!VALE) right=QRSw-2*step;
	else if(HILL) right=TurnPoint(QRSdata,maxP,QRSw-step,-1);
	else right=TurnPoint(QRSdata,minP,QRSw-step,1);
	if(right<=left) right=left+step;     //necesseray?
	Beats[QRSi].QRS.Offset=Beats[QRSi].QRS.Start+right;

	maxP=minP=left;
	for(i=left;i<right;i++) {
		if(QRSdata[i]>QRSdata[maxP]) maxP=i;
		if(QRSdata[i]<QRSdata[minP]) minP=i;
	}
	Beats[QRSi].QRS.Range=(unsigned short)((QRSdata[maxP]-QRSdata[minP])*uVperBit);  //2017 added *uVperBit
	if(QRSdata[maxP]<2*MinWave||!Hill(QRSdata,maxP,2*step,0)) Beats[QRSi].QRS.MaxP=-1;
	else Beats[QRSi].QRS.MaxP=Beats[QRSi].QRS.Start+maxP;
	if(QRSdata[minP]>-2*MinWave||!Vale(QRSdata,minP,2*step,0)) Beats[QRSi].QRS.MinP=-1;
	else Beats[QRSi].QRS.MinP=Beats[QRSi].QRS.Start+minP;
}

void MultiLead_ECG::PTUanalysis(int QRSi)
{
	int i,PTlen=Beats[QRSi+1].QRS.Onset-Beats[QRSi].QRS.Offset;
//	if(PTlen<320*SampleRate/1000) return;	//hspecg 2010.12.06
	if(PTlen<250*SampleRate/1000) return;	//320-hspecg 2010.12.06  //250-Du After Hu Y2015-7

	short *PTdata=new short[PTlen];
	for(i=0;i<PTlen;i++) PTdata[i]=sData[Beats[QRSi].QRS.Offset+i];
	Average(PTdata,PTlen,30);
	int w0=20*SampleRate/1000;

//  T-Wave Analysis	
	Tfeature Twave;
	int D1,D2;
	int minRR=550*SampleRate/1000;
	int Tlen=520*SampleRate/1000;
	D1=Beats[QRSi+1].QRS.Pos-Beats[QRSi].QRS.Pos;
	int Tlen0;
	if(D1<450*SampleRate/1000) Tlen0=4*PTlen/5;  //Tlen0<PTlen
	else if(D1<650*SampleRate/1000) Tlen0=3*PTlen/4; //Tlen0<PTlen
	else if(D1<800*SampleRate/1000) Tlen0=2*PTlen/3; //Tlen0<PTlen
	else Tlen0=3*PTlen/5; //Tlen0<PTlen
	if(Tlen>Tlen0) {   //3/5=0.6 5/8=0.625 2/3=0.65 
		Tlen=Tlen0;
		if(QRSi<QRSsN-2) {
			D2=Beats[QRSi+2].QRS.Pos-Beats[QRSi+1].QRS.Pos;
			if(abs(PTdata[Tlen])>2*MinWave&&D1<minRR&&D1<2*D2/3) Tlen=PTlen-2*w0;
		}
		else if(abs(PTdata[Tlen])>2*MinWave&&D1<minRR)
			Tlen=Tlen+4*w0<PTlen-4*w0? Tlen+4*w0:PTlen-4*w0;
	}
	Tanalysis(&PTdata[0],Tlen,&Twave);  // Tlen<PTlen
	Beats[QRSi].T.Dir=Twave.Dir;
	Beats[QRSi].T.pV=Twave.pV;
	Beats[QRSi].T.nV=Twave.nV;
	Beats[QRSi].T.Offset=Beats[QRSi].QRS.Offset+Twave.Offset;

//  P-Wave Analysis
	int Pstart=Twave.Offset;
	int Plen=PTlen-Pstart;

	Pfeature *P[3],P0,P1,P2,*Pt;
	P[0]=&P0;   P[1]=&P1;  P[2]=&P2;
	Panalysis(&PTdata[Pstart],Plen,P); 
	if(P2.Dir!=0) Pt=&P2;
	else if(P1.Dir!=0) Pt=&P1;
	else Pt=NULL;
	if(Pt!=NULL&&Beats[QRSi].U.Dir!=0 && P0.pV!=0) {
		double Vr=Pt->pV/P0.pV;
		double Wr=Pt->Wide/P0.Wide;
		if((Vr<1.2&&Vr>0.8)&&(Wr<1.2&&Wr>0.8)) Beats[QRSi].U.Dir=0;
		else if(Pt->Onset<Beats[QRSi].U.Onset+80*SampleRate/1000) Pt->Dir=0;
	} 
	if(Beats[QRSi].U.Dir!=0) {
		Beats[QRSi].U.Onset+=Beats[QRSi].T.Offset;
		Beats[QRSi].U.Offset+=Beats[QRSi].T.Offset;
	}

//Deal with sub Ps; Sub_P() is nothing related to P[0].Onset  
	if(P2.Dir!=0) Sub_P(&P2,&P0);
	if(P1.Dir!=0) Sub_P(&P1,&P0);
	if(P2.Dir!=0&&P1.Dir==0) {
		P1.Dir=P2.Dir;
		P1.Onset=P2.Onset;
		P1.Wide=P2.Wide;
		P1.pV=P2.pV;
		P2.Dir=0;
	}

//Ps setting
	for(i=0;i<3;i++) {
		Beats[QRSi+1].P[i].Dir=P[i]->Dir;
		Beats[QRSi+1].P[i].Onset=P[i]->Onset+Beats[QRSi].T.Offset;
		Beats[QRSi+1].P[i].Wide=P[i]->Wide;
		Beats[QRSi+1].P[i].pV=P[i]->pV;
		Beats[QRSi+1].P[i].nV=P[i]->nV;
	}
	if(P0.Dir==0) Beats[QRSi+1].P[0].Onset=Beats[QRSi].T.Offset+3*Plen/5;

	delete []PTdata;
}

void MultiLead_ECG::Tanalysis(short *data,int len,Tfeature *Twave)
{
	Twave->Dir=Twave->nV=Twave->pV=0;
	int i,k,maxP,minP;
	
	short stdTv=short(40/uVperBit);
	int step=50*SampleRate/1000;
	int hillP,valeP;
	hillP=valeP=2*step;
	for(i=2*step;i<len-step;i++) {
		if(data[i]-data[i-step]>stdTv&&data[i]-data[i+step]>stdTv) {
			hillP=i;
			for(k=i-step/2;k<i+step/2;k++) if(data[k]>data[hillP]) hillP=k;
			break;
		}
	}
	for(i=step;i<len-step;i++) {
		if(data[i-step]-data[i]>stdTv&&data[i+step]-data[i]>stdTv) {
			valeP=i;
			for(k=i-step/2;k<i+step/2;k++) if(data[k]<data[valeP]) valeP=k;
			break;
		}
	}

	maxP=minP=step;
	for(i=step;i<len-step;i++) {
		if(data[i]>data[maxP]) maxP=i;
		if(data[i]<data[minP]) minP=i;
	}

	if(hillP!=2*step&&data[hillP]>=data[maxP]) maxP=hillP;
	if(valeP!=2*step&&data[valeP]<=data[minP]) minP=valeP;

	step=20*SampleRate/1000;

//  positive wave process
	int pl,pr;
	pl=pr=maxP;  //in [len+step,len-step]
	while(pl-step>0&&data[pl]-data[pl-step]>-MinWave/4&&data[pl]>data[step]) pl-=step;
	if(pl+step<maxP) pl+=step;
	while(pl+step<maxP&&(data[pl+step]-data[pl])<MinWave/2) pl+=step;
	if(pl-step>0) pl-=step;
	if(pl-step>0) pl-=step;
	while(pr+step<len&&data[pr]-data[pr+step]>-MinWave/4&&data[pr]>data[step]) pr+=step;
	if(pr-step>maxP) pr-=step;
	while(pr-step>maxP&&(data[pr-step]-data[pr])<MinWave/2) pr-=step;
	if(pr+step<len) pr+=step;
	if(pr+step<len) pr+=step;
	int tempP=pr-2*step>maxP+step? pr-2*step:maxP+step;
	for(i=pr;i>tempP;i--) if(data[i]<data[pr]) pr=i;
	BOOL pW=TRUE;
	if(data[maxP]-data[pl]<MinWave||data[maxP]-data[pr]<MinWave||pr-pl<2*step) pW=FALSE;
	if(pr+step<len) pr+=step;

//  negative wave process
	int nl,nr;
	nl=nr=minP;  //in [len+step,len-step]
	while(nl-step>0&&data[nl-step]-data[nl]>-MinWave/4&&data[nl]<data[step]) nl-=step;
	if(nl+step<minP) nl+=step;
	while(nl+step<minP&&(data[nl]-data[nl+step])<MinWave/2) nl+=step;
	if(nl-step>0) nl-=step;
	while(nr+step<len&&data[nr+step]-data[nr]>-MinWave/4&&data[nr]<data[step]) nr+=step;
	if(nr-step>minP) nr-=step;
	while(nr-step>minP&&(data[nr]-data[nr-step])<MinWave/2) nr-=step;
	if(nr+step<len) nr+=step;
	if(nr+step<len) nr+=step;
	tempP=nr-2*step>maxP+step? nr-2*step:maxP+step;
	for(i=nr;i>tempP;i--) if(data[i]>data[nr]) nr=i;
	BOOL nW=TRUE;
	if(data[nl]-data[minP]<MinWave||data[nr]-data[minP]<MinWave||nr-nl<2*step) nW=FALSE;
	if(nr+step<len) nr+=step;

	if(nW&&pW&&(data[maxP]-data[pr])>5*abs(data[minP]-data[nr])) nW=FALSE;
	if(nW&&pW&&abs(data[minP]-data[nr])>5*(data[maxP]-data[pr])) pW=FALSE;

	if(nW&&pW&&data[maxP]>4*stdTv&&data[minP]>-stdTv) nW=FALSE;
	if(nW&&pW&&data[minP]>-4*stdTv&&data[maxP]<stdTv) pW=FALSE;

	if(pW&&nW&&abs(maxP-minP)>150*SampleRate/1000) {
		int seW=80*SampleRate/1000;
		if(maxP<seW||maxP>len-seW) pW=FALSE;
		if(minP<seW||minP>len-seW) nW=FALSE;
	}

	if(pW&&nW&&abs(maxP-minP)>125*SampleRate/1000) {
		if(len-maxP<2*step||maxP<2*step) pW=FALSE;
		if(len-minP<2*step||minP<2*step) nW=FALSE;
		if(pW&&nW) {
			if(data[maxP]>2*abs(data[minP])) nW=FALSE;
			if(abs(data[minP])>2*data[maxP]) pW=FALSE;
		}
		if(pW&&nW) {  ////????????
			if(abs(maxP-len/2)>abs(minP-len/2)) pW=FALSE;
			else nW=FALSE;
		}
	}
	pr=(pr+step)<(len-step)? (pr+step):(len-step);
	nr=(nr+step)<(len-step)? (nr+step):(len-step);
	if(pW&&nW) {
		Twave->pV=data[maxP];
		Twave->nV=abs(data[minP]);
		if(data[maxP]>data[minP]) Twave->Dir=2;
		else Twave->Dir=-2;
		if(maxP>minP) Twave->Offset=TurnPoint(data,maxP+step/2,pr,-1);
		else Twave->Offset=TurnPoint(data,minP+step/2,nr,1);
	}
	else if(pW) {
		Twave->Dir=1; 
		Twave->pV=data[maxP];
		while(data[maxP]-data[pr-step/2]>4*stdTv&&maxP+step<pr-step/2) maxP++;
		if(data[pr]<-MinWave) pr-=step;
		Twave->Offset=TurnPoint(data,maxP+step/4,pr,-1);
//		if(data[maxP]-data[step]<MinWave) Twave->Dir=0;  //??
	}
	else if(nW) {
		Twave->Dir=-1;
		Twave->nV=abs(data[minP]);
		while(data[nr-step/2]-data[minP]>5*stdTv&&minP+step<nr-step/2) minP++;
		if(data[nr]>MinWave) nr-=step;
		Twave->Offset=TurnPoint(data,minP+step/4,nr,1);
//		if(data[step]-data[minP]<MinWave) Twave->Dir=0;  //??
	}
	else Twave->Dir=0;

	if(Twave->Dir==0) {
		minP=len/4;
		for(i=len/4;i<3*len/4;i++) if(abs(data[i])<abs(data[minP])) minP=i;
		Twave->Offset=minP-step/2;
	}
}

int MultiLead_ECG::PositiveU(short *data,int len,Ufeature *U)
{//return 0;
	int w0=40*SampleRate/1000;
	int maxP=w0;
	int Uwave=int(85/uVperBit);
	for(int i=w0;i<len;i++) if(data[i]>data[maxP]) maxP=i;
	if(data[maxP]-data[w0]<MinWave/2||data[maxP]-data[len-w0]<MinWave/2) return 0;
	if(data[maxP]-data[w0]>Uwave&&data[maxP]-data[len-w0]>Uwave) return 0;
	int l=TurnPoint(data,w0/3,maxP,-1);
	int r=TurnPoint(data,maxP+w0/3,len-1,-1);
	if(l>2*w0||r-l<4*w0||r-l>7*w0) return 0;
	if(data[maxP]-data[l]>Uwave&&data[maxP]-data[r]>Uwave) return 0;
	if((maxP-l)>(r-maxP)) return 0;
	U->Onset=l;
	U->Offset=r;
	return 1;
}

int MultiLead_ECG::NagtiveU(short *data,int len,Ufeature *U)
{//return 0;
	int w0=40*SampleRate/1000;
	int minP=w0;
	int Uwave=int(85/uVperBit);
	for(int i=w0;i<len;i++) if(data[i]<data[minP]) minP=i;
	if(data[w0]-data[minP]<MinWave/2||data[len-w0]-data[minP]<MinWave/2) return 0;
	if(data[w0]-data[minP]>Uwave&&data[len-w0]-data[minP]>Uwave) return 0;
	int l=TurnPoint(data,w0/3,minP,1);
	int r=TurnPoint(data,minP+w0/3,len-1,1);
	if(l>w0||r-l<4*w0||r-l>7*w0) return 0;
	if(data[l]-data[minP]>Uwave&&data[r]-data[minP]>Uwave) return 0;
	if((minP-l)>(r-minP)) return 0;
	U->Onset=l;
	U->Offset=r;
	return -1;
}

void MultiLead_ECG::Panalysis(short *data,int len,Pfeature **P)
{ 
	P[0]->Dir=P[1]->Dir=P[2]->Dir=0;
	P[0]->nV=P[0]->pV=P[1]->nV=P[1]->pV=P[2]->nV=P[2]->pV=0;
	P[0]->Onset=P[1]->Onset=P[2]->Onset=0;
	P[0]->Wide=P[1]->Wide=P[2]->Wide=0;   
	if(len<100*SampleRate/1000) return;
	int seg1=70*SampleRate/1000;
	int seg2=60*SampleRate/1000;  
	int s=len-seg1-seg2-40*SampleRate/1000;  // seg1 > seg2 when s>0!!!
	if(s<0) {
		s=0;
		seg1=len/5;
		seg2=3*len/5;
	}
	int pn=0;
	int seg0=seg2;
	BOOL FirstTime=TRUE;  
	while(pn<3) { 
		if(PositiveP(&data[s],seg1,seg2,P[pn],FirstTime)) { //P found
			P[pn]->Onset+=s;  
			if(s<2*(seg1+seg2)) break;
			s-=(2*(seg1+seg2)); 
			pn++;  
		}
		else {
			if(s==0) break;
			s-=seg2;  //sge2
		}
		if(s<=0) {
			s=0;
			seg1=seg0;
		}
		else if(s<seg2) seg0=s;
		FirstTime=FALSE;
	}

	if(P[2]->Dir==0 && P[1]->Dir==0 && P[0]->Dir!=0)
		if(len-P[0]->Onset>300*SampleRate/1000 && P[0]->pV<MinWave) P[0]->Dir=0;

	Pfeature tP;
	seg1=250*SampleRate/1000;
	if(seg1>len) seg1=len;
	if(NegativeP(&data[len-seg1],seg1,&tP)) { 
		tP.Onset+=(len-seg1);
		CorrectP0(P[0],&tP);
	}

/*/ test multi_P
	if(P[0]->Dir!=0 && P[1]->Dir==0) {
		short maxVp=1;
		for(int i=2;i<P[0]->Onset-P[0]->Wide;i++) if(data[i]>data[maxVp]) maxVp=i;
		if(data[maxVp]>P[0]->pV/2 && data[maxVp]<3*P[0]->pV/2) {
			P[1]->Dir=1;
			P[1]->pV=data[maxVp];
			P[1]->Onset=maxVp-P[0]->Wide/2;
			P[1]->Wide=P[0]->Wide;
		}
	}	*/
/*	if(P[1]->Dir!=0) { 
		P[0]->Onset=P[1]->Onset;
		P[0]->Wide=P[1]->Wide;
		P[0]->pV=P[1]->pV;
		P[0]->nV=P[1]->nV;
		P[1]->Dir=0; 
	}*/
////////////////////////////////
}

BOOL MultiLead_ECG::PositiveP(short *data,int seg1,int seg2,Pfeature *P,BOOL FirstTime)
{
	int w0=10*SampleRate/1000;
	int maxP=w0;
	for(int k=w0;k<seg1+seg2;k++) if(data[k]>data[maxP]) maxP=k;
	if(maxP<seg1&&seg1>seg2) return FALSE; //at the last time seg1<=seg2
	int St=seg1>seg2? maxP-seg1:w0;   //0 changed to w0 M01_037 
	int Ed=maxP+seg1;
	if(FirstTime) if(Ed>seg1+seg2+3*w0) Ed=seg1+seg2+3*w0;

	if(data[maxP]-data[St]<3*MinWave/5||data[maxP]-data[Ed]<3*MinWave/5) return FALSE;
	if(data[maxP-w0/2]-data[St]<MinWave/2||data[maxP-w0/2]-data[Ed]<MinWave/2) return FALSE;
	if(data[maxP+w0/2]-data[St]<MinWave/2||data[maxP+w0/2]-data[Ed]<MinWave/2) return FALSE;

	int LmaxP=maxP;
	int RmaxP=maxP;
	if(data[maxP]-data[maxP-3*w0]<MinWave/2||data[maxP]-data[maxP+3*w0]<MinWave/2) {
		if(data[maxP-3*w0]-data[St]<MinWave/2&&data[maxP+3*w0]-data[Ed]<MinWave/2) return FALSE;
		if(data[maxP]-data[maxP-3*w0]<MinWave/2&&data[maxP-3*w0]-data[St]>MinWave/2)
			LmaxP=maxP-3*w0;
		if(data[maxP]-data[maxP+3*w0]<MinWave/2&&data[maxP+3*w0]-data[Ed]>MinWave/2)
			RmaxP=maxP+3*w0;
	}
	int l=LmaxP;
	int r=RmaxP;

	while(l>St+2*w0&&data[l]>MinWave&&data[l]>data[maxP]/2) l--;
	while(r<Ed-2*w0&&data[r]>MinWave&&data[r]>data[maxP]/2) r++;
	int On=TurnPoint(data,St,l,-1);
	int Off=TurnPoint(data,r,Ed,-1);
//	P->pV=abs(data[maxP]-(data[Off]+data[On])/2);
	P->pV=data[maxP]-data[On]<data[maxP]-data[Off]?	data[maxP]-data[On]:data[maxP]-data[Off];
	if(P->pV<MinWave/2||Off-On<2*w0) return FALSE;
	P->Onset=On;
	P->Wide=(Off-On)*1000/SampleRate;
	P->nV=0;
	P->Dir=1;
	return TRUE;
}

BOOL MultiLead_ECG::NegativeP(short *data,int seg,Pfeature *P)
{
	int w0=10*SampleRate/1000;
	int dW=65*SampleRate/1000;
	int minP=0;
	for(int k=1;k<seg;k++) if(data[k]<data[minP]) minP=k;
	if(minP<3*w0||minP>seg-3*w0) return FALSE;
	int St=minP-dW>0? minP-dW:0;
	int Ed=minP+dW<seg-1? minP+dW:seg-1;

	if(data[St]-data[minP]<3*MinWave/5||data[Ed]-data[minP]<3*MinWave/5) return FALSE;
	if(data[St]-data[minP-w0/2]<MinWave/2||data[Ed]-data[minP-w0/2]<MinWave/2) return FALSE;
	if(data[St]-data[minP+w0/2]<MinWave/2||data[Ed]-data[minP+w0/2]<MinWave/2) return FALSE;
	if(data[minP-3*w0]-data[minP]<MinWave/2||data[minP+3*w0]-data[minP]<MinWave/2) return FALSE;

	int l,r;
	l=r=minP;
	while(l>St+2*w0&&data[l]<-MinWave&&data[l]<data[minP]/2) l--;
	while(r<Ed-2*w0&&data[r]<-MinWave&&data[r]<data[minP]/2) r++;
	int On=TurnPoint(data,St,l,1);
	int Off=TurnPoint(data,r,Ed,1);
//	P->nV=abs(data[minP]-(data[Off]+data[On])/2);
	P->nV=data[On]-data[minP]<data[Off]-data[minP]? data[On]-data[minP]:data[Off]-data[minP];
	if(P->nV<MinWave/2||Off-On<2*w0) return FALSE;
	P->Onset=On;
	P->Wide=(Off-On)*1000/SampleRate;
	P->Dir=-1;
	return TRUE;
}

void MultiLead_ECG::CorrectP0(Pfeature *P0,Pfeature *tP)
{
	if(P0->Dir!=0) {
		int W=0;
		W=abs(P0->Onset-tP->Onset)*1000/SampleRate;
		if(W>2*P0->Wide||W>2*tP->Wide) return;
	}
	if(P0->Dir==0) {
		P0->Dir=-1;//tP->Dir;
		P0->Onset=tP->Onset;
		P0->Wide=tP->Wide;
		P0->nV=tP->nV;
		P0->pV=0;
	}
	else {  //P0->Dir==1
		if(P0->pV>5*tP->nV&&tP->nV<MinWave) return;
		if(P0->pV>tP->nV&&tP->nV>2*MinWave/3) {//Dir=2
			P0->Dir=2;
			P0->nV=tP->nV;
			if(tP->Onset<P0->Onset) P0->Onset=tP->Onset;
			P0->Wide+=(tP->Wide/3);
		}
		else {
			if(P0->pV<2*MinWave/3) {
				P0->Dir=tP->Dir;
				P0->Onset=tP->Onset;
				P0->Wide=tP->Wide;
				P0->nV=tP->nV;
				P0->pV=0;
			}
			else {  //Dir=-2
				P0->Dir=-2;
				P0->nV=tP->nV;
				if(tP->Onset<P0->Onset) P0->Onset=tP->Onset;
				P0->Wide+=(tP->Wide/3);
			}
		}
	}
}

void MultiLead_ECG::Sub_P(Pfeature *subP,Pfeature *stdP)
{
	if(subP->Dir!=stdP->Dir) subP->Dir=0;
	else {
		int stdV,subV;
		if(stdP->Dir>0) {
			stdV=stdP->pV;
			subV=subP->pV;
		}
		else {
			stdV=stdP->nV;
			subV=subP->nV;
		}
		if(2*subV<stdV||subP->Wide<2*stdP->Wide/3||stdP->Wide<2*subP->Wide/3) subP->Dir=0;
		if(2*subV<stdV||(subV<2*MinWave&&3*subV<2*stdV)) subP->Dir=0;
	}
}

BOOL MultiLead_ECG::Hill(short *data,long p,short w,short Zero)
{
    if(data[p]-Zero>2*MinWave) return TRUE;
	short zV=data[p-w]>data[p+w-1]? data[p-w]:data[p+w-1];
	if(data[p]-zV>MinWave) return TRUE;
    return FALSE;
}

BOOL MultiLead_ECG::Vale(short *data,long p,short w,short Zero)
{
    if(Zero-data[p]>2*MinWave) return TRUE;
	short zV=data[p-w]<data[p+w-1]? data[p-w]:data[p+w-1];
	if(zV-data[p]>MinWave) return TRUE;
    return FALSE;
}

void MultiLead_ECG::AntiDrift(short *data)
{   //QRSsM>=4
	int step=32*SampleRate/1000;  //12
	int w=6*SampleRate/1000;
	long p0=0;
	long p1=Beats[0].QRS.Start+step;  //exist
	long p2=Beats[1].QRS.Start+step;  //exist

	double y,y0,y1,y2;
	y1=y2=0;
	for(int s=-w;s<w;s++) {
		y1+=data[p1+s];
		y2+=data[p2+s];
	}
	y1/=(2*w);
	y2/=(2*w);
	y=y0=y1;

	double yd1,yd2,yd3,T1,T2;  //y', y", y'"
	T1=p1-p0;	T2=p2-p0; 
	yd1=(y2-y0)/T2;   
	yd3=12*(y0-y1)/(T1*T1*T1)+6*yd1/(T1*T1);
	yd2=-6*(y0-y1)/(T1*T1)-2*yd1/T1;

	int sn=0; //QRS count
	for(long i=0;i<Length;i++) {
		data[i]-=short(y);
		y+=(yd1+yd2/2+yd3/6);
		yd1+=(yd2+yd3/2);
		yd2+=yd3;
		if(i==Beats[sn].QRS.Start) {
			p0=p1;  p1=p2; 
			y0=y1;  y1=y2; 
			sn++;
			if(sn<QRSsN-1) {
				p2=Beats[sn+1].QRS.Start+step;
				y2=0;
				for(int s=-w;s<w;s++) y2+=data[p2+s];
				y2/=(2*w);
			}
			else p2=Length-1; //sn==QRSsN-1
			T1=p1-p0; 	T2=p2-p0;
			yd3=12*(y0-y1)/(T1*T1*T1)+6*(yd1+(y2-y0)/T2)/(T1*T1);
			yd2=-6*(y0-y1)/(T1*T1)-2*(2*yd1+(y2-y0)/T2)/T1;
		}
	}
}

void MultiLead_ECG::AntiDrift2(short *data)
{   //QRSsM>=4
	int step=32*SampleRate/1000;  //12
	int w=6*SampleRate/1000;
	long p0=0;
	long p1=Beats[0].QRS.Start+step;  //exist

	long p2=Beats[1].QRS.Start+step;  //exist
	if(Beats[1].P->Dir!=0 && Beats[1].QRS.Onset!=0) {
		long pOff=Beats[1].P->Onset+long(Beats[1].P->Wide)*SampleRate/1000;
		p2=pOff+(Beats[1].QRS.Onset-pOff)/2;
	}

	double y,y0,y1,y2;
	y1=y2=0;
	for(int s=-w;s<w;s++) {
		y1+=data[p1+s];
		y2+=data[p2+s];
	}
	y1/=(2*w);
	y2/=(2*w);
	y=y0=y1;

	double yd1,yd2,yd3,T1,T2;  //y', y", y'"
	T1=p1-p0;	T2=p2-p0; 
	yd1=(y2-y0)/T2;   
	yd3=12*(y0-y1)/(T1*T1*T1)+6*yd1/(T1*T1);
	yd2=-6*(y0-y1)/(T1*T1)-2*yd1/T1;

	int sn=0; //QRS count
	for(long i=0;i<Length;i++) {
		data[i]-=short(y);
		y+=(yd1+yd2/2+yd3/6);
		yd1+=(yd2+yd3/2);
		yd2+=yd3;
		if(i==Beats[sn].QRS.Start) {
			p0=p1;  p1=p2; 
			y0=y1;  y1=y2; 
			sn++;
			if(sn<QRSsN-1) {
				p2=Beats[sn+1].QRS.Start+step;
				if(Beats[sn+1].P->Dir!=0 && Beats[sn+1].QRS.Onset!=0) {
					long pOff=Beats[sn+1].P->Onset+long(Beats[sn+1].P->Wide)*SampleRate/1000;
					p2=pOff+(Beats[sn+1].QRS.Onset-pOff)/2;
				}
				y2=0;
				for(int s=-w;s<w;s++) y2+=data[p2+s];
				y2/=(2*w);
			}
			else p2=Length-1; //sn==QRSsN-1
			T1=p1-p0; 	T2=p2-p0;
			yd3=12*(y0-y1)/(T1*T1*T1)+6*(yd1+(y2-y0)/T2)/(T1*T1);
			yd2=-6*(y0-y1)/(T1*T1)-2*(2*yd1+(y2-y0)/T2)/T1;
		}
	}
}

long MultiLead_ECG::TurnPoint(short *data,long l,long r,int sign)
//sign=+1: 求凸点, sign=-1: 求凹点
{
    long i,turnP;
    double slop,s,dif,maxV;
	long start=l;
	long end=r;
	int margin=8*SampleRate/1000;
	while(end-start>2*margin) {
	    maxV=0.;
		turnP=start;
		slop= double((data[end]-data[start]))/double(end-start); //求直线斜率
		for(i=start;i<end;i++)      
		{
			s=data[start]+(i-start)*slop; //直线方程
			dif=(sign>0)? (data[i]-s):(s-data[i]); //信号与直线(+)差值     
			if(dif>maxV)              //最大差值点为所提取特征点位置
			{
				maxV=dif;
				turnP=i;
			}
		}
		if(turnP-start>margin&&end-turnP>margin) break;
		if(turnP-start<=margin) start=turnP+1;
		if(end-turnP<=margin) end=turnP-1;
	}	
	if(end-start<=2*margin) turnP=(start+end)/2;
	return turnP;
}  //end of short point int	

long MultiLead_ECG::TurnPoint(long *data,long l,long r,int sign)   //Y2015/5/28 int form
//sign=+1: 求凸点, sign=-1: 求凹点
{
    long i,turnP;
    double slop,s,dif,maxV;
	long start=l;
	long end=r;
	int margin=8*SampleRate/1000;
	while(end-start>2*margin) {
	    maxV=0.;
		turnP=start;
		slop= double((data[end]-data[start]))/double(end-start); //求直线斜率
		for(i=start;i<end;i++)      
		{
			s=data[start]+(i-start)*slop; //直线方程
			dif=(sign>0)? (data[i]-s):(s-data[i]); //信号与直线(+)差值     
			if(dif>maxV)              //最大差值点为所提取特征点位置
			{
				maxV=dif;
				turnP=i;
			}
		}
		if(turnP-start>margin&&end-turnP>margin) break;
		if(turnP-start<=margin) start=turnP+1;
		if(end-turnP<=margin) end=turnP-1;
	}	
	if(end-start<=2*margin) turnP=(start+end)/2;
	return turnP;
}  //end of short point int	

void MultiLead_ECG::Average(short *data,long len,short ms)
{
	int w=(ms/2)*SampleRate/1000;	if(w<1) return;
	int i,j,v;
	short *in;
	in=new short[len+1];
	for(i=0;i<len;i++) in[i]=data[i];
	for(i=1;i<w;i++) {
		v=0;
		for(j=0;j<2*i;j++) v+=in[j];
		data[i]=v/(2*i);
		v=0;
		for(j=0;j<2*i;j++) {
//			if(len-1-j<0) 
//				AfxMessageBox("len-1-j<0");
			v+=in[len-1-j];
		}
		data[len-1-i]=v/(2*i);
	}
	for(i=w;i<len-w;i++) {
		v=0;
		for(j=-w;j<w;j++) v+=in[i+j];
		data[i]=v/(2*w);
	}
	delete []in;
}

void MultiLead_ECG::Average(long *data,long len,short ms)   //Y2015/5/28
{
	int w=(ms/2)*SampleRate/1000;	if(w<1) return;
	int i,j,v;
	long *in;
	in=new long[len+1];
	for(i=0;i<len;i++) in[i]=data[i];
	for(i=1;i<w;i++) {
		v=0;
		for(j=0;j<2*i;j++) v+=in[j];
		data[i]=v/(2*i);
		v=0;
		for(j=0;j<2*i;j++) {
//			if(len-1-j<0) 
//				AfxMessageBox("len-1-j<0");
			v+=in[len-1-j];
		}
		data[len-1-i]=v/(2*i);
	}
	for(i=w;i<len-w;i++) {
		v=0;
		for(j=-w;j<w;j++) v+=in[i+j];
		data[i]=v/(2*w);
	}
	delete []in;
}

BOOL MultiLead_ECG::P_QRSrelations()
{
	int CountM,Count1,Count2,CountP,CountN;
	CountM=Count1=Count2=CountP=CountN=0;
	for(int i=1;i<QRSsN-1;i++) 	{
//		if(Beats[i].P[0].Dir!=0 && Beats[i].P[1].Dir!=0) CountM++;
		if(abs(Beats[i].P[0].Dir)==1) Count1++;
		if(abs(Beats[i].P[0].Dir)==2) Count2++;
		if(Beats[i].P[0].Dir>0) CountP++;
		if(Beats[i].P[0].Dir<0) CountN++;
	}
	int SM=QRSsN/4;
	if(Count1>SM && Count2>SM && CountP>SM && CountN>SM) return FALSE;

	int count=0;
	for(int i=1;i<QRSsN-1;i++) 	if(OutPut.Beats[i].Pnum==1) count++;
	if(count>4 && count>QRSsN/2) {
		int *tPR=new int[count];
		int k=0;
		for(int i=1;i<QRSsN-1;i++) if(OutPut.Beats[i].Pnum==1) tPR[k++]=OutPut.Beats[i].PR;
		BubbleSort(tPR,count);
		BOOL rPR=FALSE;
		if(4*(tPR[count-1]-tPR[0])<tPR[0] && 8*(tPR[2*count/3]-tPR[count/3])<tPR[0] && 
			6*(tPR[count-1]-tPR[count/2])<tPR[0] && 5*(tPR[count/2]-tPR[0])<tPR[0]) rPR=TRUE;
		delete []tPR;
		if(rPR) return TRUE;
/*		if(rPR) {
			count=0;
			for(int i=1;i<QRSsN-1;i++) {
				int maxP,maxT,maxTP;
				maxP=maxT=maxTP=0;
				if(Beats[i].P[0].Dir!=0 && Beats[i-1].T.Dir!=0) {
					for(int k=Beats[i-1].QRS.Offset;k<Beats[i-1].T.Offset;k++) if(sData[k]>maxT) maxT=sData[k];
					for(int k=Beats[i-1].T.Offset;k<Beats[i].P[0].Onset;k++) if(sData[k]>maxTP) maxTP=sData[k];
					for(int k=Beats[i].P[0].Onset;k<Beats[i].QRS.Onset;k++) if(sData[k]>maxP) maxP=sData[k];
				}
				if(maxT<3*maxP && maxT>maxP/2 && maxTP<3*maxP && maxTP>maxP/2) count++;
			}
			if(2*count>QRSsN) return TRUE;
		} */
	}

	int PairsN,OkN,vhN,vales,hills;
	PairsN=OkN=vhN=0;
	int step=50*SampleRate/1000;
	int start,end;
	for(int i=1;i<QRSsN-1;i++) {
		short Status=OutPut.Beats[i].Status;
		if(Status==OK||Status==P_Dir||Status==T_Dir||
			(i>1&&Status==RR_Int&&OutPut.Beats[i-1].Status!=RR_Int)) {
			OkN++;
			if(OutPut.Beats[i].Pnum==1) {
				vales=hills=0;
				if(Beats[i-1].T.Dir!=0) start=Beats[i-1].T.Offset;
				else start=Beats[i-1].QRS.End;
				if(Beats[i].P[0].Dir!=0) end=Beats[i].P[0].Onset;
				else end=Beats[i].QRS.Start;
//				end=Beats[i].QRS.Start;
				if(end-start>3*step) {
					int stdV=2*MinWave;
					for(int k=start+step/2;k<end-step/2;k++) {
						if(sData[k]-sData[k-step]>stdV&&sData[k]-sData[k+step]>stdV) hills++;
						if(sData[k-step]-sData[k]>stdV&&sData[k+step]-sData[k]>stdV) vales++;
					} 
				} 
				if(vales==0&&hills==0) PairsN++;
				vhN+=(vales+hills);
			}
		}
	}
	if(vhN>int(1.5*OkN)) return FALSE;
	if(int(1.2*PairsN)>OkN) return TRUE;
	return FALSE;
}

short MultiLead_ECG::RR_relations()
{
	int *RRsort=new int[QRSsN-1];
	for(int i=0;i<QRSsN-1;i++) RRsort[i]=1000*(Beats[i+1].QRS.Pos-Beats[i].QRS.Pos)/SampleRate;  //ms
	BubbleSort(RRsort,QRSsN-1);
	int AvgRR=RRsort[0];
	int MaxDiff=RRsort[1]-RRsort[0];
	int DiffPos=1;
	for(int i=1;i<QRSsN-1;i++) {
		AvgRR+=RRsort[i];
		if(RRsort[i]-RRsort[i-1]>MaxDiff) {
			MaxDiff=RRsort[i]-RRsort[i-1];
			DiffPos=i;
		}
	}
	AvgRR/=(QRSsN-1);

	short RRrelation=0;

	short Step=short(0.05*QRSsN);
	if(OutPut.Vrate>200 && QRSsN>30) {
		if(20*(RRsort[QRSsN-1-Step]-RRsort[Step])<RRsort[QRSsN/2] && 
			40*(RRsort[QRSsN/2]-RRsort[Step])<RRsort[QRSsN/2] && 40*(RRsort[QRSsN-1-Step]-RRsort[QRSsN/2])<RRsort[QRSsN/2]) {
//			short Tcount=0;
//			for(int i=0;i<QRSsN-2;i++) if(Beats[i].T.Dir!=0) Tcount++;
//			if(Tcount>0.4*(QRSsN-2)) 
			RRrelation=2;    //strict relation
		}
	}
	else if(OutPut.Vrate>150 && QRSsN>24) {
		if(40*(RRsort[QRSsN-1-Step]-RRsort[Step])<RRsort[QRSsN/2] && 
			80*(RRsort[QRSsN/2]-RRsort[Step])<RRsort[QRSsN/2] && 80*(RRsort[QRSsN-1-Step]-RRsort[QRSsN/2])<RRsort[QRSsN/2]) {
//			short Tcount=0;
//			for(int i=0;i<QRSsN-2;i++) if(Beats[i].T.Dir!=0) Tcount++;
//			if(Tcount>0.4*(QRSsN-2)) 
			RRrelation=2;    //strict relation
		}
	} 

	if(RRsort[0]>1200 && 8*(RRsort[QRSsN-2]-RRsort[0])<RRsort[0] && MaxDiff<70) RRrelation=2;
	
	if(RRrelation!=2) {
		int rate=int(4000./double(AvgRR))+7;   //HR=60: rate=11, HR=120: rate=15
		if(rate*(AvgRR-RRsort[0])<RRsort[0] && rate*(RRsort[QRSsN-2]-AvgRR)<RRsort[0] &&
			2*rate*MaxDiff<RRsort[0]) RRrelation=1;     //   //心率齐
		double t=(0.2/500)*AvgRR+2.5;    //
		if(t*(RRsort[DiffPos-1]-RRsort[0])<(double)MaxDiff && t*(RRsort[QRSsN-2]-RRsort[DiffPos])<(double)MaxDiff)
			RRrelation=1;                   //two groups  库1:054 两组，最好用Beats P解决
	}
	delete []RRsort;

	return RRrelation;
}

int MultiLead_ECG::AllBeatsP()    //Y2015-6-30  return 1: 80% good P, -1: 50% bad P, 0: others
{
	int AllP,pC,nC;
	AllP=pC=nC=0;
	for(int i=1;i<QRSsN;i++) {
		if(Beats[i].P[0].Dir!=0) {
			AllP++;
			if(Beats[i].P[0].pV>=Beats[i].P[0].nV) pC++;
			else nC++;
		}
	}

	int tempP=0;
	int LowV=int(45/uVperBit);
	int AverageV=0;
	if(pC>=nC) {
		for(int i=1;i<QRSsN;i++) {
			if(Beats[i].P[0].Dir!=0) {
				if(Beats[i].P[0].pV>0) {
					tempP++;
					AverageV+=Beats[i].P[0].pV;
				}
			}
		}
	}
	else {
		for(int i=1;i<QRSsN;i++) {
			if(Beats[i].P[0].Dir!=0) {
				if(Beats[i].P[0].nV>0) {
					tempP++;
					AverageV+=Beats[i].P[0].nV;
				}
			}
		}
	}
	if(tempP>0) AverageV/=tempP;

	int GoodP,BadP,PerfectP;
	GoodP=BadP=PerfectP=0;
	if(pC>=nC) {
		for(int i=1;i<QRSsN;i++) {
			if(Beats[i].P[0].Dir!=0 && Beats[i].P[0].Wide>40 && 
				(Beats[i].P[0].pV>LowV || Beats[i].P[0].pV>3*AverageV/5)) GoodP++;
			if(Beats[i].P[0].Dir==0 || (Beats[i].P[0].Dir!=0 && 
				(Beats[i].P[0].pV<3*LowV/5 || Beats[i].P[0].pV<AverageV/3))) BadP++;
			if(Beats[i].P[0].Dir!=0 && Beats[i].P[0].Wide>50 && 
				(Beats[i].P[0].pV>LowV && Beats[i].P[0].pV>2*AverageV/3)) PerfectP++;
		}
	}
	else {
		for(int i=1;i<QRSsN;i++) {
			if(Beats[i].P[0].Dir!=0 && Beats[i].P[0].Wide>40 && 
				(Beats[i].P[0].nV>LowV || Beats[i].P[0].nV>3*AverageV/5)) GoodP++;
			if(Beats[i].P[0].Dir==0 || (Beats[i].P[0].Dir!=0 && 
				(Beats[i].P[0].nV<3*LowV/5 || Beats[i].P[0].nV<AverageV/3))) BadP++;
			if(Beats[i].P[0].Dir!=0 && Beats[i].P[0].Wide>50 && 
				(Beats[i].P[0].nV>LowV && Beats[i].P[0].nV>2*AverageV/3)) PerfectP++;
		}
	}

	if(3*PerfectP>2*AllP && 2*PerfectP>QRSsN-1) return 2;
	if(3*AllP>QRSsN-1)
		if(GoodP>0.8*AllP || 2*GoodP>QRSsN-1 || (3*PerfectP>AllP && 4*PerfectP>QRSsN-1)) return 1;
	if((1.7*BadP>AllP && 2*GoodP<AllP) || (2*BadP>QRSsN-1 && 3*GoodP<QRSsN-1)) return -1;  // 2->1.7 for M02 002  
	return 0;
}

short MultiLead_ECG::F_Analysis()
{ 
	if(OutPut.Vrate>140) return 0;	//hspecg 不对快速心室率检测房颤 2016-07-01
	int ch=(ChN<=8)?2:6;		//hspecg 12导数据修正 2016-07-01

	BOOL Flutter=FlutterDetect();

	short R_Rr=RR_relations();
	BOOL Fibration=FibDecide() && R_Rr==0;

    if(R_Rr==2) return 0;  
	int AllPstatus=AllBeatsP();
	BOOL P_QRSr=P_QRSrelations();
	if(Flutter) return 1;  //2016-7
	if(Fibration && !P_QRSr && AllPstatus!=2) return 2;
	switch(AllPstatus) {
		case 2:	  return 0;
		case 1:   return 0;  // all with good Ps 
		case -1:  return 2;  // too many bad Ps
		case 0:   if(!P_QRSr && R_Rr==0) return -2;  // not make sure 
	}  
	return 0;
}

BOOL MultiLead_ECG::FlutterDetect()  //rewrite 2016-10-27
{
	int i, ch=(ChN<=8)? 2:6; 
	short *fData=new short[Length];  

	double LF=15;
	LowpassFilter LowPass=LowpassFilter(LF,SampleRate);        
	for(i=0;i<Length;i++) fData[i]=LowPass.Filter(DataOut[ch][i]);
	LowPass.Init(LF,SampleRate);
	for(i=Length-1;i>=0;i--) fData[i]=LowPass.Filter(fData[i]);

	double HF=1;
	HighpassFilter HighPass=HighpassFilter(HF,SampleRate);        
	for(i=0;i<Length;i++) fData[i]=HighPass.Filter(fData[i]);
	HighPass.Init(HF,SampleRate);
	for(i=Length-1;i>=0;i--) fData[i]=HighPass.Filter(fData[i]);

	bool FlutterState=FlutterWave(fData,SMALL)|| FlutterWave(fData,BIG);
	delete []fData;

	return FlutterState;
}

bool MultiLead_ECG::FlutterWave(short *fData,FLUTTER_WAVE_STAGE Stage)
{
	int i,k;
	short V0,V1,V2,V3;
	int Range=0;
	switch(Stage) {
		case SMALL:
			V0=short(7/uVperBit);
			V1=short(10/uVperBit);
			V2=short(30/uVperBit);
			V3=short(350/uVperBit);
			Range=5*SampleRate; 
			break;
		case MIDDLE:
			V0=V1=V2=V3=0;
			Range=7*SampleRate; 
			break;
		case BIG:
			V0=short(16/uVperBit);
			V1=short(24/uVperBit);
			V2=short(160/uVperBit);
			V3=short(500/uVperBit);
			Range=8*SampleRate; 
			break;
		default: return FALSE;
	}

	int W10=10*SampleRate/1000;   //10ms
	int W30=30*SampleRate/1000;
	int W100=100*SampleRate/1000;

	short *fPN=new short[Length];
	for(i=0;i<Length;i++) fPN[i]=0;

	i=0;
	while(i<Length-W10) {
		k=i;
		while(k+W10<Length && fData[k+W10]-fData[k]>V0) k++;   //10ms and 5uV
		if(k+W10<Length && k-i>W30 && k-i<W100 && fData[k+W10]>V1 && fData[k+W10]-fData[i]<V3 && fData[k+W10]-fData[i]>V2) 
				fPN[k]=1;
		if(k>i) i=k+W10-1;

		k=i;
		while(k+W10<Length && fData[k]-fData[k+W10]>V0) k++;   //10ms and 5uV
		if(k+W10<Length && k-i>W30 && k-i<W100 && fData[k+W10]<2*V1 && fData[i]-fData[k+W10]<V3 && fData[i]-fData[k+W10]>V2) 
				fPN[k]=-1;
		if(k>i) i=k+W10-1;
		i++;
	}

	int cMax=0;
	int Start=0;
	for(i=0;i<Length-Range;i++) {
		int count=0;
		for(k=0;k<Range;k++) if(fPN[i+k]!=0) count++;
		if(count>cMax) {
			cMax=count;
			Start=i;
		}
	}

	int cP,cN,cPaire;
	cP=cN=cPaire=0;
	short Statues=0;
	for(i=Start;i<Start+Range;i++) {
		if(fPN[i]==1) { 
			cP++; 
			if(Statues==-1) cPaire++;
			Statues=1; 
		}
		else if(fPN[i]==-1) {
			cN++;
			if(Statues==1) 	cPaire++;		
			Statues=-1; 
		}
	}
	delete []fPN;

//	int nPT=1;
//	for(i=0;i<QRSsN-1;i++) if(Beats[i].T.Dir>0 && Beats[i+1].P[0].Dir>0 && Beats[i+1].P[1].Dir==0) nPT++;
	int nP=1;
	for(i=1;i<QRSsN;i++) if(Beats[i].P[0].Dir>0 && Beats[i].P[1].Dir==0) nP++;

	switch(Stage) {
		case SMALL:
			if(OutPut.Vrate>93) {	
				if(cP>28 && cN>26 && cPaire>55) return TRUE;
				else return FALSE;
			}
			else {
				if(cP>21 && cN>20 && cPaire>33) return TRUE;
				else return FALSE;
			}
		case MIDDLE:
			break;
		case BIG: 
			if(OutPut.Vrate>105) {	
//				if(double(abs(OutPut.Vrate-OutPut.Arate))/OutPut.Vrate<0.05 && QRSsN-nPT<3) return FALSE;
				if(double(QRSsN-nP)/QRSsN<0.05) return FALSE;
				if(cP>15 && cN>15 && cPaire>27) return TRUE;
				else return FALSE;
			}
			else {   
				if(cP>20 && cN>20 && cPaire>40) return TRUE;
				else return FALSE;
			}
	}  //switch
	return FALSE;
}

// new 2016-8
BOOL MultiLead_ECG::FibDecide()
{
//	int fRR=fMaxRR-fMidRR>fMidRR-fMinRR? fMaxRR-fMidRR:fMidRR-fMinRR;
//	if(5*fRR<fMidRR) return FALSE;                            //库1： 050

	int i,k;

//	if(FibDetect(fData,fLength) || FibDetect(gData,fLength)) return TRUE;

//  Bigiminy and Trigiminy
	for(k=1;k<QRSsN-1;k++) if(OutPut.Beats[k].Status==QRS_W||OutPut.Beats[k].Status==SubQRS_W) break;
	if(k<4 && QRSsN>9) {
		BOOL Bigiminy=TRUE;
		for(i=k;i<QRSsN-3;i+=2) { 
			if(!((OutPut.Beats[i].Status==QRS_W||OutPut.Beats[i].Status==SubQRS_W)&&
				(OutPut.Beats[i+1].Status==OK||OutPut.Beats[i+1].Status==P_Dir))) {
					Bigiminy=FALSE;
					break;
			}
		}
		if(Bigiminy) return FALSE;
		BOOL Trigiminy=TRUE;
		for(i=k;i<QRSsN-4;i+=3) { 
			if(!((OutPut.Beats[i].Status==QRS_W||OutPut.Beats[i].Status==SubQRS_W)&&
			(OutPut.Beats[i+1].Status==OK||OutPut.Beats[i+1].Status==P_Dir)&&
			(OutPut.Beats[i+2].Status==OK||OutPut.Beats[i+2].Status==P_Dir))) {
					Trigiminy=FALSE;
					break;
			}
		}
		if(Trigiminy) return FALSE;
	}

	int PsN=0;
	for(i=1;i<QRSsN;i++) PsN+=OutPut.Beats[i].Pnum;
	if(3*PsN<QRSsN-1) {
		int abnN=0;
		for(i=1;i<QRSsN-1;i++) 
			if(OutPut.Beats[i].Status==QRS_W||OutPut.Beats[i].Status==SubQRS_W||
				OutPut.Beats[i].Status==QRS_Dir||OutPut.Beats[i].Status==SubQRS_Dir)
				abnN++;
		if(3*abnN>QRSsN-2) return FALSE; //3*   //库2：037,044,073
	}
	else {
		int maxPR=0;
		int minPR=500;
		for(i=1;i<QRSsN;i++) {
//			if(OutPut.Beats[i].Pnum>1) break;
			if(OutPut.Beats[i].Pnum==1) {
				if(OutPut.Beats[i].PR>maxPR) maxPR=OutPut.Beats[i].PR;
				if(OutPut.Beats[i].PR<minPR) minPR=OutPut.Beats[i].PR;
			}
		}
		if(maxPR-minPR<80) return FALSE;    
	}

	int BeatsCount=0;    //Y2015
	int PwaveCount=0;
	int maxPR=0;
	int minPR=SampleRate;
//	short uV50=short(50/uVperBit);
	for(i=0;i<QRSsN;i++) {  
		if(OutPut.Beats[i].Status==OK || OutPut.Beats[i].Status==P_Dir) {
			BeatsCount++;
			if(OutPut.Beats[i].Pdir!=0) {
				PwaveCount++;
				if(OutPut.Beats[i].PR>maxPR) maxPR=OutPut.Beats[i].PR;
				if(OutPut.Beats[i].PR<minPR) minPR=OutPut.Beats[i].PR;
			}
		}
	}
	if(BeatsCount<=(QRSsN-2)/2) return FALSE;   //库2：001  库1：028,050,056
	if(BeatsCount-PwaveCount<=BeatsCount/8 && maxPR-minPR<minPR/8)	return FALSE;
	return TRUE;
}

/* // The old one to be found no use of FieDetect
BOOL MultiLead_ECG::FibDecide(short *fData,short *gData,int fLength)
{
//	int fRR=fMaxRR-fMidRR>fMidRR-fMinRR? fMaxRR-fMidRR:fMidRR-fMinRR;
//	if(5*fRR<fMidRR) return FALSE;                            //库1： 050

	int i,k;

//	if(FibDetect(fData,fLength) || FibDetect(gData,fLength)) return TRUE;

//  Bigiminy and Trigiminy
	for(k=1;k<QRSsN-1;k++) if(OutPut.Beats[k].Status==QRS_W||OutPut.Beats[k].Status==SubQRS_W) break;
	if(k<4 && QRSsN>9) {
		BOOL Bigiminy=TRUE;
		for(i=k;i<QRSsN-3;i+=2) { 
			if(!((OutPut.Beats[i].Status==QRS_W||OutPut.Beats[i].Status==SubQRS_W)&&
				(OutPut.Beats[i+1].Status==OK||OutPut.Beats[i+1].Status==P_Dir))) {
					Bigiminy=FALSE;
					break;
			}
		}
		if(Bigiminy) return FALSE;
		BOOL Trigiminy=TRUE;
		for(i=k;i<QRSsN-4;i+=3) { 
			if(!((OutPut.Beats[i].Status==QRS_W||OutPut.Beats[i].Status==SubQRS_W)&&
			(OutPut.Beats[i+1].Status==OK||OutPut.Beats[i+1].Status==P_Dir)&&
			(OutPut.Beats[i+2].Status==OK||OutPut.Beats[i+2].Status==P_Dir))) {
					Trigiminy=FALSE;
					break;
			}
		}
		if(Trigiminy) return FALSE;
	}

	int PsN=0;
	for(i=1;i<QRSsN;i++) PsN+=OutPut.Beats[i].Pnum;
	if(3*PsN<QRSsN-1) {
		int abnN=0;
		for(i=1;i<QRSsN-1;i++) 
			if(OutPut.Beats[i].Status==QRS_W||OutPut.Beats[i].Status==SubQRS_W||
				OutPut.Beats[i].Status==QRS_Dir||OutPut.Beats[i].Status==SubQRS_Dir)
				abnN++;
		if(3*abnN>QRSsN-2) return FALSE; //3*   //库2：037,044,073
	}
	else {
		int maxPR=0;
		int minPR=500;
		for(i=1;i<QRSsN;i++) {
//			if(OutPut.Beats[i].Pnum>1) break;
			if(OutPut.Beats[i].Pnum==1) {
				if(OutPut.Beats[i].PR>maxPR) maxPR=OutPut.Beats[i].PR;
				if(OutPut.Beats[i].PR<minPR) minPR=OutPut.Beats[i].PR;
			}
		}
		if(maxPR-minPR<80) return FALSE;    
	}

	int BeatsCount=0;    //Y2015
	int PwaveCount=0;
	int maxPR=0;
	int minPR=SampleRate;
//	short uV50=short(50/uVperBit);
	for(i=0;i<QRSsN;i++) {  
		if(OutPut.Beats[i].Status==OK || OutPut.Beats[i].Status==P_Dir) {
			BeatsCount++;
			if(OutPut.Beats[i].Pdir!=0) {
				PwaveCount++;
				if(OutPut.Beats[i].PR>maxPR) maxPR=OutPut.Beats[i].PR;
				if(OutPut.Beats[i].PR<minPR) minPR=OutPut.Beats[i].PR;
			}
		}
	}
	if(BeatsCount<=(QRSsN-2)/2) return FALSE;   //库2：001  库1：028,050,056
	if(BeatsCount-PwaveCount<=BeatsCount/8 && maxPR-minPR<minPR/8)	return FALSE;
	return TRUE;
}
*/

/*   //found it no use 2016-8
BOOL MultiLead_ECG::FibDetect(short *fData,int fLength)
{
	int i;

	int w0=50*SampleRate/1000;
	short minV=short(40/uVperBit);
	short maxV=short(90/uVperBit);
	int k=2*w0;
	int pN=0;
	int nN=0;
	while(k<fLength-2*w0) {
		if((fData[k]-fData[k-2*w0]>minV&&fData[k]-fData[k+2*w0]>minV&&
			fData[k]-fData[k-2*w0]<maxV&&fData[k]-fData[k+2*w0]<maxV)||
			(fData[k]-fData[k-w0]>minV&&fData[k]-fData[k+2*w0]>minV&&
			fData[k]-fData[k-w0]<maxV&&fData[k]-fData[k+2*w0]<maxV)||
			(fData[k]-fData[k-2*w0]>minV&&fData[k]-fData[k+w0]>minV&&
			fData[k]-fData[k-2*w0]<maxV&&fData[k]-fData[k+w0]<maxV)) {
			int maxP=k;
			for(i=k-w0;i<k+w0;i++) if(fData[i]>fData[maxP]) maxP=i;
			int left=maxP-3*w0>0? maxP-3*w0:0;
			int right=maxP+3*w0<fLength? maxP+3*w0:fLength;
			int TminV=fData[maxP];
			for(i=left;i<right;i++) if(fData[i]<TminV) TminV=fData[i];
			if(fData[maxP]-TminV<2*maxV) {
				pN++;
				k=maxP+(w0*2);
			}
		}
		k++;
	}
	k=2*w0;
	while(k<fLength-2*w0) {
		if((fData[k-2*w0]-fData[k]>minV&&fData[k+2*w0]-fData[k]>minV&&
			fData[k-2*w0]-fData[k]<maxV&&fData[k+2*w0]-fData[k]<maxV)||
			(fData[k-w0]-fData[k]>minV&&fData[k+2*w0]-fData[k]>minV&&
			fData[k-w0]-fData[k]<maxV&&fData[k+2*w0]-fData[k]<maxV)||
			(fData[k-2*w0]-fData[k]>minV&&fData[k+w0]-fData[k]>minV&&
			fData[k-2*w0]-fData[k]<maxV&&fData[k+w0]-fData[k]<maxV)) {
			int minP=k;
			for(i=k-w0;i<k+w0;i++) if(fData[i]<fData[minP]) minP=i;
			int left=minP-3*w0>0? minP-3*w0:0;
			int right=minP+3*w0<fLength? minP+3*w0:fLength;
			int TmaxV=fData[minP];
			for(i=left;i<right;i++) if(fData[i]>TmaxV) TmaxV=fData[i];
			if(TmaxV-fData[k]<2*maxV) {
				nN++;
				k=minP+(w0*2);
			}
		}
		k++;
	}
	if(pN>int(2*QRSsN)&&nN>int(2*QRSsN)) return TRUE;
	return FALSE;
}
*/

void KinetEnergy(short *in0,short *in1,short *in2,long *out,int length,int q)   //Y2015/5/28
{
	long i,j;
	double L0,L1,L2,L;
	for(i=q;i<length-q;i++) {
		L=0;
		for(j=i-q+1;j<i+q;j++) {
			L0=1+(in0[j]-in0[j-1])*(in0[j]-in0[j-1]);
			L1=1+(in1[j]-in1[j-1])*(in1[j]-in1[j-1]);
			L2=1+(in2[j]-in2[j-1])*(in2[j]-in2[j-1]);
			L+=sqrt((L0+L1+L2)/3);
		}
//		if(L<32768) out[i]=short(L);
//		else out[i]=-1;
		out[i]=(long)L;  //Y2015-8-24
	}
	long minV=out[q];
	for(i=q;i<length-q;i++) if(out[i]<minV) minV=out[i];
	for(i=q;i<length-q;i++) out[i]-=minV;
	for(i=0;i<q+1;i++) out[i]=out[length-i-1]=0;
}

void EnergyCorrect(long *Edata,int length,int step)
{
	long Max[4];
	for(int k=0;k<4;k++) {
		Max[k]=0;
		for(int i=0;i<length/4;i++) if(Edata[k*length/4+i]>Max[k]) Max[k]=Edata[k*length/4+i];
	}
	long Max1=Max[0]<Max[2]? Max[0]:Max[2];
	long Max2=Max[1]<Max[3]? Max[1]:Max[3];
	long MidMax=Max1>Max2? Max1:Max2;
	for(int i=0;i<length;i++) {
		if(Edata[i]>5*MidMax) {
			int start=i-step>0? i-step:0;
			int end=i+step<length? i+step:length;
			for(int k=start;k<end;k++) Edata[k]=MidMax/length;
		}
	}
}

double SquareError(short *tData,int Range)
{
	double Mean=0;
	for(int i=0;i<Range;i++) Mean+=tData[i];
	Mean/=Range;
	double Se=0;
	for(int i=0;i<Range;i++) Se+=((double)tData[i]-Mean)*((double)tData[i]-Mean);
	return sqrt(Se/Range);
}

