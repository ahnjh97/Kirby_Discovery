#include "stdafx.h"
#include "AnimToolObject.h"

#include "Camera_Free.h"
#include "Utils.h"


CAnimToolObject::CAnimToolObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCharacter{ pDevice, pContext }
{
}

CAnimToolObject::CAnimToolObject(const CAnimToolObject& rhs)
	: CCharacter{ rhs }
{
}

HRESULT CAnimToolObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAnimToolObject::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	if (nullptr != pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 7.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	// 카메라 기준으로 움직이기에 미리 받아둔다.
	if (m_pCamera == nullptr)
	{
		m_pCamera = static_cast<CCamera_Free*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Camera"), 0));
		Safe_AddRef(m_pCamera);
	}

	m_pCamera->Set_Target(m_pTransformCom, CCamera::TARGET_FIRST, CCamera::FOCUS_FIRST);
	m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
	m_pModelCom->Set_Animation(0);

	return S_OK;
}

_int CAnimToolObject::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;
	
	//SetOn_Slope(fTimeDelta);

	return OBJ_NOEVENT;
}

void CAnimToolObject::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(fTimeDelta);

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CAnimToolObject::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_X)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CAnimToolObject::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CAnimToolObject::Render_IMGUI()
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

HRESULT CAnimToolObject::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(m_wstrModelName,
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	/*
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	*/

	// for animTool
	m_ppModelForAnimTool = &m_pModelCom;
	m_uModelCnt = 1;
	return S_OK;
}

void CAnimToolObject::Change_ModelCom(wstring _protoTag)
{
	__super::Delete_Component(m_wstrModelName, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	Safe_Release(m_pModelCom);

	HRESULT hr;
	hr = __super::Add_Component(_protoTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);
	m_ppModelForAnimTool = &m_pModelCom;

	m_wstrModelName = _protoTag;
	m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
}

HRESULT CAnimToolObject::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}


void CAnimToolObject::SetOn_Slope(_float fTimeDelta)
{
	// 지면의 up벡터
	PxVec3 slope = m_pControllerCom->Compute_Slope(m_pTransformCom);
	_vector vTerrainNormal = CUtils::To_Vector(slope);
	Lerp_UpVector(vTerrainNormal, 10.f, fTimeDelta);
}

void CAnimToolObject::Lerp_UpVector(_fvector _vTargetUp, _float _maxAngle, _float fTimeDelta)
{
	_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(_vTargetUp, m_vOriginUp));

	// 구면 선형 보간 : m_vOriginUp을 vTargetUp 방향으로 보간
	_float fInterpolateAngle = fTimeDelta * XMConvertToRadians(_maxAngle) * m_fOffsetTurn;

	if (fAngle > fInterpolateAngle) { fAngle = fInterpolateAngle / fAngle; }
	else fAngle = 1.0f;

	m_vOriginUp = XMQuaternionSlerp(m_vOriginUp, _vTargetUp, fAngle);
	m_vOriginUp = XMVector3Normalize(m_vOriginUp);

	_vector vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
	_vector vNewRight = XMVector3Cross(m_vOriginUp, vLook);
	vNewRight = XMVector3Normalize(vNewRight);
	_vector vNewLook = XMVector3Cross(vNewRight, m_vOriginUp);
	vNewLook = XMVector3Normalize(vNewLook);

	// OriginUp을 기준으로 다시 재 설정
	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, vNewRight);
	m_pTransformCom->Set_State(CTransform::STATE_UP, m_vOriginUp);
	m_pTransformCom->Set_State(CTransform::STATE_LOOK, vNewLook);
}

void CAnimToolObject::Set_Animation(_int _iAnimIndex)
{
	m_pModelCom->Set_Animation(_iAnimIndex);
}

_bool CAnimToolObject::isAnimFinish()
{
	if (m_pModelCom == nullptr)
		return false;

	return m_pModelCom->IsFinished();
}

CAnimToolObject* CAnimToolObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAnimToolObject* pInstance = new CAnimToolObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CAnimToolObject"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAnimToolObject::Clone(void* pArg)
{
	CAnimToolObject* pInstance = new CAnimToolObject(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CAnimToolObject"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAnimToolObject::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pControllerCom);
	Safe_Release(m_pCamera);

}
