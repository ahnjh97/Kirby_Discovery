#include "stdafx.h"
#include "BulbFlare.h"
#include "Camera_Main.h"

CBulbFlare::CBulbFlare(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CBulbFlare::CBulbFlare(const CBulbFlare& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CBulbFlare::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBulbFlare::Initialize(void* pArg)
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

	return S_OK;
}

_int CBulbFlare::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;


	if (m_bStart == true)
	{
		m_fBigScale += fTimeDelta * 50.f;
		if (m_fBigScale > 9.f)
			m_fBigScale = 9.f;

		m_fBigAlpha += fTimeDelta * 1.25f;
		if (m_fBigAlpha > 0.25f)
			m_fBigAlpha = 0.25f;
	}
	else if (m_bStart == false)
	{
		m_fBigScale -= fTimeDelta * 9.f;
		if (m_fBigScale < 0.f)
			m_fBigScale = 0.f;

		m_fBigAlpha -= fTimeDelta * 0.25f;
		if (m_fBigAlpha < 0.f)
			m_fBigAlpha = 0.f;
	}


	m_fTime += fTimeDelta;
	// 0.25 + (  - 0.01 ~ + 0.01 )
	m_fAlpha = m_fBigAlpha + (sin(m_fTime * 3.f) * 0.05f);
	m_fTurn += fTimeDelta * 30.f;
	// 6.f + ( - 0.7 ~ + 0.7 )
	m_fScale = m_fBigScale + (sin(m_fTime * 3.f) * 0.7f);
	if (m_fScale <= 0.f)
		m_fScale = 0.0001f;
	if (m_fAlpha < 0.f)
		m_fAlpha = 0.f;

	m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);



	return OBJ_NOEVENT;
}

void CBulbFlare::Late_Tick(_float fTimeDelta)
{
	Compute_ViewZ();
	Effect_Billboard(fTimeDelta);

	if (m_fBigScale > 0.f)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CBulbFlare::Render()
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

HRESULT CBulbFlare::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_BulbFlare"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBulbFlare::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShaderCom, TEXT("Target_Depth"), "g_DepthTexture")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

void CBulbFlare::Effect_Billboard(_float fTimeDelta)
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

	vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	m_pTransformCom->Turn(vLook, 1.f, m_fTurn);
}

CBulbFlare* CBulbFlare::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBulbFlare* pInstance = new CBulbFlare(pDevice, pContext);
	
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CBulbFlare"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBulbFlare::Clone(void* pArg)
{
	CBulbFlare* pInstance = new CBulbFlare(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Clone : CBulbFlare"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBulbFlare::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);

}
