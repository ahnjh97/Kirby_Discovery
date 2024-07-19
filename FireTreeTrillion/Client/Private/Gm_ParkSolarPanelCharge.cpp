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

	m_iGimmickIndex = Desc->iShaderVars;
	Desc->iShaderVars = 6;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_eCurState = STATE_OFFWAIT;
	m_pModelCom->Set_Animation(STATE_OFFWAIT, 30.f, FALSE, FALSE);

	m_IsInteraction = FALSE;

	//숨길 메쉬 검색하여 저장
	m_setUpdateMeshs.insert(m_pModelCom->Find_MeshIndex(string("LightM__LampC.002")));

	//피직스 추가
	m_pStaticActor = m_pNonAnimModelCom->ReturnStaticActor(m_pTransformCom->Get_WorldFloat4x4());

	for (_uint i = 0; i < m_pModelCom->Get_NumMeshes(); i++)
	{
		if (true == m_pModelCom->DoesTextureExist(TextureType_EMISSIVE, i))
			m_vecPassIndices.push_back(ANIMMODEL_EMISSIVE);
		else
			m_vecPassIndices.push_back(ANIMMODEL_LINEAR_NORMAL_O);
	}

	//림라이트 OFF
	//m_bRimLight = FALSE;

	return S_OK;
}

_int CGm_ParkSolarPanelCharge::Tick(_float fTimeDelta)
{
	//if (TRUE == m_bDead)
	//	return OBJ_DEAD;

	if (TRUE == m_pModelCom->IsFinished())
	{
		switch (m_eCurState)
		{
		case STATE_OFFWAIT: //충전 전 대기
			break;
		case STATE_OFFWAITSTART: //충전 전
			m_pModelCom->Set_Animation(STATE_OFFWAIT, 60.f, FALSE, TRUE);
			m_eCurState = STATE_OFFWAIT;
			break;

		case STATE_CHARGE: //충전 중
			m_pModelCom->Set_Animation(STATE_CHARGEDSTART, 60.f, FALSE, TRUE);
			m_eCurState = STATE_CHARGEDSTART;
			break;
		
		case STATE_CHARGEDSTART: //충전 완료
			m_pModelCom->Set_Animation(STATE_CHARGEDWAIT, 60.f, FALSE, TRUE);
			m_eCurState = STATE_CHARGEDWAIT;
			break;

		case STATE_CHARGEDWAIT: //충전 완료 대기
			m_pModelCom->Set_Animation(STATE_DECREASES, 12.5f, FALSE, TRUE);
			m_eCurState = STATE_DECREASES;
			break;

		case STATE_DECREASES: //충전 해제
			m_pModelCom->Set_Animation(STATE_OFFWAITSTART, 60.f, FALSE, TRUE);
			m_eCurState = STATE_OFFWAITSTART;
			break;
		case STATE_NONE:	
		default:	
			break;
		}
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
		//특정 애님 상태에 따라 텍스처 변경할 메쉬를 체크
		LAMP_TYPE eLampType = { LAMP_RED };
		if (m_setUpdateMeshs.find(i) != m_setUpdateMeshs.end())
		{
			switch (m_eCurState)
			{
			case STATE_CHARGE:case STATE_CHARGEDSTART:
				eLampType = LAMP_YELLOW;	
				break;
			case STATE_CHARGEDWAIT: case STATE_DECREASES:
				eLampType = LAMP_GREEN;	
				break;
			case STATE_OFFWAIT: case STATE_OFFWAITSTART:
				eLampType = LAMP_RED;	
				break;
			case STATE_NONE:	
			default:	
				break;
			}

			hr = m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", eLampType);
			CHECK_FAILED(hr);
		}

		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_EmissiveTexture", i, TextureType_EMISSIVE);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(m_vecPassIndices[i]);
		CHECK_FAILED(hr);
		
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
	
	if (m_IsInteraction) ImGui::Text(u8"Gm_ParkSolarPanelCharge :: IsInteraction : TRUE");
	else ImGui::Text(u8"Gm_ParkSolarPanelCharge :: IsInteraction : FALSE");

	string strGimmickIndex = "Index :" + to_string(m_iGimmickIndex);
	ImGui::Text(strGimmickIndex.c_str());
}
#endif

void CGm_ParkSolarPanelCharge::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{

	//충전 대기 상태에서 키입력 > 충전 시작
	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) && STATE_OFFWAIT == m_eCurState)
	{
		m_IsInteraction = TRUE;
		m_pModelCom->Set_Animation(STATE_CHARGE, 60.f, FALSE, TRUE);
		m_eCurState = STATE_CHARGE;
	}

#pragma region KEY_FRAME CUSTOM 1 SCOOP

	/*
	else //키꾹 해제 시 충전 해제 (현재 사용x)
	{
		if (STATE_CHARGE == m_pModelCom->Get_CurAnimIndex())
		{
			_float fDuration = m_pModelCom->Get_Duration(); //전체 재생길이에서 현재 재생시점을 체크
			_float fTrackPos = m_pModelCom->Get_Trackposition();
			_float fSubTrackPos = fDuration - fTrackPos; //감산하여 충전 해제 애니메이션 자연스럽게 보정

			m_pModelCom->Set_Animation(STATE_DECREASES, 60.f, FALSE, FALSE);
			m_pModelCom->Set_TrackPosition(fSubTrackPos);
			m_eCurState = STATE_DECREASES;
		}
	}
	*/

#pragma endregion

}

HRESULT CGm_ParkSolarPanelCharge::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_SolarPanelCharge_Anim"), 
		TEXT("Com_Model_Anim"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_SolarPanelCharge_NonAnim"), 
		TEXT("Com_Model_NonAnim"), (CComponent**)&m_pNonAnimModelCom);
	CHECK_FAILED(hr);

	//TEXTURE :: Lamp에 붙일 텍스처
	CHECK_FAILED(hr);	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_SolarPanelCharge_Lamp"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom);
	CHECK_FAILED(hr);

#pragma region HITBOX

	//히트박스. 해당 반경에 들어왔을 경우를 체크, 키 입력 시에 기믹 수행
	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = OBJECT;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;

	Set_BodyCollider(COLLIDER_SPHERE, 0.f, 5.f, 5.f);

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
	Safe_Release(m_pTextureCom);

}
