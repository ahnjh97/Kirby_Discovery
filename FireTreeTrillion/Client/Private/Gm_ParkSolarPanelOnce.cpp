#include "stdafx.h"
#include "Gm_ParkSolarPanelOnce.h"

#include "HitBox.h"
#include "Kirby.h"
#include "Bomber.h"
#include "SummonEffect.h"
//#include "BreakableRockParticle.h"

CGm_ParkSolarPanelOnce::CGm_ParkSolarPanelOnce(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CGm_ParkSolarPanelOnce::CGm_ParkSolarPanelOnce(const CGm_ParkSolarPanelOnce& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CGm_ParkSolarPanelOnce::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGm_ParkSolarPanelOnce::Initialize(void* pArg)
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

_int CGm_ParkSolarPanelOnce::Tick(_float fTimeDelta)
{
	if (TRUE == m_bDead)
		return OBJ_DEAD;

	_float fAnimRatio = { 0.f };
	switch (m_eCurState)
	{
	case STATE_OFFWAIT: //충전 전 대기
		break;
	case STATE_CHARGE: //충전 중
		if (TRUE == m_pModelCom->IsFinished()) //충전 중 애님 종료 시 충전 완료 상태 변경
		{
			m_pModelCom->Set_Animation(STATE_ONWAITSTART, 60.f, FALSE, TRUE);
			m_eCurState = STATE_ONWAITSTART;
		}
		break;

	case STATE_ONWAITSTART: //충전 시작
		fAnimRatio = m_pModelCom->Get_AnimRatio();
		if (0.15f < fAnimRatio)
		{
			m_eCurState = STATE_ONWAIT;
			m_pModelCom->Set_Animation(STATE_ONWAIT, 60.f, TRUE, TRUE);
		}
		break;
		
	case STATE_ONWAIT: //충전 완료
		if(false == m_bSpawn)
		{
			m_bSpawn = true;
			if (4 == m_iGimmickIndex) //크래시 능력 몬스터를 생성
			{
				HRESULT hr;

				CSummonEffect::SUMMONEFFECT_DESC SummonEffectDesc = {};
				/*SummonEffectDesc.vPosition = XMVectorSet(35.5f, 75.f, 175.4f, 1.f);
				SummonEffectDesc.vColor = XMVectorSet(1.f, 1.f, 1.f, 0.f);
				SummonEffectDesc.fScale = 1.f;
				SummonEffectDesc.fAlpha = 0.5f;
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_SummonEffect"), &SummonEffectDesc);
				CHECK_FAILED(hr);*/

				SummonEffectDesc.vPosition = XMVectorSet(35.5f, 75.f, 175.5f, 1.f);
				SummonEffectDesc.vColor = XMVectorSet(1.f, 0.f, 0.f, 0.f);
				SummonEffectDesc.fScale = 3.f;
				SummonEffectDesc.fAlpha = 1.f;
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_SummonEffect"), &SummonEffectDesc);
				CHECK_FAILED(hr);

				//_float4x4 matWorld = XMMatrixIdentity();
				//matWorld._41 = 35.5f;
				//matWorld._42 = 73.f;
				//matWorld._43 = 175.5f;
				//matWorld._44 = 1.f;
				//CMonster::MONSTER_DESC MonsterDesc = {};
				//MonsterDesc.matWorld = matWorld;
				//hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Bomber"), &MonsterDesc);
				//CHECK_FAILED(hr);
			}
		}

		break; 
	case STATE_NONE:	
		break;
		default:	break;
	}

	return OBJ_NOEVENT;
}

void CGm_ParkSolarPanelOnce::Late_Tick(_float fTimeDelta)
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

HRESULT CGm_ParkSolarPanelOnce::Render()
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

		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_EmissiveTexture", i, TextureType_EMISSIVE);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(m_vecPassIndices[i]);
		CHECK_FAILED(hr);

		LAMP_TYPE eLampType = { LAMP_RED };

		//특정 애님 상태에 따라 텍스처 변경할 메쉬를 체크
		if (STATE_OFFWAIT == m_eCurState)
		{
			if (m_setUpdateMeshs.find(i) != m_setUpdateMeshs.end())
			{
				hr = m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", eLampType);
				CHECK_FAILED(hr);
			}
		}
		
		hr = m_pModelCom->Render(i);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CGm_ParkSolarPanelOnce::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CGm_ParkSolarPanelOnce::Render_IMGUI()
{
	switch (m_eCurState)
	{
	case STATE_OFFWAIT:			ImGui::Text(u8"STATE_OFFWAIT"); break;
	case STATE_CHARGE:			ImGui::Text(u8"STATE_CHARGE"); break;
	case STATE_ONWAITSTART:	ImGui::Text(u8"STATE_ONWAITSTART"); break;
	case STATE_ONWAIT:		ImGui::Text(u8"STATE_ONWAIT"); break;
	case STATE_NONE:	default: ImGui::Text(u8"STATE_NONE"); break;
	}
	
	if (m_IsInteraction) ImGui::Text(u8"Gm_ParkSolarPanelOnce :: IsInteraction : TRUE");
	else ImGui::Text(u8"Gm_ParkSolarPanelOnce :: IsInteraction : FALSE");

	string strGimmickIndex = "Index :" + to_string(m_iGimmickIndex);
	ImGui::Text(strGimmickIndex.c_str());
}
#endif

void CGm_ParkSolarPanelOnce::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
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

HRESULT CGm_ParkSolarPanelOnce::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_SolarPanelOnce_Anim"), 
		TEXT("Com_Model_Anim"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_SolarPanelOnce_NonAnim"), 
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

HRESULT CGm_ParkSolarPanelOnce::Bind_ShaderResources()
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

CGm_ParkSolarPanelOnce* CGm_ParkSolarPanelOnce::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGm_ParkSolarPanelOnce* pInstance = new CGm_ParkSolarPanelOnce(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CGm_ParkSolarPanelOnce"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGm_ParkSolarPanelOnce::Clone(void* pArg)
{
	CGm_ParkSolarPanelOnce* pInstance = new CGm_ParkSolarPanelOnce(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CGm_ParkSolarPanelOnce"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGm_ParkSolarPanelOnce::Free()
{
	__super::Free();

	m_pGameInstance->ReleaseActor(m_pStaticActor);

	Safe_Release(m_pModelCom);
	Safe_Release(m_pNonAnimModelCom);

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
