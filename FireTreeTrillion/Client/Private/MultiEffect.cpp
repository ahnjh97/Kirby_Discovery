#include "stdafx.h"
#include "MultiEffect.h"

CMultiEffect::CMultiEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CEffect{ pDevice, pContext }
{
}

CMultiEffect::CMultiEffect(const CMultiEffect& rhs)
    :CEffect{ rhs }
    , m_FXDesc{ rhs.m_FXDesc }
{
}

HRESULT CMultiEffect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMultiEffect::Initialize_Prototype(MULTI_FX_DESC FXDesc)
{


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
	if (m_fDuration.second < m_fDuration.first && (*m_pCurrentLevelID) != LEVEL_TOOL_FX)
	{
		m_bDead = true;
	}
	else if(m_bIsLoop)
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
	if (0.f < m_fStartDelay)
		return;


	if (m_bIsColorRender)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);

	if (m_bIsBloom)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
}

HRESULT CMultiEffect::Render()
{
	if (0.f < m_fStartDelay)
		return S_OK;

	for (auto& pEffect : m_FXs)
	{
		if (FAILED(pEffect->Render()))
		{
			_ASSERT_EXPR(FALSE, TEXT("Failed To Render Composite Effect"));
			return E_FAIL;
		}
	}

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

	return pInstance;;
}

void CMultiEffect::Free()
{
	for (auto& pEffect : m_FXs)
		Safe_Release(pEffect);
	m_FXs.clear();

	__super::Free();
}
