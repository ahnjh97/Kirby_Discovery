#pragma once
#include "ItemObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CCharacterController;
END

BEGIN(Client)

class CAbility final : public CItemObject
{
public:
	struct ABILITYITEM_DESC : public CGameObject::GAMEOBJECT_DESC {
		_float	fRotateDir = { 0.f };
		_float	fAngle = { 0.f };
		_vector	vDir = {};
		_float4 vPosition = {};
		ABILITYTYPE eAbilityType = {};
	};


private:
	CAbility(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAbility(const CAbility& rhs);
	virtual ~CAbility() = default;

public:
	virtual HRESULT Initialize_Prototype()	override;
	virtual HRESULT Initialize(void* pArg)	override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render()				override;
	virtual HRESULT Render_LightDepth()		override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;


private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	void AbilityType(ABILITYTYPE eAbilityType);

	_bool RayCast_Terrain(const _float3 vMoveDir);

	// Sphere 충돌 밀어내기
	void Sphere_Collision();
	_bool Check_Sphere(CTransform* pTransform, _float* fDist);

private:
	CModel*					m_pModelCom = { nullptr };

private:
	_vector					m_vDir = { 0.f };

	_float4					m_vPosition = {};
	_float4					m_vLookDir = { 0.f, 0.f, 0.f, 0.f };

	_bool					m_bRender = { false };
	_bool					m_bTurn = { false };

	_uint					m_iDeathCount = { 0 };
	_uint					m_iRenderCount = { 0 };

	_float					m_fJumpPower = { 0.f };
	_float					m_fJumpPowerTemp = { 0.f };
	_float					m_fPower = { 0.f };
	_float					m_fSpeed = { 0.f };
	_float					m_fAngle = { 0.f };
	_float					m_fScale = { 0.f };

	_float					m_fRotateDir = { 0.f };
	_float					m_fLifeTime = { 0.f };
	_float					m_fRenderTime = { 0.f };

	_float					m_fFlyTime = { 0.f };
	_float					m_fHitPower = { 0.f };

	wstring					m_strComponentTag = { L"" };

public:
	static CAbility* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END