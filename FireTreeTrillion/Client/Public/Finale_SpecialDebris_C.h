#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CFinale_SpecialDebris_C : public CGameObject
{
private:
	CFinale_SpecialDebris_C(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinale_SpecialDebris_C(const CFinale_SpecialDebris_C& rhs);
	virtual ~CFinale_SpecialDebris_C() = default;


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
	_int			Make_Partical();

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

public:
	static CFinale_SpecialDebris_C* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END