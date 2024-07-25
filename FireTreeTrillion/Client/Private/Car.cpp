#include "stdafx.h"
#include "Car.h"
#include "HitBox.h"

CCar::CCar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CDeform{ pDevice, pContext }
{
}

CCar::CCar(const CCar& rhs)
	: CDeform{ rhs }
{
}

HRESULT CCar::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCar::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_eDeformType = DEFORM_CAR;
	m_eAnimIndex = CAR_FALL;
	m_pModelCom->Set_Animation(m_eAnimIndex, 60.f, true, false);

	m_bMotionBlur = true;

	CParticle::PARTICLE_DESC ParticleDesc{};
	ParticleDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	ParticleDesc.vInitPos = _float3(0.f, 0.5f, 0.f);
	Add_Effect("YW Deform Ora", ParticleDesc, true);

	return S_OK;
}

_int CCar::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	// 구현부
	Compute_MotionBlur();
	Compute_DeformRimLight(m_fTimeDelta);
	Set_Animation();


	if (m_ePhyXState == PO_VACUUMING)
	{
		Set_BodyCollider(COLLIDER_SPHERE, 1.f, 2.f, 1.f);
		m_eAnimIndex = CAR_SHAKE;
	}


	if (m_eAnimIndex == CAR_WAIT)
	{
		m_pControllerCom->FreeFall(m_pTransformCom, m_fTimeDelta, 3.f);

		if (m_pControllerCom->Compute_Height() > 2.f)
			m_eAnimIndex = CAR_FALL;
	}
	else if (m_eAnimIndex == CAR_FALL)
	{
		m_pControllerCom->FreeFall(m_pTransformCom, m_fTimeDelta, 3.f);
		m_fFallTime += m_fTimeDelta;

		if (m_pControllerCom->Is_Terrain() && m_fFallTime > 0.3f)
		{
			m_eAnimIndex = CAR_LANDING;
			m_pGameInstance->PlaySound_Free(L"KirbyCar_Landing.wav", 0.5f);
			m_fFallTime = 0.f;
		}
	}
	else if (m_eAnimIndex == CAR_LANDING)
	{
		m_pControllerCom->FreeFall(m_pTransformCom, m_fTimeDelta, 3.f);

		if (m_pModelCom->IsFinished())
			m_eAnimIndex = CAR_WAIT;
	}
	else if (m_eAnimIndex == CAR_SHAKE)
	{


	}

	return OBJ_NOEVENT;
}

void CCar::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 5.0f))
	{
		if (Compute_OptimizationAnimation(m_fTimeDelta) == true)
			m_pModelCom->Play_Animation(m_fAccTime);

		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);

		m_iRenderCount = 1;
	}
}

HRESULT CCar::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	if (m_iRenderCount == 0)
	{
		for (size_t i = 0; i < iNumMeshes; i++)
		{
			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vDeformRimColor", &m_vDeformRimColor, sizeof(_float3))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
				return E_FAIL;
			if (FAILED(m_pMaskTextureCom->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture")))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolveRatio", &m_fDissolveRatio, sizeof(_float))))
				return E_FAIL;
			_float2 vTEXUV = { m_fUVOffsetTime , m_fUVOffsetTime };

			if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVOffset", &vTEXUV, sizeof(_float2))))
				return E_FAIL;

			/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
			if (FAILED(m_pShaderCom->Begin(ANIMMODEL_DEFORMRIM)))
				return E_FAIL;

			m_pModelCom->Render(i);
		}

	}
	else
	{
		for (size_t i = 0; i < iNumMeshes; i++)
		{
			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
				return E_FAIL;
			_float fWhiteColorDiffuse = 0.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColorDiffuse, sizeof(_float))))
				return E_FAIL;

			/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
			if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
				return E_FAIL;

			m_pModelCom->Render(i);
		}

		m_iRenderCount = 0;
	}

	return S_OK;
}

HRESULT CCar::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CCar::Render_IMGUI()
{

}
#endif

void CCar::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{


}

HRESULT CCar::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Car"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_FireDissolve"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pMaskTextureCom)))
		return E_FAIL;


	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.fOffset = 1.0f;
	desc.tCapsuleShape.fHeight = 1.f;// 1.f;
	desc.tCapsuleShape.fRadius = 0.5f;// 0.5f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = DEFORMOBJECT;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, 1.f, 2.f, 7.f);

	return S_OK;
}

HRESULT CCar::Bind_ShaderResources()
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

void CCar::Set_Animation()
{
	if (m_ePreAnimIndex == m_eAnimIndex)
		return;

	switch (m_eAnimIndex)
	{
	case CAR_FALL:
		m_pModelCom->Set_Animation(m_eAnimIndex, 60.f, true, false);
		break;
	case CAR_LANDING:
		m_pModelCom->Set_Animation(m_eAnimIndex, 60.f, false, false);
		break;
	case CAR_SHAKE:
		m_pModelCom->Set_Animation(m_eAnimIndex, 100.f, true, true);
		break;
	case CAR_WAIT:
		m_pModelCom->Set_Animation(m_eAnimIndex, 60.f, true, true);
		break;
	default:
		break;
	}
	m_ePreAnimIndex = m_eAnimIndex;
}

CCar* CCar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCar* pInstance = new CCar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CCar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCar::Clone(void* pArg)
{
	CCar* pInstance = new CCar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CCar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCar::Free()
{
	Delete_AllEffect();

	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pMaskTextureCom);
}
