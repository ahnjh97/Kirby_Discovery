#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CFinale_SpecialDebris_B : public CGameObject
{
private:
	CFinale_SpecialDebris_B(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinale_SpecialDebris_B(const CFinale_SpecialDebris_B& rhs);
	virtual ~CFinale_SpecialDebris_B() = default;


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

	enum CUT { CUT4, CUT5, CUT6, CUT7, CUT_END };
	CUT				m_eCurCut = { CUT_END };
	CUT				m_ePreCut = { CUT_END };
	_bool			m_bRender = { false };
	void			Set_Animation();

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

public:
	static CFinale_SpecialDebris_B* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END