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
BEGIN(WaddleDee)

enum DEEEYESTATE
{
	DEEEYE_IDLE, DEEEYE_SMILE, DEEEYE_CLOSE,
	DEEEYE_ANGER, DEEEYE_SADNESS, DEEEYE_DOUBT, DEEEYE_END
};

enum DEE_ANIM
{
	DEEANIM_ANGER,
	DEEANIM_ANGERRUN,
	DEEANIM_BREAK = 3,
	DEEANIM_CASEFIND,
	DEEANIM_CASEHELPME_END,
	DEEANIM_CASEHELPME_START,

	DEEANIM_CHEERINGA = 13,
	DEEANIM_CHEERINGB,
	DEEANIM_CHEERINGC,
	DEEANIM_CHEERINGD,

	DEEANIM_CHOOSE_START = 17,
	DEEANIM_CHOOSE_WAIT,

	//가게 앞 직원
	DEEANIM_CLERKTALK = 26,
	DEEANIM_CLERKWAVEHAND,

	DEEANIM_DAMAGE,
	DEEANIM_ENEMYJUMPSTART = 43,
	DEEANIM_ENEMYJUMPSTARTLONG,

	DEEANIM_ENEMYRUN = 48,

	//원래 와들디의 행동들
	DEEANIM_ENEMYSLEEP = 50,
	DEEANIM_ENEMYWALK = 51,

	DEEANIM_FIND = 53,

	//인사 받아주기
	DEEANIM_GREETING_ONEHAND = 56,
	DEEANIM_GREETING_TWOHAND,
	DEEANIM_GREETING_TWOHANDLOOP,

	//박사님
	DEEANIM_KNOWCLOSEBOOK = 75,
	DEEANIM_KNOWFEAR,
	DEEANIM_KNOWGLASSMOVE,
	DEEANIM_KNOWOPENBOOK,
	DEEANIM_KNOWRESEARCH,
	DEEANIM_KNOWTALK,
	DEEANIM_KNOWTALKLONG,
	
	//착지
	DEEANIM_LANDING = 82,
	DEEANIM_LAUGH,

	DEEANIM_LOOKAROUND = 88,
	DEEANIM_MOVEFALL = 91,
	DEEANIM_SITTALKA = 105,
	DEEANIM_SITTALKB = 107,
	DEEANIM_SITTALKLISTEN = 109,

	DEEANIM_SITWAIT = 111,
	DEEANIM_SITSLEEP,

	DEEANIM_SLIPNO = 116,

	DEEANIM_SLIPYES = 119,
	DEEANIM_SURPRISE,

	DEEANIM_TALK1,
	DEEANIM_TALK2,
	DEEANIM_TALK3A,
	DEEANIM_TALK3B,
	DEEANIM_TALK3LISTEN,

	DEEANIM_TOWNWAIT = 127,
	DEEANIM_TROUBLE = 129,

	DEEANIM_WAIT = 133,
	DEEANIM_WALK,
	DEEANIM_WATERING,
	DEEANIM_WAVEHAND,
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
	DEESHOPANIM_RUN,
	DEESHOPANIM_WAIT = 23,
	DEESHOPANIM_ANGRYWAIT = 25,
	DEESHOPANIM_WALK,

	DEESHOPANIM_END
};

//마을에 있는 디들에게 세팅해줄 처음 행동(성격)
enum DEE_CHARACTER
{
	DEECHARACTER_IDLE,
	DEECHARACTER_WALK,
	DEECHARACTER_SIT,
	DEECHARACTER_SITTALK,
	DEECHARACTER_TROUBLE = 4,
	DEECHARACTER_ANGRY = 5,
	DEECHARACTER_SLEEPY,
	DEECHARACTER_FRONTMAN = 7,
	DEECHARACTER_KEYBOARD,
	DEECHARACTER_GUITAR,
	DEECHARACTER_DRUM,
	DEECHARACTER_LISTENER,
	DEECHARACTER_READBOOK = 12,
	DEECHARACTER_RUN = 13,
	DEECHARACTER_END,
};

enum TOWN_POINT
{
	TOWNPOINT_0,
	TOWNPOINT_1,
	TOWNPOINT_2,
	TOWNPOINT_3,
	TOWNPOINT_4,
	TOWNPOINT_5,
	TOWNPOINT_6,
	TOWNPOINT_7,
	TOWNPOINT_8,
	TOWNPOINT_9,
	TOWNPOINT_10,
	TOWNPOINT_11,
	TOWNPOINT_12,
	TOWNPOINT_FOODSHOP,
	TOWNPOINT_14,
	TOWNPOINT_FISHING,
	TOWNPOINT_16,
	TOWNPOINT_1FLOOR,
	TOWNPOINT_UNDERSTAIRA,
	TOWNPOINT_MIDSTAIRA,
	TOWNPOINT_UPSTAIRA,
	TOWNPOINT_2FLOOR,
	TOWNPOINT_UPSTAIRB,
	TOWNPOINT_MIDSTAIRB,
	TOWNPOINT_UNDERSTAIRB,
	TOWNPOINT_FLOWERUPRIGHT,
	TOWNPOINT_FLOWERDOWNRIGHT,
	TOWNPOINT_FLOWERUPLEFT,
	TOWNPOINT_FLOWERDOWNLEFT,
	TOWNPOINT_29,
	TOWNPOINT_END
};

struct WAITING_INFO
{
	_float3 vPos;
	_float	fAnimOffset;
};

struct TOWN_POINT_INFO
{
	//자신의 인덱스(사실 그냥 바로 벡터 인덱스 써도 되는데 나중에 구별할 일 있을까봐 넣음)
	TOWN_POINT			MyPoint;
	//타운 랠리포인트 기준으로부터의 상대값
	_float3				vPosOffset;
	//거기서 디가 할 행동 패턴들
	vector<DEE_ANIM>	StateOffset;
	//인접하여 갈 수 있는 지점들
	vector<TOWN_POINT>	NearPoint;
	//사용 중인가? (그곳에 디가 있는가?)
	_bool				bIsUsing = { false };
};

END

using namespace WaddleDee;

class CWaddleDee abstract : public CCharacter
{
public:
	struct DEE_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		DEE_CHARACTER eCharacter;
	};


protected:
	CWaddleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWaddleDee(const CWaddleDee& rhs);
	virtual ~CWaddleDee() = default;

public:
	void Set_DeeEyeState(DEEEYESTATE eState) { m_eEyeState = eState; }


#ifdef _DEBUG
	virtual void		Render_IMGUI() override;
#endif

	virtual void		Add_AnimEvent() override {}

	//목적지의 위치를 만든다
	virtual _float3		Make_DestPos() { return _float3::Zero; }
	virtual pair<DEE_ANIM, _bool >	Make_WhatToDo() { return { DEEANIM_END, true }; }

	void				Change_State(DEE_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);

	_bool				IsCloseToKirby() { return m_bIsKirbyInZone; }

	//커비와의 인사 여부
	void			SetHiToKirby(_bool bHi) { m_bHiToKirby = bHi; }
	_bool			GetHiToKirby() { return m_bHiToKirby; }

	_bool			IsAnimFinished() { return m_pModelCom->IsFinished(); }


	virtual HRESULT Initialize_Prototype()	override;
	virtual HRESULT Initialize(void* pArg)	override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render()				override;
	virtual HRESULT Render_LightDepth()		override;

protected:
	//커비가 상호작용 버튼 누를 정도로 가까이 왔는가?
	_bool			m_bIsKirbyInZone = { false };

	//커비한테 인사했는가?
	_bool			m_bHiToKirby = { false };
	_float			m_fResetHiTime = { 0.f };

	//이전 틱의 위치
	_float3			m_vPrePos = { 0.f, 0.f, 0.f };

	DEEEYESTATE		m_eEyeState = { DEEEYE_END };

	map<const wstring, class CPartObject*>	m_PartObjects;
	CModel*			m_pModelCom = { nullptr };
	CTexture*		m_pEyeTextureCom = { nullptr };


	_bool			Custom_Face(_uint iMeshIndex);
	void			Dee_SystemTick(_float fTimeDelta);

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;


};


END

