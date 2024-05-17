#pragma once

#include "Transform.h"

BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{
public:
	typedef struct : public CTransform::TRANSFORM_DESC
	{		
	}GAMEOBJECT_DESC;

protected:
	CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject(const CGameObject& rhs);
	virtual ~CGameObject() = default;

public:
	class CComponent* Get_Component(const wstring& strComTag);
	_float Get_ViewZ() const { return m_fViewZ; }
	void Set_Dead() { m_bDead = true; }
	_bool Get_Dead() { return m_bDead; }


public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual _int Tick(_float fTimeDelta);
	virtual void Late_Tick(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_LightDepth() { return S_OK; }

protected:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };

	class CGameInstance*				m_pGameInstance = { nullptr };
	CTransform*							m_pTransformCom = { nullptr };

	// 현재 레벨을 알 수 있는 포인터.
	_uint*								m_pCurrentLevelID = { nullptr };

protected:
	map<const wstring, class CComponent*>		m_Components;

protected:
	HRESULT Add_Component(_uint iLevelIndex, const wstring& strPrototypeTag, const wstring& strComponentTag, class CComponent** ppOut, void* pArg = nullptr);
	HRESULT Compute_ViewZ();

	_float	m_fViewZ = { 0.f };
	_float3 m_vViewPos = _float3{ 0.f, 0.f, 0.f };
	_bool	m_bDead = { false };

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END