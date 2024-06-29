#include "stdafx.h"
#include "Food.h"
#include "MultiEffect.h"

#include "Hitbox.h"

CFood::CFood(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CItemObject{ pDevice, pContext }
{
}

CFood::CFood(const CFood& rhs)
	: CItemObject{ rhs }
{
}

HRESULT CFood::Initialize_Prototype()
{
	m_eCollisionGroup = ITEM;

	return S_OK;
}

HRESULT CFood::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	if (nullptr != pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 7.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;


	if (FAILED(Add_Components(GameObjectDesc.wstrModelName)))
		return E_FAIL;

	m_eItemType = ITEM_FOOD;
	m_iItemPoint = 30;

	//CMultiEffect::MULTI_FX_DESC FXDesc{};

	//FXDesc.vInitPos = { 0.f, .3f, 0.f };
	//FXDesc.pSocketMatrix = &m_EffectSocket;
	//FXDesc.vInitScale = { 1.5f, 1.5f, 1.5f };
	//if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_ItemBubble1"), &FXDesc)))
	//	return E_FAIL;

	//Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));


	return S_OK;
}

_int CFood::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
	{
		return OBJ_DEAD;
	}

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	__super::Tick(m_fTimeDelta);
	_float3 vMyScale = m_pTransformCom->Get_Scaled();
	CUtils::Set_Scaled_Matrix(m_EffectSocket, vMyScale.x, vMyScale.y, vMyScale.z);

	// 충돌이 아직 안 되었다면
	m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fTimeDelta, 270.f);

	// 충돌이 완료 되었다면
	if (m_bCollisionComplete == true)
	{
		if (m_fDrinkTime < 0.3f)
		{
			m_fDrinkTime += m_fTimeDelta;
			_float fScaled = (0.3f + m_fDrinkTime) / 0.4f;
			if (fScaled > 1.f)
				fScaled = 1.f;

			m_pTransformCom->Set_Scaled(fScaled, fScaled, fScaled);

			_vector vTargetPos = m_pPlayer->Get_TransformCom()->Get_State_Vector(CTransform::STATE_POSITION);
			vTargetPos.m128_f32[1] += 2.f;

			_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vDir = vTargetPos - vPos;

			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vDir * m_fTimeDelta * 10.f);
			//m_pControllerCom->Move_Dir(m_pTransformCom, vDir * m_fTimeDelta * 10.f, m_fTimeDelta);
		}
		else if (0.3f <= m_fDrinkTime && m_fDrinkTime <= 0.8f)
		{
			m_fDrinkTime += m_fTimeDelta;

			_vector vTargetPos = m_pPlayer->Get_TransformCom()->Get_State_Vector(CTransform::STATE_POSITION);
			vTargetPos.m128_f32[1] += 2.f;

			_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vDir = vTargetPos - vPos;

			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vDir * m_fTimeDelta * 10.f);
			//m_pControllerCom->Move_Dir(m_pTransformCom, vDir * m_fTimeDelta * 10.f, m_fTimeDelta);
		}
		else
		{
			m_fDrinkTime += m_fTimeDelta * 4.f;

			_float fScaled = (1.8f - m_fDrinkTime) / 1.0f;
			m_pTransformCom->Set_Scaled(fScaled, fScaled, fScaled);
			if (fScaled < 0.05f)
			{
				m_bDead = true;
			}

			_vector vTargetPos = m_pPlayer->Get_TransformCom()->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vDir = vTargetPos - vPos;

			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vDir * m_fTimeDelta * 10.f);
			//m_pControllerCom->Move_Dir(m_pTransformCom, vDir * m_fTimeDelta * 10.f, m_fTimeDelta);
		}
	}

	return OBJ_NOEVENT;
}

void CFood::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CFood::Render()
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

		m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
		m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool));
		if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
			return E_FAIL;
		m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));

		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CFood::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CFood::Render_IMGUI()
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

void CFood::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (eContent == CCollisionCenter::CONTENT_ITEM)
	{
		if (m_pPlayer == nullptr)
		{
			m_pPlayer = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"));
			Safe_AddRef(m_pPlayer);

			// 충돌이 완료되었다는 뜻. 반드시 해주어야 함.
			m_bCollisionComplete = true;
		}
	}
}

HRESULT CFood::Add_Components(wstring strPrototag)
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	wstring strModelName = TEXT("Prototype_Component_Model_") + strPrototag;
	hr = __super::Add_Component(strModelName,
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = ITEM;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, 0.5f, 0.f, 0.7f);


	return S_OK;
}

HRESULT CFood::Bind_ShaderResources()
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

CFood* CFood::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFood* pInstance = new CFood(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CFood"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFood::Clone(void* pArg)
{
	CFood* pInstance = new CFood(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CFood"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFood::Free()
{
	__super::Free();

	if (m_pPlayer != nullptr)
		Safe_Release(m_pPlayer);

	Safe_Release(m_pModelCom);
}
