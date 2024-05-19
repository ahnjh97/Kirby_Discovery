#include "Utils.h"

// string --> wstring 변환
wstring CUtils::StrToWstr(const string& value)
{
	wstring tag;
	USES_CONVERSION;
	tag = std::wstring(A2W(value.c_str()));
	return tag;
}

// wstring --> string 변환
string CUtils::WstrToStr(const wstring& value)
{
	string stemp;
	USES_CONVERSION;
	stemp = std::string(W2A(value.c_str()));
	return stemp;
}

_int CUtils::Make_RandomInt(_int min, _int max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<_int> dis(min, max);
	return dis(gen);
}

_float CUtils::Make_RandomFloat(_float min, _float max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<_float> dis(min, max);

	return dis(gen);
}

// 벡터의 길이만 지정해주면, 랜덤한 방향의 백터를 리턴한다.
_vector CUtils::Make_Random_Vector(_float fVectorLength)
{
	_vector vTempVector = XMVectorSet(fVectorLength, 0.f, 0.f, 0.f);
	_matrix rotationMatrix = XMMatrixIdentity();
	rotationMatrix = XMMatrixRotationX(XMConvertToRadians(Make_RandomFloat(0.f, 360.f))) *
		XMMatrixRotationY(XMConvertToRadians(Make_RandomFloat(0.f, 360.f))) *
		XMMatrixRotationZ(XMConvertToRadians(Make_RandomFloat(0.f, 360.f)));

	return vTempVector = XMVector4Transform(vTempVector, rotationMatrix);
}

// 벡터의 방향과 랜덤 범위각도 및 벡터의 길이를 지정해주면, 각도 내에서 랜덤한 벡터를 리턴한다.
_vector CUtils::Make_RandomAngle_Vector(_float fDirAngle, _fvector vDir)
{
	_float4x4 rotationMatrix;

	XMStoreFloat4x4(&rotationMatrix, XMMatrixIdentity());
	Turn_OtherMatrix(rotationMatrix, XMVectorSet(0.f, 0.f, 1.f, 0.f), 1.f, Make_RandomFloat(-fDirAngle, fDirAngle));
	Turn_OtherMatrix(rotationMatrix, Get_State_Vector_Matrix(rotationMatrix, STATE_RIGHT), 1.f, Make_RandomFloat(-fDirAngle, fDirAngle));

	return XMVector3Transform(vDir, XMLoadFloat4x4(&rotationMatrix));
}

_vector CUtils::Make_RandomAngle_Vector(_float fDirAngle, _fvector vDir, _float fminlength, _float fmaxlength)
{
	_vector vNormalDir = XMVector3Normalize(vDir);

	_float4x4 rotationMatrix;

	XMStoreFloat4x4(&rotationMatrix, XMMatrixIdentity());
	Turn_OtherMatrix(rotationMatrix, XMVectorSet(0.f, 0.f, 1.f, 0.f), 1.f, Make_RandomFloat(-fDirAngle, fDirAngle));
	Turn_OtherMatrix(rotationMatrix, Get_State_Vector_Matrix(rotationMatrix, STATE_RIGHT), 1.f, Make_RandomFloat(-fDirAngle, fDirAngle));

	return XMVector3Transform(vNormalDir, XMLoadFloat4x4(&rotationMatrix)) * Make_RandomFloat(fminlength, fmaxlength);
}

void CUtils::Set_State_Matrix(_Inout_ _float4x4& matrix, STATE eState, _fvector vState)
{
	_float4		vTemp;
	XMStoreFloat4(&vTemp, vState);
	memcpy(&matrix.m[eState], &vTemp, sizeof(_float4));
}

void CUtils::Set_State_Matrix(_Inout_ _float4x4& matrix, STATE eState, const _float4& vState)
{
	memcpy(&matrix.m[eState], &vState, sizeof(_float4));
}

void CUtils::Set_Scaled_Matrix(_Inout_ _float4x4& matrix, _float fScaleX, _float fScaleY, _float fScaleZ)
{
	Set_State_Matrix(matrix, STATE_RIGHT, XMVector3Normalize(Get_State_Vector_Matrix(matrix, STATE_RIGHT)) * fScaleX);
	Set_State_Matrix(matrix, STATE_UP, XMVector3Normalize(Get_State_Vector_Matrix(matrix, STATE_UP)) * fScaleY);
	Set_State_Matrix(matrix, STATE_LOOK, XMVector3Normalize(Get_State_Vector_Matrix(matrix, STATE_LOOK)) * fScaleZ);
}

_vector CUtils::Get_State_Vector_Matrix(_Inout_ _float4x4& matrix, STATE eState)
{
	return XMLoadFloat4x4(&matrix).r[eState];
}

_float3 CUtils::Get_Scaled_Matrix(_Inout_ _float4x4& matrix)
{
	_matrix		Matrix = XMLoadFloat4x4(&matrix);
	return _float3(XMVectorGetX(XMVector3Length(Matrix.r[STATE_RIGHT])),
		XMVectorGetX(XMVector3Length(Matrix.r[STATE_UP])),
		XMVectorGetX(XMVector3Length(Matrix.r[STATE_LOOK])));
}

void CUtils::Turn_OtherMatrix(_Inout_ _float4x4& matrix, _fvector vAxis, _float fTimeDelta, _float fAngle)
{
	_matrix			RotationMatrix = XMMatrixRotationAxis(vAxis, XMConvertToRadians(fAngle) * fTimeDelta);

	for (size_t i = 0; i < STATE_POSITION; i++)
	{
		Set_State_Matrix(matrix, (STATE)i,
			XMVector4Transform(Get_State_Vector_Matrix(matrix, (STATE)i), RotationMatrix));
	}
}

physx::PxMat44 CUtils::To_Float4x4(const _float4x4_sm& mat)
{
	physx::PxMat44 out;
	memcpy(&out.column0, &mat.m[0], sizeof(_float4));
	memcpy(&out.column1, &mat.m[1], sizeof(_float4));
	memcpy(&out.column2, &mat.m[2], sizeof(_float4));
	memcpy(&out.column3, &mat.m[3], sizeof(_float4));
	return out;
}

_float4x4_sm CUtils::To_Float4x4(const physx::PxMat44& mat)
{
	_float4x4_sm out;
	memcpy(&out.m[0], &mat.column0, sizeof(_float4));
	memcpy(&out.m[1], &mat.column1, sizeof(_float4));
	memcpy(&out.m[2], &mat.column2, sizeof(_float4));
	memcpy(&out.m[3], &mat.column3, sizeof(_float4));
	return out;
}
