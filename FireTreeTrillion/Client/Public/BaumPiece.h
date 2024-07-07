#pragma once


#include "Client_Defines.h"
#include "PhysXObject.h"


BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)


class CBaumPiece final : public CPhysXObject
{
public:
	struct BAUMPIECEDESC : public GAMEOBJECT_DESC
	{
		wstring strModelName;
		_float	fParticalSpeed = { 0.f };
		_float4 vParticalMoveDir = { 0.f, 0.f, 0.f, 0.f };
	};

private:
	CBaumPiece(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBaumPiece(const CBaumPiece& rhs);
	virtual ~CBaumPiece() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual HRESULT Render_LightDepth()							override;

private:
	HRESULT			Add_Components(const wstring& wstrModelName);
	HRESULT			Bind_ShaderResources();

	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

	_float			m_fDeathTime = { 0.f };

	_float4			m_vFlyDir = { 0.f, 0.f, 0.f, 0.f };
	_float			m_fFlyPower = { 0.f };
	_float4			m_vGravity = { 0.f, 0.f, 0.f, 0.f };
	_float			m_fGravityPower = { 0.f };
	_float			m_vTurnSpeed = { 0.f };
	_float4			m_vTurnAxis = { 0.f, 0.f, 0.f, 0.f };

	_bool			m_bBloom = { false };

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

public:
	static CBaumPiece* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END