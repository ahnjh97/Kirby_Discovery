#pragma once

#include "Transform.h"

BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{
public:
	typedef struct : public CTransform::TRANSFORM_DESC
	{		
		_float4x4 matWorld = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		wstring wstrModelName;
	}GAMEOBJECT_DESC;

protected:
	CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject(const CGameObject& rhs);
	virtual ~CGameObject() = default;

public:
	class CComponent* Get_Component(const wstring& strComTag);
	CTransform* Get_TransformCom() const { return m_pTransformCom; }
	_float Get_ViewZ() const { return m_fViewZ; }
	void Set_Dead() { m_bDead = true; }
	_bool Get_Dead() { return m_bDead; }

	COLLISION_TYPE	Get_CollisionGroup() const { return m_eCollisionGroup; }

	// prototypeTag
	const wstring&		Get_PrototypeTag() { return m_wstrPrototypeTag; }
	void				Set_PrototypeTag(wstring _wstrProtoTag) { m_wstrPrototypeTag = _wstrProtoTag; }

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual _int Tick(_float fTimeDelta);
	virtual void Late_Tick(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_LightDepth() { return S_OK; }
	virtual void	Render_IMGUI();

protected:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };

	class CGameInstance*				m_pGameInstance = { nullptr };
	CTransform*							m_pTransformCom = { nullptr };

	// 현재 레벨을 알 수 있는 포인터.
	_uint*								m_pCurrentLevelID = { nullptr };
	// prototypeName을 들고 있는다.
	wstring								m_wstrPrototypeTag = wstring();
	COLLISION_TYPE						m_eCollisionGroup = COLLISION_TYPE::COLLI_END;

protected:
	map<const wstring, class CComponent*>		m_Components;

protected:
	HRESULT Add_Component(_uint iLevelIndex, const wstring& strPrototypeTag, const wstring& strComponentTag, class CComponent** ppOut, void* pArg = nullptr);
	HRESULT Add_Component(const wstring& strPrototypeTag, const wstring& strComponentTag, class CComponent** ppOut, void* pArg = nullptr);
	HRESULT Compute_ViewZ();

	_float	m_fViewZ = { 0.f };
	_float3 m_vViewPos = _float3{ 0.f, 0.f, 0.f };
	_bool	m_bDead = { false };

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END