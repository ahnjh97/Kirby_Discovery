#include "stdafx.h"
#include "SimbaLaser.h"
#include "HitBox.h"
#include "MultiEffect.h"

CSimbaLaser::CSimbaLaser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSimbaLaser::CSimbaLaser(const CSimbaLaser& rhs)
	: CGameObject{ rhs }
{
}

void CSimbaLaser::HideLaser()
{
	m_pDynamicActor->setGlobalPose(PxTransform(0, 0, 0));
}

void CSimbaLaser::MakeLaser()
{
	_float3 vPos = GET_POS;
	_float3 vScale = { 3.f, 3.f, 3.f };
	wstring strName = { L"HS_lion laser" };

	CMultiEffect::MULTI_FX_DESC MultiFXDesc{};
	MultiFXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	MultiFXDesc.vInitScale = vScale;

	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"),
		TEXT("Prototype_GameObject_") + strName, &MultiFXDesc)))
		return;


	//Add_Effect("HS_lion laser", MultiFXDesc);
}

HRESULT CSimbaLaser::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSimbaLaser::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = { nullptr };

	if (pArg != nullptr) {
		Desc = (GAMEOBJECT_DESC*)pArg;
		Desc->fSpeedPerSec = 7.f;
		Desc->fRotationPerSec = XMConvertToRadians(90.f);
	}
	
	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	_float fScale = 24.f;
	m_pTransformCom->Set_Scaled(_float3(fScale, fScale, fScale));

	if (FAILED(Add_Components(Desc->wstrModelName)))
		return E_FAIL;

	m_bMotionBlur = false;
	m_bRimLight = false;

	m_pDynamicActor = m_pModelCom->ReturnDynamicActor(m_pTransformCom->Get_WorldFloat4x4());

	PxU32 numShapes = m_pDynamicActor->getNbShapes();
	vector<PxShape*> vecShapes(numShapes);
	m_pDynamicActor->getShapes(vecShapes.data(), numShapes);
	for (PxShape* shape : vecShapes) {
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
	}

	m_pDynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

	m_pGameInstance->Register_Trigger(m_pDynamicActor, TRIGGER_SIMBA_ATTACK, 1);

	return S_OK;
}

_int CSimbaLaser::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_pDynamicActor->setKinematicTarget(CUtils::TransformToPxTransform(m_pTransformCom));

	//레이저와 지면 충돌하는 지점 확인
	//_float3 vCollidingPoint =
	//	CUtils::Compute_CollidingPoint(GET_POS, (_float3)m_pTransformCom->Get_State(CTransform::STATE_LOOK),
	//		{ 0.f, 1.f, -66.f }, { 26.f, 1.f, 26.f });

	return OBJ_NOEVENT;
}

void CSimbaLaser::Late_Tick(_float fTimeDelta)
{
	if (true == m_bHide)
		return;

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CSimbaLaser::Render()
{
	return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_X)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CSimbaLaser::Add_Components(const wstring& _wstrModelName)
{
	HRESULT hr{};

	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_SimbaLaser");
	hr = __super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	//CHitBox::HITBOX_DESC tAttack{};
	//tAttack.pOwner = this;
	//tAttack.pDesc = &m_tColliderDesc[ATTACK];
	//tAttack.pCollisionType = LASER_SIMBA;

	//for (_uint i = 1; i < 12; i++) {
	//	tAttack.vOffset = _float3(0, 0, 3.f * i);
	//	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &tAttack)))
	//		return E_FAIL;
	//}

	//Activate_SphereCollider(0.f, 3.4f, ATTACK);

	return S_OK;
}

HRESULT CSimbaLaser::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}



CSimbaLaser* CSimbaLaser::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSimbaLaser* pInstance = new CSimbaLaser(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSimbaLaser"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSimbaLaser::Clone(void* pArg)
{
	CSimbaLaser* pInstance = new CSimbaLaser(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CSimbaLaser"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSimbaLaser::Free()
{
	m_pGameInstance->ReleaseActor(m_pDynamicActor);

	__super::Free();

	Safe_Release(m_pTextureCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
