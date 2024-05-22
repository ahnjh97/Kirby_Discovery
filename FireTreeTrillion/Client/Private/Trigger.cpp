#include "stdafx.h"
#include "Trigger.h"

#include "GameInstance.h"
#include "Level_Loading.h"

CTrigger::CTrigger(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTrigger::CTrigger(const CTrigger & rhs)
	: CGameObject( rhs )
	, m_eChangeLevel(rhs.m_eChangeLevel)
{
}

HRESULT CTrigger::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTrigger::Initialize(void * pArg)
{
	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	if (FAILED(Add_Components()))
		return E_FAIL;	
	
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, -10.f, 0.f, 1.f));

	return S_OK;
}

_int CTrigger::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

}

void CTrigger::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CTrigger::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

void CTrigger::Render_IMGUI()
{
	// Guizmo
	_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
	m_pGameInstance->EditTransform(matWorld);
	m_pTransformCom->Set_WorldMatrix(matWorld);
	
	ImGui::Separator(); ImGui::NewLine();
}


/// <summary> 
/// 셰이더 파일에 행렬(전역변수)들을 넘기는 작업을 진행한다.
/// 1. 월드행렬 → 객체의 m_pTransformCom에서 갖고 있음
/// 2. 뷰행렬, 투영행렬 → PipeLine에서 보관중
/// </summary>
HRESULT CTrigger::Bind_ShaderResources()
{
	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4())))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	HRESULT hr;
	_uint iColliderState = 2;
	hr = m_pShaderCom->Bind_RawValue("g_iColliderState", &iColliderState, sizeof(iColliderState));
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CTrigger::Add_Components()
{
	HRESULT hr;

	///* For.Com_Shader */
	//hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
	//	TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	//CHECK_FAILED(hr);

	///* For.Com_Model */
	//if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Model_ColliderCubeMesh"),
	//	TEXT("Com_Model"),  (CComponent**)&m_pModelCom)))
	//	return E_FAIL;
	
	return S_OK;
}

CTrigger * CTrigger::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTrigger*		pInstance = new CTrigger(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CTrigger"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CTrigger::Clone(void * pArg)
{
	CTrigger*		pInstance = new CTrigger(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CTrigger"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTrigger::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);	
	Safe_Release(m_pModelCom);
}

