#include "stdafx.h"
#include "KirbyFinalCut_State.h"
#include "Kirby_State_Function.h"

#include "FinalDump.h"
#include "FinalStone.h"

#pragma region CUT STATE

CKirbyFinalCut_State::CKirbyFinalCut_State()
{
}

void CKirbyFinalCut_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyFinalCut_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	// ¹ñ´Â ·ÎÁ÷ÀÌ´Ù.
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	if (m_bJump == true)
	{
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
	}
	else
		pController->FreeFall(pTransformCom, fTimeDelta);

	if (pKirby->Get_State() == CKirby::STATE_WALK)
	{
		m_fTime += fTimeDelta;

		if (m_bCutStartPos == true)
		{
			pController->Set_Position(pTransformCom, _float4(0.f, 0.f, -20.f, 1.f));
			DESC(m_vTargetDir) = DESC(m_vMoveDir) = _float4(0.f, 0.f, 1.f, 0.f);
			m_bCutStartPos = false;
		}
		Kirbydesc->m_fMoveSpeed += fTimeDelta * 70.f;
		if (Kirbydesc->m_fMoveSpeed > 3.f)
			Kirbydesc->m_fMoveSpeed = 3.f;

		// Å¸°Ù±âÁØ
		_vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

		if (m_fTime >= 2.6f)
		{
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_IDLE);
			return;
		}
	}
	else if (pKirby->Get_State() == CKirby::FINALCUTSTATE_CUT1)
	{
		if (m_bCutStartPos == true)
		{
			DESC(m_vTargetDir) = DESC(m_vMoveDir) = _float4(0.f, 0.f, -1.f, 0.f);
			pController->Set_Position(pTransformCom, _float4(0.f, 0.f, 0.f, 1.f));
			m_bCutStartPos = false;
		}

		_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
		vPos.y += 8.f;

		_float fAnimTrackPosition = pKirby->Get_AnimTrackPosition();

		if (m_uDropCount == 0 && fAnimTrackPosition >= 45)
		{
			CFinalStone::FINALSTONEDESC stonedesc = {};
			stonedesc.vPos = vPos + _float4(3.f, 0.f, 5.f, 0.f);
			stonedesc.fScale = CUtils::Make_RandomFloat(1.5f, 1.8f);
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_FinalStone"), &stonedesc)))
				return;

			m_uDropCount = 1;
		}
		else if (m_uDropCount == 1 && fAnimTrackPosition >= 85)
		{
			CFinalStone::FINALSTONEDESC stonedesc = {};
			stonedesc.vPos = vPos + _float4(-10.f, 0.f, 1.f, 0.f);
			stonedesc.fScale = CUtils::Make_RandomFloat(1.5f, 1.8f);
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_FinalStone"), &stonedesc)))
				return;


			m_uDropCount = 2;
		}
		else if (m_uDropCount == 2 && fAnimTrackPosition >= 170)
		{
			CFinalDump::DUMPDESC DumpDesc = {};
			_float4x4 DumpMatrix = pTransformCom->Get_WorldFloat4x4();
			CUtils::Turn_OtherMatrix(DumpMatrix, _float4(0.f, 1.f, 0.f, 0.f), 1.f, 40.f);
			CUtils::Set_State_Matrix(DumpMatrix, CUtils::STATE_POSITION, vPos + _float4(5.f, 0.f, -5.f, 0.f));
			DumpDesc.Matrix = DumpMatrix;
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_FinalDump"), &DumpDesc)))
				return;

			m_uDropCount = 3;
		}




		if (fAnimTrackPosition >= 56 && fAnimTrackPosition <= 67)
		{
			DESC(m_eEyeState) = CKirby::EYE_CLOSE;
			DESC(m_eMouthState) = CKirby::MOUTH_SURPRISE;
		}
		else if (fAnimTrackPosition >= 97 && fAnimTrackPosition <= 107)
		{
			DESC(m_eEyeState) = CKirby::EYE_CLOSE;
			DESC(m_eMouthState) = CKirby::MOUTH_SURPRISE;
		}
		else if (fAnimTrackPosition >= 56 && fAnimTrackPosition <= 61)
		{
			DESC(m_eEyeState) = CKirby::EYE_CLOSE;
			DESC(m_eMouthState) = CKirby::MOUTH_SURPRISE;
		}
		else if (fAnimTrackPosition >= 180 && fAnimTrackPosition <= 200)
		{
			DESC(m_eEyeState) = CKirby::EYE_CLOSE;
			DESC(m_eMouthState) = CKirby::MOUTH_SURPRISE;
		}
		else
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			DESC(m_eMouthState) = CKirby::MOUTH_IDLE;
		}

		if (fAnimTrackPosition >= 180 && m_bJumpTrigger == true)
		{
			DESC(m_fJumpVelocity) = 8.f;
			m_bJump = true;
			m_bJumpTrigger = false;
		}
		else if (pController->Is_Terrain())
		{
			m_bJump = false;
		}


		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::FINALCUTSTATE_CUT2, 60.f, true, false, CKirby::BODY_FINALCUT, CKirby::OFFSET_FINALCUT);
			return;
		}

	}
	else if (pKirby->Get_State() == CKirby::FINALCUTSTATE_CUT2)
	{
		pController->FreeFall(pTransformCom, fTimeDelta);
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		m_fTime += fTimeDelta;

		_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
		vPos.y += 16.f;

		if (m_uDropCount == 0 && m_fTime > 1.3f)
		{
			CFinalStone::FINALSTONEDESC stonedesc = {};
			stonedesc.vPos = vPos + _float4(3.f, 0.f, 5.f, 0.f);
			stonedesc.fScale = CUtils::Make_RandomFloat(1.5f, 1.8f);
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_FinalStone"), &stonedesc)))
				return;

			m_fTime = 0.f;
			m_uDropCount = 1;
		}
		else if (m_uDropCount == 1 && m_fTime > 2.f)
		{
			CFinalStone::FINALSTONEDESC stonedesc = {};
			stonedesc.vPos = vPos + _float4(-15.f, 0.f, 15.f, 0.f);
			stonedesc.fScale = CUtils::Make_RandomFloat(1.5f, 1.8f);
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_FinalStone"), &stonedesc)))
				return;

			m_fTime = 0.f;
			m_uDropCount = 2;
		}
		else if (m_uDropCount == 2 && m_fTime > 1.4)
		{
			CFinalStone::FINALSTONEDESC stonedesc = {};
			stonedesc.vPos = vPos + _float4(-4.f, 0.f, -8.f, 0.f);
			stonedesc.fScale = CUtils::Make_RandomFloat(1.5f, 1.8f);
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_FinalStone"), &stonedesc)))
				return;

			m_fTime = 0.f;
			m_uDropCount = 3;
		}



		if (8.f < m_fTime && m_fTime <= 4.f)
		{
			DESC(m_eEyeState) = CKirby::EYE_BLINK;
			DESC(m_eMouthState) = CKirby::MOUTH_SURPRISE;
		}
		else if (4.15f < m_fTime && m_fTime <= 4.35f)
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			DESC(m_eMouthState) = CKirby::MOUTH_SURPRISE;
		}
		else if (4.35f < m_fTime && m_fTime <= 4.45f)
		{
			DESC(m_eEyeState) = CKirby::EYE_BLINK;
			DESC(m_eMouthState) = CKirby::MOUTH_SURPRISE;
		}
		else if (4.45f < m_fTime && m_fTime <= 6.f)
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			DESC(m_eMouthState) = CKirby::MOUTH_SURPRISE;
		}
		else if (6.f < m_fTime && m_fTime <= 6.2f)
		{
			DESC(m_eEyeState) = CKirby::EYE_BLINK;
			DESC(m_eMouthState) = CKirby::MOUTH_IDLE;
		}
		else if (6.f < m_fTime)
		{
			DESC(m_eEyeState) = CKirby::EYE_ANGER;
			DESC(m_eMouthState) = CKirby::MOUTH_SMILE;
		}

	}
}

void CKirbyFinalCut_State::OnStateExit()
{
	m_bCutStartPos = true;
	m_fTime = 0.f;
	m_uDropCount = 0;
	m_bJump = { false };
	m_bJumpTrigger = { true };
}

CKirbyFinalCut_State* CKirbyFinalCut_State::Create()
{
	CKirbyFinalCut_State* pInstance = new CKirbyFinalCut_State();
	return pInstance;
}

void CKirbyFinalCut_State::Free()
{
	__super::Free();
}

#pragma endregion
