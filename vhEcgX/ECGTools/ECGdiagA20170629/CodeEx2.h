/****************************************************************************
 File:	Code.h
 Version:	1.0
 Created:	Mar.9,2010

 Author:	HuSheping
 E-mail:	hspecg@163.com

 Function:	采用单项链表管理检测的明尼苏达码和分析结果。包含以下类：
	(1) class mCodeMgr: 管理检测的明尼苏达码
	(2) class rCodeMgr: 管理检测的分析结果

 Copyright (c) 2010 HuSheping

2014.01.01 class mCodeMgr,rCodeMgr用一个类

 PLEASE LEAVE THIS HEADER INTACT
*****************************************************************************/

#ifndef _HSP_CODEEX2_H_
#define _HSP_CODEEX2_H_

#include <string.h>

//////////////////////////////////////////////////////////////
//编码结构定义
//typedef	union {
//	struct {
//		unsigned short codeGroup	:5;
//		unsigned short codeItem		:4;
//		unsigned short codeSub		:7;
//	};
//	unsigned short nCode; //编码
//} VHCODE;

typedef struct {
	//union {
	//	struct {
	//		unsigned short codeGroup	:4;
	//		unsigned short codeItem		:4;
	//		unsigned short codeSub		:8;
	//	};
		unsigned short nCode; //编码
	//};
	//VHCODE	code;
	unsigned long  nLeads;//涉及的导联
	unsigned char  nClass;//级别
	unsigned char  nSort; //排序级别,按两位数,数小的排在前面
	unsigned short nIndex;//解释索引
	char		   szCse[16];//CSE编码
} Code;

typedef struct tagCodeList	{
	Code		code;
	struct tagCodeList	*link;
} CodeList;


//////////////////////////////////////////////////////////////
//编码单项链表管理类
class CodeMgr {
	CodeList	*m_head;
	CodeList	*m_current;
protected:
	Code	*m_pCode;//the size of it is from CodeMgr::getCount()
	short	m_nCodes;//the size of m_pCode；
public:
	CodeMgr();
	~CodeMgr();
	void insert(CodeList *p,Code code);
	void add(Code code);
	void remove(unsigned short code);
	void reset();//remove all
	CodeList *found(unsigned short code);
	int  getCount();
	CodeList *getFirst();
	CodeList *getNext();
	unsigned short replace(unsigned short nOldCode,unsigned short nNewCode,unsigned long nLeads);//替代
	short getInfo(unsigned short nCode,unsigned short &nLeads);//查询检测到的码信息

public:
	Code *GetCodes(int &n);//提取所有的码
};

#endif