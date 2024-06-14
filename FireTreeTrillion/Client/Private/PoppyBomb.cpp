#include "stdafx.h"
#include "PoppyBomb.h"
#include "PoppyBrosJr.h"

CPoppyBomb::CPoppyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CPoppyBomb::CPoppyBomb(const CPoppyBomb& rhs)
	: CMonster{ rhs }
{
}

HRESULT CPoppyBomb::Initialize_Prototype()
{
	m_eCollisionGroup = MONSTER;

	return S_OK;
}

HRESULT CPoppyBomb::Initialize(void* pArg)
{
	POPPYBOMB_DESC* pPoppyBombDesc = nullptr;

	if (nullptr != pArg)
	{
		pPoppyBombDesc = (POPPYBOMB_DESC*)pArg;

		pPoppyBombDesc->fSpeedPerSec = 7.f;
		pPoppyBombDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_vPosition = pPoppyBombDesc->vPosition;
		m_vLook = pPoppyBombDesc->vLook;
		m_vTargetPosition = pPoppyBombDesc->vTargetPosition;
		m_pGameObject = pPoppyBombDesc->pGameObject;
		Safe_AddRef(m_pGameObject);
	}

	if (FAILED(__super::Initialize(pPoppyBombDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_fMaxHp = 15.f;
	m_fHp = 15.f;
	m_fAttack = 10.f;
	m_eVacuumSize = SIZE_SMALL;
	m_eAbilityType = ABILITY_BOMB;

	m_bJump = true;
	m_fMoveTime = 1.f;
	m_vLookDir = m_vTargetPosition - m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	return S_OK;
}

_int CPoppyBomb::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	if (m_pGameInstance->Get_KeyState(DIK_F, KEY_DOWN))
		m_pGameInstance->Set_SecondTimerRatio(0.2f);
	if (m_pGameInstance->Get_KeyState(DIK_D, KEY_DOWN))
		m_pGameInstance->Restore_SecondTimer();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	// 폭탄이 손에서 날아갈 타이밍
	if (true == m_bBomb)
	{
		// 폭탄이 땅에 떨어졌을 때
		if (!m_bJump)
		{
			m_pControllerCom->FreeFall(m_pTransformCom, m_fTimeDelta, 6.f, 0.5f);
			m_bPhysx = true;
		}
		// 폭탄이 포물선을 그리며 날아감
		else
		{
			m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), m_fTimeDelta * 1.5f);

			m_fJumpTimeDelta += m_fTimeDelta;

			// 매 Tick 점프 중인 위치 벡터 받아오기
			_vector vGoPos = JumpAttak(m_fJumpTimeDelta);

			// 점프 위치 벡터를 physx에 던지기
			m_bJump = m_pControllerCom->Jump_Parabola(m_pTransformCom, vGoPos, m_fTimeDelta);
		}
	}
	// 폭탄이 몬스터 손에 붙어 있을 때
	else
	{
		// 폭탄을 몬스터 손에 붙힘
		_float4 vPos = dynamic_cast<CPoppyBrosJr*>(m_pGameObject)->Compute_BoneWorldMatrix();
		m_pControllerCom->Move(m_pTransformCom, vPos, m_fTimeDelta);

		if (0.5f < dynamic_cast<CPoppyBrosJr*>(m_pGameObject)->Get_AnimRatio())
		{
			// 타겟 위치에 포물선 궤적 계산
			m_pTransformCom->Look_At(m_vTargetPosition);
			Compute_Parabola(m_vTargetPosition);
			m_bBomb = true;
		}
	}

	// 바닥에 닿았을 때 물리 영향을 받음
	if (true == m_bPhysx)
	{
		_vector vGravity = XMVectorSet(0.f, -GRAVITY, 0.f, 0.f);

		// 경사면 법선 벡터
		_vector vNormal = CUtils::To_Vector(m_pControllerCom->Compute_Slope(m_pTransformCom));

		// 중력 벡터를 경사면 법선에 투영하여 평면상에서의 중력 계산
		_vector vGravityParallel = XMVector3Dot(vGravity, vNormal) * vNormal;
		_vector vGravityPerpendicular = vGravity - vGravityParallel;

		// 마찰력 계산
		// 마찰력은 중력과 반대 방향으로 작용하며, 타임 델타를 반영하여 계산
		_vector vFriction = -0.5f * vGravityPerpendicular;

		// 가속도 계산
		_vector vAcceleration = vGravityPerpendicular + vFriction;

		// 속도 계산
		_vector vVelocity = {};
		vVelocity += vAcceleration * m_fTimeDelta;

		// 이동
		_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		vPos += (vVelocity + (XMVector3Normalize(m_vLookDir) * 0.2f * m_fMoveTime)) * m_fTimeDelta * 50.f;
		m_pControllerCom->Move(m_pTransformCom, vPos, m_fTimeDelta, 0.5f);

		if (0.f < m_fMoveTime)
		{
			m_fMoveTime -= m_fTimeDelta;
		}
		else
		{
			m_fMoveTime = 0.f;
		}

		_vector vLook = vPos - m_vBeforePos;
		_float fDistance = XMVectorGetX(XMVector3Length(vLook)) / m_fTimeDelta;

		// 벡터가 0 벡터가 아닌지 확인
		if (!XMVector3Equal(vLook, XMVectorZero()))
		{
			_vector vRight = XMVector3Cross(XMVector3Normalize(vLook), XMVector3Normalize(vNormal));
			if (!XMVector3Equal(vRight, XMVectorZero()))
			{
				m_pTransformCom->Turn(-vRight, m_fTimeDelta * fDistance);
				m_vBeforePos = vPos;
			}
		}
	}
	//__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CPoppyBomb::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CPoppyBomb::Render()
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

HRESULT CPoppyBomb::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPoppyBomb::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_PoppyBomb"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = m_vPosition;
	desc.uCollisionType = m_eCollisionGroup;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	m_pControllerCom->Set_Object(this);
	//m_pControllerCom->Set_CollisionType(m_eCollisionGroup);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);

	return S_OK;
}

HRESULT CPoppyBomb::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	// 추후 변경
	_bool bStencil = true;
	_bool bRimLight = true;
	_bool bMotionBlur = true;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &bStencil, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &bMotionBlur, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

void CPoppyBomb::Compute_MotionBlur()
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

void CPoppyBomb::Compute_Parabola(_vector vEndPos)
{
	// 포물선 세팅을 위한 변수
	//m_vStartPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	m_vStartPos = dynamic_cast<CPoppyBrosJr*>(m_pGameObject)->Compute_BoneWorldMatrix();
	m_vEndPos = vEndPos;
	m_vGoPos = m_vStartPos;
	m_fMaxHeight = m_vStartPos.y + 4.f;

	m_fEndHight = m_vEndPos.y - m_vStartPos.y; // 도착지점의 높이와 시작지점 높이의 차를 구해줌
	m_fHeight = m_fMaxHeight - m_vStartPos.y;

	m_fGravity = GRAVITY * 3.f /*2.f * m_fHeight / (m_fMaxTime * m_fMaxTime)*/;

	m_fAxisY = sqrtf(2.f * m_fGravity * m_fHeight);

	_float b = -2.f * m_fAxisY;
	_float c = 2.f * m_fEndHight;

	_float fResult = b * b - 4.f * m_fGravity * c;

	if (0.f > fResult)
		m_fEndTime = -b;
	else
		m_fEndTime = (-b + sqrtf(fResult)) / (2.f * m_fGravity);

	m_fAxisX = -(m_vStartPos.x - m_vEndPos.x) / m_fEndTime;
	m_fAxisZ = -(m_vStartPos.z - m_vEndPos.z) / m_fEndTime;
}

_vector CPoppyBomb::JumpAttak(_float fTimeDelta)
{
	m_vGoPos.x = m_vStartPos.x + m_fAxisX * fTimeDelta;
	m_vGoPos.y = m_vStartPos.y + (m_fAxisY * fTimeDelta) - (0.5f * m_fGravity * fTimeDelta * fTimeDelta);
	m_vGoPos.z = m_vStartPos.z + m_fAxisZ * fTimeDelta;

	return m_vGoPos;
}

CPoppyBomb* CPoppyBomb::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPoppyBomb* pInstance = new CPoppyBomb(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CPoppyBomb"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPoppyBomb::Clone(void* pArg)
{
	CPoppyBomb* pInstance = new CPoppyBomb(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CPoppyBomb"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPoppyBomb::Free()
{
	__super::Free();

	Safe_Release(m_pGameObject);

	//Safe_Release(m_pShaderCom);
	//Safe_Release(m_pModelCom);
	//Safe_Release(m_pControllerCom);
}
