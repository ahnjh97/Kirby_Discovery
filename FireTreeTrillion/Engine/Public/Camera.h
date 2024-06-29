#pragma once

#include "GameObject.h"

/* 모든 카메라 종류마다 필수적으로 필요한 기능을 모아둔다.  */
/* 뷰스페이스 변환 행렬을 생성하여 장치에 바인딩 해준다.(m_pTransformCom) */
/* 투영행렬을 생성하여 장치에 바인딩한다. (이제 만드,ㄹ어야지)*/

BEGIN(Engine)

class ENGINE_DLL CCamera abstract : public CGameObject
{
public:
	enum CAMTARGET { TARGET_FIRST, TARGET_SECOND, TARGET_END };
	enum CAMFOCUS { FOCUS_FIRST, FOCUS_SECOND, FOCUS_BOTH, FOCUS_END };

	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		_float4 vEye = {};
		_float4 vAt = {};
		_float	fFovy = { 0.0f };
		_float	fAspect = { 0.0f };
		_float	fNear = { 0.0f };
		_float	fFar = { 0.0f };
	}CAMERA_DESC;

protected:
	CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera(const CCamera& rhs);	
	virtual ~CCamera() = default;

public:
	//트래킹할 타겟을 세팅한다. (트랜스폼, 몇 번째 타겟 슬롯에 넣을 건지, 어디에 focus할 건지, 기준점에 얼마나 오프셋 줄 건지, 보간 속도 어떻게 할 건지)
	virtual void Set_Target(CTransform* pTarget, CAMTARGET eTarget, CAMFOCUS eFocus, _float3 vAnchorOffset = _float3{ 0.f, 0.f, 0.f }, _float fInterpolateSpeed = -1.f);


public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;

	HRESULT Bind_PipeLines();


protected:
	//카메라가 트래킹할 주요 타겟
	CTransform* m_pFirstTarget = { nullptr };
	//카메라가 트래킹할 보조 타겟
	CTransform* m_pSecondTarget = { nullptr };

	_float			m_fFovy = { 0.0f };
	_float			m_fAspect = { 0.0f };
	_float			m_fNear = { 0.0f };
	_float			m_fFar = { 0.0f };

protected:
	_float4x4		m_ProjMatrix;
	//기준 값을 만들 때 같이 사용할 오프셋(로컬 기준)
	_float3			m_vAnchorOffset = { 0.f, 0.f, 0.f };
	//카메라 보간 속도
	_float			m_fInterpolateSpeed = { 2.f };




public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};


END