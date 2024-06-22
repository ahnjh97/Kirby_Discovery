#pragma once

#include "Client_Defines.h"
#include "Character.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
class CPartObject;
END

BEGIN(Client)

class CWaddleDee abstract : public CCharacter
{
public:
	enum DEEEYESTATE
	{
		DEEEYE_IDLE, DEEEYE_ANGER, DEEEYE_CLOSE, DEEEYE_SADNESS,
		DEEEYE_PUPIL, DEEEYE_BLINK, DEEEYE_DOUBT, DEEEYE_END
	};

	enum DEE_ANIM
	{
		DEEANIM_ANGER,
		DEEANIM_BREAK = 3,
		DEEANIM_CASEFIND,
		DEEANIM_CASEHELPME_END,
		DEEANIM_CASEHELPME_START,

		DEEANIM_CLERKTALK = 26,
		DEEANIM_CLERKWAVEHAND,

		DEEANIM_ENEMYSLEEP = 50,
		DEEANIM_ENEMYWALK = 51,

		DEEANIM_GREETING_ONEHAND = 56,
		DEEANIM_GREETING_TWOHAND,
		DEEANIM_GREETING_TWOHANDLOOP,
		DEEANIM_SLIPYES = 119,
		DEEANIM_SURPRISE,

		DEEANIM_TALK1,
		DEEANIM_TALK2,
		DEEANIM_TALK3A,
		DEEANIM_TALK3B,
		DEEANIM_TALK3LISTEN,

		DEEANIM_WAIT = 133,

		DEEANIM_END
	};

	enum DEE_SHOPANIM
	{
		DEESHOPANIM_CLERKCHOOSESTART = 2,

		DEESHOPANIM_CLERKCORRECT = 4,
		DEESHOPANIM_CLERKINCORRECT = 6,

		DEESHOPANIM_CLERKRESULTSUCCESS,
		DEESHOPANIM_CORRECTMOVE = 9,

		DEESHOPANIM_RESULTWIN = 12,
		DEESHOPANIM_RESULTWINSTART = 13,

		DEESHOPANIM_GUESTANGER,
		DEESHOPANIM_GUESTFIDGET,
		DEESHOPANIM_GUESTNORMAL,
		DEESHOPANIM_IMPATIENCE,
		DEESHOPANIM_INCORRECT,
		DEESHOPANIM_INCORRECTMOVE,

		DEESHOPANIM_ORDERNORMAL = 21,

		DEESHOPANIM_SADWAIT = 23,
		DEESHOPANIM_ANGRYWAIT = 25,
		DEESHOPANIM_WALK,

		DEESHOPANIM_END
	};

	//enum DEE_STATE
	//{
	//	DEE_
	//	DEE_END
	//};


	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{

	}DEE_DESC;


protected:
	CWaddleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWaddleDee(const CWaddleDee& rhs);
	virtual ~CWaddleDee() = default;

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

	virtual void	Add_AnimEvent() override {}


protected:
	CModel*			m_pModelCom = { nullptr };
	CTexture*		m_pEyeTextureCom = { nullptr };

	DEEEYESTATE		m_eEyeState = { DEEEYE_END };

	_bool			Custom_Face(_uint iMeshIndex);

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;


};

END

