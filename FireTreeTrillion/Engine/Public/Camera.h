#pragma once

#include "GameObject.h"

/* 모든 카메라 종류마다 필수적으로 필요한 기능을 모아둔다.  */
/* 뷰스페이스 변환 행렬을 생성하여 장치에 바인딩 해준다.(m_pTransformCom) */
/* 투영행렬을 생성하여 장치에 바인딩한다. (이제 만드,ㄹ어야지)*/

BEGIN(Engine)

class ENGINE_DLL CCamera abstract : public CGameObject
{
public:
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
	//트래킹할 타겟을 세팅한다. (둘째 인수: 첫 번째 타겟? 두 번째 타겟?)
	virtual void Set_Target(CTransform* pTarget, CAMFOCUS eFocus = FOCUS_FIRST)
	{
		if (nullptr == pTarget)
			return;

		if (eFocus == FOCUS_FIRST)
		{
			if (nullptr != m_pFirstTarget)
				Safe_Release(m_pFirstTarget);

			m_pFirstTarget = pTarget;
			Safe_AddRef(pTarget);
			
		}
		else if (eFocus == FOCUS_SECOND)
		{
			if (nullptr != m_pSecondTarget)
				Safe_Release(m_pSecondTarget);

			m_pSecondTarget = pTarget;
			Safe_AddRef(pTarget);
		}
	}


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

protected:



public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};


END