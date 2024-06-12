#include "stdafx.h"
#include "HitBox.h"
#include "Kirby.h"

CHitBox::CHitBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCharacter{ pDevice, pContext }
{
}

CHitBox::CHitBox(const CHitBox& rhs)
	: CCharacter( rhs )
{
}

HRESULT CHitBox::Initialize_Prototype()
{
	m_eCollisionGroup = HITBOX;

	return S_OK;
}

HRESULT CHitBox::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	HITBOX_DESC* pDesc = (HITBOX_DESC*)pArg;
	m_pOwner = pDesc->pOwner;
	m_pOwnerTransform = m_pOwner->Get_TransformCom();

	return S_OK;
}

_int CHitBox::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	_float4 vRight = XMVector3Normalize(m_pOwnerTransform->Get_State_Float4(CTransform::STATE_RIGHT)) * (-0.05f);
	_float4 vLook  = XMVector3Normalize(m_pOwnerTransform->Get_State_Float4(CTransform::STATE_LOOK)) * 0.8f;
	_float4 vPos   = m_pOwnerTransform->Get_State_Float4(CTransform::STATE_POSITION) + vRight;

	_float4 vNewPos = vLook + _float4(vPos.x, vPos.y + 1.f, vPos.z, 1.f);
	m_pControllerCom->Set_Position(m_pTransformCom, vNewPos);

	return OBJ_NOEVENT;
}

void CHitBox::Late_Tick(_float fTimeDelta)
{
	//if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	//	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CHitBox::Render()
{
	/*if (FAILED(Bind_ShaderResources()))
		return E_FAIL;*/

	//_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	//for (size_t i = 0; i < iNumMeshes; i++)
	//{
	//	if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
	//		return E_FAIL;

	//	if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
	//		return E_FAIL;

	//	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	//	if (FAILED(m_pShaderCom->Begin(1)))
	//		return E_FAIL;

	//	m_pModelCom->Render(i);
	//}

	return S_OK;
}

HRESULT CHitBox::Render_LightDepth()
{
	/*if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;*/

	return S_OK;
}

#ifdef _DEBUG
void CHitBox::Render_IMGUI()
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

_int CHitBox::Check_Collision(_float fTimeDelta)
{
	// HitBox와 충돌된 친구들을 모아봅니다.
	// 충돌된 친구들에게 특정한 함수를 호출시킵니다. like Collision_Overlap
	//auto controllers = m_pControllerCom->Get_Controllers();
	//for(auto& controller : controllers)
	//{
	//	// 히트박스와 충돌처리를 해주는 함수 발동
	//	// controller->HitBox_Attack(this);
	//}

	//m_pControllerCom->Clear_Collisions();  // 충돌 기록 초기화
	return _int();
}

HRESULT CHitBox::Add_Components()
{
	HRESULT hr;

	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
								TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	//hr = __super::Add_Component(TEXT("Prototype_Component_Model_Kabu"),
	//	TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	//CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.uCollisionType = m_eCollisionGroup;
	desc.eType = CCharacterController::CAPSULE;
	desc.tCapsuleShape = { 0.5f, 0.5f };
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
								TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	m_pControllerCom->Set_Object(this);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

	return S_OK;
}

HRESULT CHitBox::Bind_ShaderResources()
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

CHitBox* CHitBox::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHitBox* pInstance = new CHitBox(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CHitBox"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHitBox::Clone(void* pArg)
{
	CHitBox* pInstance = new CHitBox(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CHitBox"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHitBox::Free()
{
	__super::Free();
}

