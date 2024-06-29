#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

BEGIN(Engine)
class CBone;
class CModel;
class CShader;
END

BEGIN(Client)

class CFinalBossSpear final : public CPartObject
{
public:
	struct FINALBOSSSPEAR_DESC : public CPartObject::PARTOBJECT_DESC
	{
		CBone* pSocket = { nullptr };
	};

private:
	CFinalBossSpear(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinalBossSpear(const CFinalBossSpear& rhs);
	virtual ~CFinalBossSpear() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CBone* m_pSocket = { nullptr };

private:
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

	_bool			m_bGate = { false };

	_float			m_fDelayTime = { 0.f };
	_float			m_fCreateTime = { 0.f };
	_float			m_fFirstFireTime = { 4.2f };
	_float			m_fSecondFireTime = { 4.f };

	_float			m_fAngle = { 0.f };
	_float			m_fSide = { 0.f };

	_uint			m_iCnt = { 0 };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	void	Compute_MotionBlur();
	_float4 Compute_BoneWorldMatrix();

public:
	static CFinalBossSpear* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END