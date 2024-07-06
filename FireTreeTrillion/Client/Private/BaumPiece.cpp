#include "stdafx.h"
#include "BaumPiece.h"

CBaumPiece::CBaumPiece(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CBaumPiece::CBaumPiece(const CBaumPiece& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CBaumPiece::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBaumPiece::Initialize(void* pArg)
{
	BAUMPIECEDESC* pGameObjectDesc = nullptr;

	if (nullptr != pArg)
		pGameObjectDesc = (BAUMPIECEDESC*)pArg;

	HRESULT  hr = __super::Initialize(pGameObjectDesc);
	CHECK_FAILED(hr);

	if (pGameObjectDesc != nullptr)
		Add_Components(pGameObjectDesc->wstrModelName);

	m_bMotionBlur = true;
	m_bRimLight = false;
	m_bStencil = true;

	PxVec3 kickDirection(pGameObjectDesc->vParticalMoveDir.x, pGameObjectDesc->vParticalMoveDir.y, pGameObjectDesc->vParticalMoveDir.z);
	PxVec3 impulse = kickDirection * pGameObjectDesc->fParticalSpeed;
	m_pDynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
	m_pDynamicActor->addForce(impulse, PxForceMode::eIMPULSE);

	_float fMin = 0.2f;
	_float fMax = 0.8f;
	PxVec3 PxTorque = PxVec3(CUtils::Make_RandomFloat(fMin, fMax), CUtils::Make_RandomFloat(fMin, fMax)
		, CUtils::Make_RandomFloat(fMin, fMax));

	m_pDynamicActor->addTorque(PxTorque, PxForceMode::eIMPULSE);

	return S_OK;
}

_int CBaumPiece::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead(3.f);

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	Compute_MotionBlur();


	m_fDeathTime += m_fTimeDelta;
	if (m_fDeathTime > 4.f)
	{
		return OBJ_DEAD;
	}

	return OBJ_NOEVENT;
}

void CBaumPiece::Late_Tick(_float fTimeDelta)
{
	if (nullptr != m_pDynamicActor) {
		m_pTransformCom->Set_WorldMatrix(m_pGameInstance->GetActorAverageMatrix(m_pDynamicActor));

		PxVec3 PxForce = PxVec3(0.f, -0.5f, 0.f);
		m_pDynamicActor->addForce(PxForce, PxForceMode::eFORCE);
	}


	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 20.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CBaumPiece::Render()
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
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
			return E_FAIL;
		_float fWhiteColorDiffuse = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColorDiffuse, sizeof(_float))))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CBaumPiece::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBaumPiece::Add_Components(const wstring& wstrModelName)
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_") + wstrModelName;
	hr = __super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	m_pDynamicActor = m_pModelCom->ReturnDynamicActor(m_pTransformCom->Get_WorldFloat4x4());
	m_pDynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

	return S_OK;
}

HRESULT CBaumPiece::Bind_ShaderResources()
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

void CBaumPiece::Compute_MotionBlur()
{
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
	_float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
	_float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

	_float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

	m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
	m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
	m_vMotionVelocity.z = m_ePhyXState != PO_NORMAL ? 1.f : 0.f;

	m_vPreScreenPos = vCurScreenPos;
}

CBaumPiece* CBaumPiece::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBaumPiece* pInstance = new CBaumPiece(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBaumPiece"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBaumPiece::Clone(void* pArg)
{
	CBaumPiece* pInstance = new CBaumPiece(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CBaumPiece"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBaumPiece::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
