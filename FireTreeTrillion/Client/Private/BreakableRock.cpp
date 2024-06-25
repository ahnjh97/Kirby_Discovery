#include "stdafx.h"
#include "BreakableRock.h"
#include "HitBox.h"
#include "Kirby.h"
#include "BreakableRockPartical.h"

CBreakableRock::CBreakableRock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CBreakableRock::CBreakableRock(const CBreakableRock& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CBreakableRock::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBreakableRock::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components(Desc->wstrModelName)))
		return E_FAIL;

	//if (FAILED(m_pModelCom->CreateStaticActor(m_pTransformCom->Get_WorldFloat4x4())))
	//	return E_FAIL;


	m_bMotionBlur = false;

	return S_OK;
}

_int CBreakableRock::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Make_Partical();


	return OBJ_NOEVENT;
}

void CBreakableRock::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}

}

HRESULT CBreakableRock::Render()
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

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;

}

HRESULT CBreakableRock::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CBreakableRock::Render_IMGUI()
{
}
#endif

void CBreakableRock::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	CKirby* pKirby = static_cast<CKirby*>(pObject);
	if (pKirby->Get_KirbyInfo()->m_bBooster == false) 
		return;

	pKirby->Set_HitStop();

	// 바로 없어지는 식의 코드를 짠다.
	m_bDead = true;
	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float4 vPlayerPos = pObject->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
	_float4 vDir = vPos - vPlayerPos;
	vDir.Normalize();
	m_vDamegeDir = (_float3)vDir;
	m_fHitPower = pKirby->Get_KirbyInfo()->m_fMoveSpeed;
}

HRESULT CBreakableRock::Add_Components(wstring strPrototag)
{
	HRESULT hr;

	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);
	wstring wstrModelTag = TEXT("Prototype_Component_Model_") + strPrototag;
	hr = __super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);



	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = OBJECT;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, 0.7f, 0.f, 4.f);

	return S_OK;
}

HRESULT CBreakableRock::Bind_ShaderResources()
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

_int CBreakableRock::Make_Partical()
{
	for (_int i = 0; i < 9; ++i)
	{
		_float4x4 matrix = m_pTransformCom->Get_WorldFloat4x4();
		_float4 vDir = m_vDamegeDir;

		vDir = CUtils::Make_RandomAngle_Vector(120.f, vDir);
		vDir.Normalize();
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		vPos += vDir * 2.f;

		CUtils::Set_State_Matrix(matrix, CUtils::STATE_POSITION, vPos);
		CUtils::Turn_OtherMatrix(matrix, _float4(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
		CUtils::Turn_OtherMatrix(matrix, _float4(1.f, 0.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
		CUtils::Turn_OtherMatrix(matrix, _float4(0.f, 0.f, 1.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
		_float fRandomscale = CUtils::Make_RandomFloat(0.6f, 1.6f);
		CUtils::Set_Scaled_Matrix(matrix, fRandomscale, fRandomscale, fRandomscale);

		CBreakableRockPartical::BREAKABLEPARTICALDESC desc = {};
		desc.matrix = matrix;
		vDir.y += 0.5f;
		desc.vMoveDir = (_float3)vDir;
		desc.fPower = m_fHitPower * 70.f;
		desc.wstrModelName = TEXT("RockPartical");
		// Car Test
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_RockPartical"), TEXT("Prototype_GameObject_BreakableRockPartical"), &desc)))
			return OBJ_DEAD;
	}

	Ready_Dead(4.5f);

	return OBJ_DEAD;
}

CBreakableRock* CBreakableRock::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBreakableRock* pInstance = new CBreakableRock(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBreakableRock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBreakableRock::Clone(void* pArg)
{
	CBreakableRock* pInstance = new CBreakableRock(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CBreakableRock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBreakableRock::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pControllerCom);
	Safe_Release(m_pShaderCom);
}
