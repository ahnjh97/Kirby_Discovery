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

	if (m_bUpdate_FXSocketMatrix == true)
	{
		//이펙트에게 위치만 동기화시켜주는 소켓 매트릭스를 업데이트한다.
		m_EffectSocket = _float4x4::Identity;
		CUtils::Set_State_Matrix(m_EffectSocket, CUtils::STATE_POSITION, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	}


	if (m_ePhyXState == PO_FLYAWAY)
	{
		static _float fFlyTime = { 0.f };
		fFlyTime += fTimeDelta;

		if (.1f < fFlyTime)
		{
			CMultiEffect::MULTI_FX_DESC FXDesc{};

			FXDesc.vInitPos = static_cast<_float3>(m_pTransformCom->Get_State(CTransform::STATE_POSITION) - m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			FXDesc.vInitPos += static_cast<_float3>(m_pTransformCom->Get_State(CTransform::STATE_RIGHT)) * CUtils::Make_RandomFloat(-1.f, 1.f);

			FXDesc.vInitScale = { 1.5f, 1.5f, 1.5f };
			if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_FlyingStarYellow"), &FXDesc)))
				return OBJ_NOEVENT;

			fFlyTime = 0.f;
		}
	}
	return OBJ_NOEVENT;
}

_int CPhysXObject::Ready_Dead(_float fDeadScale)
{
	if (m_ePhyXState != PO_KIRBYMOUTH)
	{
		CMultiEffect::MULTI_FX_DESC FXDesc{};
		FXDesc.vInitPos = static_cast<_float3>(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
		FXDesc.vInitRot = CUtils::Make_Degree_FromDir(m_pGameInstance->Get_CamLook());
		FXDesc.vInitScale = { fDeadScale, fDeadScale, fDeadScale };
		if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_ObjDead"), &FXDesc)))
			return OBJ_DEAD;
	}

	Delete_AllEffect();

	return OBJ_DEAD;
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

		//TODO: 버블 날라갈 때 스케일 맞춰야 함
		FXDesc.vInitPos = { 0.f, 0.5f, 0.f };
		FXDesc.vInitRot = { -90.f, 0.f, 0.f };
		FXDesc.vInitScale = { 4.f, 4.f, 4.f };
		FXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();

		Add_Effect("FlyingBubble_v1",  FXDesc, true);

		//if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_FlyingBubble_v1"), &FXDesc)))
		//	return;
		//Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));

	}
	else if (ePreState == PO_FLYAWAY && eState == PO_FLYDEADAWAY)
	{

		Delete_AllEffect();

		CMultiEffect::MULTI_FX_DESC FXDesc{};
		FXDesc.vInitPos = static_cast<_float3>(m_pTransformCom->Get_State(CTransform::STATE_POSITION) - m_pGameInstance->Get_CamLook());
		FXDesc.vInitRot = CUtils::Make_Degree_FromDir(m_pGameInstance->Get_CamLook());
		FXDesc.vInitScale = { 1.f, 1.f, 1.f };

		Add_Effect("Colliding", FXDesc);

		//if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Colliding"), &FXDesc)))
		//	return;
	}
	else if (eState == PO_PRESSED)
	{
		CEffect::FX_DESC FXDesc{};

		FXDesc.vInitPos = static_cast<_float3>(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
		FXDesc.vInitScale = { 1.8f, 1.8f, 1.8f };

		Add_Effect("start particle test A", FXDesc);

		//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_start particle test A"), &FXDesc)))
		//	return;
	}
}


void CPhysXObject::Add_Effect(string strName, _bool bAddToList)
{
	wstring strProtoTag = TEXT("Prototype_GameObject_");
	strProtoTag += CUtils::StrToWstr(strName);

	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), strProtoTag)))
		return;

	if (bAddToList)
	{
		CEffect* pEffect = static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back());

		m_FXList.emplace_back(pEffect);
		Safe_AddRef(pEffect);
	}
}

void CPhysXObject::Add_Effect(string strName, CEffect::FX_DESC fxDesc, _bool bAddToList)
{
	wstring strProtoTag = TEXT("Prototype_GameObject_");
	strProtoTag += CUtils::StrToWstr(strName);

	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), strProtoTag, &fxDesc)))
		return;

	if (bAddToList)
	{
		CEffect* pEffect = static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back());

		m_FXList.emplace_back(pEffect);
		Safe_AddRef(pEffect);
	}
		//Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));

}

void CPhysXObject::Add_Effect(string strName, CParticle::PARTICLE_DESC fxDesc, _bool bAddToList)
{
}

void CPhysXObject::Add_Effect(string strName, CMultiEffect::MULTI_FX_DESC fxDesc, _bool bAddToList)
{
	wstring strProtoTag = TEXT("Prototype_GameObject_");
	strProtoTag += CUtils::StrToWstr(strName);

	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), strProtoTag, &fxDesc)))
		return;

	if (bAddToList)
	{
		CEffect* pEffect = static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back());

		m_FXList.emplace_back(pEffect);
		Safe_AddRef(pEffect);
	}
		//Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
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
	if (m_FXList.empty())
		return;


	for (auto iter = m_FXList.begin(); iter != m_FXList.end();)
	{
		if ((*iter)->Get_Name() == strTag)
		{
			(*iter)->Set_Dead();
			Safe_Release(*iter);
			iter = m_FXList.erase(iter);
			return;
		}
		else
			++iter;
	}
}

void CPhysXObject::Free()
{
	Delete_AllEffect();
	__super::Free();
}


