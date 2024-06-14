#include "stdafx.h"
#include "PhysXObject.h"

#include "FSM.h"
#include "MultiEffect.h"
#include "Effect.h"

CPhysXObject::CPhysXObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice , pContext }
{
}

CPhysXObject::CPhysXObject(const CPhysXObject& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CPhysXObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPhysXObject::Initialize(void* pArg)
{
	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	//이펙트에게 위치만 동기화시켜주는 소켓 매트릭스를 업데이트한다.
	m_EffectSocket = _float4x4::Identity;
	CUtils::Set_State_Matrix(m_EffectSocket, CUtils::STATE_POSITION, m_pTransformCom->Get_State(CTransform::STATE_POSITION));

	return S_OK;
}

_int CPhysXObject::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	//이펙트에게 위치만 동기화시켜주는 소켓 매트릭스를 업데이트한다.
	m_EffectSocket = _float4x4::Identity;
	CUtils::Set_State_Matrix(m_EffectSocket, CUtils::STATE_POSITION, m_pTransformCom->Get_State(CTransform::STATE_POSITION));

	return OBJ_NOEVENT;
}

void CPhysXObject::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

HRESULT CPhysXObject::Render()
{
	__super::Render();

	return S_OK;
}

HRESULT CPhysXObject::Render_LightDepth()
{
	__super::Render_LightDepth();

	return S_OK;
}

#ifdef _DEBUG
void CPhysXObject::Render_IMGUI()
{
	__super::Render_IMGUI();

}
#endif

void CPhysXObject::Set_PhyXState(PHYXOBJECT_CURSTATE eState)
{
	PHYXOBJECT_CURSTATE ePreState = m_ePhyXState;
	m_ePhyXState = eState;

	if (ePreState == PO_KIRBYMOUTH && eState == PO_FLYAWAY)
	{
		CMultiEffect::MULTI_FX_DESC FXDesc{};

		//m_pTransformCom->Get_Scaled();

		FXDesc.vInitPos = { 0.f, 0.f, 1.f };
		FXDesc.vInitRot = CUtils::Make_Degree_FromDir(m_pTransformCom->Get_State(CTransform::STATE_UP));
		FXDesc.vInitScale = { 4.f, 4.f, 4.f };
		FXDesc.pSocketMatrix = &m_EffectSocket;

		if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_FlyingBubble_v1"), &FXDesc)))
			return;

		Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
	}
	else if (ePreState == PO_FLYAWAY && eState == PO_FLYDEADAWAY)
	{

		Delete_AllEffect();

		//TODO: 충돌 함수 추가해야해
		/*
		CMultiEffect::MULTI_FX_DESC FXDesc{};

		FXDesc.vInitPos = { 0.f, .3f, 0.f };
		FXDesc.vInitScale = { 5.f, 5.f, 5.f };
		FXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();


		if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_FlyingBubble_v1"), &FXDesc)))
			return;

		Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
		*/
	}
}

void CPhysXObject::Add_Effect(CEffect* pEffect)
{
	m_FXList.emplace_back(pEffect);
	Safe_AddRef(pEffect);
}

void CPhysXObject::Delete_AllEffect()
{
	if (m_FXList.empty())
		return;

	for (auto& FX : m_FXList)
	{
		FX->Set_Dead();
		Safe_Release(FX);
	}

	m_FXList.clear();

}

void CPhysXObject::Delete_Effect(string strTag)
{
}

void CPhysXObject::Free()
{
	__super::Free();

	for (auto& fx : m_FXList)
		Safe_Release(fx);
	m_FXList.clear();
}
