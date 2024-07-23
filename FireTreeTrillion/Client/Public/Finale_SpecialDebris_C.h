#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "Effect.h"

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

	enum CUT { CUT10, CUT11, CUT12, CUT13, CUT14, CUT15, CUT16, CUT17, CUT18, CUT19, CUT20, CUT_END };
	CUT				m_eCurCut = { CUT_END };
	CUT				m_ePreCut = { CUT_END };
	_bool			m_bRender = { false };
	void			Set_Animation();

	_bool			m_bMakeParticle = { true };

	void			Compute_My_Look(_int iIndex);
	_float4x4		m_EffectSocket = _float4x4::Identity;
	_bool			m_bInitializeLook = { true };
	_float4			m_vPrePos = { 0.f, 0.f, 0.f,  0.f };
	_float4			m_vCurPos = { 0.f, 0.f, 0.f,  0.f };

	CEffect*		m_pEffect = { nullptr };

	_bool			m_bEffectOn = { true };


private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

public:
	static CFinale_SpecialDebris_C* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END