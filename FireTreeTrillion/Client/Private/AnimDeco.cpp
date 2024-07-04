#include "stdafx.h"
#include "AnimDeco.h"
#include "HitBox.h"
#include "MultiEffect.h"

CAnimDeco::CAnimDeco(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CAnimDeco::CAnimDeco(const CAnimDeco& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CAnimDeco::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAnimDeco::Initialize(void* pArg)
{
	ANIMDECO_DESC tAnimDecoDesc{};
	if (pArg)
		tAnimDecoDesc = *(ANIMDECO_DESC*)pArg;

	if (nullptr == tAnimDecoDesc.pAnimDecoModel)
		return E_FAIL;

	tAnimDecoDesc.matWorld = tAnimDecoDesc.pAnimDecoModel->Get_WorldMatrixForOctree();
	m_pAnimDecoModel = tAnimDecoDesc.pAnimDecoModel;
	Safe_AddRef(m_pAnimDecoModel);
	
	if (FAILED(__super::Initialize(&tAnimDecoDesc)))
		return E_FAIL;

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = ANIMDECO;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;

	string strModelName = m_pAnimDecoModel->Get_ModelName();
	_float fOffSetY{}, fRadius{};
	if ("BushL" == strModelName) {
		fOffSetY = 0.5f;
		fRadius = 0.8f;
		m_wstrNonAnimDecoName = TEXT("BushLRemainder");
	}
	else if ("BushM" == strModelName) {
		fOffSetY = 0.4f;
		fRadius = 0.6f;
		m_wstrNonAnimDecoName = TEXT("BushMRemainder");
	}
	else if ("BushS" == strModelName) {
		fOffSetY = 0.3f;
		fRadius = 0.4f;
		m_wstrNonAnimDecoName = TEXT("BushSRemainder");
	}
	else if ("PopFlower" == strModelName) {
		return S_OK;
	}
	//else if ("BoxWood" == strModelName || "BoxPlastic" == strModelName) {
	//	return S_OK;
	//}


	Set_BodyCollider(COLLIDER_SPHERE, fOffSetY, 0.f, fRadius);

	return S_OK;
}

void CAnimDeco::HideModel()
{
	if (nullptr != m_pAnimDecoModel) 
	{
		m_pAnimDecoModel->Set_Hide(true);

		if (m_wstrNonAnimDecoName != TEXT(""))
		{
			GAMEOBJECT_DESC tDesc{};
			tDesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();
			tDesc.wstrModelName = m_wstrNonAnimDecoName;
			tDesc.fRimWidth = 0.2f;
			tDesc.iShaderVars = 6;
			m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_MapDeco"), TEXT("Prototype_GameObject_NonAnimDeco"), &tDesc);

			CMultiEffect::MULTI_FX_DESC MultiFXDesc{};

			MultiFXDesc.vInitPos = static_cast<_float3>(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			MultiFXDesc.vInitScale = { 1.f, 1.f, 1.f };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Bush Cut"), &MultiFXDesc)))
				return;
		}

		Set_Dead();
	}
}

CAnimDeco* CAnimDeco::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAnimDeco* pInstance = new CAnimDeco(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CAnimDeco"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAnimDeco::Clone(void* pArg)
{
	CAnimDeco* pInstance = new CAnimDeco(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CAnimDeco"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAnimDeco::Free()
{
	__super::Free();

	Safe_Release(m_pAnimDecoModel);
}
