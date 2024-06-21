#include "stdafx.h"
#include "Grid.h"

CGrid::CGrid(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CGrid::CGrid(const CGrid& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CGrid::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGrid::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GridDesc{};
	if (pArg)
		GridDesc = *(GAMEOBJECT_DESC*)pArg;

	GridDesc.fSpeedPerSec = 10.f;
	GridDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GridDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	_float fScale = m_pVIBufferCom->Get_Scale();
	_uint iNumX = m_pVIBufferCom->Get_NumVerticesX();
	_uint iNumZ = m_pVIBufferCom->Get_NumVerticesZ();

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(-fScale * iNumX * 0.5f, 0, -fScale * iNumZ * 0.5f, 1));

	return S_OK;
}

_int CGrid::Tick(_float fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CGrid::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT , this);
}

HRESULT CGrid::Render()
{
	if (true == m_bHide)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(1))) // WireFrame
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGrid::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxNorTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(TEXT("Prototype_Component_VIBuffer_Grid"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CGrid::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

CGrid* CGrid::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGrid* pInstance = new CGrid(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CGrid"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGrid::Clone(void* pArg)
{
	CGrid* pInstance = new CGrid(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CGrid"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGrid::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
}
