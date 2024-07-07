#include "stdafx.h"
#include "BreakableBlock.h"

CBreakableBlock::CBreakableBlock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CBreakableBlock::CBreakableBlock(const CBreakableBlock& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CBreakableBlock::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBreakableBlock::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	// юс╫ц
	m_pModelCom->Set_Animation(0, 60.f, true, false);

}

_int CBreakableBlock::Tick(_float fTimeDelta)
{
	return _int();
}

void CBreakableBlock::Late_Tick(_float fTimeDelta)
{
}

HRESULT CBreakableBlock::Render()
{
	return E_NOTIMPL;
}

HRESULT CBreakableBlock::Add_Components()
{
	return E_NOTIMPL;
}

HRESULT CBreakableBlock::Bind_ShaderResources()
{
	return E_NOTIMPL;
}

CBreakableBlock* CBreakableBlock::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CBreakableBlock::Clone(void* pArg)
{
	return nullptr;
}

void CBreakableBlock::Free()
{
}
