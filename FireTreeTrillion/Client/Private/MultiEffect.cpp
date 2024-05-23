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
    return S_OK;
}

_int CMultiEffect::Tick(_float fTimeDelta)
{
    return OBJ_NOEVENT;
}

void CMultiEffect::Late_Tick(_float fTimeDelta)
{
}

HRESULT CMultiEffect::Render()
{
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
	__super::Free();
}
