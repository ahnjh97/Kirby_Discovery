#include "stdafx.h"
#include "OrbitingCamera.h"

_float3 COrbitingCamera::Get_OrbitingCameraPos()
{
	if(nullptr == m_pTransformCom)
		return _float3();

	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);

	return _float3(vPos.x, vPos.y, vPos.z);
}

COrbitingCamera::COrbitingCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) :
	CPartObject{ pDevice, pContext }
{
}

COrbitingCamera::COrbitingCamera(const COrbitingCamera& rhs) : 
	CPartObject{ rhs }
{
}

HRESULT COrbitingCamera::Initialize_Prototype()
{
	return S_OK;
}

HRESULT COrbitingCamera::Initialize(void* pArg)
{
	PARTOBJECT_DESC* tPartDesc = (PARTOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_fRadius = 5.f;

	return S_OK;
}

_int COrbitingCamera::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(0, 0, -m_fRadius, 1));

	return OBJ_NOEVENT;
}

void COrbitingCamera::Late_Tick(_float fTimeDelta)
{
	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentMatrix));
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT COrbitingCamera::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
			return E_FAIL;
		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT COrbitingCamera::Add_Components()
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_Camera");
	if (FAILED(__super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT COrbitingCamera::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

COrbitingCamera* COrbitingCamera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	COrbitingCamera* pInstance = new COrbitingCamera(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : COrbitingCamera"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* COrbitingCamera::Clone(void* pArg)
{
	COrbitingCamera* pInstance = new COrbitingCamera(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : COrbitingCamera"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void COrbitingCamera::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
