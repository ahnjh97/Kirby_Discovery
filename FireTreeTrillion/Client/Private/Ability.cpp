#include "stdafx.h"
#include "Ability.h"
#include "MultiEffect.h"
#include "HitBox.h"
#include "Kirby.h"
#include "MultiEffect.h"
#include "Camera_Main.h"
#include "Particle.h"

#include "Light.h"

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
		m_fRotateDir = pAbilityItemDesc->fRotateDir;
		m_fAngle = pAbilityItemDesc->fAngle;
		m_vDir = pAbilityItemDesc->vDir;
		m_vPosition = pAbilityItemDesc->vPosition;
		m_eAbilityType = pAbilityItemDesc->eAbilityType;
	}

	if (FAILED(__super::Initialize(pAbilityItemDesc)))
		return E_FAIL;

	if (ABILITY_DEFAULT == m_eAbilityType)
	{
		_matrix matRotate = XMMatrixRotationY(ToRadian(m_fAngle));
		m_vDir = XMVector3TransformNormal(m_vDir, matRotate);

		m_fJumpPower = 8.f;
		m_fJumpPowerTemp = m_fJumpPower;
		m_fPower = 2.f;
		m_fSpeed = 3.f;
		m_fScale = 1.f;
		m_fRimWidth = 5.f;

		CEffect::FX_DESC FXDesc{};
		FXDesc.vInitPos = { 0.f, 0.f, 0.f };
		FXDesc.vInitScale = { 1.7f, 1.7f, 1.7f };
		FXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();

		Add_Effect("ItemStar", FXDesc, true);

		LIGHT_DESC			LightDesc{};
		LightDesc.eType = LIGHT_DESC::TYPE_POINT;
		LightDesc.vPosition = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION) + m_pTransformCom->Get_State(CTransform::STATE_UP) * 0.5f;
		LightDesc.fRange = 4.f;
		LightDesc.vDiffuse = _float4(0.97f, 0.96f, 0.4f, 1.f);
		LightDesc.vAmbient = _float4(.5f, .5f, .5f, 1.f);
		LightDesc.vSpecular = _float4(0.f, 0.f, 0.0f, 1.f);
		if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
			return E_FAIL;
		m_pStarLight = CGameInstance::Get_Instance()->Get_LightLastAddress();
		Safe_AddRef(m_pStarLight);

		m_fAttack = 20.f;
	}
	else
	{
		m_fJumpPower = 7.f;
		m_fPower = 2.f;

		CMultiEffect::MULTI_FX_DESC FXDesc{};
		FXDesc.vInitPos = { 0.f, .2f, 0.f };
		FXDesc.pSocketMatrix = &m_EffectSocket;
		FXDesc.vInitScale = { 1.7f, 1.7f, 1.7f };
		if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_ItemAbility1"), &FXDesc)))
			return E_FAIL;

		Add_Effect("ItemAbility1", FXDesc, true);
	}

	AbilityType(m_eAbilityType);

	if (FAILED(Add_Components()))
		return E_FAIL;
	
	m_eItemType = ITEM_FOOD;

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
	CTransform* pTransform = pKirby->Get_TransformCom();
	m_vLookDir = pTransform->Get_State_Vector(CTransform::STATE_LOOK);

	return S_OK;
}

_int CAbility::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	if (ABILITY_DEFAULT == m_eAbilityType)
	{
		if (m_ePhyXState == PO_KIRBYMOUTH)
			Delete_AllEffect();

		CCamera_Main* pCameraMain = static_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
		CHECK_NULLPTR(pCameraMain);

		CTransform* pCameraTransform = pCameraMain->Get_TransformCom();
		_vector vCameraLook = pCameraTransform->Get_State_Vector(CTransform::STATE_LOOK);

		m_pStarLight->Update_LightPos(m_pTransformCom->Get_State(CTransform::STATE_POSITION) + m_pTransformCom->Get_State(CTransform::STATE_UP) * 0.5f);

		// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
		if (m_ePhyXState == PO_FLYAWAY)
		{
			m_pStarLight->Interpolate_Light(_float4(0.97f, 0.96f, 0.4f, 1.f), 7.f, 0.1f);

			_float3 vDamegeDir = m_vDamegeDir;
			_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vDamegeDir * m_fTimeDelta * 30.f);
			m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), m_fTimeDelta, 360.f);
			m_fFlyTime += m_fTimeDelta;

			if (RayCast_Terrain(XMVector3Normalize(vDamegeDir)) == true)
			{
				m_bDead = true;
				m_pStarLight->Set_DeadLight(true);
				Safe_Release(m_pStarLight);
				m_pStarLight = nullptr;
			}
			else if (m_fFlyTime > 2.f)
			{
				m_bDead = true;
				m_pStarLight->Set_DeadLight(true);
				Safe_Release(m_pStarLight);
				m_pStarLight = nullptr;
			}
		}
		else if (m_ePhyXState == PO_FLYDEADAWAY)
		{
			m_bDead = true;
			m_pStarLight->Set_DeadLight(true);
			Safe_Release(m_pStarLight);
			m_pStarLight = nullptr;
		}
		else if (m_ePhyXState == PO_KIRBYMOUTH)
		{
			Delete_AllEffect();
			m_pStarLight->Interpolate_Light(_float4(0.97f, 0.96f, 0.4f, 1.f), 0.01f, 0.1f);
		}
		else if (m_ePhyXState == PO_VACUUMING)
		{

		}
		else
		{
			_vector		vLook = vCameraLook;
			_vector		vRight = XMVector3Cross(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), vLook);
			_vector		vUp = XMVector3Cross(vLook, vRight);

			_float3		vScaled = m_pTransformCom->Get_Scaled();

			m_pTransformCom->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight) * vScaled.x);
			m_pTransformCom->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp) * vScaled.y);
			m_pTransformCom->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook) * vScaled.z);

			m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * m_fRotateDir, m_fTimeDelta * 3.5f);

			if (0.f < m_fSpeed)
			{
				m_fSpeed -= m_fTimeDelta;
				m_pControllerCom->Move_Dir(m_pTransformCom, m_vDir * m_fTimeDelta * m_fSpeed, m_fTimeDelta);
			}
			else
				m_fSpeed = 0.f;

			if (false == m_pControllerCom->Jump(m_pTransformCom, m_fJumpPower, m_fTimeDelta))
			{
				if (0 < m_fJumpPowerTemp)
				{
					--m_fJumpPowerTemp;
					m_fJumpPower = m_fJumpPowerTemp;

					m_fDeadTime += m_fTimeDelta;
					if (10.f < m_fDeadTime)
						m_bDead = true;
				}
				else
				{
					m_pStarLight->Interpolate_Light(_float4(0.97f, 0.96f, 0.4f, 1.f), 0.01f, 0.25f);

					m_fLifeTime += m_fTimeDelta;
					if (0.2f > m_fLifeTime)
					{
						m_fScale -= m_fTimeDelta * 4.f;
						m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);
					}
					else
					{
						m_bDead = true;
						m_pStarLight->Set_DeadLight(true);
						Safe_Release(m_pStarLight);
						m_pStarLight = nullptr;
					}
				}
			}
			m_fJumpPower -= GRAVITY * m_fTimeDelta * m_fPower;
		}
	}
	else
	{
		// 데카
		if (25 == m_iDeathCount)
			m_bDead = true;

		if (m_ePhyXState == PO_KIRBYMOUTH)
			Delete_AllEffect();

		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fTimeDelta * 2.5f);

		if (-2.6f < m_fJumpPower)
		{
			CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
			CTransform* pTransform = pKirby->Get_TransformCom();

			// 이제 날아가는 것을 구현해보자.
			m_pControllerCom->Move_Dir(m_pTransformCom, -m_vLookDir * m_fTimeDelta * 2.f, m_fTimeDelta);

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

			// n초후 깜빡이면서 사라짐
			m_fLifeTime += m_fTimeDelta;
			if (4.f < m_fLifeTime)
			{
				m_fRenderTime += m_fTimeDelta;
				if (0.05f > m_fRenderTime)
					m_bRender = true;
				else
				{
					m_fRenderTime = 0.f;
					m_bRender = false;
					++m_iDeathCount;
				}
			}
		}
	}

	__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CAbility::Late_Tick(_float fTimeDelta)
{
	if (m_ePhyXState == PO_KIRBYMOUTH)
		return;

	if (-2.6f > m_fJumpPower)
		Sphere_Collision();

	if (m_ePhyXState == PO_KIRBYMOUTH)
		return;

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

		if (ABILITY_DEFAULT == m_eAbilityType)
		{
			Compute_ViewZ();
			m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
			m_iRenderCount = 1;
		}
	}
}

HRESULT CAbility::Render()
{
	if (true == m_bRender)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	if (0 == m_iRenderCount && ABILITY_DEFAULT == m_eAbilityType)
	{
		for (size_t i = 0; i < iNumMeshes; i++)
		{
			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
				return E_FAIL;

			/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
			if (FAILED(m_pShaderCom->Begin(MODEL_STAR)))
				return E_FAIL;
			m_pModelCom->Render(i);
		}
		m_iRenderCount = 0;
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

			if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
				return E_FAIL;
			_float fWhiteColorDiffuse = 0.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColorDiffuse, sizeof(_float))))
				return E_FAIL;

			/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
			if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
				return E_FAIL;
			m_pModelCom->Render(i);
		}
		m_iRenderCount = 0;
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

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(m_strComponentTag,
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = m_vPosition;
	desc.fOffset = -0.5f;
	desc.tCapsuleShape.fHeight = 1.f;
	desc.tCapsuleShape.fRadius = 0.1f;
	desc.uCollisionType = m_eCollisionGroup;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	//m_pControllerCom->Set_CollisionType(m_eCollisionGroup);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	if(ABILITY_DEFAULT == m_eAbilityType)
		HitBox.pCollisionType = OBJECT;
	else
		HitBox.pCollisionType = ABILITYITEM;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, 0.5f, 0.f, 0.7f);

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

void CAbility::AbilityType(ABILITYTYPE eAbilityType)
{
	switch (eAbilityType)
	{
	case ABILITY_DEFAULT:
		m_strComponentTag = TEXT("Prototype_Component_Model_Item_Star");
		break;
	case ABILITY_SWORD:
		m_strComponentTag = TEXT("Prototype_Component_Model_Item_Sword");
		m_pTransformCom->Turn(XMVectorSet(1.f, 0.f, 0.f, 0.f), 1.f);
		break;
	case ABILITY_HAMMER:
		m_strComponentTag = TEXT("Prototype_Component_Model_Item_Hammer");
		break;
	case ABILITY_BOMB:
		m_strComponentTag = TEXT("Prototype_Component_Model_Item_Bomb");
		break;
	case ABILITY_CRASH:
		m_strComponentTag = TEXT("Prototype_Component_Model_Item_Crash");
		break;
	case ABILITY_END:
		break;
	default:
		break;
	}
}

_bool CAbility::RayCast_Terrain(const _float3 vMoveDir)
{
	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	PxVec3 rayOrigin = PxVec3((_float)vPos.x, (_float)vPos.y, (_float)vPos.z);
	PxVec3 rayDirection = PxVec3(vMoveDir.x, vMoveDir.y, vMoveDir.z);
	_float fMaxDistance = 1.f;

	PxRaycastHit hit;
	PxRaycastBuffer hitBuffer;
	PxQueryFilterData filterData(PxQueryFlag::eSTATIC);

	_bool isRayCast = m_pGameInstance->Get_Scene()->raycast(rayOrigin, rayDirection, fMaxDistance, hitBuffer, PxHitFlag::eNORMAL, filterData);

	if (isRayCast == true)
		return true;

	// 레이 쐈는데 터레인이 없었다.
	return false;
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
		m_pControllerCom->Move_Dir(m_pTransformCom, XMVector3Normalize(vPushVector) * m_fTimeDelta * 2.f, m_fTimeDelta);
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
	Safe_Release(m_pStarLight);
	Delete_Effect("ItemStar");
}
