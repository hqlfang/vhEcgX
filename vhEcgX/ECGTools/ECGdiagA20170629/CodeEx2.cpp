/****************************************************************************
 File:	Code.cpp
 Version:	1.0
 Created:	Mar.9,2010

 Author:	HuSheping
 E-mail:	hspecg@163.com

 Function:	采用单项链表管理检测的明尼苏达码和分析结果。包含以下类：
	(1) class mCodeMgr: 管理检测的明尼苏达码
	(2) class rCodeMgr: 管理检测的分析结果

 Copyright (c) 2010 HuSheping

 PLEASE LEAVE THIS HEADER INTACT
*****************************************************************************/
#include "stdafx.h"
#include "CodeEx2.h"

//////////////////////////////////////////////////////////////////////////////
//begin class CodeMgr

/*****************************************************************************
	构造函数
******************************************************************************/
CodeMgr::CodeMgr()
{
	m_head=0;
	m_current=0;
	m_pCode=0;
	m_nCodes=0;
}

/*****************************************************************************
	析构函数：清理内存
******************************************************************************/
CodeMgr::~CodeMgr()
{
	reset();
}

/***********************************************************************
  功能：最后结果检索
  参数：int n:	输出结果数量
  返回：Code *结果数组指针，该指针随对象的销毁而销毁
************************************************************************/
Code *CodeMgr::GetCodes(int &n)
{
	n=getCount();
	if(n<1) return 0;
	if(m_pCode) {
		if(n!=m_nCodes) {
			delete[]m_pCode;	m_pCode=0;
			m_nCodes=n;
		}
	}
	if(!m_pCode) m_pCode=new Code[n];

	int i=0;
	CodeList *r=getFirst();
	while(r) {
		m_pCode[i].nLeads=r->code.nLeads;
		m_pCode[i].nCode=r->code.nCode;
		m_pCode[i].nClass=r->code.nClass;
		m_pCode[i].nSort=r->code.nSort;
		i++;
		r=getNext();
	}
	return m_pCode;
}
/*****************************************************************************
	在指定的节点后插入一项。
	输入参数：CodeList *p: 节点指针
	          Code code:   待插入的项
	返回值：无
******************************************************************************/
void CodeMgr::insert(CodeList *p,Code code)
{
	CodeList *q=new CodeList;
	q->code=code;
	if(m_head==0) {
		m_head=q;
		q->link=0;
	}
	else {
		q->link=p->link;
		p->link=q;
	}
}

/*****************************************************************************
	在表尾添加一项。
	输入参数：Code code:   待添加的项
	返回值：无
******************************************************************************/
void CodeMgr::add(Code code)
{
	CodeList *p=m_head,*q=new CodeList;
	q->code=code;
	q->link=0;
	if(m_head==0) {			//first item
		m_head=q;
	}
	else {
		CodeList *tmp=0;
		while(p) {			//find the last item
			tmp=p;
			p=p->link;
		}
		p=tmp;				//this is the last item
		p->link=q;
	}
}

/*****************************************************************************
	删除指定分析结果码
	输入参数：short nCode: 待删除项的分析结果码
	返回值：无
******************************************************************************/
void CodeMgr::remove(unsigned short nCode)
{
	CodeList *p,*q;
	p=m_head;
	if(p->code.nCode==nCode) {
		m_head=p->link;
		delete p;p=0;
		return;
	}
	q=p;
	p=p->link;
	while((p!=0) && (p->code.nCode!=nCode)) {//find the item
		q=p;p=p->link;
	}
	if(p) {
		q->link=p->link;
		delete p;	p=0;						//free the memory
	}
}


/*****************************************************************************
	删除所有项,并清理内存
	输入参数：无
	返回值：无
******************************************************************************/
void CodeMgr::reset()//remove all
{
	CodeList *tmp,*p=m_head;
	while(p) {
		tmp=p->link;
		delete p;p=0;
		p=tmp;
	}
	m_head=m_current=0;
	if(m_pCode) delete m_pCode;	m_pCode=0;	m_nCodes=0;
}

/*****************************************************************************
	找到指定分析结果码的项
	输入参数：short nCode: 待查找项的分析结果码
	返回值：如果找到，返回节点指针，否则返回0
******************************************************************************/
CodeList *CodeMgr::found(unsigned short nCode)
{
	CodeList *p=m_head;
	while((p!=0) && (p->code.nCode!=nCode)) p=p->link;
	if(m_head) return p;
	else return 0;
}

/*****************************************************************************
	查找列表中的项数
	输入参数：无
	返回值：列表中的项数
******************************************************************************/
int CodeMgr::getCount()
{
	int count=0;
	CodeList *p=m_head;
	while(p) {
		count++;
		p=p->link;
	}
	return count;
}

/*****************************************************************************
	得到表中第一项节点指针
	输入参数：无
	返回值：表中第一项节点指针，即返回头指针(如表为空则返回0)
******************************************************************************/
CodeList *CodeMgr::getFirst()
{
	m_current=m_head;
	return m_head;
}

/*****************************************************************************
	得到表中下一项节点指针
	输入参数：无
	返回值：表中下一项节点指针(如遇最后节点，则返回0)
	注意事项：执行此成员函数前必须先执行成员函数getFirst()；
	    这两个函数用于遍历表中所有节点指针
******************************************************************************/
CodeList *CodeMgr::getNext()
{
	if(m_current) m_current=m_current->link;
	return m_current;
}
/***********************************************************************
功能：替代检测到的码
参数：
short nOldCode:			要替代的码
short nNewCode:			替代的码
unsigned short nLeads:  新的涉及那哪些导联
************************************************************************/
unsigned short CodeMgr::replace(unsigned short nOldCode,unsigned short nNewCode,unsigned long nLeads)//替代
{
	unsigned short code=nOldCode;
	CodeList *p=found(nOldCode);
	if(p) {
		if(nOldCode!=nNewCode) p->code.nCode=code=nNewCode;
		p->code.nLeads=nLeads;
	}
	return code;
}

/***********************************************************************
功能：//查询检测到的码信息
参数：
short nCode:			检测到的码
unsigned short nLeads:  需对哪些导联进行判断
输出：unsigned short nLeads:  涉及哪些导联
返回：-1:	无该码
0: 不涉及具体导联
>0: 涉及的导联数
************************************************************************/
short CodeMgr::getInfo(unsigned short nCode,unsigned short &nLeads)
{
	CodeList *m=found(nCode);
	if(!m) return -1;

	nLeads&=m->code.nLeads;
	if(!nLeads) return 0;

	short i,n=0;
	unsigned check=1;
	for(i=0;i<12;i++) {
		if(i) check<<=1;
		if(nLeads & check) n++;
	}
	return n;
}

//end class CodeMgr
//////////////////////////////////////////////////////////////////////////////

//#define _TEST_CODE_MGR_
#ifdef _TEST_CODE_MGR_
/////////////////////////////////////////////////////////////////////////////////
//以下为测试代码
#include <stdio.h>
#include <string.h>

void main()
{
//测试明尼苏达编码单项链表管理类
	printf("Test class mCodeMgr\n");
	mCode code;
	mCodeMgr mcode;
	int i,n;
	for(i=0;i<10;i++) {//测试添加功能
		code.nCode=i;
		code.nLeads=i*i;
		mcode.add(code);
	}
	n=mcode.getCount();//测试查询项数功能
	printf("%d\n",n);
	mCodeList *pm=mcode.getFirst();//测试遍历开始功能
	if(pm) {
		code.nCode=33;
		code.nLeads=34;
		mcode.insert(pm,code);//测试插入功能
	}
	mcode.remove(7);//测试删除功能
	while(pm) {//测试遍历功能
		printf("%3d,%3d\n",pm->code.nCode,pm->code.nLeads);
		pm=mcode.getNext();
	};
	mCodeList *f1=mcode.found(7),*f2=mcode.found(8);//测试查找功能
	printf("%p %p\n",f1,f2);
	if(f1) {
		printf("%3d,%3d\n",f1->code.nCode,f1->code.nLeads);
	}
	if(f2) {
		printf("%3d,%3d\n",f2->code.nCode,f2->code.nLeads);
	}
	printf("\n\n");
//测试分析结果编码单项链表管理类
	printf("Test class rCodeMgr\n");
	rCodeEx codeex;
	rCodeMgr rcode;
	for(i=0;i<10;i++) {//测试添加功能
		codeex.code.nCode=i;
		codeex.code.nClass=i%6;
		strcpy(codeex.code.szEng,"English");
		strcpy(codeex.code.szChn,"中文");
		codeex.nLeads=i*i;
		rcode.add(codeex);
	}
	n=rcode.getCount();//测试查询项数功能
	printf("%d\n",n);
	rCodeList *pr=rcode.getFirst();//测试遍历开始功能
	if(pr) {
		codeex.code.nCode=33;
		codeex.code.nClass=6;
		strcpy(codeex.code.szEng,"EnglishEnglish");
		strcpy(codeex.code.szChn,"中文中文");
		codeex.nLeads=34;
		rcode.insert(pr,codeex);//测试插入功能
	}
	rcode.remove(7);//测试删除功能
	while(pr) {//测试遍历功能
		printf("%3d,%3d,%3d,%s,%s\n",pr->code.code.nCode,pr->code.code.nClass,pr->code.nLeads,pr->code.code.szEng,pr->code.code.szChn);
		pr=rcode.getNext();
	};
	rCodeList *fr1=rcode.found(7),*fr2=rcode.found(8);//测试查找功能
	printf("%p %p\n",fr1,fr2);
	if(fr1) {
		printf("%3d,%3d,%3d,%s,%s\n",fr1->code.code.nCode,fr1->code.code.nClass,fr1->code.nLeads,fr1->code.code.szEng,fr1->code.code.szChn);
	}
	if(fr2) {
		printf("%3d,%3d,%3d,%s,%s\n",fr2->code.code.nCode,fr2->code.code.nClass,fr2->code.nLeads,fr2->code.code.szEng,fr2->code.code.szChn);
	}
}
#endif
