#include "stdafx.h"
#include "TerrainFog.h"
#include "Camera_Main.h"

CTerrainFog::CTerrainFog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTerrainFog::CTerrainFog(const CTerrainFog& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CTerrainFog::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTerrainFog::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* pGameObjectDesc = nullptr;

	if (nullptr != pArg)
	{
		pGameObjectDesc = (GAMEOBJECT_DESC*)pArg;
	}

	if (FAILED(__super::Initialize(pGameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;
	
	m_fAlpha = 0.2f;
	m_pTransformCom->Set_Scaled(100.f * pGameObjectDesc->matWorld._11, 100.f * pGameObjectDesc->matWorld._22, 1.f * pGameObjectDesc->matWorld._33);

	m_iRandomFog = 2/*CUtils::Make_RandomInt(1, 2)*/;

	return S_OK;
}

_int CTerrainFog::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CTerrainFog::Late_Tick(_float fTimeDelta)
{
	Effect_Billboard(fTimeDelta);
	Compute_ViewZ();
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CTerrainFog::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;
	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */

	if (FAILED(m_pShaderCom->Begin(POSTEX_SOFTALPHAFX)))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CTerrainFog::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_Terrain_Fog"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CTerrainFog::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShaderCom, TEXT("Target_Depth"), "g_DepthTexture")))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_iRandomFog)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

void CTerrainFog::Effect_Billboard(_float fTimeDelta)
{
	// 빌보드
	_float3   vScale = m_pTransformCom->Get_Scaled();
	_float4x4      CamMatrix;

	CTransform* pCamTransform = m_pGameInstance->Get_CurCameraPtr()->Get_TransformCom();
	CamMatrix = pCamTransform->Get_WorldFloat4x4();

	_vector vLook, vRight, vUp;

	vRight = CUtils::Get_State_Vector_Matrix(CamMatrix, CUtils::STATE_RIGHT);
	vLook = CUtils::Get_State_Vector_Matrix(CamMatrix, CUtils::STATE_LOOK);
	vUp = CUtils::Get_State_Vector_Matrix(CamMatrix, CUtils::STATE_UP);

	vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 1.f), vLook);
	vLook = XMVector3Cross(vRight, vUp);
	m_pTransformCom->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook) * vScale.z);
	m_pTransformCom->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp) * vScale.y);
	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight) * vScale.x);
	// 빌보드 끝
}

CTerrainFog* CTerrainFog::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTerrainFog* pInstance = new CTerrainFog(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CTerrainFog"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTerrainFog::Clone(void* pArg)
{
	CTerrainFog* pInstance = new CTerrainFog(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CTerrainFog"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTerrainFog::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
