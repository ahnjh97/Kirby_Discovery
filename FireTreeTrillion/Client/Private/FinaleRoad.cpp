#include "stdafx.h"
#include "FinaleRoad.h"
#include "HitBox.h"

CFinaleRoad::CFinaleRoad(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CPhysXObject{ pDevice ,pContext }
{
}

CFinaleRoad::CFinaleRoad(const CFinaleRoad& rhs)
	:CPhysXObject{ rhs }
{
}


HRESULT CFinaleRoad::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFinaleRoad::Initialize(void* pArg)
{
	ROAD_DESC RoadDesc{};

	if (nullptr != pArg)
		RoadDesc = *(ROAD_DESC*)pArg;

	RoadDesc.fSpeedPerSec = 5.f;
	RoadDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	HRESULT hr;

	hr = __super::Initialize(&RoadDesc);
	CHECK_FAILED(hr);

	hr = Add_Components(RoadDesc.wstrModelName, RoadDesc.bIsAnimModel);
	CHECK_FAILED(hr);

	m_bMotionBlur = true;
	m_bStencil = true;
	m_bRimLight = false;


	m_pDynamicActor = m_pModelCom->ReturnDynamicActor(m_pTransformCom->Get_WorldMatrix());
	m_pDynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

	return S_OK;
}

_int CFinaleRoad::Tick(_float fTimeDelta)
{
	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	__super::Tick(m_fTimeDelta);

	m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_LOOK), m_fTimeDelta * .3f);

	Compute_MotionBlur();

	return _int();
}

void CFinaleRoad::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(m_fTimeDelta);

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	//if (nullptr != m_pDynamicActor)
	//{
	//	m_pTransformCom->Set_WorldMatrix(m_pGameInstance->GetActorAverageMatrix(m_pDynamicActor));
	//}

	m_pDynamicActor->setGlobalPose(CUtils::TransformToPxTransform(m_pTransformCom));

	//시야 벗어나면 컬링
	if (m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}

}

HRESULT CFinaleRoad::Render()
{
	HRESULT hr;
	hr = Bind_ShaderResources(); CHECK_FAILED(hr);

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE); CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_NORMALS); CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_METALNESS); CHECK_FAILED(hr);


		//만약 애님모델이라면 뼈까지 바인딩하고 Anim Model Pass
		if (m_bIsAnimModel)
		{
			hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i); CHECK_FAILED(hr);
			hr = m_pShaderCom->Begin(ANIMMODEL_NORMAL_O); CHECK_FAILED(hr);
		}
		else
		{
			hr = m_pShaderCom->Begin(MODEL_NORMAL_O); CHECK_FAILED(hr);
		}

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CFinaleRoad::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

void CFinaleRoad::Render_IMGUI()
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

void CFinaleRoad::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{

}

HRESULT CFinaleRoad::Add_Components(wstring _strModelTag, _bool _bIsAnimModel)
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC,
		_bIsAnimModel ? TEXT("Prototype_Component_Shader_VtxAnimModel") : TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_") + _strModelTag,
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = OBJECT;

	hr = m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox);
	CHECK_FAILED(hr);

	Set_BodyCollider(COLLIDER_SPHERE, 0.f, 0.f, 1.f);

	return S_OK;
}

HRESULT CFinaleRoad::Bind_ShaderResources()
{
	CHECK_NULLPTR(m_pShaderCom);

	HRESULT hr;

	hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix"); CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW));	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ));	CHECK_FAILED(hr);


	hr = m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool)); CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float)); CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool)); CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4));	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float)); CHECK_FAILED(hr);


	return S_OK;
}

void CFinaleRoad::Make_Particles()
{
}

void CFinaleRoad::Compute_MotionBlur()
{
	_float3 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float3 vScreenPos = CUtils::Make_World_ToScreen(vPos);

	_float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
	_float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

	_float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

	m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
	m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
	m_vMotionVelocity.z = m_ePhyXState != PO_NORMAL ? 1.f : 0.f;

	m_vPreScreenPos = vCurScreenPos;
}

CFinaleRoad* CFinaleRoad::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinaleRoad* pInstance = new CFinaleRoad(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFinaleRoad"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFinaleRoad::Clone(void* pArg)
{
	CFinaleRoad* pInstance = new CFinaleRoad(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CFinaleRoad"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFinaleRoad::Free()
{
	__super::Free();

	m_pGameInstance->ReleaseActor(m_pDynamicActor);

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
