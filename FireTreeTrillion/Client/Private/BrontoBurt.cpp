#include "stdafx.h"
#include "BrontoBurt.h"
#include "FSM.h"
#include "BrontoBurt_State.h"
#include "HitBox.h"

CBrontoBurt::CBrontoBurt(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CBrontoBurt::CBrontoBurt(const CBrontoBurt& rhs)
	: CMonster{ rhs }
{
}

HRESULT CBrontoBurt::Initialize_Prototype()
{
	m_eCollisionGroup = MONSTER;

	return S_OK;
}

HRESULT CBrontoBurt::Initialize(void* pArg)
{
	BRONTOBURT_DESC* pBrontoBurtDesc = nullptr;

	if (nullptr != pArg)
	{
		pBrontoBurtDesc = (BRONTOBURT_DESC*)pArg;

		pBrontoBurtDesc->fSpeedPerSec = 7.f;
		pBrontoBurtDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_eMonState = pBrontoBurtDesc->eMonState;
		m_vecRallyPoint = pBrontoBurtDesc->vecRallyPoints;
	}

	for (_int i = 0; i < m_vecRallyPoint.size(); i++)
		m_vecRallyPoint[i].y += 3.f;

	if (FAILED(__super::Initialize(pBrontoBurtDesc)))
		return E_FAIL;

	if(MON_PATROL == m_eMonState)
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vecRallyPoint[0]);
	else
	{
		m_fDistance = XMVectorGetX(XMVector3Length(m_vecRallyPoint[0] - m_pTransformCom->Get_State(CTransform::STATE_POSITION)));
		m_vOriginPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	}

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pModelCom->Set_Animation(BRONTOBURT_FLY, 50.f, true, true);


	m_fMaxHp = 15.f;
	m_fHp = 15.f;
	m_fAttack = 10.f;
	m_eVacuumSize = SIZE_SMALL;
	m_eAbilityType = ABILITY_DEFAULT;
	m_eEyeState = BRONTOBURTEYE_IDLE;

	m_fSpeed = 5.f;
	
	if(MON_PATROL == m_eMonState)
		m_pTransformCom->Look_At_Dir(m_vecRallyPoint[0] - m_vecRallyPoint[1]);

	Set_Slope(false);

	return S_OK;
}

_int CBrontoBurt::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	if (BRONTOBURT_FLY == Get_State())
	{
		m_eEyeState = BRONTOBURTEYE_IDLE;

		if (true == m_bReturn)
		{
			_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), m_vLastPos)));

			_float4 vReturnLook = m_vLastPos - m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
			if (0.1f < fDistance)
				m_pControllerCom->Move_Dir(m_pTransformCom, XMVector3Normalize(vReturnLook) * m_fTimeDelta * m_fSpeed, m_fTimeDelta);
			else
			{
				m_bReturn = false;
				m_bLerp = true;
			}
		}
		else if (true == m_bLerp)
		{
			_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			vLook.m128_f32[1] = 0.f;
			//_float fAngle = XMVectorGetX(XMVector3AngleBetweenVectors(vLook, XMLoadFloat4(&m_vRally)));
			_float dotProduct = max(-1.0f, min(1.0f, XMVectorGetX(XMVector3Dot(vLook, XMVector3Normalize(m_vRally)))));
			_float fAngle = acosf(dotProduct);
			_float fY = XMVectorGetY(XMVector3Cross(vLook, XMLoadFloat4(&m_vRally)));
			if (fY < 0)
				fAngle = -fAngle;
			_float fRadian = XMConvertToRadians(5.f);
			if (abs(fAngle) >= fRadian)
				m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fTimeDelta * fAngle * 5.f);
			else
				m_bLerp = false;
		}
		else if (MON_CIRCLE == m_eMonState)
		{
			m_fAngle += m_fTimeDelta * 50.f;

			m_vRotatePos.x = m_vOriginPos.x + (m_fDistance * sin(XMConvertToRadians(m_fAngle)));
			m_vRotatePos.y = m_vOriginPos.y;
			m_vRotatePos.z = m_vOriginPos.z - (m_fDistance * cos(XMConvertToRadians(m_fAngle)));

			m_pControllerCom->Move(m_pTransformCom, m_vRotatePos, m_fTimeDelta);

			m_vRally = m_vRotatePos - m_vBeforePos;
			if (m_vRally != XMVectorZero())
				m_pTransformCom->Look_At_Axis(m_vRally);
			m_vBeforePos = m_vRotatePos;
		}
		else if (MON_PATROL == m_eMonState)
		{
			m_fMoveTime += m_fTimeDelta;
			//if (1.f < m_fMoveTime)
			//	m_fSpeed -= m_fTimeDelta * 10.f;
			//else if (1.f >= m_fMoveTime)
			//	m_fSpeed += m_fTimeDelta * 10.f;

			if (2.f < m_fMoveTime)
			{
				m_fMoveTime = 0.f;
				//m_fSpeed = 0.f;

				if (m_iCnt == 0)
				{
					m_bConvert = false;
					m_vRally = m_vecRallyPoint[m_iCnt + 1] - m_vecRallyPoint[m_iCnt];
					m_iCnt++;
				}
				else if (m_iCnt < m_vecRallyPoint.size() - 1)
				{
					if (false == m_bConvert)
					{
						m_vRally = m_vecRallyPoint[m_iCnt + 1] - m_vecRallyPoint[m_iCnt];
						m_iCnt++;
					}
					else
					{
						m_iCnt--;
						m_vRally = m_vecRallyPoint[m_iCnt] - m_vecRallyPoint[m_iCnt + 1];
					}
				}
				else
				{
					//m_iCnt = 0;
					m_bConvert = true;
					m_iCnt--;
					m_vRally = m_vecRallyPoint[m_iCnt] - m_vecRallyPoint[m_iCnt + 1];
				}
			}

			_vector   vLook = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(vLook, XMLoadFloat4(&m_vRally)));
			_float fY = ::XMVectorGetY(::XMVector3Cross(vLook, XMLoadFloat4(&m_vRally)));
			if (fY < 0)
				fAngle = -fAngle;
			if (abs(fAngle) >= XMConvertToRadians(3.f))
				m_pTransformCom->Turn(::XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * fAngle * 5.f);

			m_pControllerCom->Move_Dir(m_pTransformCom, XMVector3Normalize(m_vRally) * m_fTimeDelta * m_fSpeed, m_fTimeDelta);
		}
	}
	
	if (m_ePhyXState == PO_VACUUMING || m_ePhyXState == PO_FLYDEADAWAY)
		Change_State(BRONTOBURT_DAMAGE, 120.f, true, false);

	__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CBrontoBurt::Late_Tick(_float fTimeDelta)
{
	// 커비 입 안에 있고, Fly가 아닐땐 입 안에 있는 상황이므로, Render되지않는다.
	if (m_ePhyXState == PO_KIRBYMOUTH)
		return;


	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		// 날아갈 땐, 애니메이션 재생이 되지 않는다.
		if (m_ePhyXState != PO_FLYAWAY)
		{
			if (Compute_OptimizationAnimation(m_fTimeDelta) == true)
				m_ePhyXState == PO_FLYDEADAWAY ? m_pModelCom->Play_Animation(m_fAccTime * 0.3f) : m_pModelCom->Play_Animation(m_fAccTime);
		}
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CBrontoBurt::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (Custom_Face(i) == true)
			continue;

		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_X)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CBrontoBurt::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CBrontoBurt::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}

	//ImGui::Text("RePress : %d", m_bRePressBlock);
	//ImGui::Text("Land : %d", INFO(m_isLanding));

	//ImGui::Text("JUMP : %d", INFO(m_isJump));
	//ImGui::Text("Velocity : %.2f", INFO(m_fJumpVelocity));
	//ImGui::Text("Input C? : %d", m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS));
	//ImGui::Text("FSM : %d", m_pFSM->Get_State());
	ImGui::Separator(); ImGui::NewLine();

	//	ImGui::Text("MoveDir X : %.2f \tMoveDir Y : %.2f \tMoveDir Z : %.2f ", INFO(m_vMoveDir).x, INFO(m_vMoveDir).y, INFO(m_vMoveDir).z); ImGui::NewLine();
	//	ImGui::Text("TargetDir X : %.2f \tTargetDir Y : %.2f \tTargetDir Z : %.2f ", INFO(m_vTargetDir).x, INFO(m_vTargetDir).y, INFO(m_vTargetDir).z);
	__super::Render_IMGUI();
}
#endif

void CBrontoBurt::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (eContent == CCollisionCenter::CONTENT_BODY)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
			m_vLastPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			Change_State(BRONTOBURT_DAMAGE, 50.f, false, true);
			m_eEyeState = BRONTOBURTEYE_HALF;
			m_bReturn = true;
		}
	}
	else if (eContent == CCollisionCenter::CONTENT_VACUUMOBJECT)
	{

	}
	else if (eContent == CCollisionCenter::CONTENT_ATTACK)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
			m_vLastPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			Change_State(BRONTOBURT_DAMAGE, 50.f, false, true);
			m_eEyeState = BRONTOBURTEYE_HALF;
			m_bReturn = true;
		}
	}
}

void CBrontoBurt::Change_State(BRONTOBURT_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CBrontoBurt::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

HRESULT CBrontoBurt::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_BrontoBurt"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	/* For.Com_Texture */
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_BrontoBurt_Eye"),
		TEXT("Com_Texture"), (CComponent**)&m_pEyeTextureCom);
	CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.fOffset = 1.f;
	desc.uCollisionType = m_eCollisionGroup;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	m_pControllerCom->Set_Object(this);

	//m_pControllerCom->Set_CollisionType(m_eCollisionGroup);

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = MONSTER;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_CYLINDER, 0.5f, 1.f, 0.85f);


	SetUp_FSM();

	return S_OK;
}

HRESULT CBrontoBurt::Bind_ShaderResources()
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

void CBrontoBurt::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();
	m_pFSM->Add_State(BRONTOBURT_FLY, CBrontoBurt_Idle_State::Create());

	m_pFSM->Add_State(BRONTOBURT_DAMAGE, CBrontoBurt_Damage_State::Create());
	//m_pFSM->Add_State(KABU_WARP1, CKabu_Warp_State::Create());

	//상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = BRONTOBURT_FLY;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

_bool CBrontoBurt::Custom_Face(_uint iMeshIndex)
{
	if (iMeshIndex == 2)
	{
		HRESULT hr;

		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", iMeshIndex, TextureType_DIFFUSE);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", iMeshIndex);
		CHECK_FAILED(hr);

		hr = m_pEyeTextureCom->Bind_ShaderResource(m_pShaderCom, "g_KirbyEyeTexture", (_uint)m_eEyeState);
		CHECK_FAILED(hr);

		m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
		m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool));
		m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float));
		m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));
		m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4));

		m_pShaderCom->Begin(ANIMMODEL_EYE);
		m_pModelCom->Render(iMeshIndex);

		return true;
	}

	return false;
}

CBrontoBurt* CBrontoBurt::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBrontoBurt* pInstance = new CBrontoBurt(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBrontoBurt"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBrontoBurt::Clone(void* pArg)
{
	CBrontoBurt* pInstance = new CBrontoBurt(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CBrontoBurt"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBrontoBurt::Free()
{
	__super::Free();

	Safe_Release(m_pEyeTextureCom);
}
