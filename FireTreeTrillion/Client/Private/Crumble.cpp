#include "stdafx.h"
#include "Crumble.h"
#include "HitBox.h"

CCrumble::CCrumble(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CCrumble::CCrumble(const CCrumble& rhs)
	: CPhysXObject(rhs)
{
}

HRESULT CCrumble::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCrumble::Initialize(void* pArg)
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
	m_pDynamicActor = m_pNonAnimModelCom->ReturnDynamicActor(m_pTransformCom->Get_WorldFloat4x4());
	m_pDynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

	return S_OK;
}

_int CCrumble::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_pModelCom->Play_Animation(fTimeDelta);

	return OBJ_NOEVENT;
}

void CCrumble::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CCrumble::Render()
{
	HRESULT hr;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
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

		m_pModelCom->Render(i);
	}
	
	return S_OK;
}

HRESULT CCrumble::Render_LightDepth()
{
	HRESULT hr = m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

#ifdef _DEBUG
void CCrumble::Render_IMGUI()
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

void CCrumble::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

void CCrumble::Add_Components(wstring& wstrModelName)
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = OBJECT;
	hr = m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox);
	CHECK_FAILED(hr);
	Set_BodyCollider(COLLIDER_SPHERE, 1.f, 0.f, 1.f);

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
}

HRESULT CCrumble::Bind_ShaderResources()
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
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
	//	return E_FAIL;

	return S_OK;
}

CCrumble* CCrumble::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCrumble* pInstance = new CCrumble(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CCrumble"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCrumble::Clone(void* pArg)
{
	CCrumble* pInstance = new CCrumble(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CCrumble"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCrumble::Free()
{
	__super::Free();

	m_pGameInstance->ReleaseActor(m_pDynamicActor);

	Safe_Release(m_pNonAnimModelCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}

