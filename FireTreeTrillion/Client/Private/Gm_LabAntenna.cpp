#include "stdafx.h"
#include "Gm_LabAntenna.h"
#include "HitBox.h"
#include "Kirby.h"
#include "BreakableRockParticle.h"

CGm_LabAntenna::CGm_LabAntenna(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CGm_LabAntenna::CGm_LabAntenna(const CGm_LabAntenna& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CGm_LabAntenna::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGm_LabAntenna::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = { nullptr };

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pModelCom->Set_Animation(STATE_WAIT, 0.f, TRUE /*_bool bInterpolation = false, _float fLerpTime = 0.1f*/);

	return S_OK;
}

_int CGm_LabAntenna::Tick(_float fTimeDelta)
{
	if (TRUE == m_bDead)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CGm_LabAntenna::Late_Tick(_float fTimeDelta)
{
	//에피리스 공격패턴에 해당 오브젝트가 피격당할 경우, Break 애님 재생
	m_pModelCom->Play_Animation(m_pGameInstance->Get_SecondTimer());

	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD1, KEY_DOWN)) //테스트용
		m_pModelCom->Set_Animation(STATE_BREAK, 100.f, FALSE);

#pragma region FRUSTUM_CULLING

	//절두체 컬링 처리
	if (TRUE == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 200.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}

#pragma endregion

	//애니메이션 재생종료 시 Set_Dead
	if (TRUE == m_pModelCom->IsFinished())
		Set_Dead();
}

HRESULT CGm_LabAntenna::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	HRESULT hr;

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(ANIMMODEL_NORMAL_O);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Render(i);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CGm_LabAntenna::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CGm_LabAntenna::Render_IMGUI()
{
}
#endif

void CGm_LabAntenna::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	/*
	if (true == m_bStartAnimation)
		return;

	CKirby* pKirby = static_cast<CKirby*>(pObject);
	if (pKirby->Get_KirbyInfo()->m_bBooster == false)
		return;

	pKirby->Set_HitStop();
	m_pModelCom->Set_Animation(0, 60.f, false, false);
	m_bStartAnimation = true;
	SwitchAfterBefore();

	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float4 vPlayerPos = pObject->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
	_float4 vDir = vPos - vPlayerPos;
	vDir.Normalize();
	m_vDamegeDir = (_float3)vDir;
	m_fHitPower = pKirby->Get_KirbyInfo()->m_fMoveSpeed;
	*/
}

HRESULT CGm_LabAntenna::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	wstring wstrProtoTagMod = TEXT("Prototype_Component_Model_LbAntenna_Anim");
	hr = __super::Add_Component(wstrProtoTagMod, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

#pragma region HITBOX

	//히트박스. 보스 패턴 중 충돌하면 해당 오브젝트 Break를 위해 생성
	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = ANIMDECO;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;

	Set_BodyCollider(COLLIDER_CYLINDER, 10.f, 20.f, 20.f);

#pragma endregion

	return S_OK;
}

HRESULT CGm_LabAntenna::Bind_ShaderResources()
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

void CGm_LabAntenna::SwitchAfterBefore()
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

CGm_LabAntenna* CGm_LabAntenna::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGm_LabAntenna* pInstance = new CGm_LabAntenna(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CGm_LabAntenna"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGm_LabAntenna::Clone(void* pArg)
{
	CGm_LabAntenna* pInstance = new CGm_LabAntenna(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CGm_LabAntenna"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGm_LabAntenna::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
