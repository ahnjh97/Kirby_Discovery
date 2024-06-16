#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)
 
class CAwoofy final : public CMonster
{
public:
	enum AWOOFY_ANIM { AWOOFY_BRAKE, AWOOFY_DAMAGE, AWOOFY_FALL, AWOOFY_FIND, AWOOFY_FINDSUB, AWOOFY_FINDWAIT, AWOOFY_GROOMING, 
		AWOOFY_HOWLING, AWOOFY_JUMP, AWOOFY_JUMPEND, AWOOFY_LANDING, AWOOFY_LOOKAROUND, AWOOFY_LOOKAROUNDAFTERBRAKE, AWOOFY_RUN, 
		AWOOFY_SLEEP, AWOOFY_SLEEPFALL, AWOOFY_WAIT, AWOOFY_WAKEUP, AWOOFY_WALK, 
		AWOOFY_END };

	enum AWOOFYEYE_STATE { AWOOFYEYE_IDLE, AWOOFYEYE_HALF, AWOOFYEYE_SLEEP, AWOOFYEYE_HAPPY, AWOOFYEYE_ANGER, AWOOFYEYE_END };

private:
	CAwoofy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAwoofy(const CAwoofy& rhs);
	virtual ~CAwoofy() = default;

public:
	void Set_AwoofyEye(AWOOFYEYE_STATE eEyeState) { m_eEyeState = eEyeState; }

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
	void Change_State(AWOOFY_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();
	_bool IsAnimFinished(_uint iCurrentAnimIndex);
	void Compute_Angle(_vector vOrginLook, _vector vTargetLook);

private:
	CTexture*			m_pEyeTextureCom = { nullptr };

	AWOOFY_ANIM			m_eCurrentState = { AWOOFY_END };
	AWOOFYEYE_STATE		m_eEyeState = { AWOOFYEYE_END };

	_float				m_fAngle = { 0.f };
	_float4				m_vPos = {};

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();
	_bool Custom_Face(_uint iMeshIndex);

public:
	static CAwoofy* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END