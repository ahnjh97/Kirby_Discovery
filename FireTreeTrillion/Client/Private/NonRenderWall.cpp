#include "stdafx.h"
#include "NonRenderWall.h"

CNonRenderWall::CNonRenderWall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CNonRenderWall::CNonRenderWall(const CNonRenderWall& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CNonRenderWall::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNonRenderWall::Initialize(void* pArg)
{
	GAMEOBJECT_DESC tDesc{};
	if (pArg)
		tDesc = *(GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(&tDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pModelCom->CreateStaticActor(m_pTransformCom->Get_WorldFloat4x4());

	return S_OK;
}

_int CNonRenderWall::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

HRESULT CNonRenderWall::Render()
{
	return S_OK;
}

HRESULT CNonRenderWall::Add_Components()
{
	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_NonRenderWall");
	if (FAILED(__super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

CNonRenderWall* CNonRenderWall::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNonRenderWall* pInstance = new CNonRenderWall(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CNonRenderWall"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNonRenderWall::Clone(void* pArg)
{
	CNonRenderWall* pInstance = new CNonRenderWall(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CNonRenderWall"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNonRenderWall::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}
