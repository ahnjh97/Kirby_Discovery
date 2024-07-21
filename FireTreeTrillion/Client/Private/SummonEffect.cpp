#include "stdafx.h"
#include "SummonEffect.h"
#include "Camera_Main.h"
#include "Bomber.h"

CSummonEffect::CSummonEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CSummonEffect::CSummonEffect(const CSummonEffect& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CSummonEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSummonEffect::Initialize(void* pArg)
{
	SUMMONEFFECT_DESC* pSummonEffectDesc = nullptr;

	if (nullptr != pArg)
	{
		pSummonEffectDesc = (SUMMONEFFECT_DESC*)pArg;

		pSummonEffectDesc->fSpeedPerSec = 7.f;
		pSummonEffectDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_vPosition = pSummonEffectDesc->vPosition; 
		m_fAlpha = pSummonEffectDesc->fAlpha;
		m_fScale = pSummonEffectDesc->fScale;
	}

	if (FAILED(__super::Initialize(pSummonEffectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_fRatio = 1.f;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);
	m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);

	CEffect::FX_DESC FXDesc{};

	FXDesc.vInitPos = m_vPosition;
	//FXDesc.vInitRot = { CUtils::Make_RandomFloat(0.f, 90.f), 0.f, 0.f };
	//FXDesc.vInitScale = { 1.f, 1.f, 1.f };
	//FXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();

	Add_Effect("ParticleSummonJS", FXDesc, false);

	m_vColor = XMVectorSet(1.f, 0.f, 0.f, 0.f);

	return S_OK;
}

_int CSummonEffect::Tick(_float fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), ToRadian(CUtils::Make_RandomFloat(0.f, 360.f)));

	if (0.f < m_fRatio)
	{
		m_fRatio -= m_fTimeDelta * 0.75f;
		m_pTransformCom->Set_Scaled(m_fScale * m_fRatio, m_fScale * m_fRatio, m_fScale * m_fRatio);
	}
	else
	{
		//HRESULT hr;
		//_float4x4 matWorld = XMMatrixIdentity();
		//matWorld._41 = 35.5f;
		//matWorld._42 = 75.f;
		//matWorld._43 = 175.5f;
		//matWorld._44 = 1.f;
		//CMonster::MONSTER_DESC MonsterDesc = {};
		//MonsterDesc.matWorld = matWorld;
		//hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Bomber"), &MonsterDesc);
		//CHECK_FAILED(hr);

		m_bDead = true;
	}

	Compute_ViewZ();
	Billboarding();

	return OBJ_NOEVENT;
}

void CSummonEffect::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
}

HRESULT CSummonEffect::Render()
{
	HRESULT hr;

	hr = Bind_ShaderResources();
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Begin(25);
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Bind_Buffers();
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Render();
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CSummonEffect::Render_LightDepth()
{
	return S_OK;
}

#ifdef _DEBUG
void CSummonEffect::Render_IMGUI()
{
}
#endif

HRESULT CSummonEffect::Add_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture_Diffuse */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_SpawnEffect"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSummonEffect::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	HRESULT hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW));
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ));
	CHECK_FAILED(hr);

	_bool bStencil = false;
	_bool bRimLight = false;
	_bool bMotionBlur = false;
	m_pShaderCom->Bind_RawValue("g_bStencil", &bStencil, sizeof(_bool));
	m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool));
	m_pShaderCom->Bind_RawValue("g_bMotionBlur", &bMotionBlur, sizeof(_bool));

	hr = m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture");
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_vector));
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(_float));
	CHECK_FAILED(hr);

	return S_OK;
}

void CSummonEffect::Billboarding()
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

CSummonEffect* CSummonEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSummonEffect* pInstance = new CSummonEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSummonEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSummonEffect::Clone(void* pArg)
{
	CSummonEffect* pInstance = new CSummonEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CSummonEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSummonEffect::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
