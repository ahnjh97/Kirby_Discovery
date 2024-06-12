#include "stdafx.h"
#include "Kabu.h"
#include "FSM.h"
#include "Kabu_State.h"

CKabu::CKabu(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CKabu::CKabu(const CKabu& rhs)
	: CMonster{ rhs }
{
}

HRESULT CKabu::Initialize_Prototype()
{
	m_eCollisionGroup = MONSTER;

	return S_OK;
}

HRESULT CKabu::Initialize(void* pArg)
{
	KABU_DESC* pKabuDesc = nullptr;

	if (nullptr != pArg)
	{
		pKabuDesc = (KABU_DESC*)pArg;

		pKabuDesc->fSpeedPerSec = 7.f;
		pKabuDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_eMoveState = pKabuDesc->eMoveState;
	}

	if (FAILED(__super::Initialize(pKabuDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pModelCom->Set_Animation(KABU_WAIT, 50.f, true, true);


	m_fMaxHp = 15.f;
	m_fHp = 15.f;
	m_fAttack = 10.f;
	m_eVacuumSize = SIZE_SMALL;
	m_eAbilityType = ABILITY_DEFAULT;

	m_fDistance = 10.f;
	m_fSpeed = 0.f;

	//m_vRally = m_vecRallyPoint[m_iCnt + 1] - m_vecRallyPoint[m_iCnt];

	return S_OK;
}

_int CKabu::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fTimeDelta * 5.f);

	if(KABUMOVING_CIRCLE == m_eMoveState)
	{
		m_fAngle += m_fTimeDelta * 50.f;

		m_vRotatePos.x = m_vOriginPos.x + (m_fDistance * sin(XMConvertToRadians(m_fAngle)));
		m_vRotatePos.z = m_vOriginPos.z - (m_fDistance * cos(XMConvertToRadians(m_fAngle)));

		m_pControllerCom->Move(m_pTransformCom, m_vRotatePos, m_fTimeDelta);
	}
	else if (KABUMOVING_PATROL == m_eMoveState)
	{
		m_fMoveTime += m_fTimeDelta;

		if (1.f < m_fMoveTime)
			m_fSpeed -= m_fTimeDelta * 10.f;
		else if (1.f >= m_fMoveTime)
			m_fSpeed += m_fTimeDelta * 10.f;

		if (2.f < m_fMoveTime)
		{
			m_fMoveTime = 0.f;
			m_fSpeed = 0.f;

			if (m_iCnt == 0)
			{
				m_bConvert = false;
				m_vRally = m_vecRallyPoint[m_iCnt + 1] - m_vecRallyPoint[m_iCnt];
				m_iCnt++;
			}
			else if (m_iCnt < m_vecRallyPoint.size() - 1)
			{
				if(false == m_bConvert)
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

		m_pControllerCom->Move_Dir(m_pTransformCom, XMVector3Normalize(m_vRally) * m_fTimeDelta * m_fSpeed, m_fTimeDelta);
	}

	__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CKabu::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(m_fTimeDelta);

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}

}

HRESULT CKabu::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CKabu::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CKabu::Render_IMGUI()
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

void CKabu::Collision_Attack(CGameObject* pOtherObj)
{
}

void CKabu::Change_State(KABU_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CKabu::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

HRESULT CKabu::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Kabu"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.uCollisionType = m_eCollisionGroup;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	m_pControllerCom->Set_Object(this);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	m_vOriginPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	m_vecRallyPoint.push_back(vPos);
	m_vecRallyPoint.push_back(XMVectorSet(3.f, 7.f, -176.f, 1.f));
	//m_vecRallyPoint.push_back(XMVectorSet(2.f, 6.5f, -187.f, 1.f));

	return S_OK;
}

HRESULT CKabu::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	_bool bStencil = true;
	_bool bRimLight = true;
	_bool bMotionBlur = true;
	m_pShaderCom->Bind_RawValue("g_bStencil", &bStencil, sizeof(_bool));
	m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool));
	m_pShaderCom->Bind_RawValue("g_bMotionBlur", &bMotionBlur, sizeof(_bool));
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

void CKabu::Compute_MotionBlur()
{
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
	_float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
	_float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

	_float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

	m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
	m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
	m_vPreScreenPos = vCurScreenPos;
}

void CKabu::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();
	m_pFSM->Add_State(KABU_WAIT, CKabu_Idle_State::Create());

	//상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = KABU_WAIT;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

CKabu* CKabu::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKabu* pInstance = new CKabu(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CKabu"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CKabu::Clone(void* pArg)
{
	CKabu* pInstance = new CKabu(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CKabu"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKabu::Free()
{
	__super::Free();

}
