#include "stdafx.h"
#include "Gm_ParkSolarPanelCharge.h"

#include "HitBox.h"
#include "Kirby.h"
//#include "BreakableRockParticle.h"

CGm_ParkSolarPanelCharge::CGm_ParkSolarPanelCharge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CGm_ParkSolarPanelCharge::CGm_ParkSolarPanelCharge(const CGm_ParkSolarPanelCharge& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CGm_ParkSolarPanelCharge::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGm_ParkSolarPanelCharge::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = { nullptr };

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_eCurState = STATE_OFFWAIT;
	m_pModelCom->Set_Animation(STATE_OFFWAIT, 60.f, TRUE /*_bool bInterpolation = false, _float fLerpTime = 0.1f*/);

	//숨길 메쉬 검색하여 저장
	//m_setBeforeHideMeshs.insert(m_pModelCom->Find_MeshIndex(string("TnnelWallM__FhEntranceAlienTunnelWallC")));

	//피직스 추가
	m_pStaticActor = m_pNonAnimModelCom->ReturnStaticActor(m_pTransformCom->Get_WorldFloat4x4());

	//림라이트 OFF
	//m_bRimLight = FALSE;

	return S_OK;
}

_int CGm_ParkSolarPanelCharge::Tick(_float fTimeDelta)
{
	//if (TRUE == m_bDead)
	//	return OBJ_DEAD;

	switch (m_eCurState)
	{
	case STATE_CHARGE: //충전 중
		if (TRUE == m_pModelCom->IsFinished()) //충전 중 애님 종료 시 충전 완료 상태 변경
		{
			m_pModelCom->Set_Animation(STATE_CHARGEDSTART, 60.f, FALSE);
			m_eCurState = STATE_CHARGEDSTART;
		}
		break;
	case STATE_CHARGEDSTART: //충전 완료
		break;
	case STATE_CHARGEDWAIT: //충전 완료 대기
		break;
	case STATE_DECREASES: //충전 해제
		if (TRUE == m_pModelCom->IsFinished()) //충전 해제 중 애님 종료 시 충전 전 상태 변경
		{
			m_pModelCom->Set_Animation(STATE_OFFWAITSTART, 60.f, FALSE);
			m_eCurState = STATE_OFFWAITSTART;
		}
		break;
	case STATE_OFFWAITSTART: //충전 전
		if (TRUE == m_pModelCom->IsFinished())
		{
			m_pModelCom->Set_Animation(STATE_OFFWAIT, 60.f, FALSE);
			m_eCurState = STATE_OFFWAIT;
		}
		break;
	case STATE_OFFWAIT: //충전 전 대기
		break;
	case STATE_NONE:	default:	break;
	}

	return OBJ_NOEVENT;
}

void CGm_ParkSolarPanelCharge::Late_Tick(_float fTimeDelta)
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
	//if (TRUE == m_pModelCom->IsFinished())
	//	Set_Dead();
}

HRESULT CGm_ParkSolarPanelCharge::Render()
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

		//특정 애님 상태에 따라 렌더할 메쉬를 체크
		//if (STATE_LOOP == m_eAnimState)
		//{
		//	if (m_setBeforeHideMeshs.find(i) != m_setBeforeHideMeshs.end())
		//		continue;
		//}
		
		hr = m_pModelCom->Render(i);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CGm_ParkSolarPanelCharge::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CGm_ParkSolarPanelCharge::Render_IMGUI()
{
	switch (m_eCurState)
	{
	case STATE_CHARGE:			ImGui::Text(u8"STATE_CHARGE"); break;
	case STATE_CHARGEDSTART:	ImGui::Text(u8"STATE_CHARGEDSTART"); break;
	case STATE_CHARGEDWAIT:		ImGui::Text(u8"STATE_CHARGEDWAIT"); break;
	case STATE_DECREASES:		ImGui::Text(u8"STATE_DECREASES"); break;
	case STATE_OFFWAIT:			ImGui::Text(u8"STATE_OFFWAIT"); break;
	case STATE_OFFWAITSTART:	ImGui::Text(u8"STATE_OFFWAITSTART"); break;
	case STATE_NONE:	default: ImGui::Text(u8"STATE_NONE"); break;
	}
	
	if (m_IsInteraction) ImGui::Text(u8"IsInteraction : TRUE");
	else ImGui::Text(u8"IsInteraction : FALSE");
}
#endif

void CGm_ParkSolarPanelCharge::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	m_IsInteraction = TRUE;

	//충전 중 대기 상태에서 키꾹 > 충전 시작
	if (m_pGameInstance->Get_DIKeyState(DIK_A, KEY_DOWN) && STATE_OFFWAIT == m_eCurState)
	{
		m_pModelCom->Set_Animation(STATE_CHARGE, 60.f, FALSE);
		m_eCurState = STATE_CHARGE;
		//m_bStartCharge = TRUE;
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_A, KEY_UP) && STATE_CHARGE == m_eCurState)
	{
		m_pModelCom->Set_Animation(STATE_DECREASES, 60.f, FALSE);
		m_eCurState = STATE_DECREASES;

	}
}

HRESULT CGm_ParkSolarPanelCharge::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_SolarPanelCharge_Anim"), 
		TEXT("Com_Model_Anim"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_SolarPanelCharge_NonAnim"), 
		TEXT("Com_Model_NonAnim"), (CComponent**)&m_pNonAnimModelCom);
	CHECK_FAILED(hr);

#pragma region HITBOX

	//히트박스. 해당 반경에 들어왔을 경우를 체크, 키 입력 시에 기믹 수행
	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = OBJECT;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;

	Set_BodyCollider(COLLIDER_CYLINDER, 0.f, 2.5f, 2.5f);

#pragma endregion

	return S_OK;
}

HRESULT CGm_ParkSolarPanelCharge::Bind_ShaderResources()
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

CGm_ParkSolarPanelCharge* CGm_ParkSolarPanelCharge::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGm_ParkSolarPanelCharge* pInstance = new CGm_ParkSolarPanelCharge(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CGm_ParkSolarPanelCharge"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGm_ParkSolarPanelCharge::Clone(void* pArg)
{
	CGm_ParkSolarPanelCharge* pInstance = new CGm_ParkSolarPanelCharge(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CGm_ParkSolarPanelCharge"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGm_ParkSolarPanelCharge::Free()
{
	__super::Free();

	m_pGameInstance->ReleaseActor(m_pStaticActor);

	Safe_Release(m_pModelCom);
	Safe_Release(m_pNonAnimModelCom);

	Safe_Release(m_pShaderCom);

}
