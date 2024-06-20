#include "stdafx.h"
#include "TerrainFog.h"

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
	GAMEOBJECT_DESC		GameObjectDesc{};
	GameObjectDesc.fSpeedPerSec = 1.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;



	return S_OK;
}

_int CTerrainFog::Tick(_float fTimeDelta)
{



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

	if (FAILED(m_pShaderCom->Begin(3)))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CTerrainFog::Add_Components()
{
	return S_OK;
}

HRESULT CTerrainFog::Bind_ShaderResources()
{


	return S_OK;
}

void CTerrainFog::Effect_Billboard(_float fTimeDelta)
{
	// 빌보드
	_float3   vScale = m_pTransformCom->Get_Scaled();
	_float4x4      CamMatrix;
	const CTransform* pCamTransform = dynamic_cast<const CTransform*>(m_pGameInstance->Get_Component(LEVEL_GAMEPLAY, TEXT("Layer_Camera"), g_strTransformTag));
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
