#include "stdafx.h"
#include "UnKnownFireMan.h"

CUnKnownFireMan::CUnKnownFireMan(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CUnKnownFireMan::CUnKnownFireMan(const CUnKnownFireMan& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CUnKnownFireMan::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUnKnownFireMan::Initialize(void* pArg)
{
	UNKNOWNFIREDESC desc = {};

	if (pArg != nullptr)
		desc = *(UNKNOWNFIREDESC*)pArg;

	if (FAILED(__super::Initialize(&desc)))
		return E_FAIL;


	CFire::FIREDESC Firedesc = {};
	Firedesc.vFirePos = desc.vFirePos;
	Firedesc.vFirstColor = desc.vFireColor;
	Firedesc.vTargetColor = desc.vTargetColor;
	Firedesc.fUpRange = desc.fUpRange;
	Firedesc.fScale = desc.fScale;
	Firedesc.fTimeRatio = desc.fTimeRatio;

	Firedesc.bPoolingFire = true;
	Firedesc.vMoveDir = desc.vMoveDir;

	_int iCount = desc.iFireCount;

	for (_int i = 0; i < iCount; ++i)
	{
		CFire* pFire = static_cast<CFire*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Fire"), &Firedesc));
		m_vecMyFires.push_back(pFire);
	}

	return S_OK;
}

_int CUnKnownFireMan::Tick(_float fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;

	for (auto& Fire : m_vecMyFires)
		Fire->Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CUnKnownFireMan::Late_Tick(_float fTimeDelta)
{
	for (auto& Fire : m_vecMyFires)
		Fire->Late_Tick(fTimeDelta);
}

HRESULT CUnKnownFireMan::Render()
{
	return S_OK;
}

CUnKnownFireMan* CUnKnownFireMan::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUnKnownFireMan* pInstance = new CUnKnownFireMan(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CUnKnownFireMan"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUnKnownFireMan::Clone(void* pArg)
{
	CUnKnownFireMan* pInstance = new CUnKnownFireMan(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CUnKnownFireMan"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUnKnownFireMan::Free()
{
	__super::Free();

	for (auto& Fire : m_vecMyFires)
		Safe_Release(Fire);

}
