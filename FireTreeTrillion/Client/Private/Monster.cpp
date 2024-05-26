#include "stdafx.h"
#include "Monster.h"

CMonster::CMonster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice , pContext }
{
}

CMonster::CMonster(const CMonster& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CMonster::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonster::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

_int CMonster::Tick(_float fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CMonster::Late_Tick(_float fTimeDelta)
{
}

HRESULT CMonster::Render()
{
	return S_OK;
}

HRESULT CMonster::Render_LightDepth()
{
	return S_OK;
}

void CMonster::Render_IMGUI()
{
}

void CMonster::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pControllerCom);
}
