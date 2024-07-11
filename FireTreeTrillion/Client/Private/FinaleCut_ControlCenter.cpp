#include "stdafx.h"
#include "FinaleCut_ControlCenter.h"
#include "Camera_Main.h"
#include "FinaleKirby.h"

CFinaleCut_ControlCenter::CFinaleCut_ControlCenter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CFinaleCut_ControlCenter::CFinaleCut_ControlCenter(const CFinaleCut_ControlCenter& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CFinaleCut_ControlCenter::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFinaleCut_ControlCenter::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	return S_OK;

}

_int CFinaleCut_ControlCenter::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;


	if (m_pGameInstance->Get_KeyState(DIK_LCONTROL, KEY_PRESS) && m_pGameInstance->Get_KeyState(DIK_LSHIFT, KEY_PRESS))
	{

		if (m_pGameInstance->Get_KeyState(DIK_2, KEY_DOWN))
			Set_CutScene(2);

		if (m_pGameInstance->Get_KeyState(DIK_3, KEY_DOWN))
			Set_CutScene(3);

		if (m_pGameInstance->Get_KeyState(DIK_4, KEY_DOWN))
			Set_CutScene(4);

		if (m_pGameInstance->Get_KeyState(DIK_5, KEY_DOWN))
			Set_CutScene(5);

		if (m_pGameInstance->Get_KeyState(DIK_6, KEY_DOWN))
			Set_CutScene(6);

		if (m_pGameInstance->Get_KeyState(DIK_7, KEY_DOWN))
			Set_CutScene(7);

		if (m_pGameInstance->Get_KeyState(DIK_8, KEY_DOWN))
			Set_CutScene(8);

		if (m_pGameInstance->Get_KeyState(DIK_9, KEY_DOWN))
			Set_CutScene(9);

		if (m_pGameInstance->Get_KeyState(DIK_0, KEY_DOWN))
			Set_CutScene(10);

		if (m_pGameInstance->Get_KeyState(DIK_Q, KEY_DOWN))
			Set_CutScene(11);

		if (m_pGameInstance->Get_KeyState(DIK_W, KEY_DOWN))
			Set_CutScene(12);

		if (m_pGameInstance->Get_KeyState(DIK_E, KEY_DOWN))
			Set_CutScene(13);

		if (m_pGameInstance->Get_KeyState(DIK_R, KEY_DOWN))
			Set_CutScene(14);

		if (m_pGameInstance->Get_KeyState(DIK_T, KEY_DOWN))
			Set_CutScene(15);

		if (m_pGameInstance->Get_KeyState(DIK_Y, KEY_DOWN))
			Set_CutScene(16);

		if (m_pGameInstance->Get_KeyState(DIK_U, KEY_DOWN))
			Set_CutScene(17);

		if (m_pGameInstance->Get_KeyState(DIK_I, KEY_DOWN))
			Set_CutScene(18);

		if (m_pGameInstance->Get_KeyState(DIK_O, KEY_DOWN))
			Set_CutScene(19);

		if (m_pGameInstance->Get_KeyState(DIK_P, KEY_DOWN))
			Set_CutScene(20);
	}

	return OBJ_NOEVENT;
}

void CFinaleCut_ControlCenter::Late_Tick(_float fTimeDelta)
{
}

HRESULT CFinaleCut_ControlCenter::Render()
{
	return S_OK;
}

void CFinaleCut_ControlCenter::Set_CutScene(_uint iScene)
{
	m_iCutScene = iScene;

	if (iScene != 6 && iScene != 8)
	{
		//카메라 컷신
		CCamera_Main* pCameraMain = static_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
		CHECK_NULLPTR(pCameraMain);
		pCameraMain->Make_Sequence(CCamera_Main::CAMSEQ((_uint)CCamera_Main::SEQ_FINALECUT1 - 1 + iScene));
	}
	/*
	else if (iScene == 6)
	{
		//카메라 받기
		CCamera_Main* pCameraMain = static_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
		CHECK_NULLPTR(pCameraMain);

		CCamera_Main::CAMACTION newAction = {};
		newAction.fTime = 0.f;
		newAction.eCamCut = CCamera_Main::CUT_HARD;

		//피날레 커비
		CFinaleKirby* pKirby =
			dynamic_cast<CFinaleKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"), 0));
		CHECK_NULLPTR(pKirby);

		newAction.fFOVY = 45.f;
		newAction.eCamPos = CCamera_Main::POS_ABSOLUTE;
		newAction.vPos = pKirby->m_vBonePos + _float3{ -20.f, -2.f, 0.f };
		newAction.eCamDir = CCamera_Main::DIR_ABSOLUTE;
		newAction.vDir = _float3{ 1.f, .15f, -.03f };


		pCameraMain->Make_One_Sequence(newAction);
	}
	*/
}

CFinaleCut_ControlCenter* CFinaleCut_ControlCenter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinaleCut_ControlCenter* pInstance = new CFinaleCut_ControlCenter(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFinaleCut_ControlCenter"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFinaleCut_ControlCenter::Clone(void* pArg)
{
	CFinaleCut_ControlCenter* pInstance = new CFinaleCut_ControlCenter(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CFinaleCut_ControlCenter"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFinaleCut_ControlCenter::Free()
{
	__super::Free();
}
