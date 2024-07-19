#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CRayArrow final : public CPhysXObject
{
public:
	struct RAYARROW_DESC : public CGameObject::GAMEOBJECT_DESC {
		_float fAngle = { 0.f };
		_float fHeight = { 0.f };
		_float fDelayTime = { 0.f };
		_float fSpeedWeight = { 0.f };
		_vector vPosition = {};
		_vector vSide = {};
		_float4x4 WorldMatrix = {};
	};

private:
	CRayArrow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRayArrow(const CRayArrow& rhs);
	virtual ~CRayArrow() = default;

public:
	void Set_Active(_bool bActive) { m_bActive = bActive; }

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;
	virtual HRESULT Render_LightDepth()				override;
#ifdef _DEBUG
	virtual	void Render_IMGUI()			override;
#endif
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

private:
	CModel*			m_pModelCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };

	_float			m_fTimeDelta = { 0.f };
	_float			m_fTurnTime = { 0.f };
	_float			m_fElapsedTime = { 0.f };
	_float			m_fFireTime = { 0.f };
	_float			m_fRotateTime = { 0.f };
	_float			m_fDelayTime = { 0.f };
	_float			m_fDeadTime = { 0.f };

	_float			m_fAngle = { 0.f };
	_float			m_fHeight = { 0.f };
	_float			m_fSpeedWeight = { 0.f };
	_float			m_fRotateSpeed = { 0.f };

	_vector			m_vPosition = {};
	_vector			m_vKirbyPos = {};
	_vector			m_vControllPos = {};
	_vector			m_vSide = {};

	_vector			m_vLook = {};
	_vector			m_vUp = {};
	_vector			m_vRight = {};

	_bool			m_bActive = { false };
	_bool			m_bFire = { false };
	_bool			m_bFireActive = { false };

	//ÀÌÆåÆ® Ã¼Å© À§ÇÑ Å¸ÀÓ
	_float			m_fFlyingTime = { 0.f };
	//¶¥¿¡ ²ÈÈ÷´Â ¼ø°£ ÆÄ¾ÇÇÏ´Â 1Æ½
	_bool			m_bCollided = { false };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	void	Compute_MotionBlur();

public:
	static CRayArrow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END