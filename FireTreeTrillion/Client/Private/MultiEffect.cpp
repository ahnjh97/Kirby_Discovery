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

CMultiEffect* CMultiEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CMultiEffect* CMultiEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MULTI_FX_DESC EditEffectDesc)
{
    return nullptr;
}

CGameObject* CMultiEffect::Clone(void* pArg)
{
    return nullptr;
}

void CMultiEffect::Free()
{
}
