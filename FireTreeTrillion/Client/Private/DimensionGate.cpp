#include "stdafx.h"
#include "DimensionGate.h"
#include "SpikeSpear.h"
#include "Camera_Main.h"

CDimensionGate::CDimensionGate(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CDimensionGate::CDimensionGate(const CDimensionGate& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CDimensionGate::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDimensionGate::Initialize(void* pArg)
{
	DIMENSIONGATE_DESC* pDimensionGateDesc = nullptr;

	if (nullptr != pArg)
	{
		pDimensionGateDesc = (DIMENSIONGATE_DESC*)pArg;

		pDimensionGateDesc->fSpeedPerSec = 7.f;
		pDimensionGateDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_bSwitch = pDimensionGateDesc->bSwitch;
		m_bCamera = pDimensionGateDesc->bCamera;
		m_vPosition = pDimensionGateDesc->vPosition;
		m_fScale = pDimensionGateDesc->fScale;
	}

	if (FAILED(__super::Initialize(pDimensionGateDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	if (false == m_bSwitch)
	{
		m_vPosition.m128_f32[1] += 10.f;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);

		m_pTransformCom->Turn(XMVectorSet(-1.f, 0.f, 0.f, 0.f), 2.f);

		CSpikeSpear::SPIKESPEAR_DESC SpikeSpearDesc = {};
		SpikeSpearDesc.vPosition = m_vPosition;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Gate"), TEXT("Prototype_GameObject_SpikeSpear"), &SpikeSpearDesc)))
			return E_FAIL;
	}
	else
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);

	if(true == m_bCamera)
		m_pTransformCom->Turn(XMVectorSet(1.f, 0.f, 0.f, 0.f), 1.25f);
	m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);

	m_pModelCom->Set_Animation(1, 60.f, false, false);

	return S_OK;
}

_int CDimensionGate::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	m_fLifeTime += m_fTimeDelta;

	Compute_DimensionGateMaskRatio(m_fTimeDelta);

	//if (true == m_bCamera)
	//{
	//	CCamera_Main* pCameraMain = static_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
	//	CHECK_NULLPTR(pCameraMain);

	//	CTransform* pCameraTransform = pCameraMain->Get_TransformCom();
	//	_vector vCameraLook = -pCameraTransform->Get_State_Vector(CTransform::STATE_LOOK);

	//	_vector		vLook = vCameraLook;
	//	_vector		vRight = XMVector3Cross(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), vLook);
	//	_vector		vUp = XMVector3Cross(vLook, vRight);

	//	_float3		vScaled = m_pTransformCom->Get_Scaled();

	//	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight) * vScaled.x);
	//	m_pTransformCom->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp) * vScaled.y);
	//	m_pTransformCom->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook) * vScaled.z);
	//}

	if(true == m_bCamera)
	{
		if (18.f < m_fLifeTime)
		{
			m_fLifeTime = 0.f;
			m_bSetAnim = true;
		}
	}
	else
	{
		if (6.f < m_fLifeTime)
		{
			m_fLifeTime = 0.f;
			m_bSetAnim = true;
		}
	}
	
	if(true == m_bSetAnim)
	{
		m_bSetAnim = false;
		m_pModelCom->Set_Animation(2, 50.f, false, false);
	}

	if (true == m_pModelCom->IsFinished(2))
		m_bDead = true;

	return OBJ_NOEVENT;
}

void CDimensionGate::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(m_fTimeDelta);


	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SPECIALBLEND, this);
		//m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CDimensionGate::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pTextureCom[TYPE_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture")))
			return E_FAIL;
		if (FAILED(m_pTextureCom[TYPE_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture")))
			return E_FAIL;
		if (FAILED(m_pTextureCom[TYPE_MASK2]->Bind_ShaderResource(m_pShaderCom, "g_MaskTextureSub")))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fDimensionMin", &m_fDimensionMin, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fDimensionMax", &m_fDimensionTime, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_DIMENSIONGATE)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}
	return S_OK;
}

HRESULT CDimensionGate::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CDimensionGate::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}

	//ImGui::Text("RePress : %d", m_bRePressBlock);
	//ImGui::Text("Land : %d", INFO(m_isLanding));

	//ImGui::Text("JUMP : %d", INFO(m_isJump));
	//ImGui::Text("Velocity : %.2f", INFO(m_fJumpVelocity));
	//ImGui::Text("Input C? : %d", m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS));
	//ImGui::Text("FSM : %d", m_pFSM->Get_State());
	ImGui::Separator(); ImGui::NewLine();

	//	ImGui::Text("MoveDir X : %.2f \tMoveDir Y : %.2f \tMoveDir Z : %.2f ", INFO(m_vMoveDir).x, INFO(m_vMoveDir).y, INFO(m_vMoveDir).z); ImGui::NewLine();
	//	ImGui::Text("TargetDir X : %.2f \tTargetDir Y : %.2f \tTargetDir Z : %.2f ", INFO(m_vTargetDir).x, INFO(m_vTargetDir).y, INFO(m_vTargetDir).z);
	__super::Render_IMGUI();
}

#endif

HRESULT CDimensionGate::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_DimensionGateL"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	if (FAILED(__super::Add_Component(LEVEL_FINALBOSS, TEXT("Prototype_Component_Texture_GateDiffuse"),
		TEXT("Com_Texture_Diffuse"), (CComponent**)&m_pTextureCom[TYPE_DIFFUSE])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_FINALBOSS, TEXT("Prototype_Component_Texture_GateMask"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTextureCom[TYPE_MASK])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_FINALBOSS, TEXT("Prototype_Component_Texture_GateMask2"),
		TEXT("Com_Texture_Mask2"), (CComponent**)&m_pTextureCom[TYPE_MASK2])))
		return E_FAIL;

	return S_OK;
}

HRESULT CDimensionGate::Bind_ShaderResources()
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

void CDimensionGate::Compute_DimensionGateMaskRatio(_float fTimeDelta)
{

	m_fDimensionTime += fTimeDelta * (1.f / 18.f);
	if (m_fDimensionTime > 0.8f)
		m_fDimensionTime = 0.8f;
	m_fDimensionMin = (sin(m_fDimensionTime) * 0.15f) + 0.3f;

}

void CDimensionGate::Compute_MotionBlur()
{
}

CDimensionGate* CDimensionGate::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDimensionGate* pInstance = new CDimensionGate(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CDimensionGate"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDimensionGate::Clone(void* pArg)
{
	CDimensionGate* pInstance = new CDimensionGate(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CDimensionGate"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDimensionGate::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

	for (auto pTexture : m_pTextureCom)
		Safe_Release(pTexture);

}
