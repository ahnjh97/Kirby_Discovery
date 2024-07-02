#include "stdafx.h"
#include "Meteor.h"
#include "Kirby.h"
#include "HitBox.h"

CMeteor::CMeteor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CMeteor::CMeteor(const CMeteor& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CMeteor::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeteor::Initialize(void* pArg)
{
	METEOR_DESC* pMeteorDesc = nullptr;

	if (nullptr != pArg)
	{
		pMeteorDesc = (METEOR_DESC*)pArg;

		pMeteorDesc->fSpeedPerSec = 7.f;
		pMeteorDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_bBig = pMeteorDesc->bBig;
		m_vPosition = pMeteorDesc->vPosition;
		m_fDelayTime = pMeteorDesc->fDelayTime;
		m_strTag = pMeteorDesc->strTag;
	}

	if (FAILED(__super::Initialize(pMeteorDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);
	m_vAxis = CUtils::Make_Random_Vector(1.f);
	m_pTransformCom->Turn(m_vAxis, 1.f);

	if(true == m_bBig)
	{
		Set_BodyCollider(COLLIDER_SPHERE, 0.f, 1.5f, 18.f);
		m_fTurnSpeed = 0.1f;
		m_fMeteorSpeed = 130.f;
	}
	else
	{
		Set_BodyCollider(COLLIDER_SPHERE, 0.f, 1.5f, 5.f);
		m_fTurnSpeed = 0.5f;
		m_fMeteorSpeed = 150.f;
	}

	return S_OK;
}

_int CMeteor::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	m_fRunTime += m_fTimeDelta;
	if (m_fDelayTime < m_fRunTime)
	{
		if (true == m_bBig)
		{
			m_vTargetPos = XMVectorSet(-0.115f, -5.f, -0.218f, 1.f);
			_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

			_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, m_vTargetPos)));
			if (30.f < fDistance)
			{
				if (1.f > m_fIncreSpeed)
					m_fIncreSpeed += m_fTimeDelta;
				else m_fIncreSpeed = 1.f;

				vPos += XMVector3Normalize(m_vTargetPos - vPos) * m_fTimeDelta * EaseInQuart(m_fIncreSpeed) * m_fMeteorSpeed;
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
			}
			else if (1.5f < fDistance)
			{
				// 브레이크 : 제곱 감속
				_float fDeceleration = m_fDecreSpeed * m_fDecreSpeed;

				if (0.f < m_fDecreSpeed)
					m_fDecreSpeed -= fTimeDelta * 0.4f;
				else
					m_fDecreSpeed = 0.f;

				vPos += XMVector3Normalize(m_vTargetPos - vPos) * m_fTimeDelta * EaseOutCubic(m_fDecreSpeed) * 15.f;
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

				_float fAngleVelocity = EaseOutCubic(m_fDecreSpeed) / m_fTimeDelta;
				_vector vRight = XMVector3Cross(XMVector3Normalize(m_vTargetPos - vPos), XMVectorSet(0.f, 1.f, 0.f, 0.f));
				if (!XMVector3Equal(vRight, XMVectorZero()))
					m_pTransformCom->Turn(-vRight, m_fTimeDelta * fDeceleration * 0.8f);
			}
			else
			{
				m_fDeadTime += m_fTimeDelta;
				if (0.5f < m_fDeadTime)
					m_bDead = true;
			}
		}
		else
		{
			_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, m_vTargetPos)));
			if (1.5f < fDistance)
			{
				m_fTurnSpeed += m_fTimeDelta;

				m_pTransformCom->Turn(m_vAxis, m_fTimeDelta * m_fTurnSpeed);

				if (1.f > m_fIncreSpeed)
					m_fIncreSpeed += m_fTimeDelta;
				else m_fIncreSpeed = 1.f;

				vPos += XMVector3Normalize(m_vTargetPos - vPos) * m_fTimeDelta * EaseInQuart(m_fIncreSpeed) * m_fMeteorSpeed;
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
			}
			else
			{
				m_fDeadTime += m_fTimeDelta;
				if (0.2f < m_fDeadTime)
					m_bDead = true;
			}
		}
	}
	else
	{
		m_pTransformCom->Turn(m_vAxis, fTimeDelta * m_fTurnSpeed);

		CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
		CTransform* pKirbyTransform = pKirby->Get_TransformCom();

		m_vTargetPos = pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION);

		if (true == m_bBig)
		{
			vPos += XMVector3Normalize(m_vTargetPos - vPos) * m_fTimeDelta * EaseInQuart(m_fIncreSpeed) * m_fMeteorSpeed;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		}
	}

	return OBJ_NOEVENT;
}

void CMeteor::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 100.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CMeteor::Render()
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

HRESULT CMeteor::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CMeteor::Render_IMGUI()
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

void CMeteor::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

HRESULT CMeteor::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(m_strTag,
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = MONSTER;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMeteor::Bind_ShaderResources()
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

	return S_OK;
}

void CMeteor::Compute_MotionBlur()
{
}

_float CMeteor::EaseInQuart(_float fNumber)
{
	return fNumber * fNumber * fNumber * fNumber;
}

_float CMeteor::EaseOutCubic(_float fNumber)
{
	return fNumber == 1 ? 1 : 1 - pow(2, -10 * fNumber);
}

CMeteor* CMeteor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeteor* pInstance = new CMeteor(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CMeteor"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeteor::Clone(void* pArg)
{
	CMeteor* pInstance = new CMeteor(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CMeteor"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeteor::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
