#include "stdafx.h"
#include "MultiEffect.h"
#include "SingleEffect.h"
#ifdef _DEBUG
#include "FXToolDirector.h"
#endif

CMultiEffect::CMultiEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CEffect{ pDevice, pContext }
{
}

CMultiEffect::CMultiEffect(const CMultiEffect& rhs)
	:CEffect{ rhs }
	, m_FXDesc{ rhs.m_FXDesc }
{
}

void CMultiEffect::Fill_SaveData(MULTI_FX_DATA* pFXData)
{
	pFXData->iNameStrLen = (_uint)m_strFXName.size();
	pFXData->strName = m_strFXName;


	pFXData->iFXsNum = (_uint)m_FXs.size();
	for (auto effect : m_FXs)
	{
		string strName = effect->Get_Name();
		pFXData->FXs.push_back({ (_uint)strName.size(), strName });
	}
}

HRESULT CMultiEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMultiEffect::Initialize_Prototype(MULTI_FX_DESC FXDesc)
{
	m_strFXName = FXDesc.strFXName;
	m_FXDesc = FXDesc;

	return S_OK;
}

HRESULT CMultiEffect::Initialize(void* pArg)
{
	MULTI_FX_DESC FXDesc{};

	if (pArg != nullptr)
	{
		FXDesc = *(MULTI_FX_DESC*)pArg;
	}

	HRESULT hr;

	hr = __super::Initialize(&FXDesc);
	CHECK_FAILED(hr);


#ifdef _DEBUG
	//툴 레벨에서는 tool editor에게 이펙트 포인터를 받아 단순하게 추가해요~
	if (*m_pCurrentLevelID == LEVEL_TOOL_FX)
	{
		m_strFXName = FXDesc.strFXName;

		if (!FXDesc.FXs.empty())
		{
			for (auto& FXName : FXDesc.FXs)
			{
				CEffect* pFX = static_cast<CFXToolDirector*>
					(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_UI"), 0))->Find_Effect(FXName);
				if (nullptr != pFX)
					m_FXs.push_back(pFX);

				m_fDuration.second = pFX->Get_BiggerDuration(m_fDuration.second);
			}
		}
		return S_OK;
	}
#endif



	//클라에서는 정보를 받아 prototype name으로 clone
	if (m_FXDesc.strFXName != "NONE")
	{
		//정보 들어왔을 때 크자이를 같이 전달해 준다.
		if (!m_FXDesc.FXs.empty())
		{

			for (auto& FXName : m_FXDesc.FXs)
			{
				FX_DESC SingleFXDesc{};
				SingleFXDesc.vInitPos = FXDesc.vInitPos;
				SingleFXDesc.vInitRot = FXDesc.vInitRot;
				SingleFXDesc.vInitScale = FXDesc.vInitScale;
				m_bIsColorRender = true;
				SingleFXDesc.bIsColorRender = true;
				SingleFXDesc.pSocketMatrix = FXDesc.pSocketMatrix;

				wstring wstrProtoName = L"Prototype_GameObject_" + CUtils::StrToWstr(FXName);

				CSingleEffect* pFX = static_cast<CSingleEffect*>(m_pGameInstance->Clone_GameObject(wstrProtoName, &SingleFXDesc));
				CHECK_NULLPTR(pFX);

				m_FXs.push_back(pFX);
				m_fDuration.second = pFX->Get_BiggerDuration(m_fDuration.second);

			}
		}
	}

	m_fStartDelay = FXDesc.fStartDelay;

	return S_OK;
}

_int CMultiEffect::Tick(_float fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;

	if (0.f < m_fStartDelay)
	{
		m_fStartDelay -= fTimeDelta;

		if (m_fStartDelay <= 0.f)
			m_fStartDelay = 0.f;

		return OBJ_NOEVENT;
	}

	m_fDuration.first += fTimeDelta;

	if (m_fDuration.second - .05f <= m_fDuration.first && (*m_pCurrentLevelID) != LEVEL_TOOL_FX)
	{
		m_bDead = true;
	}
	else if (m_bIsLoop)
	{
		m_fDuration.first = 0.f;
		for (auto& pEffect : m_FXs)
			pEffect->Reset_Duration();
	}

	for (auto& pEffect : m_FXs)
		pEffect->Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CMultiEffect::Late_Tick(_float fTimeDelta)
{
	if (0.f < m_fStartDelay || m_bDead ||
		(m_fDuration.second - .05f <= m_fDuration.first && (*m_pCurrentLevelID) != LEVEL_TOOL_FX))
		return;

	for (auto& pEffect : m_FXs)
	{
		pEffect->Late_Tick(fTimeDelta);
	}

}

HRESULT CMultiEffect::Render()
{
	if (0.f < m_fStartDelay)
		return S_OK;


	return S_OK;
}

CMultiEffect* CMultiEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMultiEffect* pInstance = new CMultiEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CMultiEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CMultiEffect* CMultiEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MULTI_FX_DESC FXDesc)
{
	CMultiEffect* pInstance = new CMultiEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(FXDesc)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CMultiEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMultiEffect::Clone(void* pArg)
{
	CMultiEffect* pInstance = new CMultiEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : CMultiEffect"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMultiEffect::Free()
{
	for (auto& pEffect : m_FXs)
		Safe_Release(pEffect);

	m_FXs.clear();

	__super::Free();
}
