#include "stdafx.h"
#include "Camera_Main.h"

#include "Kirby.h"
#include "PartTimerKirby.h"
#include "FinaleKirby.h"
#include "FinalBoss.h"
#include "FinaleBoss.h"

#include "Simba.h"

#include "EventCenter.h"
#include "PartTimeHelper.h"
#include "Particle.h"
#include "FinaleCut_ControlCenter.h"

#include "Level_loading.h"
#include "UI_Fading.h"

#define ORIG_POS _float3(2550.f, 242.f, -136.f)
#define BATTLE_POS _float3(2525.f, 220.f, -136.f)
#define BOSS_POS _float3(2548.f, 242.f, -136.f)

#define ISDEFAULTCNT(intvalue) (intvalue == -1)
#define FINALEBOSS (static_cast<CFinaleBoss*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, L"Layer_BossMonster", L"Prototype_GameObject_FinaleBoss")))
#define FINALEKIRBY (static_cast<CFinaleKirby*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, L"Layer_Player", L"Prototype_GameObject_FinaleKirby")))

#define QTE1 6
#define QTE2 13
#define QTE3 18

_float3 vertices[] = {
	// Front face
   { -1.0f, -1.0f, -1.0f },
   { -1.0f,  1.0f, -1.0f },
   {  1.0f,  1.0f, -1.0f },
   {  1.0f, -1.0f, -1.0f },
   // Back face
   { -1.0f, -1.0f,  1.0f },
   { -1.0f,  1.0f,  1.0f },
   {  1.0f,  1.0f,  1.0f },
   {  1.0f, -1.0f,  1.0f },
   // Left face
   { -1.0f, -1.0f, -1.0f },
   { -1.0f,  1.0f, -1.0f },
   { -1.0f,  1.0f,  1.0f },
   { -1.0f, -1.0f,  1.0f },
   // Right face
   {  1.0f, -1.0f, -1.0f },
   {  1.0f,  1.0f, -1.0f },
   {  1.0f,  1.0f,  1.0f },
   {  1.0f, -1.0f,  1.0f },
   // Top face
   { -1.0f,  1.0f, -1.0f },
   { -1.0f,  1.0f,  1.0f },
   {  1.0f,  1.0f,  1.0f },
   {  1.0f,  1.0f, -1.0f },
   // Bottom face
   { -1.0f, -1.0f, -1.0f },
   { -1.0f, -1.0f,  1.0f },
   {  1.0f, -1.0f,  1.0f },
   {  1.0f, -1.0f, -1.0f },
};

float SmoothStep(float edge0, float edge1, float x) {
	// Scale, bias and saturate x to 0..1 range
	x = (x - edge0) / (edge1 - edge0);
	x = max(0.0f, min(1.0f, x));
	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}

XMVECTOR CustomLerp(XMVECTOR A, XMVECTOR B, XMVECTOR C, float t) {
	t = SmoothStep(0.0f, 1.0f, t); // 매끄러운 비율 적용
	if (t < 0.5f) {
		// A와 C 사이 보간
		float localT = t / 0.5f;
		return _float3::Lerp(A, C, localT);
	}
	else {
		// C와 B 사이 보간
		float localT = (t - 0.5f) / 0.5f;
		return _float3::Lerp(C, B, localT);
	}
}

CCamera_Main::CCamera_Main(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{
}

CCamera_Main::CCamera_Main(const CCamera_Main& rhs)
	: CCamera{ rhs }
{
}

void CCamera_Main::EventFunc(CGameObject* pObj)
{
	//Make_Sequence(SEQ_SOFTCUT_TEST);
	_int a = 0;
}

void CCamera_Main::Lock_Position(_float3 vPos, _bool bInterpolate)
{
	m_eCamLockMode = LOCK_POS;

	_float3 vDestPos = ISDEFAULTFLOAT3(vPos) ? GET_POS : vPos;

	if (!bInterpolate)
	{
		SET_POS(vDestPos);
		m_vCurCamPos = vDestPos;
	}

	m_vDestCamPos = vDestPos;
}

void CCamera_Main::Lock_Direction(_float3 vLook, _bool bInterpolate)
{
	m_eCamLockMode = LOCK_DIR;

	vLook.Normalize();
	_float3 vDestDir = ISDEFAULTFLOAT3(vLook) ? (_float3)m_pTransformCom->Get_State(CTransform::STATE_LOOK) : vLook;

	if (!bInterpolate)
	{
		m_pTransformCom->Look_At_Dir(Dir(vLook));
		m_vCurCamDir = vDestDir;
	}

	m_vDestCamDir = vDestDir;
}

void CCamera_Main::Lock_All(_float3 vPos, _float3 vLook, _bool bInterpolate)
{
	m_eCamLockMode = LOCK_ALL;

	_float3 vDestPos = ISDEFAULTFLOAT3(vPos) ? GET_POS : vPos;
	vLook.Normalize();
	_float3 vDestDir = ISDEFAULTFLOAT3(vLook) ? (_float3)m_pTransformCom->Get_State(CTransform::STATE_LOOK) : vLook;

	if (!bInterpolate)
	{
		SET_POS(vDestPos);
		m_vCurCamPos = vDestPos;

		m_pTransformCom->Look_At_Dir(Dir(vLook));
		m_vCurCamDir = vDestDir;
	}

	m_vDestCamPos = vDestPos;
	m_vDestCamDir = vDestDir;

}

void CCamera_Main::Set_Target(CTransform* pTarget, CAMTARGET eTarget, CAMFOCUS eFocus, _float3 vAnchorOffset, _float fInterpolateSpeed)
{
	if (nullptr == pTarget)
		return;

	if (eTarget == TARGET_FIRST)
	{
		if (nullptr != m_pFirstTarget)
			Safe_Release(m_pFirstTarget);

		m_pFirstTarget = pTarget;
		Safe_AddRef(pTarget);

	}
	else if (eTarget == TARGET_SECOND)
	{
		if (nullptr != m_pSecondTarget)
			Safe_Release(m_pSecondTarget);

		m_pSecondTarget = pTarget;
		Safe_AddRef(pTarget);
	}

	m_vAnchorOffset = vAnchorOffset;

	if (0.f < fInterpolateSpeed)
		m_fInterpolateSpeed = fInterpolateSpeed;

	m_eCamFocus = eFocus;
	//m_vAnchorOffset = vAnchorOffset;

	if ((eFocus == FOCUS_SECOND || eFocus == FOCUS_BOTH) && m_pSecondTarget == nullptr)
		m_eCamFocus = FOCUS_FIRST;

}

HRESULT CCamera_Main::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera_Main::Initialize(void* pArg)
{
	CAMERA_KIRBY_DESC pCamDesc{};

	if (nullptr != pArg)
	{
		pCamDesc = *(CAMERA_KIRBY_DESC*)pArg;

		m_fCamSensor = pCamDesc.fCamSensor;
		m_fDestFovy = pCamDesc.fFovy;
		m_fOrigDistance = pCamDesc.fOrigDistance;


		HRESULT hr = __super::Initialize(pArg);
		CHECK_FAILED_MSG(hr, "Failed To Initialize : CCamera_Main");
	}

	m_pGameInstance->Add_Camera(this);

	//트리거 세팅
	_uint iLevel = *m_pGameInstance->Get_CurrentLevelID();

	if (iLevel >= LEVEL_PARTTIME && iLevel < LEVEL_END)
	{
		function<void(_int)> func = bind(&CCamera_Main::StartLerpByTriggerInfo, this, placeholders::_1);
		m_pGameInstance->Emplace_TriggerFunc(TRIGGER_CAMERA, func);

		function<void(void)> exitFunc = bind(&CCamera_Main::EndLerpByTriggerInfo, this);
		m_pGameInstance->Emplace_ExitFunc(TRIGGER_CAMERA, exitFunc);
	}


	//// 파트타임헬퍼에 옵저버로 카메라를 알게하고 있습니다. JYWI's ps : 카메라 클래스 하나 더 팔걸~~
	//HYO's ps : 에이 파면 또 귀찮어~~~~~
	if (iLevel == LEVEL_PARTTIME)
		CPartTimeHelper::Get_Instance()->Register_Camera(this);

	//distance 세팅
	m_fDestDistance = m_fOrigDistance;
	m_fCurDistance = m_fOrigDistance;


	m_pTransformCom->Set_State(CTransform::STATE_POSITION, Pos(pCamDesc.vEye));
	m_pTransformCom->Look_At(pCamDesc.vAt);


	_float4 vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	m_vDestCamDir = _float3(vLook);
	m_vDestCamDir.Normalize();
	m_vCurCamDir = m_vOrigCamDir = m_vDestCamDir;

	//여러 이벤트에 함수를 등록한다.
	Subscribe_Events();

	//y로 올리는 오프셋
	m_CamTriggerUpOffsets.reserve(LEVEL_END);
	m_CamTriggerUpOffsets.resize(LEVEL_END);

	m_CamTriggerUpOffsets[LEVEL_INTRO] = { 0.f, 0.f, 0.f, .2f, .15f, 0.f, 0.f, 0.f, 0.f, 0.1f, 0.1f, 0.1f }; //9 == 11
	//m_CamTriggerUpOffsets[LEVEL_PARK] = { 0.f, 0.f, 0.f, 100.f, .15f, 0.f, 0.f, 0.f, 0.f };
	m_CamTriggerUpOffsets[LEVEL_FINALBOSS] = { .05f };
	m_CamTriggerUpOffsets[LEVEL_FINALE] = { .4f, 0.f, 0.f, .4f, .4f, .5f, 0.2f, 0.2f, 0.f };


	//시퀀스 이벤트 트리거를 초기화
	m_SeqEventTriggers.reserve(10);
	m_SeqEventTriggers.resize(10);
	fill(m_SeqEventTriggers.begin(), m_SeqEventTriggers.end(), true);

	//별 이펙트 테스트용
	//if (*m_pCurrentLevelID == LEVEL_FINALBOSS)
	//{
	//		CEffect::FX_DESC FxDesc{};
	//		FxDesc.pSocketMatrix = &m_EffectSocket;
	//		if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_final sky"), &FxDesc)))
	//			return E_FAIL;
	//}

	if (*m_pCurrentLevelID == LEVEL_FINALE)
	{
		CParticle::PARTICLE_DESC FxDesc{};
		FxDesc.pSocketMatrix = &m_EffectSocket;

		if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_night star test 2"), &FxDesc)))
			return E_FAIL;
	}

	m_FinaleSeqATime =
	{
		0.f,
		298.f / 50.f, //cut1 - cut2
		210.f / 50.f, /*210.f*/ //cut2 - cut3
		200.f / 50.f, /*200.f*/ //cut3 - cut4
		151.f / 50.f, /*150.f*/ //cut4 - cut5//운석 던지기 시작
		190.f / 50.f,
	};

	//7부터
	m_FinaleSeqBTime =
	{
		0.f,
		110.f / 50.f, //cut7 - cut8. 2배 느림
		142.f / 50.f, //cut8 - cut9
		130.f / 50.f, //cut9 - cut10
		310.f / 50.f, //cut10 - cut11
		110.f / 50.f, //cut11 - cut12
		80.f / 50.f, /*70.f*/
	};

	m_FinaleSeqCTime =
	{
		0.f,
		105.f / 50.f, //컷신 14
		55.f / 50.f, //컷신 15
		499.f / 50.f, //컷신 16
	};

	return S_OK;
}


void CCamera_Main::System_Tick(_float fTimeDelta)
{
	//이펙트 소켓 업데이트
	m_EffectSocket = _float4x4::Identity;
	CUtils::Set_State_Matrix(m_EffectSocket, CUtils::STATE_POSITION, m_pTransformCom->Get_State(CTransform::STATE_POSITION));

}

void CCamera_Main::Check_FinaleScene(_float fTimeDelta)
{

	CFinaleCut_ControlCenter* pCenter =
		static_cast<CFinaleCut_ControlCenter*>(m_pGameInstance->Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinaleCut_ControlCenter")));
	if (nullptr == pCenter)
		return;


	if (m_bChangedScene
		&& (m_iCurSceneIdx != QTE1 && m_iCurSceneIdx != QTE2 && m_iCurSceneIdx != QTE3))
	{
		Make_Sequence(CAMSEQ((_uint)SEQ_FINALECUT1 + m_iCurSceneIdx - 1));
		Deferred_Blackoperation(CAMSEQ((_uint)SEQ_FINALECUT1 + m_iCurSceneIdx - 1));
	}

}

void CCamera_Main::Deferred_Blackoperation(CAMSEQ eSEQ)
{
	if (eSEQ == SEQ_FINALECUT2) {
		m_pGameInstance->Set_ObjectBlack(1.f);
	}
	else if (eSEQ == SEQ_FINALECUT3) {
		m_pGameInstance->Set_ObjectBlack(0.5f);
	}
	else if (eSEQ == SEQ_FINALECUT4) {
		m_pGameInstance->Set_ObjectBlack(0.4f);
	}
	else if (eSEQ == SEQ_FINALECUT5) {
		m_pGameInstance->Set_ObjectBlack(0.6f);
	}
	else if (eSEQ == SEQ_FINALECUT6) {
		m_pGameInstance->Set_ObjectBlack(0.6f);
	}
	else if (eSEQ == SEQ_FINALECUT7) {
		m_pGameInstance->Set_ObjectBlack(1.f);
	}
	else if (eSEQ == SEQ_FINALECUT8) {
		m_pGameInstance->Set_ObjectBlack(0.3f);
	}
	else if (eSEQ == SEQ_FINALECUT9) {
		m_pGameInstance->Set_ObjectBlack(0.3f);
	}
	else if (eSEQ == SEQ_FINALECUT10) {
		m_pGameInstance->Set_ObjectBlack(1.f, 0.f);
	}
	else if (eSEQ == SEQ_FINALECUT11) {
		m_pGameInstance->Set_ObjectBlack(0.5f, 0.f);
	}
	else if (eSEQ == SEQ_FINALECUT12) {
		m_pGameInstance->Set_ObjectBlack(1.f);
	}
	else if (eSEQ == SEQ_FINALECUT13) {
		m_pGameInstance->Set_ObjectBlack(1.f);
	}
	else if (eSEQ == SEQ_FINALECUT14) {
	}
	else if (eSEQ == SEQ_FINALECUT15) {
		m_pGameInstance->Set_ObjectBlack(1.f);
	}
	else if (eSEQ == SEQ_FINALECUT16) {
		m_pGameInstance->Set_ObjectBlack(1.f);
	}
	else if (eSEQ == SEQ_FINALECUT17) {
		m_pGameInstance->Set_ObjectBlack(1.f);
	}
	else if (eSEQ == SEQ_FINALECUT18) {
		m_pGameInstance->Set_ObjectBlack(1.f);
	}
	else if (eSEQ == SEQ_FINALECUT19) {
		m_pGameInstance->Set_ObjectBlack(0.3f);
	}
}

void CCamera_Main::Check_FinaleTime(_float fTimeDelta)
{
	CFinaleCut_ControlCenter* pCenter =
		static_cast<CFinaleCut_ControlCenter*>(m_pGameInstance->Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinaleCut_ControlCenter")));
	if (nullptr == pCenter)
		return;

	m_bChangedScene = false;

	////이전 인덱스 갱신
	m_iPreSceneIdx = m_iCurSceneIdx;

	//현재 인덱스 확인
	m_iCurSceneIdx = pCenter->Get_CutScene();

	if (m_iPreSceneIdx == QTE3 + 1 && m_iCurSceneIdx == QTE3 + 2)
	{
		Make_Sequence(SEQ_FINALECUT20);


		CEffect::FX_DESC FxDesc{};
		if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_finale rect A"), &FxDesc)))
			return;
		if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_finale rect C"), &FxDesc)))
			return;
		//CEffect::FX_DESC FxDesc{};
		//FxDesc.vInitPos = (_float3)GET_POS + (_float3)(m_pTransformCom->Get_State(CTransform::STATE_LOOK) * 150.f);
		FxDesc.vInitPos = BATTLE_POS + _float3(0.f, 0.f, -50.f);
		FxDesc.vInitRot = CUtils::Make_Degree_FromDir({ 0.f, 0.f, -1.f });
		//FxDesc.pSocketMatrix = &m_EffectSocket;

		if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_finale rect B"), &FxDesc)))
			return;
		//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_finale rect B"), &FxDesc)))

		return;
	}

	//시퀀스 a 시간 체크
	if (1 <= m_iCurSceneIdx && !m_FinaleSeqATime.empty())
	{
		m_FinaleSeqATime.front() -= fTimeDelta;

		//m_fSeqPlayedTime = m_FinaleSeqATime.front();

		if (m_FinaleSeqATime.front() <= 0.f)
		{
			m_FinaleSeqATime.pop_front();

			//처음 세팅은 패스
			if (m_iPreSceneIdx == 0 && m_iCurSceneIdx == 1)
			{
				Make_Sequence(SEQ_FINALECUT1);
			}
			else/* if (!m_FinaleSeqATime.empty())*/
			{
				m_iCurSceneIdx++;
				pCenter->Set_CutScene(m_iCurSceneIdx);
				m_bChangedScene = true;
			}

		}

	}

	if (7 <= m_iCurSceneIdx && !m_FinaleSeqBTime.empty())
	{
		m_FinaleSeqBTime.front() -= fTimeDelta;
		//m_fSeqPlayedTime = m_FinaleSeqBTime.front();

		if (m_FinaleSeqBTime.front() <= 0.f)
		{
			m_FinaleSeqBTime.pop_front();

			//처음 세팅은 패스
			if (m_iPreSceneIdx == QTE1 && m_iCurSceneIdx == QTE1 + 1)
			{
				Make_Sequence(SEQ_FINALECUT7);
			}
			else if (m_iCurSceneIdx != QTE2)
			{
				m_iCurSceneIdx++;
				pCenter->Set_CutScene(m_iCurSceneIdx);
				m_bChangedScene = true;

			}

		}

	}

	if (14 <= m_iCurSceneIdx && !m_FinaleSeqCTime.empty())
	{
		m_FinaleSeqCTime.front() -= fTimeDelta;
		//m_fSeqPlayedTime = m_FinaleSeqCTime.front();

		if (m_FinaleSeqCTime.front() <= 0.f)
		{
			m_FinaleSeqCTime.pop_front();

			//처음 세팅은 패스
			if (m_iPreSceneIdx == QTE2 && m_iCurSceneIdx == QTE2 + 1)
			{
				Make_Sequence(SEQ_FINALECUT14);
			}
			else if (m_iCurSceneIdx != QTE3)
			{
				m_iCurSceneIdx++;
				pCenter->Set_CutScene(m_iCurSceneIdx);
				m_bChangedScene = true;

			}

		}
	}
}


void CCamera_Main::Fill_HardCutSet(CAMACTION& Action, _float fTime)
{
	Action.eCamCut = CUT_HARD;
	Action.fTime = fTime;
}

void CCamera_Main::Fill_InterpolateCutSet(CAMACTION& Action, _float fTime, EASING eEase, _float fInterpolateSpeed)
{
	Action.eCamCut = CUT_INTERPOLATE;
	Action.fTime = fTime;
	Action.eEase = eEase;
	Action.fInterpolateSpeed = fInterpolateSpeed;
}

void CCamera_Main::Fill_ActionPos(CAMACTION& Action, CAMPOS eCamPos, _float3 vPos)
{
	Action.eCamPos = eCamPos;
	Action.vPos = vPos;
}

void CCamera_Main::Fill_ActionDir(CAMACTION& Action, CAMDIR eCamDir, _float3 vDir)
{
	Action.eCamDir = eCamDir;
	Action.vDir = vDir;
}

// 임시입니다.
void CCamera_Main::Change_LevelTrigger()
{
	static _bool bOnceFade = false;
	static _bool bOnceChangeLevel = false;
	_float4 vPos = GET_POS;
	if (vPos.y >= 95.f)
	{
		CGameObject* pUIObj = m_pGameInstance->Get_GameObject_ByTag(LEVEL_STATIC, TEXT("Layer_ChangerUI"), TEXT("Prototype_GameObject_UI_Fading"));
		CUI_Fading* pFadingUI = static_cast<CUI_Fading*>(pUIObj);
		if (bOnceFade == false)
		{
			pFadingUI->Set_InOutState(CUI_Fading::FADEOUT);
			pFadingUI->Set_IsRender(true);
			bOnceFade = true;
		}
		else if (pFadingUI->Get_FadeRatio() <= 0.f)
		{
			if (bOnceChangeLevel == false)
			{
				m_pGameInstance->Reserve_Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_DEEDEEDEE));
				bOnceChangeLevel = true;
			}
		}
	}
}

_int CCamera_Main::Tick(_float fTimeDelta)
{

	//타임 델타를 보정한다.
	_float fRealTimeDelta = fTimeDelta;
	if (.1f < fRealTimeDelta)
		fRealTimeDelta = 1.f / 60.f;

	System_Tick(fTimeDelta);

	Check_FinaleScene(fTimeDelta);

	Control(fRealTimeDelta);

	//후보정 카메라 설정 값을 초기화한다.
	Reset_DeferredCamSet();


	//카메라 lock 되어 있는 경우, 다르게 계산
	if (m_eCamLockMode != LOCK_END)
		Compute_Set_CamLock(fRealTimeDelta);
	else
	{
		m_eSpecialSeq == SEQ_END ?
			//실제 타겟을 기준으로 업데이트하는 경우
			Track_Anchor(fRealTimeDelta) :
			//특정 시퀀스가 세팅되어 있는 경우
			Play_Sequence(fRealTimeDelta);
	}

	//후보정
	Set_DeferredCamSet(fRealTimeDelta);


	// 임시입니다.
	if (*m_pCurrentLevelID == LEVEL_RACING)
		Change_LevelTrigger();

	return OBJ_NOEVENT;
}

void CCamera_Main::Late_Tick(_float fTimeDelta)
{
	Check_FinaleTime(fTimeDelta);
}

//타겟 위치로부터 카메라 위치를 갱신, 보간한다.
void CCamera_Main::Track_Anchor(_float fTimeDelta)
{

	if (nullptr == m_pFirstTarget)
		return;

	//**** 타겟 위치를 만듬 ****//
	Update_Anchor(fTimeDelta);


	//**** 카메라 방향 설정 ****//

	// 두 타겟을 잡을 때의 설정
	if (m_eCamFocus == FOCUS_BOTH)
		Compute_Set_BothFocus(fTimeDelta);
	else if (m_eCamFocus == FOCUS_BATTLE)
		Compute_Set_BattleFocus(fTimeDelta);
	//트리거 안에 들어가 있을 경우 트리거 사이에서의 카메라 설정
	else if (m_bLerpByTriggerInfo)
		Compute_Set_Trigger(m_iMatrixIndex);


	/////Dest 값 설정 끝

	//**** 카메라 세팅 값 보간 ****//
	Interpolate_CamSet(fTimeDelta);


	//**** 목표 위치 마지막 저장 ****//
	Update_CurCamPos(fTimeDelta);


	//**** 목표 위치로 이동 ****//
	MoveTo_CurCamPos_Interpolate(fTimeDelta);

}


void CCamera_Main::Play_Sequence(_float fTimeDelta)
{

	if (m_eSpecialSeq == SEQ_END)
		return;

	//시퀀스에서 특수한 이벤트를 체크하기 위한 시간
	if (0.f < m_fSeqEventTime)
	{
		m_fSeqEventTime -= fTimeDelta;

		//이벤트 충간 체크
		switch (m_eSpecialSeq)
		{
		case SEQ_LUNCHTIME:
		{
			//4초 남았을 시
			if (m_SeqEventTriggers[0] == true && m_fSeqEventTime < 4.f)
			{
				m_SeqEventTriggers[0] = false;
				m_pGameInstance->Restore_FirstTimer(.1f);

				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_lunch time logo test"))))
					return;
			}
		}
		break;
		case SEQ_FINALBOSS_APPEAR:
			if (m_SeqEventTriggers[0] == true && m_fSeqEventTime < 3.f)
			{
				m_SeqEventTriggers[0] = false;
				m_pGameInstance->Set_ObjectBlack(.3f);
			}

			if (m_SeqEventTriggers[1] == true && m_fSeqEventTime < 1.f)
			{
				m_SeqEventTriggers[1] = false;

			}
			break;
		default:
			break;
		}

		//시퀀스 시간 다 깠았다~~
		if (m_fSeqEventTime < 0.f)
		{
			m_fSeqEventTime = 0.f;

			switch (m_eSpecialSeq)
			{
				//case SEQ_BREAKCARSHOP:
			case SEQ_BREAKRACINGMAP:
			{
				m_pGameInstance->StopSound(CHANNEL_BGM_STREAMING);
				m_pGameInstance->PlayBGM(CHANNEL_BGM_STREAMING, L"Welcome to the New World!.mp3");
				m_pGameInstance->SetVolume(CHANNEL_BGM_STREAMING, 0.5f);
			}
			break;

			case SEQ_PARTTIMESTART:
			{
				CPartTimeHelper::Get_Instance()->Handle_GameStart();
				Lock_All({ 16.4f, 25.7f, 35.75f }, { .16f, -.08f, -1.f });
				Set_FOVY(43.f);
			}

			break;
			case SEQ_LUNCHTIME:
			{
				Lock_All({ 16.4f, 25.7f, 35.75f }, { .16f, -.08f, -1.f });
				m_pGameInstance->Restore_SecondTimer(.1f);
			}
			break;

			case SEQ_FINALBOSS_APPEAR:
			{
				CEventCenter::Get_Instance()->Notify(KEVENT_FINALBOSS_APPEAR, this);

				//갓레이 세팅, state 변경

				m_pGameInstance->Setting_GodRay({ 0.f, 1550.f, 2000.f, 1.f });

				CFinalBoss* pFinalBoss = dynamic_cast<CFinalBoss*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_BossMonster")));
				if (pFinalBoss != nullptr)
				{
					Set_Target(pFinalBoss->Get_TransformCom(), TARGET_SECOND, FOCUS_BOTH, { 0.f, -2.f, 0.f });
				}
			}
			break;
			case SEQ_FINALESTART:
			{
				m_fCurShakeTime = m_fInitialShakeTime = 0.f;
			}
			break;
			case SEQ_FINALECUT5:
			{
				Lock_All({ 2057.f, 24.5f, -136.f }, { 1.f, .08f, -.12f });
				Unlock();
			}
			break;
			default:
				break;
			}

			fill(m_SeqEventTriggers.begin(), m_SeqEventTriggers.end(), true);

		}
	}

	//시퀀스 경과 시간 체크
	m_fSeqCheckTime += fTimeDelta;


	//예약 동작이 모두 끝나면 다시 기본 상태로 만든다.
	if (m_CamSeq.empty() && abs(m_fSeqInterpolateTime.first - m_fSeqInterpolateTime.second) < .01f)
	{

		Set_DOFMode(false);

		CAMSEQ eSeq = m_eSpecialSeq;

		m_eSpecialSeq = SEQ_END;
		m_eCurSeqEase = EASE_END;
		m_fSeqInterpolateTime = { 0.f, 0.f };
		m_fSeqTotalTime = m_fSeqCheckTime;

		m_vDestCamDir = m_vCurCamDir;
		m_fDestFovy = m_fFovy;
		m_fDestZAngle = m_fCurZAngle;
		m_fDestZoomOffset = m_fCurZoomOffset;

		m_fDestDistance = m_fCurDistance = _float3::Distance(F4toF3(m_pFirstTarget->Get_State(CTransform::STATE_POSITION)), GET_POS);

		//끝나고 목표 위치로 딱 맞춰주기
		if (eSeq == SEQ_SIMBA_BATTLESTART
			/*|| eSeq == SEQ_FINALECUT5*/)
		{
			//카메라 세팅 스냅
			Snap_CamSet(fTimeDelta);
			//카메라가 갈 포지션 업데이트
			Update_CurCamPos(fTimeDelta);
			//바로 이동
			MoveTo_CurCamPos_Absolute(fTimeDelta);
		}

		return;
	}

	//예약 리스트의 잔여 시간을 모두 깎는다.
	if (!m_CamSeq.empty())
	{
		for (auto& seqKey : m_CamSeq)
		{
			seqKey.fTime -= fTimeDelta;
		}

		//시간이 다 된, 앞쪽에 있는 동작 정보를 읽는다.
		if (m_CamSeq.front().fTime <= 0.f)
		{
			CAMACTION curAction = m_CamSeq.front();

			Update_Anchor(fTimeDelta);

			if (curAction.eCamCut == CUT_HARD)
			{
				m_eCamCut = CUT_HARD;
				if (!ISDEFAULTFLOAT3(curAction.vPos))
				{
					if (curAction.eCamPos == POS_RELATIVE)
					{
						_float4x4 ToWorldMatrix = m_pFirstTarget->Get_WorldFloat4x4();

						_float4 vRight = ToWorldMatrix.Right();
						_float4 vUp = ToWorldMatrix.Up();
						_float4 vLook = ToWorldMatrix.Backward();
						vRight.Normalize();
						vUp.Normalize();
						vLook.Normalize();
						CUtils::Set_State_Matrix(ToWorldMatrix, CUtils::STATE_RIGHT, Dir(vRight));
						CUtils::Set_State_Matrix(ToWorldMatrix, CUtils::STATE_UP, Dir(vUp));
						CUtils::Set_State_Matrix(ToWorldMatrix, CUtils::STATE_LOOK, Dir(vLook));

						_float3 vWorldPos = _float3::Transform(curAction.vPos, ToWorldMatrix);
						SET_POS(Pos(vWorldPos));
					}
					else
						SET_POS(Pos(curAction.vPos));
				}

				// 카메라 액션의 x, y, z가 모두 default일 경우, 타겟 위치를 곧바로 쳐다본다.
				if (ISDEFAULTFLOAT3(curAction.vDir))
				{
					m_pTransformCom->Look_At(Pos(m_vAnchor));
					m_vDestCamDir = m_vCurCamDir = (_float3)m_pTransformCom->Get_State(CTransform::STATE_LOOK);
				}
				//default가 아닐 경우, 설정된 방향으로 쳐다본다.
				else
				{
					m_pTransformCom->Look_At_Axis(curAction.vDir);
					m_vDestCamDir = m_vCurCamDir = curAction.vDir;
				}

				if (!ISDEFAULTFLOAT(curAction.fFOVY))
					m_fFovy = m_fDestFovy = ToRadian(curAction.fFOVY);

				if (!ISDEFAULTFLOAT(curAction.fZAngle))
					m_fDestZAngle = m_fCurZAngle = curAction.fZAngle;
				else
					m_fDestZAngle = m_fCurZAngle = 0.f;

				if (!ISDEFAULTFLOAT(curAction.fZoomOffset))
					m_fDestZoomOffset = m_fCurZoomOffset = curAction.fZoomOffset;
			}
			else
			{
				m_eCamCut = CUT_INTERPOLATE;
				m_eCurSeqEase = curAction.eEase;
				//보간 시간을 계산할 친구를 초기화해준다.
				m_fSeqInterpolateTime = { 0.f, curAction.fInterpolateSpeed };

				//position O
				if (!ISDEFAULTFLOAT3(curAction.vPos))
				{
					if (curAction.eCamPos == POS_RELATIVE)
					{
						_float4x4 ToWorldMatrix = m_pFirstTarget->Get_WorldFloat4x4();

						_float4 vRight = ToWorldMatrix.Right();
						_float4 vUp = ToWorldMatrix.Up();
						_float4 vLook = ToWorldMatrix.Backward();
						vRight.Normalize();
						vUp.Normalize();
						vLook.Normalize();
						CUtils::Set_State_Matrix(ToWorldMatrix, CUtils::STATE_RIGHT, Dir(vRight));
						CUtils::Set_State_Matrix(ToWorldMatrix, CUtils::STATE_UP, Dir(vUp));
						CUtils::Set_State_Matrix(ToWorldMatrix, CUtils::STATE_LOOK, Dir(vLook));

						_float3 vWorldPos = _float3::Transform(curAction.vPos, ToWorldMatrix);

						m_vDestCamPos = vWorldPos;
					}
					else
						m_vDestCamPos = curAction.vPos;

					m_vStartCamPos = GET_POS;
				}
				else
				{
					m_vDestCamPos = m_vStartCamPos = GET_POS;
				}

				// 카메라 목표 방향 값이 default일 경우와 아닐 경우를 구별한다.
				m_bSeqDestDirIsAbsolute = ISDEFAULTFLOAT3(curAction.vDir) ? false : true;

				m_vStartCamDir = (_float3)m_pTransformCom->Get_State(CTransform::STATE_LOOK);
				m_vDestCamDir = curAction.vDir;

				if (!ISDEFAULTFLOAT(curAction.fFOVY))
				{
					m_fStartFovy = m_fFovy;
					m_fDestFovy = ToRadian(curAction.fFOVY);
				}
				else
				{
					m_fDestFovy = m_fStartFovy = m_fFovy;
				}

				if (!ISDEFAULTFLOAT(curAction.fZAngle))
				{
					m_fStartZAngle = m_fCurZAngle;
					m_fDestZAngle = curAction.fZAngle;
				}
				else
					m_fDestZAngle = m_fStartZAngle = 0.f;

				if (!ISDEFAULTFLOAT(curAction.fZoomOffset))
				{
					m_fStartZoomOffset = m_fCurZoomOffset;
					m_fDestZoomOffset = curAction.fZoomOffset;
				}
			}

			m_CamSeq.pop_front();
		}
	}


	if (m_eCamCut != CUT_INTERPOLATE)
		return;


	//보간 카메라
	if (m_fSeqInterpolateTime.first < m_fSeqInterpolateTime.second)
	{
		m_fSeqInterpolateTime.first += fTimeDelta;

		if (m_fSeqInterpolateTime.second < m_fSeqInterpolateTime.first)
			m_fSeqInterpolateTime.first = m_fSeqInterpolateTime.second;
	}

	_float fInterpolateRatio = SATURATE(m_fSeqInterpolateTime.first / m_fSeqInterpolateTime.second);

	switch (m_eCurSeqEase)
	{
	case EASE_IN:
		fInterpolateRatio = EASE_IN(fInterpolateRatio);
		break;
	case EASE_IN_FAST:
		fInterpolateRatio = EASE_IN_FAST(fInterpolateRatio);
		break;
	case EASE_OUT:
		fInterpolateRatio = EASE_OUT(fInterpolateRatio);
		break;
	case EASE_OUT_FAST:
		fInterpolateRatio = EASE_OUT_FAST(fInterpolateRatio);
		break;
	case EASE_INOUT:
		fInterpolateRatio = EASE_INOUT(fInterpolateRatio);
		break;
	case EASE_INOUT_FAST:
		fInterpolateRatio = EASE_INOUT_FAST(fInterpolateRatio);
		break;
	default: //그냥 Linear도 여기 포함
		break;
	}


	_float3 vCamPos = _float3::Lerp(m_vStartCamPos, m_vDestCamPos, fInterpolateRatio);

	_float3 vCamDir = m_pFirstTarget->Get_State(CTransform::STATE_POSITION) - (_float4)GET_POS;

	if (m_bSeqDestDirIsAbsolute)
		vCamDir = _float3::Lerp(m_vStartCamDir, m_vDestCamDir, fInterpolateRatio);

	vCamDir.Normalize();


	_float fFOVY = LERP(m_fStartFovy, m_fDestFovy, fInterpolateRatio);
	_float fZAngle = LERP(m_fStartZAngle, m_fDestZAngle, fInterpolateRatio);
	_float fZoomOffset = LERP(m_fStartZoomOffset, m_fDestZoomOffset, fInterpolateRatio);


	m_vCurCamPos = vCamPos;
	m_vCurCamDir = vCamDir;
	m_fFovy = fFOVY;
	m_fCurZAngle = fZAngle;
	m_fCurZoomOffset = fZoomOffset;

	MoveTo_CurCamPos_Absolute(fTimeDelta);


	//SET_POS(Pos(m_vCurCamPos));
	//m_pTransformCom->Look_At_Axis(m_vCurCamDir);

}

void CCamera_Main::Set_MatrixIndex(_int iMatrixIndex)
{
	if (nullptr == m_pTransformCom || m_vecCamMatrices.empty())
		return;

	if (iMatrixIndex < 0 || iMatrixIndex == m_iMatrixIndex || iMatrixIndex >= m_vecCamMatrices.size())
		return;

	m_pTransformCom->Set_WorldMatrix(m_vecCamMatrices[iMatrixIndex]);
	m_iMatrixIndex = iMatrixIndex;
}

void CCamera_Main::EmplaceBackDirRadius(_int iCamType, _fvector vDir, _float fRadius)
{
	if (CAM_FRONT == iCamType)
		m_vecFrontDirRadius.emplace_back(vDir, fRadius);
	else if (CAM_REAR == iCamType)
		m_vecRearDirRadius.emplace_back(vDir, fRadius);
}

void CCamera_Main::Compute_Set_BothFocus(_float fTimeDelta)
{
	_float3 vDir = _float3(m_pSecondTarget->Get_State(CTransform::STATE_POSITION) - m_pFirstTarget->Get_State(CTransform::STATE_POSITION));
	m_vDestCamDir = vDir;
	m_vDestCamDir.Normalize();

	m_vDestCamDir.y = m_vOrigCamDir.y;
	m_vDestCamDir.Normalize();

	_float fDist = vDir.Length();
	fDist = clamp(fDist, 20.f, 50.f);
	fDist = MAPVALUE(fDist, 20.f, 50.f, 0.f, 1.f);
	fDist = EASE_OUT(fDist);

	m_vDestCamDir.y += MAPVALUE(fDist, 0.f, 1.f, -.1f, .2f);
	m_vDestCamDir.Normalize();

	m_fDestDistance = m_fOrigDistance * .5f + (m_pFirstTarget->Get_State(CTransform::STATE_POSITION) - m_pSecondTarget->Get_State(CTransform::STATE_POSITION)).Length() * 1.2f /** .7f*/;

}

void CCamera_Main::Compute_Set_BattleFocus(_float fTimeDelta)
{

	_float3 vKirbyToBoss = FINALEBOSS->Get_RootPos() + _float4{ 0.f, 0.f, -10.f, 0.f } - FINALEKIRBY->m_vBonePos;
	_float3 vBossToKirby = FINALEKIRBY->m_vBonePos + _float4{ 0.f, 0.f, -10.f, 0.f } - FINALEBOSS->Get_RootPos();

	_float3 vCenterDir = _float3(0.f, 0.f, -1.f);


	_float fRatio{ .5f };


	_float3 vAxis = { -.3f, 1.f, 0.f };
	vAxis.Normalize();

	Quaternion vQuat = Quaternion::CreateFromAxisAngle(vAxis, ToRadian((m_fBothFocusRatio - .5f) * 140.f));
	_float3 vResultVector = _float3::Transform(vCenterDir, vQuat);


	m_vDestCamDir = vResultVector;
	m_vDestCamDir.Normalize();

	_float fDistRatio = (.5f < m_fBothFocusRatio) ?
		m_fBothFocusRatio * 2.f : 1.f - (m_fBothFocusRatio * 2.f - 1.f);

	m_fDestDistance = 30.f + (30.f * fDistRatio);

}

void CCamera_Main::Compute_Set_CamLock(_float fTimeDelta)
{
	if (m_eCamLockMode == LOCK_POS)
		Update_Anchor(fTimeDelta);

	//Interpolate_CamSet(fTimeDelta);
	if (.05f < _float3::Distance(m_vDestCamPos, m_vCurCamPos))
	{
		m_vCurCamPos += (m_vDestCamPos - m_vCurCamPos) * fTimeDelta * m_fInterpolateSpeed;
		if (_float3::Distance(m_vDestCamPos, m_vCurCamPos) < .05f)
			m_vCurCamPos = m_vDestCamPos;
	}

	MoveTo_CurCamPos_Interpolate(fTimeDelta);

}

void CCamera_Main::Compute_Set_Trigger(_int iTriggerIndex)
{
	if (nullptr == m_pTransformCom || m_vecFrontDirRadius.empty() || m_vecRearDirRadius.empty())
		return;

	if (m_iMatrixIndex < 0 || m_iMatrixIndex >= m_vecFrontDirRadius.size())
		return;

	_float vRatio = Compute_TriggerPosRatio(m_iMatrixIndex);

	if (vRatio < 0.f || 1.f < m_fTriggerRatio)
	{
		m_fTriggerRatio = -1.f;
		return;
	}

	m_fTriggerRatio = SATURATE(vRatio);

	//QZR  >> Y값 살짝 올리는거
	if (*m_pCurrentLevelID == LEVEL_PARK && 3 == m_iMatrixIndex)
	{
		//_float4 vPos = GET_POS;
		//SET_POS(_float4(vPos.x, vPos.y + 0.2f, vPos.z, 1.f));
	}
	//CGameObject* pKirby = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"), 0);
	//if (nullptr == pKirby)
	//	return;
	//CTransform* pKirbyTransform = static_cast<CTransform*>(pKirby->Get_Component(g_strTransformTag));
	//if (nullptr == pKirbyTransform)
	//	return;
	//_vector pKirbyPos = pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION);

	m_vSlerpedDir = _float3::Lerp(m_vecFrontDirRadius[m_iMatrixIndex].first, m_vecRearDirRadius[m_iMatrixIndex].first, m_fTriggerRatio);
	m_fLerpedRadius = LERP(m_vecFrontDirRadius[m_iMatrixIndex].second, m_vecRearDirRadius[m_iMatrixIndex].second, m_fTriggerRatio);

	m_vDestCamDir = m_vSlerpedDir;
	m_fDestDistance = m_fLerpedRadius;


	//카메라 보는 기준점 위로 올려주는 놈
	if (*m_pCurrentLevelID == LEVEL_INTRO
		|| *m_pCurrentLevelID == LEVEL_PARTTIME
		|| *m_pCurrentLevelID == LEVEL_FINALBOSS
		|| *m_pCurrentLevelID == LEVEL_FINALE
		)
	{
		m_fDestUpOffset = m_CamTriggerUpOffsets[*m_pCurrentLevelID][m_iMatrixIndex];
	}
	// = { 0.f, 0.f, 0.f, .15f, .15f, 0.f, 0.f, 0.f }
	//m_fDestUpOffset = m_CamTriggerUpOffsets[m_iMatrixIndex];
}

void CCamera_Main::EmplaceBackCamMatrix(const _float4x4& matWorld)
{
	_float4x4 mat = matWorld;
	m_vecCamMatrices.emplace_back(mat);
}

void CCamera_Main::EmplaceBackTriggerInfo(const _float4x4& matWorld, _float fScale)
{
	pair<_float4x4, _float> vPair = make_pair(matWorld, fScale);
	m_vecTriggerInfo.emplace_back(vPair);
}

_float CCamera_Main::Compute_TriggerPosRatio(_int iTriggerIndex)
{
	if (m_vecTriggerInfo.empty())
		return _float();


	_float3 vLocalTargetPos = Make_TargetPos();

	_float fZ = _float3::Transform(vLocalTargetPos, m_vecTriggerInfo[iTriggerIndex].first).z;

	// rear : 0, middle : 0.5, front: 1
	_float fRatio = 0.5f * fZ + 0.5f;
	return fRatio;
}

_vector CCamera_Main::SlerpDirVec(_fvector vStart, _fvector vEnd, _float fRatio)
{
	_float fDot = ::XMVectorGetX(::XMVector3Dot(vStart, vEnd));
	fDot = clamp(fDot, -1.f, 1.f);
	_float fTheta = acosf(fDot) * fRatio;
	_vector vRelative = ::XMVector3Normalize(vEnd - vStart * fRatio);

	return XMVector3Normalize(vStart * cosf(fTheta) + vRelative * sinf(fTheta));
}


//일련의 동작을 하나의 시퀀스로 선예약한다.
void CCamera_Main::Make_Sequence(CAMSEQ eSeq)
{
	if (SEQ_END <= eSeq)
		return;


	//기존 시퀀스가 있었다면, 지우기
	if (!m_CamSeq.empty())
	{
		//m_fSeqPlayedTime = m_fSeqCheckTime;
		m_CamSeq.clear();
	}

	//해당 시퀀스를 명시적으로 변수로 저장!
	m_eSpecialSeq = eSeq;
	m_fSeqCheckTime = 0.f;


	switch (eSeq)
	{
	case SEQ_HARDCUT_TEST:
	{
		CAMACTION newAction{};
		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_HARD;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ -8.f, 6.f, -8.f };
		newAction.vDir = _float3{ 0.f, -.2f, 1.f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = 1.4f;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 8.f, 6.f, -8.f };
		newAction.vDir = _float3{ 0.f, -.2f, 1.f };
		m_CamSeq.push_back(newAction);


		newAction = {};
		newAction.fTime = 2.f;
		newAction.eCamCut = CUT_HARD;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 0.f, 3.f, 15.f };
		newAction.vDir = _float3{ 0.f, -.2f, 1.f };
		m_CamSeq.push_back(newAction);
	}
	break;

	case SEQ_ZOOMINOUT:
	{

	}
	break;

	case SEQ_SOFTCUT_TEST:
	{
		CAMACTION newAction{};
		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = .5f;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 8.f, 6.f, -8.f };
		m_CamSeq.push_back(newAction);


		newAction = {};
		newAction.fTime = .5f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = .5f;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ -8.f, 6.f, -8.f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 1.f;
		newAction.eCamCut = CUT_HARD;
		//newAction.eEase = EASE_OUT;
		//newAction.fInterpolateSpeed =.5f;
		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 10.5f, 55.f,10.5f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 1.5f;
		newAction.eCamCut = CUT_HARD;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = .5f;
		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 40.f, 60.f, 10.f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 2.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = .5f;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 0.f, 3.f, 15.f };
		m_CamSeq.push_back(newAction);
	}
	break;

	case SEQ_BREAKCARSHOP:
	{
		CAMACTION newAction{};
		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_HARD;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 3.f, -4.f, 25.f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 0.1f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_INOUT;
		newAction.fInterpolateSpeed = 2.f;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 5.f, -12.f, 40.f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 2.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_INOUT_FAST;
		newAction.fInterpolateSpeed = 1.5f;

		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ -170.f, 23.17f, -153.f };

		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -0.73f, .2f, .67f };

		newAction.fZAngle = 10.f;
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 4.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_INOUT;
		newAction.fInterpolateSpeed = 1.f;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 30.f, 6.f, 0.f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ 0.f, -.2f, 1.f };
		newAction.fZAngle = 0.f;
		m_CamSeq.push_back(newAction);
	}
	break;

	case SEQ_BREAKRACINGMAP:
	{
		CAMACTION newAction{};
		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_HARD;
		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 66.8f, 26.3f, 46.3f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.26f, 0.f, 1.f };
		newAction.fZAngle = 0.f;
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 1.3f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_INOUT;
		newAction.fInterpolateSpeed = 2.5f;
		newAction.fFOVY = 20.f;
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.15f, .6f, .8f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 4.f;
		newAction.eCamCut = CUT_HARD;
		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 65.5f, 25.4f, 181.8f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.23f, .1f, -1.f };
		m_CamSeq.push_back(newAction);

		//다리 입구 뷰
		newAction = {};
		newAction.fTime = 8.f;
		newAction.eCamCut = CUT_HARD;
		newAction.fFOVY = 25.f;

		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 65.6f, 25.6f, 80.f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.45f, 0.f, -.9f };
		m_CamSeq.push_back(newAction);

		m_fSeqEventTime = 8.08f;

		//라디오 보기
		newAction = {};
		newAction.fTime = 10.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_INOUT;
		newAction.fInterpolateSpeed = .8f;

		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.8f, 0.f, -.6f };
		newAction.fFOVY = 20.f;
		m_CamSeq.push_back(newAction);


		//라디오 줌 인

		newAction = {};
		newAction.fTime = 11.f;
		newAction.eCamCut = CUT_HARD;
		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 57.f, 26.8f, 71.5f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.84f, -.14f, -.53f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 11.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_LINEAR;
		newAction.fInterpolateSpeed = 3.f;
		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 54.64f, 26.45f, 70.f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.84f, -.14f, -.53f };
		m_CamSeq.push_back(newAction);

		//다리 입구 뷰 복귀
		newAction = {};
		newAction.fTime = 14.f;
		newAction.eCamCut = CUT_HARD;
		newAction.fFOVY = 25.f;
		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 65.6f, 25.6f, 80.f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.8f, 0.f, -.6f };
		m_CamSeq.push_back(newAction);

		//커비 뷰
		newAction = {};
		newAction.fTime = 15.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_INOUT;
		newAction.fInterpolateSpeed = 1.5f;
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.45f, 0.f, -.9f };
		m_CamSeq.push_back(newAction);

		//시동
		newAction = {};
		newAction.fTime = 19.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_INOUT;
		newAction.fInterpolateSpeed = 1.f;
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.3f, 0.f, -.95f };
		m_CamSeq.push_back(newAction);

		//다리 너머
		newAction = {};
		newAction.fTime = 20.1f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = 4.f;
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.24f, 0.f, 1.2f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 26.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_INOUT;
		newAction.fInterpolateSpeed = 10.f;
		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 65.6f, 95.1f, 80.f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.24f, 0.5f, 1.f };
		m_CamSeq.push_back(newAction);


		CEffect::FX_DESC FXDesc{};
		FXDesc.fStartDelay = { 28.f };
		if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Kirby Title Logo"), &FXDesc)))
			return;

		newAction = {};
		newAction.fTime = 60.f;
		newAction.eCamCut = CUT_HARD;
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.24f, 0.5f, 1.f };
		m_CamSeq.push_back(newAction);

	}
	break;

	case SEQ_PARTTIMESTART:
	{
		//이벤트 호출
		m_fSeqEventTime = 7.f;

		CAMACTION newAction = {};

		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_HARD;
		newAction.fFOVY = 30.f;
		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 19.6f, 30.5f, 12.2f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.15f, 0.f, 1.f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_INOUT_FAST;
		newAction.fInterpolateSpeed = 7.f;

		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 19.6f, 26.f, 12.2f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.15f, 0.f, 1.f };
		m_CamSeq.push_back(newAction);

	}
	break;

	case SEQ_LUNCHTIME:
	{
		//이벤트 호출
		m_fSeqEventTime = 5.f;

		CAMACTION newAction = {};
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, 1.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, { 16.4f, 25.7f, 25.75f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { .16f, -.08f, -1.f });
		m_CamSeq.push_back(newAction);

		Fill_InterpolateCutSet(newAction, 4.f, EASE_INOUT, 1.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, { 16.4f, 25.7f, 35.75f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { .16f, -.08f, -1.f });
		m_CamSeq.push_back(newAction);

		Fill_InterpolateCutSet(newAction, 5.f, EASE_INOUT, .5f);
		m_CamSeq.push_back(newAction);

	}
	break;

#pragma region 사자
	//통 보여주고 줌아웃
	case SEQ_SIMBA_START:
	{
		CAMACTION newAction{};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, { 1.f, 20.4f, -41.f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { -.1f, .1f, 1.f });
		newAction.fFOVY = 40.f;
		m_CamSeq.push_back(newAction);


		//
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, 2.f);
		Fill_ActionPos(newAction, POS_ABSOLUTE, { -1.f, 20.4f, -41.f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { .1f, .1f, 1.f });

		m_CamSeq.push_back(newAction);

		//
		Fill_InterpolateCutSet(newAction, 2.f, EASE_INOUT_FAST, 1.f);
		Fill_ActionPos(newAction, POS_ABSOLUTE, { .3f, 4.6f, -99.4f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { 0.f, .05f, 1.f });
		m_CamSeq.push_back(newAction);

		Fill_InterpolateCutSet(newAction, 3.f, EASE_INOUT, 30.f);
		m_CamSeq.push_back(newAction);

	}
	break;
	//심바 배틀 start
	case SEQ_SIMBA_BATTLESTART:
	{
		CAMACTION newAction{};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, { 11.25f, 6.4f, -41.6f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { -.64f, .07f, .76f });
		m_CamSeq.push_back(newAction);


		//
		Fill_InterpolateCutSet(newAction, 0.f, EASE_LINEAR, 2.f);
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { -.64f, .07f, .76f });
		newAction.vDir.Normalize();

		newAction.vPos -= newAction.vDir * 2.f;
		m_CamSeq.push_back(newAction);

		// 망토 잡기
		Fill_HardCutSet(newAction, 5.5f);
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { 0.f, .12f, 1.f });
		Fill_ActionPos(newAction, POS_ABSOLUTE, { .7f, 9.5f, -53.f });
		newAction.fFOVY = 30.f;
		m_CamSeq.push_back(newAction);

		Fill_InterpolateCutSet(newAction, 5.5f, EASE_OUT_FAST, .7f);
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { 0.f, .12f, 1.f });
		Fill_ActionPos(newAction, POS_ABSOLUTE, { .7f, 9.5f, -49.f });
		m_CamSeq.push_back(newAction);

		// 망토 던지기
		Fill_InterpolateCutSet(newAction, 6.9f, EASE_INOUT_FAST, .5f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, { 0.f, .12f, 1.f });
		Fill_ActionPos(newAction, POS_ABSOLUTE, { .7f, 8.3f, -54.3f });
		newAction.fFOVY = 40.f;
		m_CamSeq.push_back(newAction);

		Fill_InterpolateCutSet(newAction, 7.4f, EASE_INOUT, 1.2f);
		m_CamSeq.push_back(newAction);

	}

	break;
	//어깨 뷰
	case SEQ_SIMBA_SHOULDER:
	{
		Set_DOFMode(false);

		CAMACTION newAction{};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, { 11.25f, 6.4f, -41.6f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { -.64f, .07f, .76f });
		newAction.fFOVY = 30.f;
		m_CamSeq.push_back(newAction);

		//
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, 3.f);
		Fill_ActionPos(newAction, POS_ABSOLUTE, { 12.f, 7.5f, -41.8f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { -.68f, .14f, .72f });
		m_CamSeq.push_back(newAction);

		Fill_InterpolateCutSet(newAction, 3.f, EASE_INOUT, 20.f);
		m_CamSeq.push_back(newAction);

	}
	break;
	//어깨 - 통
	case SEQ_SIMBA_TONG:
	{
		Set_DOFMode(false);

		CAMACTION newAction{};
		Fill_HardCutSet(newAction, 0.f);
		Fill_ActionPos(newAction, POS_ABSOLUTE, { 12.f, 7.5f, -41.8f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { -.68f, .14f, .72f });
		m_CamSeq.push_back(newAction);
		newAction.fFOVY = 30.f;

		//
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, 3.f);
		Fill_ActionPos(newAction, POS_ABSOLUTE, { 10.21f, 8.9f, -42.f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { -.41f, .13f, .9f });
		m_CamSeq.push_back(newAction);

		Fill_InterpolateCutSet(newAction, 3.f, EASE_INOUT, 20.f);
		m_CamSeq.push_back(newAction);
	}
	break;
	//얼굴뷰
	case SEQ_SIMBA_FRONTVIEW:
	{
		Set_DOFMode(false);

		CAMACTION newAction{};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, { 0.f, 5.8f, -46.f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { 0.f, .13f, 1.f });
		newAction.fFOVY = 30.f;
		m_CamSeq.push_back(newAction);

		//
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, 3.f);
		Fill_ActionPos(newAction, POS_ABSOLUTE, { 0.f, 8.6f, -46.f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { 0.f, .13f, 1.f });
		m_CamSeq.push_back(newAction);

		Fill_InterpolateCutSet(newAction, 3.f, EASE_INOUT, 20.f);
		m_CamSeq.push_back(newAction);
	}
	break;
	//로우 앵글
	case SEQ_SIMBA_LOW:
	{
		Set_DOFMode(false);

		CAMACTION newAction{};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, { 9.52f, 2.77f, -50.38f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { -.42f, .37f, .83f });
		newAction.vDir.Normalize();
		newAction.fFOVY = 30.f;
		m_CamSeq.push_back(newAction);

		//
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, 2.f);
		newAction.vPos -= newAction.vDir;
		m_CamSeq.push_back(newAction);

		Fill_InterpolateCutSet(newAction, 2.f, EASE_INOUT, 20.f);
		m_CamSeq.push_back(newAction);
	}
	break;

#pragma endregion

#pragma region 에피리스
	case SEQ_FINALBOSS_APPEAR:
	{
		Set_DOFMode(false);

		CAMACTION newAction = {};

		//탑 앵글
		_float3 vStartPos = { 41.6f, 43.f, 18.7f };
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos);
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { -.65f, -.71f, -.28f });
		m_CamSeq.push_back(newAction);

		//
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, 3.f);
		m_CamSeq.push_back(newAction);


		//커비 뒤
		vStartPos = { 0.f, 6.4f, -56.f };
		newAction.fFOVY = 40.f;
		Fill_HardCutSet(newAction, 3.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos);
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { 0.f, .15f, 1.f });
		m_CamSeq.push_back(newAction);

		//
		Fill_InterpolateCutSet(newAction, 3.f, EASE_INOUT, 3.f);
		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos + _float3{ 0.f, 0.f, -3.f });
		m_CamSeq.push_back(newAction);


		//사이드 앵글
		vStartPos = { 61.f, 7.f, -14.f };
		Fill_HardCutSet(newAction, 6.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos);
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { -.95f, .12f, .29f });
		m_CamSeq.push_back(newAction);

		_float fDuration{ 2.9f };
		//
		Fill_InterpolateCutSet(newAction, 6.f, EASE_INOUT, fDuration);
		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos + _float3{ 0.f, 0.f, 3.f });
		m_CamSeq.push_back(newAction);



		_float fLastSeqStartTime{ 6.f + fDuration };

		//에피리스 애니메이션 교체, target set
		m_fSeqEventTime = fLastSeqStartTime;

		//보스 정면으로
		vStartPos = { 0.f, 35.f, -3.f };
		Fill_HardCutSet(newAction, fLastSeqStartTime);

		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos);
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { 0.f, .2f, 1.f });
		m_CamSeq.push_back(newAction);


		//
		Fill_InterpolateCutSet(newAction, fLastSeqStartTime, EASE_INOUT, 5.f);
		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos + _float3{ 0.f, -5.f, -5.f });
		m_CamSeq.push_back(newAction);

		Fill_InterpolateCutSet(newAction, fLastSeqStartTime + 5.f, EASE_INOUT, 8.f);
		m_CamSeq.push_back(newAction);

	}
	break;
#pragma endregion

#pragma region 피날레

	case SEQ_FINALESTART:
	{
		//이벤트 호출
		m_fSeqEventTime = 7.15f;

		_float fCutStartTime = 0.f;

		CAMACTION newAction = {};

		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_HARD;
		newAction.fFOVY = 45.f;
		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ -22.37f, 6.9f, -10.5f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ .96f, -.11f, .26f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_INOUT;
		newAction.fInterpolateSpeed = 1.5f;

		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ -14.57f, 3.9f, -7.25f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ .95f, -.02f, .3f };
		m_CamSeq.push_back(newAction);


		newAction = {};
		newAction.fTime = 1.5f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_INOUT;
		newAction.fInterpolateSpeed = 2.f;

		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ -10.17f, 2.f, -5.87f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ .93f, .05f, .35f };
		m_CamSeq.push_back(newAction);

		//Cut 2
		fCutStartTime = 5.1f;

		newAction.fTime = fCutStartTime;
		newAction.eCamCut = CUT_HARD;

		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 41.81f, .56f, -3.f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.96f, .13f, .23f };
		m_CamSeq.push_back(newAction);


		newAction = {};
		newAction.fTime = fCutStartTime;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_LINEAR;
		newAction.fInterpolateSpeed = 2.f;

		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 11.17f, .71f, -1.97f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.94f, .12f, .31f };
		m_CamSeq.push_back(newAction);

		//Cut 3
		fCutStartTime = 7.15f;

		newAction.fTime = fCutStartTime;
		newAction.eCamCut = CUT_HARD;
		newAction.fFOVY = 40.f;

		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 2.57f, 3.63f, -7.63f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.72f, -.14f, .68f };
		m_CamSeq.push_back(newAction);

		//땡겨짐
		newAction = {};
		newAction.fTime = fCutStartTime;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = 1.5f;
		newAction.fFOVY = 55.f;

		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 4.64f, 2.74f, -5.56f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.86f, .08f, .5f };
		m_CamSeq.push_back(newAction);


		newAction = {};
		newAction.fTime = fCutStartTime + 2.3f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = .5f;
		newAction.fFOVY = 45.f;
		newAction.fZAngle = 30.f;
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.84f, .31f, .44f };
		m_CamSeq.push_back(newAction);

		//쾅
		newAction = {};
		newAction.fTime = fCutStartTime + 3.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_IN;
		newAction.fInterpolateSpeed = .5f;
		newAction.fFOVY = 48.f;
		newAction.fZAngle = 0.f;
		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 4.64f, 2.44f, -5.56f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.86f, .08f, .5f };
		m_CamSeq.push_back(newAction);


		newAction = {};
		newAction.fTime = fCutStartTime + 3.8f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = 1.3f;
		newAction.fFOVY = 55.f;
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ -.86f, .08f, .5f };
		m_CamSeq.push_back(newAction);

		//Cut 4

		newAction = {};
		Fill_HardCutSet(newAction, fCutStartTime + 5.3f);

		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ .89f, -.45f, 0.f };
		newAction.vDir.Normalize();


		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = -newAction.vDir * 25.f;

		newAction.fFOVY = 55.f;
		m_CamSeq.push_back(newAction);

	}
	break;

	case SEQ_FINALECUT1:
	{
		//1808.f, -41.6f, -136.9f
		_float3 vKirbyStartPos = _float3{ 1808.f, -41.6f, -136.9f };

		//1
		CAMACTION newAction = {};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE,
			vKirbyStartPos + _float3{ -40.f, -10.f, 0.f });

		Fill_ActionDir(newAction, DIR_ABSOLUTE,
			{ .96f, -.11f, 0.f });

		newAction.fFOVY = 45.f;
		newAction.fZAngle = 0.f;
		m_CamSeq.push_back(newAction);


		//2
		newAction = {};
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, 2.4f);

		Fill_ActionPos(newAction, POS_ABSOLUTE,
			vKirbyStartPos + _float3{ -60.f, 10.f, 0.f });

		Fill_ActionDir(newAction, DIR_ABSOLUTE,
			{ .96f, .31f, 0.f });

		m_CamSeq.push_back(newAction);

		//3
		newAction = {};
		Fill_InterpolateCutSet(newAction, 2.5f, EASE_INOUT, 4.f + .3f);

		Fill_ActionPos(newAction, POS_ABSOLUTE,
			vKirbyStartPos + _float3{ -60.f, 10.f, 0.f });

		Fill_ActionDir(newAction, DIR_ABSOLUTE,
			{ .96f, .31f, 0.f });

		m_CamSeq.push_back(newAction);
	}
	break;

	//커비를 보는 보스의 뒷모습
	case SEQ_FINALECUT2:
	{
		m_fSeqEventTime = 2.f;


		_float fDuration = m_FinaleSeqATime.front();
		_float3 vActionPos = BOSS_POS + _float3{ 0.f, 1.f, 2.f };
		_float3 vActionDir = _float3{ -1.f, -.3f, -.05f };
		vActionDir.Normalize();
		//1
		CAMACTION newAction = {};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, vActionDir);
		Fill_ActionPos(newAction, POS_ABSOLUTE, vActionPos - vActionDir * 8.f);

		m_CamSeq.push_back(newAction);


		newAction = {};
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, 5.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, vActionDir);
		Fill_ActionPos(newAction, POS_ABSOLUTE, vActionPos - vActionDir * 15.f);

		//newAction.fFOVY = 55.f;
		m_CamSeq.push_back(newAction);


		newAction = {};
		Fill_InterpolateCutSet(newAction, 5.f, EASE_LINEAR, fDuration - 5.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, vActionDir);
		Fill_ActionPos(newAction, POS_ABSOLUTE, vActionPos - vActionDir * 15.f);

		m_CamSeq.push_back(newAction);
	}
	break;

	//열받는 보스
	case SEQ_FINALECUT3:
	{
		CAMACTION newAction = {};
		_float fDuration = m_FinaleSeqATime.front();
		_float3 vStartPos = BOSS_POS + _float3{ 0.f, 2.f, 0.f };

		newAction.fFOVY = 45.f;
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, _float3{ .15f, .06f, -.2f });
		newAction.vDir.Normalize();

		//newAction.vPos = BOSS_POS + _float3{ -15.f, -4.f, 20.f };
		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - newAction.vDir * 12.f);

		m_CamSeq.push_back(newAction);

		//newAction = {};
		newAction.fFOVY = 55.f;
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, 2.5f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - newAction.vDir * 15.f);

		m_CamSeq.push_back(newAction);

		//
		Fill_InterpolateCutSet(newAction, 2.5f, EASE_INOUT, fDuration - 2.5f);
		m_CamSeq.push_back(newAction);


	}
	break;
	//별 옆에 운석을 소환한다. far
	case SEQ_FINALECUT4:
	{
		_float fDuration = m_FinaleSeqATime.front();
		_float3 vStartPos = BOSS_POS + _float3{ 0.f, 30.f, 0.f };
		CAMACTION newAction = {};

		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE,
			vStartPos + _float3{ -183.4f, 12.f, 0.f });

		_float3 vActionDir = XMVector3Normalize(_float3{ 1.f, .07f, 0.f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE,
			vActionDir);

		newAction.fFOVY = 45.f;
		m_CamSeq.push_back(newAction);


		//
		//newAction = {};
		newAction.fFOVY = 55.f;
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, 2.f);

		m_CamSeq.push_back(newAction);

		//
		Fill_InterpolateCutSet(newAction, 2.5f, EASE_INOUT, fDuration - 2.5f);
		m_CamSeq.push_back(newAction);

	}
	break;
	//운석 던지기
	case SEQ_FINALECUT5:
	{
		m_fSeqEventTime = m_FinaleSeqATime.front();
		_float fDuration = m_FinaleSeqATime.front();

		CAMACTION newAction = {};

		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE,
			BOSS_POS + _float3{ -15.f, -3.f, 20.f });

		_float3 vActionDir = XMVector3Normalize(_float3{ .65f, .04f, -.76f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE,
			vActionDir);

		newAction.fFOVY = 45.f;
		m_CamSeq.push_back(newAction);


		//
		newAction = {};
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, fDuration - .2f);

		Fill_ActionPos(newAction, POS_ABSOLUTE,
			BOSS_POS + _float3{ -15.f, -3.f, 20.f });

		Fill_ActionDir(newAction, DIR_ABSOLUTE,
			vActionDir);

		newAction.fFOVY = 60.f;
		m_CamSeq.push_back(newAction);




		//Fill_InterpolateCutSet(newAction, fDuration - .2f, EASE_INOUT, .2f);
		//m_CamSeq.push_back(newAction);

	}
	break;

	case SEQ_FINALECUT7:
	{
		_float fDuration = m_FinaleSeqBTime.front();

		_float3 vStartPos = (_float3)FINALEKIRBY->m_vBonePos;
		vStartPos = ORIG_POS + _float3{ -150.f, -60.f, 0.f };


		CAMACTION newAction = {};
		Fill_HardCutSet(newAction, 0.f);

		//_float3 vActionDir = XMVector3Normalize(_float3{ .24f, .37f, -.9f });

		Fill_ActionDir(newAction, DIR_ABSOLUTE, { .24f, .37f, -.9f });
		newAction.vDir.Normalize();

		Fill_ActionPos(newAction, POS_ABSOLUTE, { 2371.f, 127.5f, -22.f });

		newAction.fFOVY = 45.f;
		m_CamSeq.push_back(newAction);


		//앵글 유지용 더미
		Fill_InterpolateCutSet(newAction, 0.f, EASE_LINEAR, fDuration);

		//Fill_ActionPos(newAction, POS_ABSOLUTE,
		//	vStartPos + -newAction.vDir * 80.f);

		m_CamSeq.push_back(newAction);

	}
	break;
	//보스를 뒤에서 본다
	case SEQ_FINALECUT8:
	{
		//위치 상수로
		_float3 vStartPos = _float3{ 2453.f, 168.3f, -136.f };

		_float fDuration = m_FinaleSeqBTime.front();

		CAMACTION newAction = {};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, { .74f, .61f, -.29f });
		newAction.vDir.Normalize();
		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - newAction.vDir * 40.f);

		newAction.fFOVY = 45.f;
		m_CamSeq.push_back(newAction);


		//
		newAction = {};
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, fDuration);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, { .72f, .66f, -.22f });
		newAction.vDir.Normalize();
		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - newAction.vDir * 40.f);

		m_CamSeq.push_back(newAction);
	}
	break;
	//보스 얼굴 줌인
	case SEQ_FINALECUT9:
	{
		_float fDuration = m_FinaleSeqBTime.front();
		_float3 vStartPos = BOSS_POS + _float3{ 0.f, 1.f, 0.f };


		CAMACTION newAction = {};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE,
			_float3{ 1.f, 0.f, 0.f });
		newAction.vDir.Normalize();

		Fill_ActionPos(newAction, POS_ABSOLUTE,
			vStartPos - (newAction.vDir * 30.f));

		m_CamSeq.push_back(newAction);


		//앵글 유지용 더미
		Fill_InterpolateCutSet(newAction, 0.f, EASE_LINEAR, fDuration);

		m_CamSeq.push_back(newAction);
	}
	break;

	//운석 소환. 원경
	case SEQ_FINALECUT10:
	{
		_float fDuration = m_FinaleSeqBTime.front();
		_float3 vStartPos = BOSS_POS + _float3{ 0.f, 1.f, 0.f };

		//얼굴 컷
		CAMACTION newAction = {};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, { 1.f, 0.f, 0.f });
		newAction.vDir.Normalize();

		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos + _float3{ -4.f, 2.f, 0.f });

		m_CamSeq.push_back(newAction);


		//원경
		newAction = {};
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, 1.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, BOSS_POS + _float3{ -10.f, 2.f, 0.f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { 1.f, 0.f, 0.f });

		m_CamSeq.push_back(newAction);


		//
		newAction = {};
		Fill_InterpolateCutSet(newAction, 1.f, EASE_INOUT_FAST, fDuration - 1.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, BOSS_POS + _float3{ -70.f, 15.f, 0.f });
		Fill_ActionDir(newAction, DIR_ABSOLUTE, { 1.f, 0.f, 0.f });

		newAction.fFOVY = 50.f;
		m_CamSeq.push_back(newAction);


	}
	break;
	// 커비 뒤에서 보스 봄
	case SEQ_FINALECUT11:
	{
		_float3 vStartPos = (_float3)FINALEKIRBY->m_vBonePos;
		vStartPos = _float3{ 2479.f, 187.f, -136.f };

		_float fDuration = m_FinaleSeqBTime.front();

		CAMACTION newAction = {};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, _float3{ .74f, .67f, -.05f });
		newAction.vDir.Normalize();
		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - newAction.vDir * 50.f);

		newAction.fFOVY = 45.f;
		m_CamSeq.push_back(newAction);


		//
		newAction = {};
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT_FAST, fDuration);
		Fill_ActionDir(newAction, DIR_ABSOLUTE, _float3{ .74f, .67f, -.05f });
		newAction.vDir.Normalize();
		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - newAction.vDir * 40.f);

		//Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos + _float3{ -15.f, -20.f, 10.f } /*+ _float3{ 10.f, 10.f, 0.f}*/);
		//Fill_ActionDir(newAction, DIR_ABSOLUTE, _float3{ -1.5f, -2.f, 1.f } *-1.f);
		//newAction.vDir.Normalize();

		m_CamSeq.push_back(newAction);

	}
	break;
	//커비 다가옴
	case SEQ_FINALECUT12:
	{
		_float3 vStartPos = BATTLE_POS;
		_float fDuration = m_FinaleSeqBTime.front();


		CAMACTION newAction = {};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, _float3{ -.57f, -.27f, -.77f });
		newAction.vDir.Normalize();
		//Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - (newAction.vDir * 60.f));
		Fill_ActionPos(newAction, POS_ABSOLUTE, { 2566.4f, 247.5f, -25.6f });

		m_CamSeq.push_back(newAction);


		_float3 vEndPos =
		{ 2495.f, 208.f, -136.f };

		//
		newAction = {};
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, fDuration);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, _float3{ 0.f, 0.f, -1.f });
		newAction.vDir.Normalize();
		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - (newAction.vDir * 60.f));

		m_CamSeq.push_back(newAction);



	}
	break;

	case SEQ_FINALECUT14:
	{

		_float fDuration = m_FinaleSeqCTime.front();
		_float3 vStartPos = BOSS_POS;

		CAMACTION newAction = {};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, _float3{ .94f, .05f, -.32f });
		newAction.vDir.Normalize();
		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - newAction.vDir * 15.f);

		newAction.fFOVY = 45.f;
		m_CamSeq.push_back(newAction);

		//
		newAction.fTime = 0.f;
		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, fDuration);
		m_CamSeq.push_back(newAction);

	}
	break;
	//커비 밀려남
	case SEQ_FINALECUT15:
	{
		CAMACTION newAction = {};
		_float fDuration = m_FinaleSeqCTime.front();

		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, _float3{ .32f, .29f, -.9f });
		newAction.vDir.Normalize();
		Fill_ActionPos(newAction, POS_ABSOLUTE, BATTLE_POS - newAction.vDir * 40.f);
		m_CamSeq.push_back(newAction);

		Fill_InterpolateCutSet(newAction, 0.f, EASE_INOUT, fDuration);
		m_CamSeq.push_back(newAction);

	}
	break;
	//커비 기모으기
	case SEQ_FINALECUT16:
	{
		_float3 vStartPos = BATTLE_POS + _float3{ -25.f, -10.f, 0.f };
		_float fDuration = m_FinaleSeqCTime.front();

		CAMACTION newAction = {};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, _float3{ -.8f, -.4f, -.45f });
		newAction.vDir.Normalize();

		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - newAction.vDir * 20.f);
		m_CamSeq.push_back(newAction);


		//
		newAction = {};
		Fill_InterpolateCutSet(newAction, 3.f, EASE_INOUT, 3.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, _float3{ -.42f, -.17f, -.6f });
		newAction.vDir.Normalize();

		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - newAction.vDir * 20.f);
		m_CamSeq.push_back(newAction);

		//
		Fill_InterpolateCutSet(newAction, 6.f, EASE_INOUT, fDuration - 6.f);
		m_CamSeq.push_back(newAction);

	}
	break;
	//QTE 끝. 밀려남

	//피날레. 보스 뒤짐
	case SEQ_FINALECUT20:
	{

		_float3 vStartPos = BOSS_POS;
		_float fDuration = 1029.f / 50.f;

		CAMACTION newAction = {};
		Fill_HardCutSet(newAction, 0.f);

		Fill_ActionDir(newAction, DIR_ABSOLUTE, _float3{ 0.f, 0.f, -1.f });
		newAction.vDir.Normalize();

		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - newAction.vDir * 15.f);
		m_CamSeq.push_back(newAction);

		//줌 아웃
		Fill_InterpolateCutSet(newAction, 2.f, EASE_INOUT, 1.f);

		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - newAction.vDir * 50.f);

		m_CamSeq.push_back(newAction);


		//
		newAction = {};
		Fill_InterpolateCutSet(newAction, 3.f, EASE_INOUT, 5.f);
		Fill_ActionDir(newAction, DIR_ABSOLUTE, _float3{ 0.f, 0.f, -1.f });
		newAction.vDir.Normalize();
		Fill_ActionPos(newAction, POS_ABSOLUTE, vStartPos - newAction.vDir * 50.f);
		m_CamSeq.push_back(newAction);

		Fill_InterpolateCutSet(newAction, 8.f, EASE_INOUT, 30.f);
		m_CamSeq.push_back(newAction);
	}
	break;

#pragma endregion

	default:
		break;
	}
}

//원하는 정도의 카메라 쉐이킹을 세팅한다.
void CCamera_Main::Make_Shake(_float fPower, _float fTime, _float2 vDir)
{
	m_bIsShaking = true;
	m_fShakePower = fPower;
	m_fInitialShakeTime = m_fCurShakeTime = fTime;

	vDir.Normalize();
	m_vShakeDir = vDir;
}

void CCamera_Main::Make_One_Sequence(CAMACTION newAction)
{
	m_eSpecialSeq = SEQ_ONE;
	m_CamSeq.push_back(newAction);
}

void CCamera_Main::Ready_Cam_DeeDeeDee(CGameObject* pNotifier)
{
	m_vOrigCamDir = { 0.f, -.6f, 1.f, 1.f };
	Set_Target(pNotifier->Get_TransformCom(), TARGET_SECOND, FOCUS_BOTH);
}

void CCamera_Main::Ready_Monsters_Leongar(CGameObject* pNotifier)
{
	//위에서 보기
	CAMACTION newAction{};
	Fill_HardCutSet(newAction, 0.f);

	Fill_ActionPos(newAction, POS_ABSOLUTE, { 1.2f, 14.f, -102.f });
	Fill_ActionDir(newAction, DIR_ABSOLUTE, { 0.f, -.35f, 1.f });
	Make_One_Sequence(newAction);
	Set_TargetAnchor({ 0.f, 4.f, 5.f });

	//Make_Sequence(SEQ_SIMBA_BATTLESTART);
}

void CCamera_Main::Ready_Dialog1_Leongar(CGameObject* pNotifier)
{
	Make_Sequence(SEQ_SIMBA_SHOULDER);
}

void CCamera_Main::Ready_Dialog2_Leongar(CGameObject* pNotifier)
{
	Make_Sequence(SEQ_SIMBA_FRONTVIEW);
}

void CCamera_Main::Ready_Dialog3_Leongar(CGameObject* pNotifier)
{
	Make_Sequence(SEQ_SIMBA_LOW);
}

void CCamera_Main::Ready_Cam_FinalBoss(CGameObject* pNotifier)
{
	Make_Sequence(SEQ_FINALBOSS_APPEAR);
}

void CCamera_Main::Start_ShutterSeq(CGameObject* pNotifier)
{
	Make_Sequence(SEQ_BREAKCARSHOP);
}

void CCamera_Main::Start_BridgeSeq(CGameObject* pNotifier)
{
	Make_Sequence(SEQ_BREAKRACINGMAP);
}

HRESULT CCamera_Main::Render()
{
	return S_OK;
}

void CCamera_Main::Reset_DeferredCamSet()
{
	//마지막 보정 앵글을 초기화한다.
	m_pTransformCom->Move(Dir(-m_vPreFinalOffset));

	//z 앵글을 초기화한다.
	m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_LOOK), 1.f, -m_fPreZAngle);

	//후보정 값을 초기화한다.
	m_pTransformCom->Move(static_cast<_float4>(-m_vPreShakeDir));

}

void CCamera_Main::Set_DeferredCamSet(_float fTimeDelta)
{

	//마지막 보정 값 보간
	if (.01f < _float3::Distance(m_vCurFinalOffset, m_vDestFinalOffset))
	{
		m_vCurFinalOffset += (m_vDestFinalOffset - m_vCurFinalOffset) * fTimeDelta * m_fFinalOffsetInterpolateSpeed;

		if (_float3::Distance(m_vCurFinalOffset, m_vDestFinalOffset) <= .01f)
			m_vCurFinalOffset = m_vDestFinalOffset;
	}

	//z angle 보간
	if (.001f < abs(m_fCurZAngle - m_fDestZAngle))
		m_fCurZAngle += (m_fDestZAngle - m_fCurZAngle) * fTimeDelta * m_fZAngleInterpolateSpeed;

	//이동
	m_pTransformCom->Move(Dir(m_vCurFinalOffset));
	m_vPreFinalOffset = m_vCurFinalOffset;

	//Z 앵글
	m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_LOOK), 1.f, m_fCurZAngle);
	m_fPreZAngle = m_fCurZAngle;

	//카메라 쉐이크
	_float4 vDir = Make_ShakeDir(fTimeDelta);
	m_pTransformCom->Move(vDir);


	//컷신용 dof 위치 갱신(자동)
	if (m_bAutoDOF)
	{
		m_pGameInstance->Update_DofFocus(m_pFirstTarget->Get_State(CTransform::STATE_POSITION));
	}
	//수동 dof
	else
	{
		//피날레
		if (0 < m_iCurSceneIdx)
		{
			//커비
			if (m_iCurSceneIdx == 1
				|| m_iCurSceneIdx == 6
				|| m_iCurSceneIdx == 7
				|| m_iCurSceneIdx == 8
				|| m_iCurSceneIdx == 11
				|| m_iCurSceneIdx == 12
				|| m_iCurSceneIdx == 14
				|| m_iCurSceneIdx == 15
				|| m_iCurSceneIdx == 16
				|| m_iCurSceneIdx == 17)
			{

				m_pGameInstance->Update_DofFocus(FINALEKIRBY->m_vBonePos);
			}
			else if (m_iCurSceneIdx == 2)
			{
				(0.f < m_fSeqEventTime) ?
					m_pGameInstance->Update_DofFocus(FINALEKIRBY->m_vBonePos) :
					m_pGameInstance->Update_DofFocus(FINALEBOSS->Get_RootPos());
			}
			//중간점
			else if (m_iCurSceneIdx == 13
				|| m_iCurSceneIdx == 18)
			{
				m_pGameInstance->Update_DofFocus(m_vAnchor);
			}
			//보스
			else
			{
				m_pGameInstance->Update_DofFocus(FINALEBOSS->Get_RootPos());
			}
		}

		//사자 컷
		if (SEQ_SIMBA_START <= m_eSpecialSeq && m_eSpecialSeq <= SEQ_SIMBA_LOW)
		{
			//사자 포커스
			if (m_eSpecialSeq != SEQ_SIMBA_START)
			{
				CGameObject* pSimba = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Simba"));
				if (nullptr != pSimba)
					m_pGameInstance->Update_DofFocus(pSimba->Get_TransformCom()->Get_State(CTransform::STATE_POSITION));
			}
			//커비
			else
			{
				m_pGameInstance->Update_DofFocus(m_pFirstTarget->Get_State(CTransform::STATE_POSITION));
			}
		}

		//파이널 보스 등장
		if (m_eSpecialSeq == SEQ_FINALBOSS_APPEAR)
		{
			if (m_fSeqEventTime <= 0.f)
			{
				CGameObject* pBoss = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, L"Layer_BossMonster");

				if (pBoss != nullptr)
					m_pGameInstance->Update_DofFocus(pBoss->Get_TransformCom()->Get_State(CTransform::STATE_POSITION));
			}
			//커비
			else
			{
				m_pGameInstance->Update_DofFocus(m_pFirstTarget->Get_State(CTransform::STATE_POSITION));
			}
		}
	}

}

void CCamera_Main::Control(_float fTimeDelta)
{
	static _bool bStopTimerToggle{ false };
	static _bool bFinaleOffsetToggle{ false };

	if (m_pGameInstance->Get_KeyState(DIK_LCONTROL, KEY_PRESS) &&
		m_pGameInstance->Get_KeyState(DIK_LSHIFT, KEY_PRESS))
	{
		//timer 일시 정지!!
		if (m_pGameInstance->Get_DIKeyState(DIK_P, KEY_DOWN))
		{
			bStopTimerToggle = !bStopTimerToggle;

			m_pGameInstance->Set_FirstTimerRatio((bStopTimerToggle) ? 0.f : 1.f);
			m_pGameInstance->Set_SecondTimerRatio((bStopTimerToggle) ? 0.f : 1.f);
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_N, KEY_DOWN))
		{
			bFinaleOffsetToggle = !bFinaleOffsetToggle;

			Set_FinalOffset((bFinaleOffsetToggle) ? _float3{ 0.f, -5.f, 0.f } : _float3{ 0.f, 0.f, 0.f });
		}

		//시퀀스 테스트
		if (m_pGameInstance->Get_KeyState(DIK_L, KEY_DOWN))
		{
			//Lock_All({ 109.9f, 25.2f, 108.5f }, { 1.f, .15f, -.12f });
			//Set_TargetAnchor({ 0.f, 6.f, 0.f });
			//Unlock();

			CAMACTION newAct = {};
			newAct.fTime = 0.f;
			newAct.eCamCut = CUT_HARD;

			newAct.eCamPos = POS_ABSOLUTE;
			newAct.vPos = { 109.9f, 25.2f, 108.5f };

			newAct.eCamDir = DIR_ABSOLUTE;
			newAct.vDir = { 1.f, -.15f, -.12f };

			Make_One_Sequence(newAct);
			//Make_Sequence(SEQ_SOFTCUT_TEST);
		}

		if (m_pGameInstance->Get_KeyState(DIK_P, KEY_DOWN))
		{
			//SEQ_HARDCUT_TEST
			//Make_Sequence(SEQ_SOFTCUT_TEST);

			//CEffect::FX_DESC FxDesc{};
			////FxDesc.vInitPos = (_float3)GET_POS + (_float3)(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			//FxDesc.pSocketMatrix = &m_EffectSocket;
			//FxDesc.vInitRot = CUtils::Make_Degree_FromDir( m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_finale rect"), &FxDesc)))
			//	return;
		}


		if (m_pGameInstance->Get_KeyState(DIK_8, KEY_DOWN))
		{
			Make_Shake(10.f, 2.f);
		}

	}

}

// 맵내 커비 이동제어할때 사용되는 카메라 제어 함수입니다.
// written by JYWI
void CCamera_Main::Move_ForTrigger(_float fTimeDelta, _float3 vPos, _float3 vDir)
{
	if (LEVEL_TOWN == *m_pCurrentLevelID)
	{
		vPos = { 109.9f, 25.2f, 108.5f };
		vDir = { 1.f, .15f, -.12f };
		CAMACTION newAct = {};
		newAct.fTime = 0.f;
		newAct.eCamCut = CUT_HARD;

		newAct.eCamPos = POS_ABSOLUTE;
		newAct.vPos = vPos;

		newAct.eCamDir = DIR_ABSOLUTE;
		newAct.vDir = vDir;

		Make_One_Sequence(newAct);
		Set_TargetAnchor({ 0.f, 5.f, 0.f });

	}
	//else if()
}

void CCamera_Main::Update_Anchor(_float fTimeDelta)
{

	if (nullptr == m_pFirstTarget)
	{
		m_vAnchor = _float3(ZeroVecPos);
		return;
	}

	//타겟 위치를 정한다.
	_float4 vTargetPos = Make_TargetPos();


	//실제 타겟 위치에서 조금 위로 기준점 정하기
	_float fYOffset = m_fCurUpOffset + (m_fCurDistance / 40.f);
	_float3 vAnchorOffset = _float3();

	//설정한 anchor 기준점
	//if (m_eCamFocus != FOCUS_FINALE)
	{
		_float4x4 RotMat = m_pFirstTarget->Get_WorldMatrix();
		RotMat._41 = RotMat._42 = RotMat._43 = 0.f;
		vAnchorOffset = CUtils::Make_Local_ToWorld(m_vAnchorOffset, RotMat);
	}


	//기준점 저장
	m_vAnchor = F4toF3(vTargetPos) + vAnchorOffset + _float3(0.f, fYOffset, 0.f);
}

_float3 CCamera_Main::Make_TargetPos()
{
	_float3 vTargetPos = _float3();

	//첫번째 타겟 포커스
	if (m_eCamFocus == FOCUS_FIRST)
	{
		vTargetPos = (_float3)m_pFirstTarget->Get_State(CTransform::STATE_POSITION);
		//지형 위치를 구하여 같이 쓰기
		_float4 vTerrainPos = static_cast<CCharacter*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"), 0))->Compute_TerrainPosition();

		if (vTerrainPos.y != 0.f && m_eCamFocus != FOCUS_BOTH)
			vTargetPos.y = (vTargetPos.y + vTerrainPos.y) * .5f;
	}
	//두번째 타겟 포커스
	else if (m_eCamFocus == FOCUS_SECOND)
		vTargetPos = (_float3)m_pSecondTarget->Get_State(CTransform::STATE_POSITION);
	//두 타겟 사이의 중심점.
	else if (m_eCamFocus == FOCUS_BOTH)
	{
		vTargetPos =
			(_float3)m_pFirstTarget->Get_State(CTransform::STATE_POSITION)
			+ (m_pSecondTarget->Get_State(CTransform::STATE_POSITION) - m_pFirstTarget->Get_State(CTransform::STATE_POSITION)) * m_fBothFocusRatio;
	}

	//피날레 카메라 포커스
	else if (m_eCamFocus == FOCUS_FINALE)
	{
		CFinaleKirby* pKirby = dynamic_cast<CFinaleKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"), 0));
		if (nullptr != pKirby)
			vTargetPos = (_float3)pKirby->m_vBonePos;
	}
	//피날레 배틀 포커스
	else if (m_eCamFocus == FOCUS_BATTLE)
	{
		CFinaleKirby* pKirby = FINALEKIRBY;
		CHECK_NULLPTR(pKirby);
		_float3 vKirbyPos = (_float3)pKirby->m_vBonePos;

		CFinaleBoss* pBoss = FINALEBOSS;
		CHECK_NULLPTR(pBoss);
		_float3 vBossPos = (_float3)pBoss->Get_RootPos();

		//vTargetPos = vKirbyPos * (1.f - m_fBothFocusRatio) + vBossPos * (m_fBothFocusRatio);
		vTargetPos = vKirbyPos * .5f + vBossPos * .5f + _float3{ 0.f, -10.f, 0.f };
	}

	return vTargetPos;
}

void CCamera_Main::Interpolate_CamSet(_float fTimeDelta)
{
	_float fSlerpSpeed = (m_eCamFocus == FOCUS_BOTH) ? 7.f : m_fInterpolateSpeed;

	//떨어진 거리 보간
	if (abs((m_fDestDistance + m_fCurZoomOffset) - m_fCurDistance) > .1f)
		m_fCurDistance += ((m_fDestDistance + m_fCurZoomOffset) - m_fCurDistance) * clamp(fTimeDelta * fSlerpSpeed, 0.f, 1.f);

	//y 오프셋 보간
	if (abs(m_fDestUpOffset - m_fCurUpOffset) > .001f)
		m_fCurUpOffset += (m_fDestUpOffset - m_fCurUpOffset) * fTimeDelta * 2.f;

	//fov y 보간
	if (.01f < abs(m_fFovy - m_fDestFovy))
		m_fFovy += (m_fDestFovy - m_fFovy) * fTimeDelta * 3.f;

	//각도 보간
	fSlerpSpeed = (m_eCamFocus == FOCUS_BOTH || m_eCamFocus == FOCUS_BATTLE) ? 12.f : 4.f;
	if (m_eCamFocus == FOCUS_BATTLE)
		fSlerpSpeed = 10.f;
	m_vCurCamDir = CUtils::SlerpDirVec(m_vCurCamDir, m_vDestCamDir, clamp(fTimeDelta * fSlerpSpeed, 0.f, 1.f));


}

void CCamera_Main::Update_CurCamPos(_float fTimeDelta)
{
	m_vCurCamPos = m_vAnchor - (m_vCurCamDir * m_fCurDistance);
}

void CCamera_Main::Subscribe_Events()
{
	CCamera_Main* pCam = this;


	//셔터 뿌수기
	function<void(CGameObject*)> func = bind(&CCamera_Main::Start_ShutterSeq, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_BREAK_CARSHOP, this, func);

	//다리 
	func = bind(&CCamera_Main::Start_BridgeSeq, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_BREAK_RACINGMAP, this, func);

	//디디디 전투 시작
	func = bind(&CCamera_Main::Ready_Cam_DeeDeeDee, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_DDD_BATTLESTART, this, func);



	//사자 컷

	func = bind(&CCamera_Main::Ready_Dialog1_Leongar, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_SIMBA_NEXT_DIALOG1, this, func);

	func = bind(&CCamera_Main::Ready_Dialog2_Leongar, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_SIMBA_NEXT_DIALOG2, this, func);

	func = bind(&CCamera_Main::Ready_Dialog3_Leongar, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_SIMBA_LAST_DIALOG, this, func);

	func = bind(&CCamera_Main::Ready_Monsters_Leongar, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_SIMBA_APPEAR_END, this, func);


}


_float3 CCamera_Main::Make_ShakeDir(_float fTimeDelta)
{
	//**** 카메라 쉐이킹 오프셋 ****//
	_float3 vShakeDir = _float3();

	//쉐이크 세팅 존재 시, 그만큼 팅궈준다.
	if (m_bIsShaking)
	{
		_float fSinOffset = sinf(m_fCurShakeTime * m_fShakeFrequency) * m_fShakeAmplitude * (m_fCurShakeTime / m_fInitialShakeTime) * m_fShakePower;

		vShakeDir = static_cast<_float3>(m_vShakeDir * fSinOffset);

		if (m_fCurShakeTime <= 0.f)
		{
			m_bIsShaking = false;
			m_fCurShakeTime = m_fInitialShakeTime = 0.f;
			vShakeDir = _float3();
		}
		m_fCurShakeTime -= fTimeDelta;
	}

	m_vPreShakeDir = static_cast<_float2>(vShakeDir);

	return vShakeDir;
}

void CCamera_Main::MoveTo_CurCamPos_Absolute(_float fTimeDelta)
{
	SET_POS(Pos(m_vCurCamPos));
	m_pTransformCom->Look_At_Axis(Dir(m_vCurCamDir));
}

void CCamera_Main::MoveTo_CurCamPos_Interpolate(_float fTimeDelta)
{

	//**** 목표 위치를 따라간다 ****//
	_float4 vCurPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

	_float4 vDestDir = Dir(Pos(m_vCurCamPos) - Pos(vCurPos));
	_float4 vDestXZDir = { vDestDir.x, 0.f, vDestDir.z , 0.f };
	_float4 vDestYDir = { 0.f, vDestDir.y, 0.f , 0.f };


	_float fInterpolateSpeed = (m_eCamFocus == FOCUS_BOTH) ? 12.f : m_fInterpolateSpeed;
	_float fInterpolateYSpeed = fInterpolateSpeed * 1.2f;


	if (m_eCamFocus == FOCUS_BATTLE)
	{
		fInterpolateSpeed = 20.f;
		fInterpolateYSpeed = 20.f;
	}

	//x 가기
	if (.1f <= vDestXZDir.Length())
		m_pTransformCom->Move(vDestXZDir * fTimeDelta * fInterpolateSpeed);

	//y로 가기
	if (.1f <= vDestYDir.Length())
	{
		//위쪽으로 이동하는 거라면 일단 절대값으로.
		if (0.f < vDestYDir.y)
			m_pTransformCom->Move(vDestYDir * fTimeDelta * fInterpolateYSpeed);
		else
		{
			_float4 vDir = _float4();
			//if (vDestYDir.Length() < 1.f)
			//	vDir = F4toF3(vDestYDir) * m_fInterpolateSpeed;
			//else
			vDir = vDestYDir * fInterpolateSpeed;

			m_pTransformCom->Move(vDir * fTimeDelta);
		}
	}

	fInterpolateSpeed = (m_eCamFocus == FOCUS_BOTH) ? 10.f : 1.f;

	if (m_eCamFocus == FOCUS_BATTLE)
	{
		fInterpolateSpeed = 20.f;
	}

	//보간하여 바라보기
	m_pTransformCom->Look_At_Interpolate(m_pTransformCom->Get_State(CTransform::STATE_POSITION) + Dir(m_vCurCamDir) + _float4{ 0.f, m_fCurUpOffset, 0.f, 0.f },
		fTimeDelta * fInterpolateSpeed);

	//m_pTransformCom->Move(Dir(Make_ShakeDir(fTimeDelta)));
}

void CCamera_Main::Snap_CamSet(_float fTimeDelta)
{
	m_fCurDistance = m_fDestDistance;

	m_fCurUpOffset = m_fDestUpOffset;

	m_fFovy = m_fDestFovy;

	m_vCurCamDir = m_vDestCamDir;

}

#ifdef _DEBUG
void CCamera_Main::Render_IMGUI()
{
	ImGui::Begin(u8"메인 카메라");

	if (ImGui::TreeNode("Revise Offset"))
	{
		string strPos = "pos";
		_float4 vCurPos = GET_POS;
		ImGui::DragFloat3(strPos.c_str(), (_float*)&vCurPos, -10.f, 50);

		string strLook = "look";
		_float4 vLook = m_pTransformCom->Get_State_Float4(CTransform::STATE_LOOK);
		ImGui::DragFloat3(strLook.c_str(), (_float*)&vLook, 0.f, 10);

		Lock_All(_float3(vCurPos.x, vCurPos.y, vCurPos.z), _float3(vLook.x, vLook.y, vLook.z));
		ImGui::Separator(); ImGui::NewLine();

		//----------------------------------------------------------------
		ImGui::DragFloat("fovy", &m_fFovyTemp, .1f, 10.f, 50.f, "%.1f");
		m_fDestFovy = XMConvertToRadians(m_fFovyTemp);

		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}

	_float4x4 WorldMat = m_pTransformCom->Get_WorldMatrix();
	_float4 vPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);


	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._11, WorldMat._12, WorldMat._13, WorldMat._14);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._21, WorldMat._22, WorldMat._23, WorldMat._24);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._31, WorldMat._32, WorldMat._33, WorldMat._34);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._41, WorldMat._42, WorldMat._43, WorldMat._44);



	ImGui::Dummy(ImVec2(0, 10));
	static _float3 vEffectSocketOffset{};
	ImGui::DragFloat3(u8"이펙트 소켓 이동", &(vEffectSocketOffset.x), .01f, -1000.f, 1000.f, "%.2f");
	m_EffectSocket = _float4x4::Identity;
	CUtils::Set_State_Matrix(m_EffectSocket, CUtils::STATE_POSITION,
		m_pTransformCom->Get_State(CTransform::STATE_POSITION) + vEffectSocketOffset);


	ImGui::Dummy(ImVec2(0, 10));
	ImGui::DragFloat(u8"배틀 포커스 비율", &m_fBothFocusRatio, .01f, 0.f, 1.f, "%.2f");


	ImGui::SeparatorText(u8"시퀀스");
	ImGui::Text(u8"지나간 시퀀스 시간: %.2f", m_fSeqPlayedTime);

	ImGui::Text(u8"현재 시퀀스 시간: %.2f", m_fSeqCheckTime);


	ImGui::Dummy(ImVec2(0, 10));
	ImGui::SeparatorText(u8"카메라 거리");
	ImGui::Text(u8"현재 거리 : %.2f", m_fCurDistance);
	ImGui::DragFloat(u8"목표 거리", &m_fDestDistance, .05f, 1.f, 50.f, "%.1f");

	ImGui::Dummy(ImVec2(0, 10));
	ImGui::SeparatorText(u8"카메라 방향");
	ImGui::Text(u8" 현재 Dir %.2f\t%.2f\t%.2f", m_vCurCamDir.x, m_vCurCamDir.y, m_vCurCamDir.z);
	ImGui::Text(u8" 목표 Dir %.2f\t%.2f\t%.2f", m_vDestCamDir.x, m_vDestCamDir.y, m_vDestCamDir.z);

	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Text(u8"쉐이크 시간: %.2f", m_fCurShakeTime);

	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Text(u8"보간 시간: %.2f", m_fSeqInterpolateTime.first);
	ImGui::Text(u8"목표 보간 시간: %.2f", m_fSeqInterpolateTime.second);

	ImGui::Dummy(ImVec2(0, 10));
	ImGui::SeparatorText(u8"카메라 트리거 설정");
	ImGui::Text(u8"보간 ratio: %.2f", m_fTriggerRatio);


	if (0 < m_iMatrixIndex)
	{
		_float3 vFrontDir = m_vecFrontDirRadius[m_iMatrixIndex].first;
		_float3 vRearDir = m_vecRearDirRadius[m_iMatrixIndex].first;

		ImGui::Text(u8" 앞 Dir %.2f\t%.2f\t%.2f", vFrontDir.x, vFrontDir.y, vFrontDir.z);
		ImGui::Text(u8" 뒤 Dir %.2f\t%.2f\t%.2f", vRearDir.x, vRearDir.y, vRearDir.z);
	}

	static _bool bRenderTriggers{ true };
	ImGui::Checkbox(u8"트리거 렌더", &bRenderTriggers);

	if (bRenderTriggers)
	{
		for (const auto& mat : m_vecTriggerInfo)
		{
			_float4x4 origMat = mat.first.Invert();
			Render_GraphicIMGUI(origMat);
		}
	}

	ImGui::Dummy(ImVec2(0, 10));
	ImGui::Text(u8"현재 up offset: %.2f", m_fCurUpOffset);


	ImGui::Dummy(ImVec2(0, 10));
	ImGui::SeparatorText(u8"FOV");

	ImGui::Text(u8"현재 FOV: %.2f", ToDegree(m_fFovy));

	static _float fFOVY = ToDegree(m_fDestFovy);
	if (ImGui::DragFloat(u8"목표 FOV", &fFOVY, .1f, 10.f, 50.f, "%.1f"))
		m_fDestFovy = ToRadian(fFOVY);

	ImGui::Dummy(ImVec2(0, 10));
	ImGui::SeparatorText(u8"Z 앵글");
	ImGui::DragFloat(u8"현재 Z 앵글", &m_fCurZAngle, .05f, -90.f, 90.f, "%.2f", ImGuiSliderFlags_NoInput);
	ImGui::DragFloat(u8"목표 Z 앵글", &m_fDestZAngle, .05f, -90.f, 90.f, "%.2f");

	ImGui::Dummy(ImVec2(0, 10));
	ImGui::SeparatorText(u8"줌 오프셋");
	ImGui::DragFloat(u8"현재 줌 오프셋", &m_fCurZoomOffset, .05f, -20.f, 20.f, "%.1f", ImGuiSliderFlags_NoInput);
	ImGui::DragFloat(u8"목표 줌 오프셋", &m_fDestZoomOffset, .05f, -20.f, 20.f, "%.1f");

	ImGui::End();
}
void CCamera_Main::Render_GraphicIMGUI(_float4x4 _worldMat)
{
	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 windowSize = ImGui::GetWindowSize();
	float w = windowSize.x;
	float h = windowSize.y;

	// 변환 행렬 적용
	//_float4x4 worldMat = _worldMat;
	_float4x4 viewMat = m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW);  // 뷰 행렬 설정
	_float4x4 projMat = m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ);  // 투영 행렬 설정
	_float4x4 mvpMatrix = _worldMat * viewMat * projMat;

	// 각 면의 정점 인덱스
	const int indices[] = {
		0, 1, 2, 3,   // Front face
		4, 5, 6, 7,   // Back face
		8, 9, 10, 11, // Left face
		12, 13, 14, 15, // Right face
		16, 17, 18, 19, // Top face
		20, 21, 22, 23  // Bottom face
	};


	// 각 면을 순회하며 사각형 그리기
	for (int i = 0; i < 24; i += 4) {
		_float3 v0 = vertices[indices[i]];
		_float3 v1 = vertices[indices[i + 1]];
		_float3 v2 = vertices[indices[i + 2]];
		_float3 v3 = vertices[indices[i + 3]];

		v0 = _float3::Transform(v0, _worldMat);
		v1 = _float3::Transform(v1, _worldMat);
		v2 = _float3::Transform(v2, _worldMat);
		v3 = _float3::Transform(v3, _worldMat);

		if (_float3::Transform(v0, viewMat).z < 0.f
			|| _float3::Transform(v1, viewMat).z < 0.f
			|| _float3::Transform(v2, viewMat).z < 0.f
			|| _float3::Transform(v3, viewMat).z < 0.f)
			continue;

		ImVec2 screenPos0 = CUtils::WorldPosTo_ImguiProjPos(v0);
		ImVec2 screenPos1 = CUtils::WorldPosTo_ImguiProjPos(v1);
		ImVec2 screenPos2 = CUtils::WorldPosTo_ImguiProjPos(v2);
		ImVec2 screenPos3 = CUtils::WorldPosTo_ImguiProjPos(v3);

		// 사각형을 그립니다.
		ImGui::GetForegroundDrawList()->AddQuad(screenPos0, screenPos1, screenPos2, screenPos3, IM_COL32(0, 255, 255, 255));
	}
}
#endif

CCamera_Main* CCamera_Main::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Main* pInstance = new CCamera_Main(pDevice, pContext);

	HRESULT hr = pInstance->Initialize_Prototype();

	if (FAILED(hr))
	{
		CHECK_FAILED_MSG(hr, "Failed To Clone : CCamera_Main");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCamera_Main::Clone(void* pArg)
{
	CCamera_Main* pInstance = new CCamera_Main(*this);


	HRESULT hr = pInstance->Initialize(pArg);
	CHECK_FAILED_MSG(hr, "Failed To Clone : CCamera_Main");

	return pInstance;
}

void CCamera_Main::Free()
{
	CEventCenter::Get_Instance()->Unsubscribe(this);

	__super::Free();
}


