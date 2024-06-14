#include "stdafx.h"
#include "Monster.h"
#include "MultiEffect.h"

CMonster::CMonster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCharacter{ pDevice , pContext }
{
}

CMonster::CMonster(const CMonster& rhs)
	: CCharacter{ rhs }
{
}

HRESULT CMonster::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonster::Initialize(void* pArg)
{
	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	return S_OK;
}

_int CMonster::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CMonster::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

}

HRESULT CMonster::Render()
{
	__super::Render();

	return S_OK;
}

HRESULT CMonster::Render_LightDepth()
{
	return S_OK;
}

#ifdef _DEBUG
void CMonster::Render_IMGUI()
{
	__super::Render_IMGUI();
}
#endif

void CMonster::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}
