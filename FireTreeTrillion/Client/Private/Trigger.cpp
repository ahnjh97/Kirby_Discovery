#include "stdafx.h"
#include "Trigger.h"
#include "GameInstance.h"

CTrigger::CTrigger(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTrigger::CTrigger(const CTrigger & rhs)
	: CGameObject( rhs )
{
}

HRESULT CTrigger::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTrigger::Initialize(void * pArg)
{
	TRIGGER_DESC tTriggerDesc{};
	if (nullptr != pArg) {
		tTriggerDesc = *(TRIGGER_DESC*)pArg;
		m_eTriggerType		= TRIGGER_TYPE(tTriggerDesc.iTriggerType);
		m_iTriggerIndex		= tTriggerDesc.iTriggerIndex;
		m_eCollisionGroup	= tTriggerDesc.eCollisionGroup;
	}

	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	if (FAILED(Add_Components()))
		return E_FAIL;	

	return S_OK;
}

_int CTrigger::Tick(_float fTimeDelta)
{
	//__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CTrigger::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	if (*m_pGameInstance->Get_CurrentLevelID() == LEVEL_TOOL_MAP)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CTrigger::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pShaderCom->Begin(MODEL_TRIGGER))) // Trigger
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

#ifdef _DEBUG
void CTrigger::Render_IMGUI()
{
	//// Guizmo
	//_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
	//m_pGameInstance->EditTransform(matWorld);
	//m_pTransformCom->Set_WorldMatrix(matWorld);
	//
	//ImGui::Separator(); ImGui::NewLine();
}
#endif

HRESULT CTrigger::Bind_ShaderResources()
{
	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iTriggerType", &m_eTriggerType, sizeof(_uint))))
		return E_FAIL;
	return S_OK;
}

HRESULT CTrigger::Add_Components()
{
	HRESULT hr;

	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Model_Trigger"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_RigidBody */
	switch (m_eTriggerType)
	{
	case TRIGGER_CAMERA:
	case TRIGGER_SHADER:
	case TRIGGER_STAR:
	case TRIGGER_LEVELCHANGER:
	case TRIGGER_MONSTER:
	{
		CRigidBody::RIGIDBODY_DESC tRigidDesc(RIGID_BOX, m_pTransformCom->Get_WorldMatrix(), true, false);
		if (FAILED(__super::Add_Component(TEXT("Prototype_Component_RigidBody"),
			TEXT("Com_RigidBody"), (CComponent**)&m_pRigidBodyCom, &tRigidDesc)))
			return E_FAIL;
		m_pRigidBodyCom->SetUp_TriggerType(m_eTriggerType);
		m_pRigidBodyCom->SetUp_TriggerIndex(m_iTriggerIndex);
		m_pRigidBodyCom->Activate(true);
	}
	break;
	}
	return S_OK;
}

CTrigger * CTrigger::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTrigger*		pInstance = new CTrigger(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CTrigger"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CTrigger::Clone(void * pArg)
{
	CTrigger*		pInstance = new CTrigger(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CTrigger"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTrigger::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);	
	Safe_Release(m_pModelCom);
	Safe_Release(m_pRigidBodyCom);
}

