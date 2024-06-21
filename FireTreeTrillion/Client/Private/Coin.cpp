#include "stdafx.h"
#include "Coin.h"
#include "KirbyArmours.h"
#include "HitBox.h"

CCoin::CCoin(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CItemObject{ pDevice, pContext }
{
}

CCoin::CCoin(const CCoin& rhs)
	: CItemObject{ rhs }
{
}

HRESULT CCoin::Initialize_Prototype()
{
	m_eCollisionGroup = ITEM;

	return S_OK;
}

HRESULT CCoin::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	if (nullptr != pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 7.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(5.f, 10.f, -175.f, 1.f));

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_eItemType = ITEM_COIN;
	m_iItemPoint = 1;

	m_vTargetPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	m_vTargetPos.y += 4.f;

	return S_OK;
}

_int CCoin::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	__super::Tick(m_fTimeDelta);

	// 충돌이 완료 되었다면
	if (m_bCollisionComplete == true)
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fTimeDelta, 960.f);

		m_fCoinTime += m_fTimeDelta;
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

		_float4 fUpDelta = (m_vTargetPos - vPos);

		if (m_fCoinTime > 0.7f)
		{
			m_bDead = true;
		}

		//_float vRevisedPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
		//m_pControllerCom->Move_Dir(m_pTransformCom, fUpDelta * m_fTimeDelta * 5.f, m_fTimeDelta);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + (fUpDelta * m_fTimeDelta * 5.f));
	}
	// 충돌이 안 되었다면
	else
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fTimeDelta, 270.f);

	return OBJ_NOEVENT;
}

void CCoin::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);

		if (m_fCoinTime > 0.35f)
		{
			m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
		}
	}

}

HRESULT CCoin::Render()
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

HRESULT CCoin::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG

void CCoin::Render_IMGUI()
{
}

#endif

void CCoin::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (eContent == CCollisionCenter::CONTENT_ITEM)
	{
		m_bCollisionComplete = true;
	}
}

HRESULT CCoin::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	// 커비의 기본 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Item_Coin"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = ITEM;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, 0.7f, 0.f, 0.7f);

	return S_OK;

}

HRESULT CCoin::Bind_ShaderResources()
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

CCoin* CCoin::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCoin* pInstance = new CCoin(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CCoin"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCoin::Clone(void* pArg)
{
	CCoin* pInstance = new CCoin(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CCoin"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCoin::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}
