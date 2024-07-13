#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CKirbyArmours final : public CPartObject
{
public:
	typedef struct : public CPartObject::PARTOBJECT_DESC
	{
		_float4x4* pBoneMatrix = { nullptr };
		ABILITYTYPE* pAbilityType = { nullptr };
		_float* pWhite = { nullptr };
		_float* pOverPower = { nullptr };
	}KIRBYARMOURS_DESC;

private:
	CKirbyArmours(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirbyArmours(const CKirbyArmours& rhs);
	virtual ~CKirbyArmours() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;
	virtual HRESULT	Render_DeferredInfo() override;


private:
	// 랜더가 되지 않아야 하는 것들
	_bool Block_Render() {
		return *m_pAbilityType == ABILITY_END || *m_pAbilityType == ABILITY_DEFAULT;
	}

	CModel* m_pModelCom[ABILITY_END] = { nullptr };
	CShader* m_pShaderCom = { nullptr };

private:
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

	_float4x4* m_pBoneMatrix = { nullptr };
	ABILITYTYPE* m_pAbilityType = { nullptr };

	_float* m_pWhiteColorDiffuse = { nullptr };
	_float* m_fOverPowerColor = { nullptr };

	_float	m_fEmissivePower = { 0.f };

	_int	m_iRenderCount = { 0 };
	_bool	Render_Emissive();

	class CLight* m_pLight = { nullptr };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	void	Compute_MotionBlur();

public:
	static CKirbyArmours* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END