#include "stdafx.h"
#include "Gm_LabBossRoomDoor.h"
#include "HitBox.h"
#include "Kirby.h"
#include "BreakableRockParticle.h"

CGm_LabBossRoomDoor::CGm_LabBossRoomDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CGm_LabBossRoomDoor::CGm_LabBossRoomDoor(const CGm_LabBossRoomDoor& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CGm_LabBossRoomDoor::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGm_LabBossRoomDoor::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = { nullptr };

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_eAnimState = STATE_WAIT;
	m_pModelCom->Set_Animation(STATE_WAIT, 0.f, TRUE /*_bool bInterpolation = false, _float fLerpTime = 0.1f*/);

	//숨길 메쉬 검색하여 저장
	m_setBeforeHideMeshs.insert(m_pModelCom->Find_MeshIndex(string("LeftDoor2M__LbBossRoomDoorC")));
	m_setBeforeHideMeshs.insert(m_pModelCom->Find_MeshIndex(string("RightDoor2M__LbBossRoomDoorC")));

	m_setAfterHideMeshs.insert(m_pModelCom->Find_MeshIndex(string("LeftDoorM__LbBossRoomDoorC")));
	m_setAfterHideMeshs.insert(m_pModelCom->Find_MeshIndex(string("RightDoorM__LbBossRoomDoorC")));

	return S_OK;
}

_int CGm_LabBossRoomDoor::Tick(_float fTimeDelta)
{
	//if (TRUE == m_bDead)
	//	return OBJ_DEAD;

	//컷씬에서 특정 시간 경과할 경우 애님 변경
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD9, KEY_DOWN)) //테스트용. 현재는 키입력으로 확인 가능
		m_bStartAnim = TRUE;

	if (m_bStartAnim)
	{
		m_fBreakAnimTime += fTimeDelta;
		m_pModelCom->Set_Animation(STATE_JOLT, 100.f, TRUE);
		m_eAnimState = STATE_JOLT;

		if (m_fBreakAnimTime > 4.f)
		{
			m_pModelCom->Set_Animation(STATE_BREAK, 60.f, FALSE);
			m_fBreakAnimTime = 0.f;
			m_bStartAnim = FALSE;
			m_eAnimState = STATE_BREAK;
		}
	}

	return OBJ_NOEVENT;
}

void CGm_LabBossRoomDoor::Late_Tick(_float fTimeDelta)
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

HRESULT CGm_LabBossRoomDoor::Render()
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

		hr = m_pShaderCom->Begin(ANIMMODEL_NORMAL_O);
		CHECK_FAILED(hr);

		//특정 애님 상태에 따라 렌더할 메쉬를 체크
		if (STATE_WAIT == m_eAnimState)
		{
			if (m_setBeforeHideMeshs.find(i) != m_setBeforeHideMeshs.end())
				continue;
		}
		if (STATE_WAIT != m_eAnimState)
		{
			//해당 메쉬 렌더안하면 좀 짜지는 부분이 있는데 일단 추가
			if (m_setAfterHideMeshs.find(i) != m_setAfterHideMeshs.end())
				continue;
		}
		
		hr = m_pModelCom->Render(i);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CGm_LabBossRoomDoor::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CGm_LabBossRoomDoor::Render_IMGUI()
{
	switch (m_eAnimState)
	{
	case STATE_WAIT:	ImGui::Text(u8"STATE_WAIT");	break;
	case STATE_JOLT:	ImGui::Text(u8"STATE_JOLT");	break;
	case STATE_BREAK:	ImGui::Text(u8"STATE_BREAK"); break;
	case STATE_NONE:	default: ImGui::Text(u8"STATE_NONE"); break;
	}
}
#endif

void CGm_LabBossRoomDoor::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	/*
	if (true == m_bStartAnimation)
		return;

	CKirby* pKirby = static_cast<CKirby*>(pObject);
	if (pKirby->Get_KirbyInfo()->m_bBooster == false)
		return;

	pKirby->Set_HitStop();
	m_pModelCom->Set_Animation(0, 60.f, false, false);
	m_bStartAnimation = true;
	SwitchAfterBefore();

	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float4 vPlayerPos = pObject->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
	_float4 vDir = vPos - vPlayerPos;
	vDir.Normalize();
	m_vDamegeDir = (_float3)vDir;
	m_fHitPower = pKirby->Get_KirbyInfo()->m_fMoveSpeed;
	*/
}

HRESULT CGm_LabBossRoomDoor::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_LbBossRoomDoor_Anim"), 
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CGm_LabBossRoomDoor::Bind_ShaderResources()
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

CGm_LabBossRoomDoor* CGm_LabBossRoomDoor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGm_LabBossRoomDoor* pInstance = new CGm_LabBossRoomDoor(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CGm_LabBossRoomDoor"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGm_LabBossRoomDoor::Clone(void* pArg)
{
	CGm_LabBossRoomDoor* pInstance = new CGm_LabBossRoomDoor(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CGm_LabBossRoomDoor"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGm_LabBossRoomDoor::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
