#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)
 
class CBossChimera final : public CMonster
{
public:
	enum CHIMERA_ANIM { CHIMERA_NONE };
	enum CHIMERA_EYE { CHIMERA_EYENONE };

private:
	CBossChimera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBossChimera(const CBossChimera& rhs);
	virtual ~CBossChimera() = default;

public:
	//void Set_AwoofyEye(AWOOFYEYE_STATE eEyeState) { m_eEyeState = eEyeState; }

	_float4 Get_Pos() { return m_vPos; }

public:
	virtual HRESULT Initialize_Prototype()	override;
	virtual HRESULT Initialize(void* pArg)	override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render()				override;
	virtual HRESULT Render_LightDepth()		override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif
	virtual void	Add_AnimEvent()			override;
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;


public:
	//void Change_State(AWOOFY_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();
	_bool IsAnimFinished(_uint iCurrentAnimIndex);
	void Compute_Angle(_vector vOrginLook, _vector vTargetLook);

private:
	CTexture*			m_pEyeTextureCom = { nullptr };

	CHIMERA_ANIM		m_eCurState = { CHIMERA_NONE };
	CHIMERA_EYE			m_eEyeState = { CHIMERA_EYENONE };

	_float				m_fAngle = { 0.f };
	_float4				m_vPos = {};

	_bool				m_bRenderEye = { true };
	_bool				m_bRenderBody = { true };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();
	_bool Custom_Face(_uint iMeshIndex);

public:
	static CBossChimera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END