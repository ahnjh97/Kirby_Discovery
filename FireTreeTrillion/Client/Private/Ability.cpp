#include "stdafx.h"
#include "Ability.h"
#include "HitBox.h"
#include "Kirby.h"

CAbility::CAbility(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CItemObject{ pDevice, pContext }
{
}

CAbility::CAbility(const CAbility& rhs)
	: CItemObject{ rhs }
{
}

HRESULT CAbility::Initialize_Prototype()
{
	m_eCollisionGroup = ITEM;

	return S_OK;
}

HRESULT CAbility::Initialize(void* pArg)
{
	ABILITYITEM_DESC* pAbilityItemDesc = nullptr;

	if (nullptr != pArg)
	{
		pAbilityItemDesc = (ABILITYITEM_DESC*)pArg;

		pAbilityItemDesc->fSpeedPerSec = 7.f;
		pAbilityItemDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_vPosition = pAbilityItemDesc->vPosition;
	}

	if (FAILED(__super::Initialize(pAbilityItemDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pTransformCom->Turn(XMVectorSet(1.f, 0.f, 0.f, 0.f), 1.f);
	
	m_eItemType = ITEM_FOOD;
	m_fJumpPower = 7.f;
	m_fPower = 2.f;

	//CMultiEffect::MULTI_FX_DESC FXDesc{};

	//FXDesc.vInitPos = { 0.f, .3f, 0.f };
	//FXDesc.vInitScale = { 1.5f, 1.5f, 1.5f };
	//FXDesc.pSocketMatrix = &m_EffectSocket;
	//if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_ItemBubble2"), &FXDesc)))
	//	return E_FAIL;
	//Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));

	return S_OK;
}

_int CAbility::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_FirstTimer();

	m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fTimeDelta * 2.5f);

	if (-2.6f < m_fJumpPower)
	{
		CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
		CTransform* pTransform = pKirby->Get_TransformCom();

		_float3 vLookDir = pTransform->Get_State_Vector(CTransform::STATE_LOOK);

		// 이제 날아가는 것을 구현해보자.
		m_pControllerCom->Move_Dir(m_pTransformCom, -vLookDir * m_fTimeDelta * 2.f, m_fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		m_pControllerCom->Jump(m_pTransformCom, m_fJumpPower, m_fTimeDelta);
		m_fJumpPower -= GRAVITY * m_fTimeDelta * m_fPower;

		if (0.4f < m_fPower)
			m_fPower -= m_fTimeDelta * 3.f;
		else
			m_fPower = 0.4f;
		//m_fPower -= m_fTimeDelta;
		m_pControllerCom->Set_FallVelocity(m_fJumpPower);
	}
	else
	{
		m_pControllerCom->FreeFall(m_pTransformCom, m_fTimeDelta, m_fPower);
	
		if (0.1f < m_fPower)
			m_fPower -= m_fTimeDelta * 3.f;
		else
			m_fPower = 0.1f;
	}

	__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CAbility::Late_Tick(_float fTimeDelta)
{
	if (-2.6f > m_fJumpPower)
		Sphere_Collision();

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CAbility::Render()
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

HRESULT CAbility::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CAbility::Render_IMGUI()
{

}
#endif

void CAbility::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	//if (eContent == CCollisionCenter::CONTENT_ITEM)
	//{
	//	// 충돌이 완료되었다는 뜻. 반드시 해주어야 함.
	//	m_bCollisionComplete = true;
	//}
}

HRESULT CAbility::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Item_Sword"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	CCharacterController::CONTROLLER_DESC desc{};
	m_vPosition.y += 1.f;
	desc.vInitialPos = m_vPosition;
	desc.fOffset = -0.5f;
	desc.tCapsuleShape.fHeight = 1.f;
	desc.tCapsuleShape.fRadius = 0.1f;
	desc.uCollisionType = m_eCollisionGroup;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	//m_pControllerCom->Set_CollisionType(m_eCollisionGroup);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);

	//CHitBox::HITBOX_DESC HitBox{};
	//HitBox.pOwner = this;
	//HitBox.pDesc = &m_tColliderDesc[BODY];
	//HitBox.pCollisionType = ITEM;
	//if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
	//	return E_FAIL;
	//Set_BodyCollider(COLLIDER_SPHERE, 0.5f, 0.f, 0.7f);


	return S_OK;
}

HRESULT CAbility::Bind_ShaderResources()
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

void CAbility::Sphere_Collision()
{
	_float fDist = { 0.f };

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
	CTransform* pTransform = pKirby->Get_TransformCom();

	_vector vPlayerPos = pTransform->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vItemPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	_float fPlayerSize = pTransform->Get_Scaled().x * 0.5f;
	_float fItemSize = m_pTransformCom->Get_Scaled().x * 0.5f;

	if (Check_Sphere(pTransform, &fDist))
	{
		_float fOverlap = (fPlayerSize + fItemSize) - fDist;

		_vector vDirection = XMVector3Normalize(XMVectorSetY(XMVectorSubtract(vItemPos, vPlayerPos), 0.f));

		_vector vPushVector = XMVectorScale(vDirection, fOverlap);

		//pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(XMVectorAdd(vPlayerPos, vPushVector), 1.f));
		//m_pControllerCom->Move(m_pTransformCom, XMVectorSetW(XMVectorAdd(vPlayerPos, vPushVector)), m_fTimeDelta);
		m_pControllerCom->Move_Dir(m_pTransformCom, XMVector3Normalize(vPushVector) * m_fTimeDelta, m_fTimeDelta);
	}
}

_bool CAbility::Check_Sphere(CTransform* pTransform, _float* fDist)
{
	_vector vItemPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vPlayerPos = pTransform->Get_State_Vector(CTransform::STATE_POSITION);

	_float fItemSize = m_pTransformCom->Get_Scaled().x;
	_float fPlayerSize = pTransform->Get_Scaled().x;

	*fDist = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPlayerPos, vItemPos))) - 0.5f;

	return *fDist < (fPlayerSize + fItemSize) * 0.5f;
}

CAbility* CAbility::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAbility* pInstance = new CAbility(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CAbility"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAbility::Clone(void* pArg)
{
	CAbility* pInstance = new CAbility(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CAbility"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAbility::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}
