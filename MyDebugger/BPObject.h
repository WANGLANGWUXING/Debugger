#pragma once
#include "DbgObject.h"
#include "Expression.h"
typedef enum
{
	e_bt_none = 0,
	breakpointType_tf,/*�����ϵ�*/
	breakpointType_soft,/*����ϵ�*/

	breakpointType_acc,/*�ڴ���ʶϵ�*/
	breakpointType_acc_e = breakpointType_acc,
	breakpointType_acc_r,
	breakpointType_acc_w,
	breakpointType_acc_rw,

	breakpointType_hard,/*Ӳ���ϵ�*/
	breakpointType_hard_e = breakpointType_hard,
	breakpointType_hard_r,
	breakpointType_hard_w,
	breakpointType_hard_rw
}E_BPType;

class BPObject
{
public:
	DbgObject& m_dbgObj;    // ���Զ���
	CStringA   m_condition; // �������ʽ
	bool       m_bOnce;     // �Ƿ���һ���Զϵ�
	uaddr      m_uAddress;  // �ϵ��ַ
public:
	BPObject(DbgObject& dbgObj);
	virtual ~BPObject();

public:
	// ��ȡ��ַ
	virtual uaddr GetAddress() const;
	// ��ȡ�������ʽ
	virtual const char* GetCondition() const;
	// ����ϵ�
	virtual bool Install() = 0;
	// �Ƴ��ϵ�
	virtual bool Remove()=0;
	// �ж϶ϵ��Ƿ�����
	virtual bool IsHit() const;
	// �ж϶ϵ��Ƿ��ɾ��
	virtual bool NeedRemove() const;
	// �жϵ�ǰ�쳣�����Ķϵ��Ƿ����Լ�
	virtual bool IsMe(const EXCEPTION_DEBUG_INFO& execption)const = 0;
	// �ж϶ϵ��Լ�������
	virtual E_BPType Type() const=0;
	// ���öϵ�����б��ʽ
	virtual void SetCondition(const char* strCondition);
	// ���ϵ�����Ϊһ����
	virtual void SetCondition(bool bOnce);

	
};

