#include "stdafx.h"
#include "StarBlock.h"
#include "StarBlockPiece.h"
#include "HitBox.h"

CStarBlock::CStarBlock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CStarBlock::CStarBlock(const CStarBlock& rhs)
	: CPhysXObject( rhs )
{
}

HRESULT CStarBlock::Initialize_Prototype()
{
	m_eCollisionGroup = OBJECT;
	return S_OK;
}

HRESULT CStarBlock::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* desc = {};

	if (pArg != nullptr)
		GAMEOBJECT_DESC* desc = (GAMEOBJECT_DESC*)pArg;


	HRESULT hr;
	hr = __super::Initialize(desc);
	CHECK_FAILED(hr);

	Add_Components(desc->wstrModelName);

	if (FAILED(m_pModelCom->CreateStaticActor(m_pTransformCom->Get_WorldFloat4x4())))
		return E_FAIL;

	return S_OK;
}

_int CStarBlock::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	__super::Tick(fTimeDelta);

	Compute_MotionBlur();

	if (m_ePhyXState == PO_VACUUMING && m_bStaticOffTrigger == true)
	{
		m_pModelCom->DisableActors();
		m_bStaticOffTrigger = false;
	}

	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	if (m_ePhyXState == PO_FLYAWAY)
	{
		_float3 vDamegeDir = m_vDamegeDir;
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vDamegeDir * m_fTimeDelta * 30.f);
		m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), m_fTimeDelta, 360.f);
		m_fFlyTime += m_fTimeDelta;

		if (m_fFlyTime > 2.f)
			m_bDead = true;
	}
	else if (m_ePhyXState == PO_FLYDEADAWAY)
	{
		m_bDead = true;
	}

	return OBJ_NOEVENT;
}

void CStarBlock::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CStarBlock::Render()
{
	HRESULT hr;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(MODEL_NORMAL_O);
		CHECK_FAILED(hr);
		
		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CStarBlock::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CStarBlock::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}
	ImGui::Separator(); ImGui::NewLine();

	__super::Render_IMGUI();
}
#endif

void CStarBlock::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	//// 여기서 StarBlockPiece 뿌리기
	//HRESULT hr = S_OK;
	//CStarBlockPiece::PIECE_DESC desc{};
	//_float4 vInitialpos = GET_POS;
	//desc.vInitialPos = _float4(vInitialpos.x + 1.f, vInitialpos.y + 1.5f, vInitialpos.z + 1.f, 1.f);
	////desc.vDir =k
	//hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_MapObject"), TEXT("Prototype_GameObject_StarBlockPiece"), &desc);
	//CHECK_FAILED(hr);
	//m_bDead = true;
}

HRESULT CStarBlock::Add_Components(wstring wstrModelProtoTag)
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = OBJECT;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;

	wstring wstrModeltag = TEXT("Prototype_Component_Model_") + wstrModelProtoTag;
	hr = __super::Add_Component(wstrModeltag,
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	if (wstrModelProtoTag == TEXT("StarBlockS"))
		Set_BodyCollider(COLLIDER_SPHERE, 0.5f, 0.f, 1.f);
	else if (wstrModelProtoTag == TEXT("StarBlockM"))
		Set_BodyCollider(COLLIDER_SPHERE, 0.5f, 0.f, 1.5f);
	else if (wstrModelProtoTag == TEXT("StarBlockL"))
		Set_BodyCollider(COLLIDER_SPHERE, 0.5f, 0.f, 2.f);

	return S_OK;
}

HRESULT CStarBlock::Bind_ShaderResources()
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

	_float fWhiteColor = 0.f;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColor, sizeof(_float))))
		return E_FAIL;


	return S_OK;
}

void CStarBlock::Compute_MotionBlur()
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


CStarBlock* CStarBlock::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStarBlock* pInstance = new CStarBlock(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CStarBlock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CStarBlock::Clone(void* pArg)
{
	CStarBlock* pInstance = new CStarBlock(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CStarBlock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CStarBlock::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}

