#include "stdafx.h"
#include "Ladder.h"
#include "CollisionCenter.h"


CLadder::CLadder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CLadder::CLadder(const CLadder& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CLadder::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLadder::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	if (nullptr != pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 7.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(40.33f, 8.f, -32.5f, 1.f));

	if (FAILED(Add_Components()))
		return E_FAIL;

	_float3 fScale = m_pTransformCom->Get_Scaled();
	m_fHeight *= fScale.y;

	return S_OK;
}

_int CLadder::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;




	CCollisionCenter::Get_Instance()->Add_Ladder(this);
	return OBJ_NOEVENT;
}

void CLadder::Late_Tick(_float fTimeDelta)
{
	//if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CLadder::Render()
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
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

#ifdef _DEBUG
void CLadder::Render_IMGUI()
{
}
#endif


_bool CLadder::Is_Collide(_fvector vPos)
{
	// X 와 Z 검사를 진행한다.
	_float4 vLadderPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float4 vLadderLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	_float4 vKirbyPos = vPos;

	_float2 vMyXZOffSet = { vLadderLook.x * 0.8f, vLadderLook.z * 0.8f };
	_float2 vMyXZ = { vLadderPos.x, vLadderPos.z };

	vMyXZ -= vMyXZOffSet;

	_float2 vKirbyXZ = { vKirbyPos.x, vKirbyPos.z };

	_float fLadder_XZDistance = XMVectorGetX(XMVector2Length(vMyXZ - vKirbyXZ));

	// 2차원 범위가 지정된 거리보다 멀 경우 충돌 가능성이 0 이기 때문에 return false 시킨다.
	if (m_fRadius < fLadder_XZDistance)
		return false;

	// 사다리 위 아래 보정치
	_float fOffSet = 0.5f;
	_float fMinY = vLadderPos.y - fOffSet;
	_float fMaxY = vLadderPos.y + m_fHeight + fOffSet;

	// 사다리의 최소Y, 최대Y를 비교했을 때, 안에 들었다면 최종적인 충돌을 하였을 것이다.
	if (fMinY < vKirbyPos.y && vKirbyPos.y < fMaxY)
	{
		//vMyXZ.y 는 포지션의 z값으로 쓰이기 때문에 나중에 헷갈리지 말라.
		m_vLadderPoint = { vMyXZ.x, vKirbyPos.y + 1.f, vMyXZ.y, 1.f };
		m_vLadderOriginalPos = vLadderPos;
		return true;
	}

	// 위 조건에서 true로 빠져나가지 못 했다면, 충돌을 하지 않은 것이다.
	return false;
}

HRESULT CLadder::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	// 커비의 기본 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Ladder"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CLadder::Bind_ShaderResources()
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

CLadder* CLadder::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLadder* pInstance = new CLadder(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CLadder"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLadder::Clone(void* pArg)
{
	CLadder* pInstance = new CLadder(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CLadder"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLadder::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

}
