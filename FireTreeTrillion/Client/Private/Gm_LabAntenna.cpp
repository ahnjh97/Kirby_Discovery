#include "stdafx.h"
#include "Gm_LabAntenna.h"
#include "HitBox.h"
#include "Kirby.h"
#include "BreakableRockParticle.h"
#include "Bone.h"

CGm_LabAntenna::CGm_LabAntenna(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CGm_LabAntenna::CGm_LabAntenna(const CGm_LabAntenna& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CGm_LabAntenna::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGm_LabAntenna::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = { nullptr };

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_bStencil = true;

	m_eAnimState = STATE_WAIT;
	m_pModelCom->Set_Animation(STATE_WAIT, 0.f, TRUE /*_bool bInterpolation = false, _float fLerpTime = 0.1f*/);

	//잔존 메쉬 검색하여 저장
	m_setDebrisMeshs.insert(m_pModelCom->Find_MeshIndex(string("DebrisMesh__LbLastBossOutFrameC")));
	m_setDebrisMeshs.insert(m_pModelCom->Find_MeshIndex(string("DebrisMesh__LbLastBossOutFrame2C")));

	return S_OK;
}

_int CGm_LabAntenna::Tick(_float fTimeDelta)
{
	return OBJ_DEAD;
	if (TRUE == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	if (m_bCollision == true)
	{
		m_fAlpha += m_fTimeDelta * 0.2f;
		if (m_fAlpha > 1.f)
			m_fAlpha = 1.f;
	}

	return OBJ_NOEVENT;
}

void CGm_LabAntenna::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(m_fTimeDelta);

	////에피리스 공격패턴에 해당 오브젝트가 피격당할 경우, Break 애님 재생
	//if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD9, KEY_DOWN)) //테스트용. 현재는 키입력으로 확인 가능
	//	m_pModelCom->Set_Animation(STATE_BREAK, 60.f, FALSE);

#pragma region FRUSTUM_CULLING

	//절두체 컬링 처리
	if (TRUE == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 50.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}

#pragma endregion

	//애니메이션 재생종료 시 Set_Dead >> 디졸브 효과 추가 필요
	if (TRUE == m_pModelCom->IsFinished())
	{
		m_eAnimState = STATE_BREAK;
		Set_Dead();
	}
}

HRESULT CGm_LabAntenna::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	HRESULT hr;

	//특정 애님 상태에 따라 렌더할 메쉬를 체크
	if (STATE_BREAK == m_eAnimState)
	{
		for (size_t i = 0; i < iNumMeshes; i++)
		{
			if (m_setDebrisMeshs.find(i) == m_setDebrisMeshs.end())
				continue;
			
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

			hr = m_pModelCom->Render(i);
			CHECK_FAILED(hr);
		}
	}
	else
	{
		for (size_t i = 0; i < iNumMeshes; i++)
		{
			hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
			CHECK_FAILED(hr);

			hr = m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(_float));
			CHECK_FAILED(hr);

			m_pTextureComMask->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture");

			hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
			CHECK_FAILED(hr);

			hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS);
			CHECK_FAILED(hr);

			hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
			CHECK_FAILED(hr);

			hr = m_pShaderCom->Begin(ANIMMODEL_ANTENNA);
			CHECK_FAILED(hr);

			hr = m_pModelCom->Render(i);
			CHECK_FAILED(hr);
		}
	}

	return S_OK;
}

HRESULT CGm_LabAntenna::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CGm_LabAntenna::Render_IMGUI()
{
	switch (m_eAnimState)
	{
	case STATE_WAIT:	ImGui::Text(u8"STATE_WAIT");	break;
	case STATE_BREAK:	ImGui::Text(u8"STATE_BREAK"); break;
	case STATE_NONE:	default: ImGui::Text(u8"STATE_NONE"); break;
	}
}
#endif

void CGm_LabAntenna::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	
	if (m_bCollision == true)
		return;


	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	vPos.y += 5.f;
	CMultiEffect::MULTI_FX_DESC Effectdesc = {};
	Effectdesc.vInitPos = (_float3)vPos;
	Effectdesc.vInitRot = CUtils::Make_Degree_FromDir(m_pGameInstance->Get_CamLook());
	Effectdesc.vInitScale = { 20.f, 20.f, 20.f };
	if (FAILED(m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Car Collisions"), &Effectdesc)))
		return;

	CMultiEffect::MULTI_FX_DESC MDesc = {};
	MDesc.vInitPos = (_float3)vPos;
	MDesc.vInitRot = CUtils::Make_Degree_FromDir(m_pGameInstance->Get_CamLook());
	MDesc.vInitScale = { 1.2f, 1.2f , 1.2f };
	this->Add_Effect("YW CarCenter Crash Effects", MDesc, false);


	m_bCollision = true;
	m_pModelCom->Set_Animation(STATE_BREAK, 60.f, FALSE);
}

HRESULT CGm_LabAntenna::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_LbAntenna_Anim"), 
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_FireDissolve"),
		TEXT("Com_TextureMask"), (CComponent**)&m_pTextureComMask);
	CHECK_FAILED(hr);

#pragma region HITBOX

	//히트박스. 보스 패턴 중 충돌하면 해당 오브젝트 Break를 위해 생성
	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = FINALGM;
	HitBox.pSocket = m_pModelCom->Get_BonePtrByIndex(0);
	HitBox.vBoneOffset = _float3(0.f, 5.f, 13.f);
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;

	Set_BodyCollider(COLLIDER_SPHERE, 0.f, 0.f, 15.f);

#pragma endregion

	return S_OK;
}

HRESULT CGm_LabAntenna::Bind_ShaderResources()
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

CGm_LabAntenna* CGm_LabAntenna::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGm_LabAntenna* pInstance = new CGm_LabAntenna(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CGm_LabAntenna"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGm_LabAntenna::Clone(void* pArg)
{
	CGm_LabAntenna* pInstance = new CGm_LabAntenna(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CGm_LabAntenna"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGm_LabAntenna::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureComMask);
}
