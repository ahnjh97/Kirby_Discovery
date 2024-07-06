#include "BaumPiece.h"

HRESULT CBaumPiece::Render_LightDepth()
{
	return E_NOTIMPL;
}

HRESULT CBaumPiece::Add_Components()
{
	return E_NOTIMPL;
}

HRESULT CBaumPiece::Bind_ShaderResources()
{
	return E_NOTIMPL;
}

void CBaumPiece::Compute_MotionBlur()
{
}

CBaum* CBaumPiece::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CBaumPiece::Clone(void* pArg)
{
	return nullptr;
}

void CBaumPiece::Free()
{
}
