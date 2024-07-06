#pragma once
#include "PhysXObject.h"


BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)


class CBaumPiece final : public CPhysXObject
{
public:
	typedef struct BAUMDESC
	{
		wstring strModelName;
		_float	fParticalSpeed = { 0.f };
		_float4 vParticalMoveDir = { 0.f, 0.f, 0.f, 0.f };
	}BAUMDESC;

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
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	PxRigidDynamic* m_pDynamicActor = { nullptr };

public:
	static CBaumPiece* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END