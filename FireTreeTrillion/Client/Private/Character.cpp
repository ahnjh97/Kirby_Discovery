#include "stdafx.h"
#include "Character.h"
#include "FSM.h"

CCharacter::CCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice , pContext }
{
}

CCharacter::CCharacter(const CCharacter& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CCharacter::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCharacter::Initialize(void* pArg)
{
	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	return S_OK;
}

_int CCharacter::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

    // FSM 제어
    if (m_pFSM != nullptr)
   		m_pFSM->Update(this, fTimeDelta);

	return OBJ_NOEVENT;
}

void CCharacter::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

HRESULT CCharacter::Render()
{
	__super::Render();

	return S_OK;
}

HRESULT CCharacter::Render_LightDepth()
{
	__super::Render();

	return S_OK;
}

void CCharacter::Render_IMGUI()
{
	__super::Render_IMGUI();

}

void CCharacter::SetOn_Slope(_float fTimeDelta)
{
	// 지면의 up벡터
	PxVec3 slope = m_pControllerCom->Compute_Slope(m_pTransformCom);
	_vector vTerrainNormal = CUtils::To_Vector(slope);
	Lerp_UpVector(vTerrainNormal, 10.f, fTimeDelta);
}

void CCharacter::Lerp_UpVector(_fvector _vTargetUp, _float _maxAngle, _float fTimeDelta)
{
	_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(_vTargetUp, m_vOriginUp));

	// 구면 선형 보간 : m_vOriginUp을 vTargetUp 방향으로 보간
	_float fInterpolateAngle = fTimeDelta * XMConvertToRadians(_maxAngle) * m_fOffsetTurn;

	if (fAngle > fInterpolateAngle) { fAngle = fInterpolateAngle / fAngle; }
	else fAngle = 1.0f;

	m_vOriginUp = XMQuaternionSlerp(m_vOriginUp, _vTargetUp, fAngle);
	m_vOriginUp = XMVector3Normalize(m_vOriginUp);

	_vector vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
	_vector vNewRight = XMVector3Cross(m_vOriginUp, vLook);
	vNewRight = XMVector3Normalize(vNewRight);
	_vector vNewLook = XMVector3Cross(vNewRight, m_vOriginUp);
	vNewLook = XMVector3Normalize(vNewLook);

	// OriginUp을 기준으로 다시 재 설정
	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, vNewRight);
	m_pTransformCom->Set_State(CTransform::STATE_UP, m_vOriginUp);
	m_pTransformCom->Set_State(CTransform::STATE_LOOK, vNewLook);
}

void CCharacter::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pControllerCom);
	Safe_Release(m_pFSM);
}
