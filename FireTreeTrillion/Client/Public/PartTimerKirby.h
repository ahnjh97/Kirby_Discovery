#pragma once
#include "Client_Defines.h"
#include "Character.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CPartTimerKirby final : public CCharacter
{
public:
	enum STATE { 
		STATE_IDLE, STATE_MOVE, STATE_GRAB, STATE_END};
	enum ANIM {		//FOODSHOP_CONFLICT,
		FOODSHOP_CORRECT = 1,		//FOODSHOP_ERROR,	FOODSHOP_FOOTSUBL, FOODSHOP_FOOTSUBR,
		FOODSHOP_INCORRECT = 5,
		FOODSHOP_INCORRECTSTART,
		FOODSHOP_MOVEL,
		FOODSHOP_MOVER,
		FOODSHOP_RESULTERROR,
		FOODSHOP_RESULTERRORSTART, 	//FOODSHOP_RESULTLOSE, FOODSHOP_RESULTLOSESTART,
		FOODSHOP_RESULTWIN = 13,
		FOODSHOP_RESULTWINSTART,
		FOODSHOP_SELECT,			//FOODSHOP_SELECTSERIOUS, HANDOVER,
		HANDOVERSHORT = 18,
		HANDOVERSHORTL,
		ANIM_END
	};
	enum EYESTATE { EYE_IDLE, EYE_ANGER, EYE_CLOSE, EYE_SADNESS, EYE_PUPIL, EYE_BLINK, EYE_END };
	enum MOUTHSTATE { MOUTH_IDLE, MOUTH_ANGER, MOUTH_HAPPY, MOUTH_SMILE, MOUTH_SURPRISE, MOUTH_END };

private:
	CPartTimerKirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPartTimerKirby(const CPartTimerKirby& rhs);
	virtual ~CPartTimerKirby() = default;

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;
	virtual HRESULT Render_LightDepth()				override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()					override;
#endif
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;
	virtual void	Collision_Hitbox(CPhysXObject* pGameObject) override;

public:
	void			Change_State(ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool			IsAnimFinished();
	_float4			Compute_TerrainPosition();

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

	// FSM
	void			SetUp_FSM();

	// set-up camera
	HRESULT			Make_TargetToCams();

private:
	CModel*					m_pModelCom   = { nullptr };
	CTexture*				m_pEyeTexture[EYE_END] = { nullptr };
	CTexture*				m_pMouthTexture[MOUTH_END] = { nullptr };
	class CCamera*			m_pCamera = { nullptr };

	// °í°´ ¿Íµéµð

	ANIM					m_eCurrentState = { ANIM_END };
	EYESTATE				m_eEyeState = { EYE_END };
	MOUTHSTATE				m_eMouthState = { MOUTH_END };

	_float					m_fScore = _float();

public:
	static CPartTimerKirby* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg)	override;
	virtual void			Free()				override;
};

END