#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CFinalStone final : public CGameObject
{
public:
	struct FINALSTONEDESC : GAMEOBJECT_DESC
	{
		_float4 vPos;
		_float fScale;
	};


private:
	CFinalStone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinalStone(const CFinalStone& rhs);
	virtual ~CFinalStone() = default;

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

	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	_float	m_fGravity = { 0.f };

public:
	static CFinalStone* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END