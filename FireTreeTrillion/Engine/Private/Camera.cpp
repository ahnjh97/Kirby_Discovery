#include "..\Public\Camera.h"
#include "GameInstance.h"

CCamera::CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{


}

CCamera::CCamera(const CCamera& rhs)
	: CGameObject(rhs)
{
}

void CCamera::Set_Target(CTransform* pTarget, CAMTARGET eTarget, CAMFOCUS eFocus, _float3 _vAnchorOffset, _float _fInterpolateSpeed)
{
	if (nullptr == pTarget)
		return;

	if (eFocus == FOCUS_FIRST)
	{
		if (nullptr != m_pFirstTarget)
			Safe_Release(m_pFirstTarget);

		m_pFirstTarget = pTarget;
		Safe_AddRef(pTarget);

	}
	else if (eFocus == FOCUS_SECOND)
	{
		if (nullptr != m_pSecondTarget)
			Safe_Release(m_pSecondTarget);

		m_pSecondTarget = pTarget;
		Safe_AddRef(pTarget);
	}

	m_vAnchorOffset = _vAnchorOffset;

	if (0.f < _fInterpolateSpeed)
		m_fInterpolateSpeed = _fInterpolateSpeed;
}

void CCamera::Lock_Position(_float3 vPos, _bool bInterpolate)
{
	m_eCamLockMode = LOCK_POS;

	if (bInterpolate == false)
	{
		if(ISDEFAULTFLOAT3(vPos))
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, Pos(vPos));

	}

}

void CCamera::Lock_Direction(_float3 vLook, _bool bInterpolate)
{
	m_eCamLockMode = LOCK_DIR;

}

void CCamera::Lock_All(_float3 vPos, _float3 vLook, _bool bInterpolate)
{
	m_eCamLockMode = LOCK_ALL;
}

void CCamera::Lock_Camera(_float3 vPos, _float3 vLook, _float fFOVY)
{
	m_bLockCamera = true;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, Pos(vPos));

	vLook.Normalize();
	m_pTransformCom->Look_At_Dir(Dir(vLook));
	m_fFovy = ToRadian(fFOVY);

}

HRESULT CCamera::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera::Initialize(void* pArg)
{
	CAMERA_DESC* pCameraDesc = (CAMERA_DESC*)pArg;

	m_fFovy = pCameraDesc->fFovy;
	m_fAspect = pCameraDesc->fAspect;
	m_fNear = pCameraDesc->fNear;
	m_fFar = pCameraDesc->fFar;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&pCameraDesc->vEye));
	m_pTransformCom->Look_At(XMLoadFloat4(&pCameraDesc->vAt));

	return S_OK;
}

_int CCamera::Tick(_float fTimeDelta)
{

	return OBJ_NOEVENT;
}

void CCamera::Late_Tick(_float fTimeDelta)
{
}


HRESULT CCamera::Bind_PipeLines()
{
	/* dx9 : 고정기능렌더링파이프라인. 현재 카메라에서 설정할 수 있는 행렬들을 장치에 바인딩하여 추후 렌더릴ㅇ되는 정점들에게 알아서 곱할 수 있도록 한다. */
	/* dx11 : 사용자 정의 렌더링 파이프라인(ㅅㅖ이더). */
	m_pGameInstance->Set_Transform(CPipeLine::D3DTS_VIEW, m_pTransformCom->Get_WorldMatrix_Inv());
	m_pGameInstance->Set_Transform(CPipeLine::D3DTS_PROJ, XMMatrixPerspectiveFovLH(m_fFovy, m_fAspect, m_fNear, m_fFar));

	return S_OK;
}

void CCamera::Free()
{
	Safe_Release(m_pFirstTarget);
	Safe_Release(m_pSecondTarget);

	__super::Free();

}
