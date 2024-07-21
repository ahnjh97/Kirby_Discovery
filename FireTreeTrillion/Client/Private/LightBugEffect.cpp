#include "stdafx.h"
#include "LightBugEffect.h"

#include "Effect.h"
#include "Camera_Main.h"
#include "Particle.h"

CLightBugEffect::CLightBugEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CLightBugEffect::CLightBugEffect(const CLightBugEffect& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CLightBugEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLightBugEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	// 반딧불이 세팅
	CParticle::PARTICLE_DESC FXPDesc{};
	FXPDesc.vInitPos = _float3{ 0.f, 0.f, 0.f };
	FXPDesc.vInitScale = { 1.f, 1.f, 1.f };
	Add_Effect("Test_LightBug_WI", FXPDesc, true);

	return S_OK;
}

_int CLightBugEffect::Tick(_float fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	//Billboarding();

	return OBJ_NOEVENT;
}

void CLightBugEffect::Late_Tick(_float fTimeDelta)
{
	//m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
	//m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
}

HRESULT CLightBugEffect::Render()
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

HRESULT CLightBugEffect::Render_LightDepth()
{
	return S_OK;
}

#ifdef _DEBUG
void CLightBugEffect::Render_IMGUI()
{
}
#endif

HRESULT CLightBugEffect::Add_Components()
{
	/* For.Com_Shader */
	//if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
	//	TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
	//	return E_FAIL;

	///* For.Com_Texture */
	//if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_SmokeNormal"),
	//	TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
	//	return E_FAIL;

	///* For.Com_VIBuffer */
	//if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
	//	TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CLightBugEffect::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	HRESULT hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW));
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ));
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("g_fTimeDelta", &m_fShaderTime, sizeof(_float));
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(_float));
	CHECK_FAILED(hr);

	return S_OK;
}

void CLightBugEffect::Billboarding()
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

CLightBugEffect* CLightBugEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLightBugEffect* pInstance = new CLightBugEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CLightBugEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLightBugEffect::Clone(void* pArg)
{
	CLightBugEffect* pInstance = new CLightBugEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CLightBugEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLightBugEffect::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);

	Delete_Effect("Test_LightBug_WI");
}
