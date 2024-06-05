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
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

private:
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

	_float4x4* m_pBoneMatrix = { nullptr };

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