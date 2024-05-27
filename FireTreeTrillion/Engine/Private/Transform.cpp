#include "..\Public\Transform.h"
#include "Shader.h"

CTransform::CTransform(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent(pDevice, pContext)
{	

}

CTransform::CTransform(const CTransform & rhs)
	: CComponent(rhs)
	, m_WorldMatrix(rhs.m_WorldMatrix)
{

}

void CTransform::Set_Scaled(_float fScaleX, _float fScaleY, _float fScaleZ)
{
	Set_State(STATE_RIGHT, XMVector3Normalize(Get_State_Vector(STATE_RIGHT)) * fScaleX);
	Set_State(STATE_UP, XMVector3Normalize(Get_State_Vector(STATE_UP)) * fScaleY);
	Set_State(STATE_LOOK, XMVector3Normalize(Get_State_Vector(STATE_LOOK)) * fScaleZ);
		
}

void CTransform::Set_Scaled(_float3 vScale)
{
	Set_Scaled(vScale.x, vScale.y, vScale.z);
}

HRESULT CTransform::Initialize_Prototype()
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	return S_OK;
}

HRESULT CTransform::Initialize(void * pArg)
{
	if (nullptr != pArg)
	{
		TRANSFORM_DESC*		pDesc = (TRANSFORM_DESC*)pArg;

		m_fSpeedPerSec = pDesc->fSpeedPerSec;
		m_fRotationPerSec = pDesc->fRotationPerSec;
	}

	return S_OK;
}

HRESULT CTransform::Bind_ShaderResource(CShader * pShader, const _char * pConstantName)
{
	return pShader->Bind_Matrix(pConstantName, &m_WorldMatrix);
	
}


void CTransform::Go_Straight(_float fTimeDelta)
{
	_vector		vPosition = Get_State_Vector(STATE_POSITION);
	_vector		vLook = Get_State_Vector(STATE_LOOK);

	vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);
}



void CTransform::Go_Backward(_float fTimeDelta)
{
	_vector		vPosition = Get_State_Vector(STATE_POSITION);
	_vector		vLook = Get_State_Vector(STATE_LOOK);

	vPosition -= XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Go_Left(_float fTimeDelta)
{
	_vector		vPosition = Get_State_Vector(STATE_POSITION);
	_vector		vRight = Get_State_Vector(STATE_RIGHT);

	vPosition -= XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Go_Right(_float fTimeDelta)
{
	_vector		vPosition = Get_State_Vector(STATE_POSITION);
	_vector		vRight = Get_State_Vector(STATE_RIGHT);

	vPosition += XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Go_Up(_float fTimeDelta)
{
	_vector		vPosition = Get_State_Vector(STATE_POSITION);
	m_WorldMatrix.Up().Normalize();
	vPosition += XMVector3Normalize(m_WorldMatrix.Up()) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Go_Down(_float fTimeDelta)
{
	_vector		vPosition = Get_State_Vector(STATE_POSITION);

	vPosition += XMVector3Normalize(m_WorldMatrix.Down()) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Look_At(_fvector vAt)
{
	_vector		vLook = vAt - Get_State_Vector(STATE_POSITION);
	_vector		vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	_vector		vUp = XMVector3Cross(vLook, vRight);

	_float3		vScaled = Get_Scaled();

	Set_State(STATE_RIGHT, XMVector3Normalize(vRight) * vScaled.x);
	Set_State(STATE_UP, XMVector3Normalize(vUp) * vScaled.y);
	Set_State(STATE_LOOK, XMVector3Normalize(vLook) * vScaled.z);
}

void CTransform::Look_At_ForLandObject(_fvector vAt)
{
	// XMMatrixDecompose();

	_vector		vUp = Get_State_Vector(STATE_UP);
	_vector		vLook = vAt - Get_State_Vector(STATE_POSITION);
	_vector		vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	vLook = XMVector3Cross(vRight, vUp);

	_float3		vScaled = Get_Scaled();

	Set_State(STATE_RIGHT, XMVector3Normalize(vRight) * vScaled.x);	
	Set_State(STATE_LOOK, XMVector3Normalize(vLook) * vScaled.z);
}

void CTransform::Look_At_Rotate(_vector vAt, _float fTimeDelta)
{
	_vector vLook = Get_State_Vector(CTransform::STATE_LOOK);
	vLook.m128_f32[1] = 0.f;
	_vector vTargetLook = vAt - Get_State_Vector(CTransform::STATE_POSITION);
	vTargetLook.m128_f32[1] = 0.f;

	_vector vLerpLook = XMVectorLerp(vLook, vTargetLook, m_fRotationPerSec * fTimeDelta);

	_vector vLerpRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLerpLook);
	_vector vLerpUp = XMVector3Cross(vLerpLook, vLerpRight);

	_float3 vScaled = Get_Scaled();

	Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vLerpRight) * vScaled.x);
	Set_State(CTransform::STATE_UP, XMVector3Normalize(vLerpUp) * vScaled.y);
	Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLerpLook) * vScaled.z);
}

void CTransform::Look_At_Angle(_fvector vAt, _fvector vAxis, _float fRadian)
{
	if (XMVector3Equal(vAxis, XMVectorZero()))
		return;

	_vector vLook = vAt;
	_matrix matRotate = XMMatrixRotationAxis(vAxis, fRadian);
	vLook = XMVector3TransformNormal(vLook, matRotate);

	_vector	vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	_vector	vUp = XMVector3Cross(vLook, vRight);

	_float3	vScaled = Get_Scaled();

	Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight) * vScaled.x);
	Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp) * vScaled.y);
	Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook) * vScaled.z);
}

void CTransform::Move_toTarget(_fvector vTargetPos, _float fTimeDelta, _float fMinDistance)
{
	_vector		vPosition = Get_State_Vector(STATE_POSITION);

	_vector		vLook = vTargetPos - vPosition;

	if(fMinDistance <= XMVector3Length(vLook).m128_f32[0])
		vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;
}

void CTransform::Turn(_fvector vAxis, _float fTimeDelta)
{
	_matrix			RotationMatrix  = XMMatrixRotationAxis(vAxis, m_fRotationPerSec * fTimeDelta);

	for (size_t i = 0; i < STATE_POSITION; i++)
	{
		Set_State(STATE(i), 
			XMVector4Transform(Get_State_Vector((STATE)i), RotationMatrix));
		/*Set_State(STATE(i),
			XMVector3TransformNormal(Get_State_Vector((STATE)i), RotationMatrix));*/
	}

}

void CTransform::Turn(_fvector vAxis, _float fTimeDelta, _float fAngle)
{
	_matrix			RotationMatrix = XMMatrixRotationAxis(vAxis, XMConvertToRadians(fAngle) * fTimeDelta);

	for (size_t i = 0; i < STATE_POSITION; i++)
	{
		Set_State(STATE(i),
			XMVector4Transform(Get_State_Vector((STATE)i), RotationMatrix));
	}
}

void CTransform::Turn_Absolute(_float4 _vQuat)
{
	//Quaternion vCurQuat = _vQuat;
	_float4x4 RotMat = _float4x4::CreateFromQuaternion(_vQuat);
	_float3	vScale = Get_Scaled();
	_float3 vPos = Get_State(STATE_POSITION);

	//_float4x4 ResultWorld = 
	m_WorldMatrix = _float4x4::CreateScale(vScale) * RotMat * _float4x4::CreateTranslation(vPos);
}

void CTransform::Rotation(_fvector vAxis, _float fRadian)
{
	_float3		vScaled = Get_Scaled();

	_vector		vState[] = {
		XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScaled.x,
		XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScaled.y, 
		XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScaled.z, 		
	};

	
	// XMConvertToRadians(Degree);

	_matrix			RotationMatrix = XMMatrixRotationAxis(vAxis, fRadian);

	for (size_t i = 0; i < STATE_POSITION; i++)
	{
		Set_State(STATE(i),
			XMVector4Transform(vState[(STATE)i], RotationMatrix));
	}
}

void CTransform::Orbit(_fvector vTarget, _fvector vAxis, _float fTimeDelta)
{
	_vector vDir = Get_State_Vector(STATE_POSITION) - vTarget;
	Set_State(STATE_POSITION, vTarget);

	_matrix	RotationMatrix = XMMatrixRotationAxis(vAxis, ToRadian(90.f) * fTimeDelta);
	Turn(vAxis, fTimeDelta);
	vDir = XMVector4Transform(vDir, RotationMatrix);
	Set_State(STATE_POSITION, vTarget + vDir);
}

CTransform * CTransform::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTransform*		pInstance = new CTransform(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CTransform"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CTransform::Clone(void * pArg)
{
	CTransform*		pInstance = new CTransform(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CTransform"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTransform::Free()
{
	__super::Free();

}
