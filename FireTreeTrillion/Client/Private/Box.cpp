#include "stdafx.h"
#include "Box.h"
#include "HitBox.h"
#include "Kirby.h"
#include "Camera_Main.h"

CBox::CBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CBox::CBox(const CBox& rhs)
	: CPhysXObject(rhs)
{
}

HRESULT CBox::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBox::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* desc = {};

	if (pArg != nullptr)
		desc = (GAMEOBJECT_DESC*)pArg;


	HRESULT hr;
	hr = __super::Initialize(desc);
	CHECK_FAILED(hr);

	if (desc != nullptr)
		Add_Components(desc->wstrModelName);

	m_bMotionBlur = false;
	m_bRimLight = true;
	m_bStencil = true;

	m_iParticleMesh = m_pModelCom->Find_MeshIndex("WoodPartsM__BoxWoodC");

	unordered_set<_uint> setExcludedMesh = { m_iParticleMesh };

	m_pDynamicActor = m_pNonAnimModelCom->ReturnDynamicActor_FilterByIndex(m_pTransformCom->Get_WorldFloat4x4(), setExcludedMesh, false);
	m_pDynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

	CKirby* pKirby = dynamic_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));

	if(pKirby != nullptr)
		pKirby->RegisterActorsToPlayer_ForBox(m_pDynamicActor, this);

	return S_OK;
}

_int CBox::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	if (true == m_bPlayAnim)
	{
		m_pModelCom->Play_Animation(fTimeDelta);

		if (m_pModelCom->IsFinished())
			m_bDead = true;
	}
		
	return OBJ_NOEVENT;
}

void CBox::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 3.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		if(false == m_bPlayAnim)
			m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CBox::Render()
{
	HRESULT hr;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	if (true == m_bPlayAnim)
	{
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_iParticleMesh, TextureType_DIFFUSE);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", m_iParticleMesh, TextureType_NORMALS);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", m_iParticleMesh, TextureType_METALNESS);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", m_iParticleMesh);
		CHECK_FAILED(hr);
		hr = m_pShaderCom->Begin(ANIMMODEL_NORMAL_O);
		CHECK_FAILED(hr);

		m_pModelCom->Render(m_iParticleMesh);
	}
	else
	{
		for (size_t i = 0; i < iNumMeshes; i++)
		{
			if (i == m_iParticleMesh)
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

			m_pModelCom->Render(i);
		}
	}

	return S_OK;
}

HRESULT CBox::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CBox::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}
	ImGui::Separator(); ImGui::NewLine();

	__super::Render_IMGUI();
}
#endif

void CBox::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (true == m_bPlayAnim)
		return;

	if (eContent == CCollisionCenter::CONTENT_ATTACK)
	{
		CKirby* pKirby = static_cast<CKirby*>(pObject);
		if (pKirby == nullptr)
			return;

		pKirby->Set_HitStop();
		CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
		if (pCamera != nullptr)
			pCamera->Make_Shake(0.5f);

		m_pGameInstance->DisableActor(m_pDynamicActor);
		m_pModelCom->Set_Animation(0, 50.f, false, false);
		m_bPlayAnim = true;
	}
	else if (eContent == CCollisionCenter::CONTENT_ATTACKBULLET)
	{
		CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
		if (pCamera != nullptr)
			pCamera->Make_Shake(0.5f);

		m_pGameInstance->DisableActor(m_pDynamicActor);
		m_pModelCom->Set_Animation(0, 50.f, false, false);
		m_bPlayAnim = true;
	}
	else if (eContent == CCollisionCenter::CONTENT_VACUUMOBJECT)
	{
		CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
		if (pCamera != nullptr)
			pCamera->Make_Shake(0.5f);

		m_pGameInstance->DisableActor(m_pDynamicActor);
		m_pModelCom->Set_Animation(0, 50.f, false, false);
		m_bPlayAnim = true;
	}
}

void CBox::Break_From_Car()
{
	if (true == m_bPlayAnim)
		return;

	CMultiEffect::MULTI_FX_DESC Effectdesc = {};
	Effectdesc.vInitPos = (_float3)m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	Effectdesc.vInitScale = { 2.f, 2.f, 2.f };
	if (FAILED(m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Car Collisions"), &Effectdesc)))
		return;

	m_pGameInstance->PlaySound_Free(L"KirbyCar_BoxCollision.wav", 0.5f);

	CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
	pCamera->Make_Shake(1.6f, 0.5f);
	/*CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
	if (pKirby != nullptr)
		pKirby->Set_HitStop();*/
	m_pGameInstance->Setting_RadialBlur(10.f, 10.f);

	m_pGameInstance->DisableActor(m_pDynamicActor);
	m_pModelCom->Set_Animation(0, 50.f, false, false);
	m_bPlayAnim = true;
}

HRESULT CBox::Add_Components(wstring& wstrModelName)
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = BOXOBJECT;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;

	wstring wstrModeltag = TEXT("Prototype_Component_Model_") + wstrModelName;
	hr = __super::Add_Component(wstrModeltag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	_uint iWstrLength = wstrModelName.length();
	if (iWstrLength > 5)
	{
		wstring wstrNonAnimModelTag = TEXT("Prototype_Component_Model_") + wstrModelName.substr(0, iWstrLength - 5);
		hr = __super::Add_Component(wstrNonAnimModelTag, TEXT("Com_NonAnimModel"), (CComponent**)&m_pNonAnimModelCom);
		CHECK_FAILED(hr);
	}
	
	Set_BodyCollider(COLLIDER_SPHERE, 1.f, 0.f, 2.f);

	return S_OK;
}

HRESULT CBox::Bind_ShaderResources()
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CBox* CBox::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBox* pInstance = new CBox(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBox"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBox::Clone(void* pArg)
{
	CBox* pInstance = new CBox(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CBox"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBox::Free()
{
	__super::Free();

	m_pGameInstance->ReleaseActor(m_pDynamicActor);

	Safe_Release(m_pNonAnimModelCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}

