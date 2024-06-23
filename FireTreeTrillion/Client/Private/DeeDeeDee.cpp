#include "stdafx.h"
#include "DeeDeeDee.h"
#include "FSM.h"

#include "HitBox.h"
#include "Bone.h"

CDeeDeeDee::CDeeDeeDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CDeeDeeDee::CDeeDeeDee(const CDeeDeeDee& rhs)
	: CMonster{ rhs }
{
}

HRESULT CDeeDeeDee::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDeeDeeDee::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_fMaxHp = 300.f;
	m_fHp = 300.f;
	m_fAttack = 15.f;
	m_eVacuumSize = SIZE_BIG;
	m_eAbilityType = ABILITY_DEFAULT;

	Add_AnimEvent();

	m_pModelCom->Set_Animation(0, 60.f, true, true);

	m_vBoneLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);

	return S_OK;
}

_int CDeeDeeDee::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	m_pControllerCom->FreeFall(m_pTransformCom, m_fTimeDelta, 6.0f);

	__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CDeeDeeDee::Late_Tick(_float fTimeDelta)
{

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		if (Compute_OptimizationAnimation(m_fTimeDelta) == true && m_ePhyXState != PO_PRESSED)
			m_ePhyXState == PO_FLYDEADAWAY ? m_pModelCom->Play_Animation(m_fAccTime * 0.3f) : m_pModelCom->Play_Animation(m_fAccTime);
	}
	// 뼈를 꺾는다.
	Look_Player(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
}

HRESULT CDeeDeeDee::Render()
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
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;

}

HRESULT CDeeDeeDee::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG

void CDeeDeeDee::Render_IMGUI()
{
}

#endif

void CDeeDeeDee::Add_AnimEvent()
{
}

void CDeeDeeDee::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

void CDeeDeeDee::Change_State(STATE_TYPE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CDeeDeeDee::Look_Player(_float fTimeDelta)
{
	CGameObject* pPlayer = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"));

	if (pPlayer == nullptr)
		return;

	////// 목을 돌린다.

	CBone* pBone = m_pModelCom->Get_BonePtr("C_FaceJ");
	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float4 vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);

	_float4 vTargetPos = pPlayer->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
	_float4 vTargetDir = vTargetPos - vPos;
	vTargetDir.y = 0.f;
	vTargetDir.Normalize();

	if (ToDegree(acos(vLook.Dot(vTargetDir))) > 40.f)
	{
		vTargetDir = vLook;
	}
	
	Bone_Turn_Interpolate(m_vBoneLook, vTargetDir, fTimeDelta);
	Quaternion vStartQuat = CUtils::Make_Quat_FromDir(vLook);
	Quaternion vDestQuat = CUtils::Make_Quat_FromDir(m_vBoneLook);
	vStartQuat.Inverse(vStartQuat);
	Quaternion vResultQuat = vDestQuat * vStartQuat;
	_float4x4::CreateFromQuaternion(vResultQuat);
	_matrix RotationMatrix = _float4x4::CreateFromQuaternion(vResultQuat);
	_float4x4* pEditMatrix = pBone->Get_EditMatrixPtr();
	*pEditMatrix = RotationMatrix;

	////////

}

void CDeeDeeDee::Bone_Turn_Interpolate(_float4& vMoveDir, const _float4& vTargetDir, _float fTimeDelta)
{
	if (vMoveDir == vTargetDir)
		return;

	///////// 보간 속도 조정임
	_float fInterpolate = fTimeDelta * 12.f;
	_vector vTargetDirXZ = XMVectorSet(XMVectorGetX(vTargetDir), 0.0f, XMVectorGetZ(vTargetDir), 0.0f);
	_vector vMoveDirXZ = XMVectorSet(XMVectorGetX(vMoveDir), 0.0f, XMVectorGetZ(vMoveDir), 0.0f);

	vTargetDirXZ = XMVector3Normalize(vTargetDirXZ);
	vMoveDirXZ = XMVector3Normalize(vMoveDirXZ);
	_float fcosTheta = XMVectorGetX(XMVector4Dot(vTargetDirXZ, vMoveDirXZ));

	if (fcosTheta < -0.9995f || fcosTheta > 0.9995f)
	{
		// 180도로 NaN 방지 랜덤으로 -1, 1도 틀어줌
		_float4x4 rotationMatrix;
		XMStoreFloat4x4(&rotationMatrix, XMMatrixIdentity());
		CUtils::Turn_OtherMatrix(rotationMatrix, XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomInt(0, 1) == 1 ? 1.f : -1.f);
		vMoveDir = XMVector3Transform(vMoveDir, XMLoadFloat4x4(&rotationMatrix));
		vMoveDir = XMVectorSetW(vMoveDir, 0.0f);
	}
	else
	{
		_float ftheta = acos(fcosTheta);
		_float fAngleDegrees = XMConvertToDegrees(ftheta);

		if (fAngleDegrees < 3.0f)
		{
			vMoveDir = vTargetDir;
		}
		else
		{
			_float fsinTheta = sqrt(1.0f - fcosTheta * fcosTheta);
			_float fAlpha = sin((1 - fInterpolate) * ftheta) / fsinTheta;
			_float fBeta = sin(fInterpolate * ftheta) / fsinTheta;
			_float4 vResult = vMoveDirXZ * fAlpha + vTargetDirXZ * fBeta;
			vMoveDir = XMVector4Normalize(vResult);
			vMoveDir = XMVector3Normalize(vMoveDir);
		}
	}
}


_bool CDeeDeeDee::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

HRESULT CDeeDeeDee::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_DeeDeeDee"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	desc.fOffset = 1.f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	CHECK_FAILED(hr);

	/* FSM */
	SetUp_FSM();

	return S_OK;
}

HRESULT CDeeDeeDee::Bind_ShaderResources()
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

void CDeeDeeDee::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();


	// 상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.pModel = &m_pModelCom;
	FSM_Desc.iState = 0;
	m_pFSM->Initialize(&FSM_Desc);
}

CDeeDeeDee* CDeeDeeDee::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDeeDeeDee* pInstance = new CDeeDeeDee(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CDeeDeeDee"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDeeDeeDee::Clone(void* pArg)
{
	CDeeDeeDee* pInstance = new CDeeDeeDee(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CDeeDeeDee"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDeeDeeDee::Free()
{
	__super::Free();
}
