#include "stdafx.h"
#include "Gm_ParkShutter.h"

#include "HitBox.h"
#include "Kirby.h"

CGm_ParkShutter::CGm_ParkShutter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CGm_ParkShutter::CGm_ParkShutter(const CGm_ParkShutter& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CGm_ParkShutter::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGm_ParkShutter::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = { nullptr };

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_eCurState = STATE_CLOSEIDLE;
	m_pModelCom->Set_Animation(STATE_CLOSEIDLE, 60.f, TRUE /*_bool bInterpolation = false, _float fLerpTime = 0.1f*/);

	//피직스 추가
	m_pStaticActor = m_pNonAnimModelCom->ReturnStaticActor(m_pTransformCom->Get_WorldFloat4x4());

	//림라이트 OFF
	m_bRimLight = FALSE;

	return S_OK;
}

_int CGm_ParkShutter::Tick(_float fTimeDelta)
{
	if (TRUE == m_bDead)
		return OBJ_DEAD;

	if (nullptr == m_pGimmickSPOnce)
		return OBJ_NOEVENT;

	//태양전자판 기믹 활성화 애님일 경우, 셔터 애님 변경
	
	CGm_ParkSolarPanelOnce::PANELONCE_STATE eSPOnceState = m_pGimmickSPOnce->Get_CurState();
	if (CGm_ParkSolarPanelOnce::PANELONCE_STATE::STATE_ONWAIT == eSPOnceState
		&& 4 == m_pGimmickSPOnce->Get_GimmickIndex()
		&& STATE_CLOSEIDLE == m_eCurState)
	{
		m_pModelCom->Set_Animation(STATE_TOOPEN, 60.f, FALSE, TRUE);
		m_eCurState = STATE_TOOPEN;

		m_pGameInstance->StopSound(CHANNEL_GIMMICK);
		m_pGameInstance->PlayMySound(L"ParkShutter_ToOpen.wav", CHANNEL_GIMMICK, 0.5f);
	}

	return OBJ_NOEVENT;
}

void CGm_ParkShutter::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(m_pGameInstance->Get_SecondTimer());

#pragma region FRUSTUM_CULLING

	//절두체 컬링 처리
	if (TRUE == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 50.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}

#pragma endregion

	//애니메이션 재생종료 시 Set_Dead
	if (TRUE == m_pModelCom->IsFinished())
		Set_Dead();
}

HRESULT CGm_ParkShutter::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	HRESULT hr;

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(ANIMMODEL_LINEAR_NORMAL_O_NONDISCARD);
		CHECK_FAILED(hr);
		
		hr = m_pModelCom->Render(i);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CGm_ParkShutter::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CGm_ParkShutter::Render_IMGUI()
{
	switch (m_eCurState)
	{
	case STATE_CLOSEIDLE:	ImGui::Text(u8"STATE_CLOSEIDLE"); break;
	case STATE_OPENIDLE:	ImGui::Text(u8"STATE_OPENIDLE"); break;
	case STATE_TOCLOSE:		ImGui::Text(u8"STATE_TOCLOSE"); break;
	case STATE_TOOPEN:		ImGui::Text(u8"STATE_TOOPEN"); break;
	case STATE_NONE:	default: ImGui::Text(u8"STATE_NONE"); break;
	}
}
#endif

void CGm_ParkShutter::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

HRESULT CGm_ParkShutter::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Shutter_Anim"), 
		TEXT("Com_Model_Anim"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Shutter_NonAnim"), 
		TEXT("Com_Model_NonAnim"), (CComponent**)&m_pNonAnimModelCom);
	CHECK_FAILED(hr);


	return S_OK;
}

HRESULT CGm_ParkShutter::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CGm_ParkShutter* CGm_ParkShutter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGm_ParkShutter* pInstance = new CGm_ParkShutter(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CGm_ParkShutter"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGm_ParkShutter::Clone(void* pArg)
{
	CGm_ParkShutter* pInstance = new CGm_ParkShutter(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CGm_ParkShutter"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGm_ParkShutter::Free()
{
	__super::Free();

	m_pGameInstance->ReleaseActor(m_pStaticActor);

	Safe_Release(m_pModelCom);
	Safe_Release(m_pNonAnimModelCom);

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pGimmickSPOnce);

}
