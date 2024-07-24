#include "stdafx.h"
#include "DeeDeeDeeHammer.h"
#include "Bone.h"

#include "Ability.h"
#include "UI_MessageWindow.h"
#include "Camera_Main.h"
#include "Kirby.h"
#include "UI_Fading.h"

CDeeDeeDeeHammer::CDeeDeeDeeHammer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPartObject{ pDevice, pContext }
{
}

CDeeDeeDeeHammer::CDeeDeeDeeHammer(const CDeeDeeDeeHammer& rhs)
	: CPartObject{ rhs }
{
}

HRESULT CDeeDeeDeeHammer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDeeDeeDeeHammer::Initialize(void* pArg)
{
	DEEDEEDEEHAMMER_DESC* pWeaponDesc = (DEEDEEDEEHAMMER_DESC*)pArg;
	m_pBoneMatrix = pWeaponDesc->pBoneMatrix;
	m_pWhiteColorDiffuse = pWeaponDesc->pWhite;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	return S_OK;
}

_int CDeeDeeDeeHammer::Tick(_float fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;

	Compute_MotionBlur();

	m_WorldMatrix = m_pTransformCom->Get_WorldMatrix() * *m_pBoneMatrix * *m_pParentMatrix;

	if (m_bItemTrigger == true && CUtils::Get_Scaled_Matrix(m_WorldMatrix).x < 0.1f)
	{
		HRESULT hr = S_OK;
		CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
		AbilityItemDesc.vPosition = CUtils::Get_State_Vector_Matrix(m_WorldMatrix, CUtils::STATE_POSITION);
		AbilityItemDesc.eAbilityType = ABILITY_HAMMER;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);
		m_bItemTrigger = false;

		m_bShowDialog = TRUE;
	}

	//특정 시간 경과 후 다이얼로그 출력
	if (m_bShowDialog)
	{
		m_fShowDialog += fTimeDelta;
		if (m_fShowDialog > 1.5f)
		{
			(m_bFadeOutEnd == false) ? Ready_FadeOut() : Ready_FadeIn();
			m_fShowDialog = 0.f;
		}
	}

	return OBJ_NOEVENT;
}

void CDeeDeeDeeHammer::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(XMVectorSet(m_WorldMatrix._41, m_WorldMatrix._42, m_WorldMatrix._43, m_WorldMatrix._44), 5.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CDeeDeeDeeHammer::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(MODEL_MONSTERPART)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CDeeDeeDeeHammer::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_PartObject(m_pShaderCom, &m_WorldMatrix, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CDeeDeeDeeHammer::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_DeeDeeDeeHammer"),
		TEXT("Com_Model_Sword"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CDeeDeeDeeHammer::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	// 추후 변경
	_bool bStencil = true;
	_bool bRimLight = true;
	_bool bMotionBlur = true;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &bStencil, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &bMotionBlur, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", m_pWhiteColorDiffuse, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

void CDeeDeeDeeHammer::Compute_MotionBlur()
{
	_vector vPos = CUtils::Get_State_Vector_Matrix(m_WorldMatrix, CUtils::STATE_POSITION);
	_matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
	_float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
	_float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

	_float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

	m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
	m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
	m_vPreScreenPos = vCurScreenPos;
}

void CDeeDeeDeeHammer::Ready_FadeIn()
{
	static _bool bOnceFade = false;
	static _bool bOnceChanger = false;

	CGameObject* pUIObj = m_pGameInstance->Get_GameObject_ByTag(LEVEL_STATIC, TEXT("Layer_ChangerUI"), TEXT("Prototype_GameObject_UI_Fading"));
	CUI_Fading* pFadingUI = dynamic_cast<CUI_Fading*>(pUIObj);
	
	CKirby* pKirby = dynamic_cast<CKirby*>(m_pGameInstance->Get_GameObject(LEVEL_DEEDEEDEE, TEXT("Layer_Player")));
	CHECK_NULLPTR(pKirby);

	if (bOnceFade == false)
	{
		pFadingUI->Set_InOutState(CUI_Fading::FADEIN);
		pFadingUI->Set_IsRender(true);
		bOnceFade = true;
		
	}
	else if (pFadingUI->Get_FadeRatio() >= 1.f)
	{
		if (bOnceChanger == false)
		{
			CUI_MessageWindow* pMWindow = dynamic_cast<CUI_MessageWindow*>
				(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_UI_Msg_DeeDeeDee")));
			CHECK_NULLPTR(pMWindow);
			pMWindow->Show_DialogMessage();
			bOnceChanger = true;
		}
	}
}

void CDeeDeeDeeHammer::Ready_FadeOut()
{
	static _bool bOnceFade = false;

	CGameObject* pUIObj = m_pGameInstance->Get_GameObject_ByTag(LEVEL_STATIC, TEXT("Layer_ChangerUI"), TEXT("Prototype_GameObject_UI_Fading"));
	CUI_Fading* pFadingUI = dynamic_cast<CUI_Fading*>(pUIObj);
	if (bOnceFade == false)
	{
		pFadingUI->Set_InOutState(CUI_Fading::FADEOUT);
		pFadingUI->Set_IsRender(true);
		bOnceFade = true;
	}
	else if (pFadingUI->Get_FadeRatio() <= 0.f)
	{
#pragma region 카메라 컷신 조정
		//효선아 여기야 와들디 세팅

		CCamera_Main* pCameraMain = dynamic_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
		CHECK_NULLPTR(pCameraMain);

		CKirby* pKirby = dynamic_cast<CKirby*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Kirby")));
		CHECK_NULLPTR(pKirby);

		CCharacterController* pKirbyController = dynamic_cast<CCharacterController*>(pKirby->Get_Component(TEXT("Com_Controller")));
		CHECK_NULLPTR(pKirbyController);

		//커비 위치 세팅
		CTransform* pKirbyTrans = pKirby->Get_TransformCom();
		_float4 vDialogKirbyPos = { 2.73f, 23.11f, -11.54f, 1.f };
		pKirbyController->Set_Position(pKirbyTrans, vDialogKirbyPos);

		_float4 vDialogKirbyDir = { 1.f, 1.f, 1.f, 1.f };
		pKirby->DialogOn(vDialogKirbyDir);

		pCameraMain->Lock_All({ 4.85f,  27.19f,  -31.27f }, { -0.09f, -0.09f, 0.99f }, FALSE);

#pragma endregion

		m_bFadeOutEnd = true;

	}
}

CDeeDeeDeeHammer* CDeeDeeDeeHammer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDeeDeeDeeHammer* pInstance = new CDeeDeeDeeHammer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CDeeDeeDeeHammer"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDeeDeeDeeHammer::Clone(void* pArg)
{
	CDeeDeeDeeHammer* pInstance = new CDeeDeeDeeHammer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CDeeDeeDeeHammer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDeeDeeDeeHammer::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

}
