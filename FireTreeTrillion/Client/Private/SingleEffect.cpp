#include "stdafx.h"
#include "SingleEffect.h"

CSingleEffect::CSingleEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CEffect{ pDevice, pContext }
{
}

CSingleEffect::CSingleEffect(const CSingleEffect& rhs)
    :CEffect{rhs}
    , m_FXDesc{rhs.m_FXDesc}
{
}

CSingleEffect* CSingleEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CSingleEffect* CSingleEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, FX_DESC EditEffectDesc)
{
    return nullptr;
}

CGameObject* CSingleEffect::Clone(void* pArg)
{
    return nullptr;
}

void CSingleEffect::Free()
{
}
