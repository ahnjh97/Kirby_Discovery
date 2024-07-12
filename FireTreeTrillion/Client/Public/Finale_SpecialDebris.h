#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CFinale_SpecialDebris final : public CPhysXObject
{
public:
	enum DEBRISTYPE { DEBRIS_A, DEBRIS_B, DEBRIS_C, DEBRIS_END };

	struct DEBRISDESC : public GAMEOBJECT_DESC
	{
		DEBRISTYPE eType = { DEBRIS_END };
	};

private:
	CFinale_SpecialDebris(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinale_SpecialDebris(const CFinale_SpecialDebris& rhs);
	virtual ~CFinale_SpecialDebris() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual HRESULT Render_LightDepth()							override;

	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

private:
	HRESULT			Add_Components(wstring wstrModelProtoTag);
	HRESULT			Bind_ShaderResources();
	_int			Make_Partical();
	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

	_float4			m_vTargetPos = { 0.f, 0.f, 0.f, 0.f };
	DEBRISTYPE		m_eDeBrisType = { DEBRIS_END };

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

public:
	static CFinale_SpecialDebris* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END