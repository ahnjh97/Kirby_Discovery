#include "stdafx.h"
#include "Disaster_Master.h"
#include "Baum.h"
#include "FinaleKirby.h"

CDisaster_Master::CDisaster_Master(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CDisaster_Master::CDisaster_Master(const CDisaster_Master& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CDisaster_Master::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDisaster_Master::Initialize(void* pArg)
{
	m_pKirby = (CFinaleKirby*)pArg;
	Safe_AddRef(m_pKirby);

	GAMEOBJECT_DESC Desc = {};
	Desc.fSpeedPerSec = 0.f;
	Desc.fRotationPerSec = 0.f;

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	return S_OK;
}

_int CDisaster_Master::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;
	if (m_pKirby == nullptr)
		return OBJ_DEAD;

	m_fMakeBaumDelay += fTimeDelta;

	// 헛방 바움을 생성한다.
	Make_MissBaum();

	// 캐릭터가 트리거를 밟으면 리얼 지형에 충돌하는 미친 바움을 생성한다.
	Make_OnTerrainBaum();


	return OBJ_NOEVENT;
}

void CDisaster_Master::Late_Tick(_float fTimeDelta)
{

}

void CDisaster_Master::Make_MissBaum()
{
	// 운석은 2.5초 간격으로 생성되며
	// 운석은 2초동안 공중에서 날아든다.
	if (m_fMakeBaumDelay > 2.5f)
	{
		_float4 vKirbyPos = m_pKirby->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
		_float fKirbySpeed = m_pKirby->Get_KirbyInfo()->m_fMoveSpeed;
		_float4 vNewMyPos = _float4(vKirbyPos.x + (fKirbySpeed * 2.f) + 2.f, vKirbyPos.y + 20.f, vKirbyPos.z, 1.f);

		CBaum::BAUMDESC baumdesc = {};
		baumdesc.vBaumMoveDir = vKirbyPos - vNewMyPos;
		baumdesc.fBaumSpeed = 0.5f;
		baumdesc.vPos = vNewMyPos;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_Baum"), TEXT("Prototype_GameObject_Baum"), &baumdesc)))
			return;
		m_fMakeBaumDelay -= 2.5f;
	}
}

void CDisaster_Master::Make_OnTerrainBaum()
{

}

CDisaster_Master* CDisaster_Master::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDisaster_Master* pInstance = new CDisaster_Master(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CDisaster_Master"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDisaster_Master::Clone(void* pArg)
{
	CDisaster_Master* pInstance = new CDisaster_Master(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CDisaster_Master"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDisaster_Master::Free()
{
	__super::Free();
	Safe_Release(m_pKirby);
}
