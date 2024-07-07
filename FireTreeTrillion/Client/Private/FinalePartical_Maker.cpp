#include "stdafx.h"
#include "FinalePartical_Maker.h"

CFinalePartical_Maker::CFinalePartical_Maker(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CFinalePartical_Maker::CFinalePartical_Maker(const CFinalePartical_Maker& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CFinalePartical_Maker::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFinalePartical_Maker::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	for (_int i = 0; i < 300; ++i)
	{
		CFinalePartical* pObj = static_cast<CFinalePartical*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_FinalePartical")));
		m_FinaleParticals.push_back(pObj);
	}

	return S_OK;
}

_int CFinalePartical_Maker::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	for (auto& pPartical : m_FinaleParticals)
	{
		pPartical->Tick(fTimeDelta);
	}

	return OBJ_NOEVENT;
}

void CFinalePartical_Maker::Late_Tick(_float fTimeDelta)
{
	for (auto& pPartical : m_FinaleParticals)
	{
		pPartical->Late_Tick(fTimeDelta);
	}
}

HRESULT CFinalePartical_Maker::Render()
{
	return S_OK;
}

void CFinalePartical_Maker::Make_Partical(_int iNum, _float4 vPos, _float fPosOffset, _float vScale, _float fScaleOffset, _float4 vDir, _float fRandomAngle, _float fPower)
{
	for (_int i = 0; i < iNum; ++i)
	{
		_float4 vOffSetPos = CUtils::Make_Random_Vector(fPosOffset);
		_float4 vNewPos = vPos + vOffSetPos;
		vNewPos.w = 1.f;

		_float4 vNewDir = CUtils::Make_RandomAngle_Vector(fRandomAngle, vDir);
		vNewDir.w = 0.f;

		_float vNewScale = vScale + CUtils::Make_RandomFloat(-fScaleOffset, fScaleOffset);
		
		m_FinaleParticals[m_iCount]->Set_Partical(vNewPos, vNewScale, vNewDir, fPower);
		m_iCount++;

		if (m_iCount >= 300)
			m_iCount = 0;
	}
}

CFinalePartical_Maker* CFinalePartical_Maker::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinalePartical_Maker* pInstance = new CFinalePartical_Maker(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFinalePartical_Maker"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFinalePartical_Maker::Clone(void* pArg)
{
	CFinalePartical_Maker* pInstance = new CFinalePartical_Maker(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CFinalePartical_Maker"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFinalePartical_Maker::Free()
{
	__super::Free();

	for (auto& Partical : m_FinaleParticals)
	{
		Safe_Release(Partical);
	}
	m_FinaleParticals.clear();
}
