#include "stdafx.h"
#include "CarShopWall.h"
#include "HitBox.h"
#include "Kirby.h"
#include "BreakableRockParticle.h"
#include "EventCenter.h"

CCarShopWall::CCarShopWall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CCarShopWall::CCarShopWall(const CCarShopWall& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CCarShopWall::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCarShopWall::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = { nullptr };

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_bMotionBlur = false;
	m_bRimLight = false;
	m_fRimWidth = 0.15f;

	m_setNonRenderMeshes.insert(m_pModelCom->Find_MeshIndex(string("Cruck2M__CruckLightC")));
	m_setNonRenderMeshes.insert(m_pModelCom->Find_MeshIndex(string("Cruck3M__CruckShadeC")));
	m_setNonRenderMeshes.insert(m_pModelCom->Find_MeshIndex(string("WallFrameBeforeM__BreakC")));
	m_setNonRenderMeshes.insert(m_pModelCom->Find_MeshIndex(string("WallFrameBeforeM__WallC")));
	_uint iA = m_pModelCom->Find_MeshIndex(string("Cruck1M__CruckC"));
	//_uint iB = m_pModelCom->Find_MeshIndex(string("Cruck3M__CruckShadeC"));
	
	m_setBeforeMeshIndices.insert(iA);
	//m_setBeforeMeshIndices.insert(iB);

	m_setNormalXMesh.insert(iA);
	//m_setNormalXMesh.insert(iB);

	m_pStaticActor = m_pNonAnimModelCom->ReturnStaticActor(m_pTransformCom->Get_WorldFloat4x4());

	return S_OK;
}

_int CCarShopWall::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Make_Partical();

	return OBJ_NOEVENT;
}

void CCarShopWall::Late_Tick(_float fTimeDelta)
{
	if(m_bStartAnimation)
		m_pModelCom->Play_Animation(m_pGameInstance->Get_SecondTimer());

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 50.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		if (false == m_bStartAnimation)
			m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}

	if (true == m_pModelCom->IsFinished())
		Set_Dead();
}

HRESULT CCarShopWall::Render()
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
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (m_setNormalXMesh.end() != m_setNormalXMesh.find(i))
		{
			if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_X)))
				return E_FAIL;
		}
		else
		{
			if (FAILED(m_pShaderCom->Begin(ANIMMODEL_LINEAR_NORMAL_O)))
				return E_FAIL;
		}
		
		if (true == m_bStartAnimation)
		{
			if (m_setBeforeMeshIndices.end() != m_setBeforeMeshIndices.find(i))
				continue;
		}

		if (m_setNonRenderMeshes.end() != m_setNonRenderMeshes.find(i))
			continue;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CCarShopWall::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CCarShopWall::Render_IMGUI()
{
}
#endif

void CCarShopWall::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (true == m_bStartAnimation)
		return;

	CKirby* pKirby = static_cast<CKirby*>(pObject);
	if (pKirby->Get_KirbyInfo()->m_bBooster == false)
		return;

	pKirby->Set_HitStop();
	m_pModelCom->Set_Animation(0, 60.f, false, false);
	m_bStartAnimation = true;





	SwitchAfterBefore();
	m_pGameInstance->DisableActor(m_pStaticActor);

	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float4 vPlayerPos = pObject->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
	_float4 vDir = vPos - vPlayerPos;
	vDir.Normalize();
	m_vDamegeDir = (_float3)vDir;
	m_fHitPower = pKirby->Get_KirbyInfo()->m_fMoveSpeed;


	vPos.y += 20.f;
	CMultiEffect::MULTI_FX_DESC MDesc = {};
	MDesc.vInitPos = (_float3)vPos;
	MDesc.vInitRot = CUtils::Make_Degree_FromDir(vDir);
	MDesc.vInitScale = { 0.6f, 0.6f , 0.6f };
	this->Add_Effect("YW CarCenter Crash Effects", MDesc, false);

	CMultiEffect::MULTI_FX_DESC Effectdesc = {};
	Effectdesc.vInitPos = (_float3)vPos;
	Effectdesc.vInitRot = CUtils::Make_Degree_FromDir(vDir);
	Effectdesc.vInitScale = { 4.f, 4.f, 4.f };
	if (FAILED(m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Car Collisions"), &Effectdesc)))
		return;

	m_pTransformCom->Move(_float4{ 0.f, -10.f, 0.f, 0.f });

	//이벤트 호출
	CEventCenter::Get_Instance()->Notify(KEVENT_BREAK_CARSHOP, this);
	//m_pGameInstance->Set_FirstTimerRatio(.2f);
	//m_pGameInstance->Set_SecondTimerRatio(.2f);

}

HRESULT CCarShopWall::Add_Components()
{
	HRESULT hr;

	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	wstring wstrModelTag = TEXT("Prototype_Component_Model_CarShopBreakableWall");
	hr = __super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	wstring wstrNonAnimModelTag = TEXT("Prototype_Component_Model_CarShopFrameBefore");
	hr = __super::Add_Component(wstrNonAnimModelTag, TEXT("Com_NonAnimModel"), (CComponent**)&m_pNonAnimModelCom);
	CHECK_FAILED(hr);

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = OBJECT;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, 15.f, 0.f, 7.f);

	return S_OK;
}

HRESULT CCarShopWall::Bind_ShaderResources()
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

_int CCarShopWall::Make_Partical()
{
	//for (_int i = 0; i < 9; ++i)
	//{
	//	_float4x4 matrix = m_pTransformCom->Get_WorldFloat4x4();
	//	_float4 vDir = m_vDamegeDir;

	//	vDir = CUtils::Make_RandomAngle_Vector(120.f, vDir);
	//	vDir.Normalize();
	//	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	//	vPos += vDir * 2.f;

	//	CUtils::Set_State_Matrix(matrix, CUtils::STATE_POSITION, vPos);
	//	CUtils::Turn_OtherMatrix(matrix, _float4(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
	//	CUtils::Turn_OtherMatrix(matrix, _float4(1.f, 0.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
	//	CUtils::Turn_OtherMatrix(matrix, _float4(0.f, 0.f, 1.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
	//	_float fRandomscale = CUtils::Make_RandomFloat(0.6f, 1.6f);
	//	CUtils::Set_Scaled_Matrix(matrix, fRandomscale, fRandomscale, fRandomscale);

	//	CBreakableRockParticle::BREAKABLEPARTICALDESC desc = {};
	//	desc.matrix = matrix;
	//	vDir.y += 0.5f;
	//	desc.vMoveDir = (_float3)vDir;
	//	desc.fPower = m_fHitPower * 70.f;
	//	desc.wstrModelName = TEXT("CarShopWallParticle");
	//	// Car Test
	//	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_RockPartical"), TEXT("Prototype_GameObject_BreakableRockPartical"), &desc)))
	//		return OBJ_DEAD;
	//}

	//Ready_Dead(4.5f);

	return OBJ_DEAD;
}

void CCarShopWall::SwitchAfterBefore()
{
	list<CGameObject*>* pWallFrameList = m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_WallFrame"));
	if (nullptr == pWallFrameList || pWallFrameList->empty())
		return;

	for (auto& wallFrame : *pWallFrameList)
	{
		if (nullptr == wallFrame || true == wallFrame->Get_Dead())
			continue;

		CModel* pModel = dynamic_cast<CModel*>(wallFrame->Get_Component(TEXT("Com_Model")));
		if (nullptr == pModel)
			continue;

		string strModelName = pModel->Get_ModelName();
		if ("CarShopWallFrame" == strModelName)
			wallFrame->Set_Hide(false);
		else
			wallFrame->Set_Dead();
	}
}

CCarShopWall* CCarShopWall::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCarShopWall* pInstance = new CCarShopWall(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CCarShopWall"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCarShopWall::Clone(void* pArg)
{
	CCarShopWall* pInstance = new CCarShopWall(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CCarShopWall"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCarShopWall::Free()
{
	__super::Free();

	m_pGameInstance->ReleaseActor(m_pStaticActor);
	Safe_Release(m_pNonAnimModelCom);

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
