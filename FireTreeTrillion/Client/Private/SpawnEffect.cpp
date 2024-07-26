#include "stdafx.h"
#include "SpawnEffect.h"
#include "Effect.h"
#include "Camera_Main.h"

CSpawnEffect::CSpawnEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CSpawnEffect::CSpawnEffect(const CSpawnEffect& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CSpawnEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSpawnEffect::Initialize(void* pArg)
{
	SPAWNEFFECT_DESC* pSpawnEffectDesc = nullptr;

	if (nullptr != pArg)
	{
		pSpawnEffectDesc = (SPAWNEFFECT_DESC*)pArg;

		pSpawnEffectDesc->fSpeedPerSec = 7.f;
		pSpawnEffectDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_bTrail = pSpawnEffectDesc->bTrail;
		m_vPosition = pSpawnEffectDesc->vPosition;
		m_fScale = pSpawnEffectDesc->fScale;
	}

	if (FAILED(__super::Initialize(pSpawnEffectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);
	m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);

	m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), ToRadian(CUtils::Make_RandomFloat(0.f, 90.f)));

	CEffect::FX_DESC FXDesc{};

	FXDesc.vInitPos = GET_POS;
	//FXDesc.vInitRot = { CUtils::Make_RandomFloat(0.f, 90.f), 0.f, 0.f };
	//FXDesc.vInitScale = { 1.f, 1.f, 1.f };
	//FXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();

	Add_Effect("ParkParticle5JS", FXDesc, true);
	Add_Effect("ParkParticle4JS", FXDesc, true);

	return S_OK;
}

_int CSpawnEffect::Tick(_float fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	m_fShaderTime += m_fTimeDelta;

	if (true == m_bTrail)
	{
		if (2.f < m_fShaderTime)
			m_bDead = true;

		if (0.f < m_fAlpha)
			m_fAlpha -= m_fTimeDelta * 0.5f;
		else
			m_fAlpha = 0.f;
		m_pTransformCom->Set_Scaled(m_fScale * m_fAlpha, m_fScale * m_fAlpha, m_fScale * m_fAlpha);
	}
	else
	{
		if (2.f < m_fShaderTime)
			m_bDead = true;
		else if (1.f < m_fShaderTime)
		{
			m_pTransformCom->Set_Scaled(m_fScale * m_fAlpha, m_fScale * m_fAlpha, m_fScale * m_fAlpha);

			m_fAlpha -= m_fTimeDelta * 0.5f;
		}
	}

	Billboarding();

	return OBJ_NOEVENT;
}

void CSpawnEffect::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
}

HRESULT CSpawnEffect::Render()
{
	HRESULT hr;

	hr = Bind_ShaderResources();
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Begin(23);
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Bind_Buffers();
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Render();
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CSpawnEffect::Render_LightDepth()
{
	return S_OK;
}

#ifdef _DEBUG
void CSpawnEffect::Render_IMGUI()
{
}
#endif

HRESULT CSpawnEffect::Add_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture_Diffuse */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_SmokeNormal"),
		TEXT("Com_Texture_Diffuse"), (CComponent**)&m_pTextureCom[TYPE_DIFFUSE])))
		return E_FAIL;

	/* For.Com_Texture_Mask */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_ParkSmoke"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTextureCom[TYPE_MASK])))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSpawnEffect::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	HRESULT hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW));
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ));
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TYPE_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture");
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TYPE_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture");
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_RawValue("g_fTimeDelta", &m_fShaderTime, sizeof(_float));
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(_float));
	CHECK_FAILED(hr);

	return S_OK;
}

void CSpawnEffect::Billboarding()
{
	CCamera_Main* pCameraMain = static_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
	CHECK_NULLPTR(pCameraMain);

	CTransform* pCameraTransform = pCameraMain->Get_TransformCom();
	_vector vCameraLook = pCameraTransform->Get_State_Vector(CTransform::STATE_LOOK);

	_vector		vLook = vCameraLook;
	_vector		vRight = XMVector3Cross(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), vLook);
	_vector		vUp = XMVector3Cross(vLook, vRight);

	_float3		vScaled = m_pTransformCom->Get_Scaled();

	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight) * vScaled.x);
	m_pTransformCom->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp) * vScaled.y);
	m_pTransformCom->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook) * vScaled.z);
}

CSpawnEffect* CSpawnEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSpawnEffect* pInstance = new CSpawnEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSpawnEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSpawnEffect::Clone(void* pArg)
{
	CSpawnEffect* pInstance = new CSpawnEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CSpawnEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSpawnEffect::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	for (size_t i = 0; i < TYPE_END; i++)
		Safe_Release(m_pTextureCom[i]);
	Safe_Release(m_pVIBufferCom);

	Delete_Effect("ParkParticle3JS");
	Delete_Effect("ParkParticle4JS");
}
