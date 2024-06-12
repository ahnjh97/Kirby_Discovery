#include "Utils.h"

// string --> wstring 변환
wstring CUtils::StrToWstr(const string& value)
{
	wstring tag;
	USES_CONVERSION;
	tag = std::wstring(A2W(value.c_str()));
	return tag;
}

wstring CUtils::StrToWstrUTF8(const string& value)
{
	//UTF8 인코딩 변환하여 RenderFont를 위함
	int len;
	int strLength = static_cast<int>(value.length()); //+ 1;
	len = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), strLength, 0, 0);
	std::wstring wstr(len, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, value.c_str(), strLength, &wstr[0], len);
	return wstr;
}

// wstring --> string 변환
string CUtils::WstrToStr(const wstring& value)
{
	string stemp;
	USES_CONVERSION;
	stemp = std::string(W2A(value.c_str()));
	return stemp;
}

string CUtils::WstrToStrUTF8(const std::wstring& value)
{
	//UTF8 인코딩 변환하여 RenderFont를 위함
	int len;
	int wstrLength = static_cast<int>(value.length()); //+ 1;
	len = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), wstrLength, 0, 0, 0, 0);
	std::string str(len, '\0');
	WideCharToMultiByte(CP_UTF8, 0, value.c_str(), wstrLength, &str[0], len, 0, 0);
	return str;
}

string CUtils::StrToUTF8(const string& value)
{
	// ANSI 문자열을 wide 문자열로 변환
	int wideCharLen = MultiByteToWideChar(CP_ACP, 0, value.c_str(), -1, NULL, 0);
	if (wideCharLen == 0) {
		return "";
	}

	std::wstring wideString(wideCharLen, 0);
	MultiByteToWideChar(CP_ACP, 0, value.c_str(), -1, &wideString[0], wideCharLen);

	// Wide 문자열을 UTF-8로 변환
	int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, NULL, 0, NULL, NULL);
	if (utf8Len == 0) {
		return "";
	}

	std::string utf8String(utf8Len, 0);
	WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, &utf8String[0], utf8Len, NULL, NULL);

	return utf8String;
}

void CUtils::WCharToChar(const wchar_t* szWchar, char* szChar)
{
	_int len;
	_int slength = lstrlen(szWchar) + 1;
	len = ::WideCharToMultiByte(CP_ACP, 0, szWchar, slength, 0, 0, 0, 0);
	::WideCharToMultiByte(CP_ACP, 0, szWchar, slength, szChar, len, 0, 0);
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

Quaternion CUtils::Make_Quat_FromMatrix(const _float4x4& _mat)
{
	Matrix curMat = _mat;
	_float3 vScale, vPos;
	Quaternion vRot;
	curMat.Decompose(vScale, vRot, vScale);
	return vRot;
}

Quaternion CUtils::Make_Quat_FromDir(const _float4& _dir)
{
	_float3 vStartDir{ 0.f, 0.f, 1.f };
	_float3 vDestDir = _dir;
	

	return Quaternion::FromToRotation(vStartDir, vDestDir);
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

physx::PxMat44 CUtils::To_Float4x4(const _float4x4& mat)
{
	physx::PxMat44 out;
	memcpy(&out.column0, &mat.m[0], sizeof(_float4));
	memcpy(&out.column1, &mat.m[1], sizeof(_float4));
	memcpy(&out.column2, &mat.m[2], sizeof(_float4));
	memcpy(&out.column3, &mat.m[3], sizeof(_float4));
	return out;
}

_float4x4 CUtils::To_Float4x4(const physx::PxMat44& mat)
{
	_float4x4 out;
	memcpy(&out.m[0], &mat.column0, sizeof(_float4));
	memcpy(&out.m[1], &mat.column1, sizeof(_float4));
	memcpy(&out.m[2], &mat.column2, sizeof(_float4));
	memcpy(&out.m[3], &mat.column3, sizeof(_float4));
	return out;
}

PxVec3 CUtils::To_PxVec3(const _fvector& vector) 
{
	_float3 temp;
	XMStoreFloat3(&temp, vector);

	// PxVec3로 변환하여 반환
	return PxVec3(temp.x, temp.y, temp.z);
}

_vector CUtils::To_Vector(const PxVec3 pxVec3)
{
	return XMVectorSet(pxVec3.x, pxVec3.y, pxVec3.z, 0.0f);
}

PxTransform CUtils::mat44ToTransform(const PxMat44& mat)
{
	// 위치 벡터 추출
	PxVec3 position = mat.getPosition();

	// 상단 좌측 3x3 부분 행렬로부터 회전 쿼터니언 추출
	PxMat33 rotationMat33(mat.column0.getXYZ(), mat.column1.getXYZ(), mat.column2.getXYZ());
	PxQuat rotation = PxQuat(rotationMat33);

	// NaN 및 Inf 값 처리: NaN 또는 Inf가 있는 경우, 해당 성분을 0으로 설정
	if (!std::isfinite(rotation.x) || std::isnan(rotation.x)) rotation.x = 0.0f;
	if (!std::isfinite(rotation.y) || std::isnan(rotation.y)) rotation.y = 0.0f;
	if (!std::isfinite(rotation.z) || std::isnan(rotation.z)) rotation.z = 0.0f;
	if (!std::isfinite(rotation.w) || std::isnan(rotation.w)) rotation.w = 1.0f; // w 성분은 회전을 표현하므로 1로 설정

	// 정규화: 크기가 0인 경우 기본값 (단위 쿼터니언)으로 설정
	float norm = rotation.magnitude();
	if (norm > 1e-6) {
		rotation.x /= norm;
		rotation.y /= norm;
		rotation.z /= norm;
		rotation.w /= norm;
	}
	else {
		rotation = PxQuat(PxIdentity);
	}
	return PxTransform(position, rotation);
}

HRESULT CUtils::Load_Effect(path _FilePath, SINGLE_FX_DATA* _pData)
{
	ifstream InputFile(_FilePath, ios::binary | ios::in);

	if (!InputFile.is_open())
		ALARM_FAIL(TEXT("망했어"));


	//이펙트 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iNameStrLen), sizeof(_uint));
	_pData->strName.resize(_pData->iNameStrLen);
	InputFile.read(&_pData->strName[0], _pData->iNameStrLen);


	//버퍼 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iBufferStrLen), sizeof(_uint));
	_pData->strBufferName.resize(_pData->iBufferStrLen);
	InputFile.read(&_pData->strBufferName[0], _pData->iBufferStrLen);


	//텍스쳐 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iTexStrLen), sizeof(_uint));
	_pData->strTexName.resize(_pData->iTexStrLen);
	InputFile.read(&_pData->strTexName[0], _pData->iTexStrLen);


	//마스크 텍스쳐 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iMaskTexStrLen), sizeof(_uint));
	_pData->strMaskTexName.resize(_pData->iMaskTexStrLen);
	InputFile.read(&_pData->strMaskTexName[0], _pData->iMaskTexStrLen);

	InputFile.read(reinterpret_cast<char*>(&_pData->fDuration), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fLifetime.first), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fLifetime.second), sizeof(_float));


	InputFile.read(reinterpret_cast<char*>(&_pData->iPassIdx), sizeof(_int));
	InputFile.read(reinterpret_cast<char*>(&_pData->iTexIdx), sizeof(_int));
	InputFile.read(reinterpret_cast<char*>(&_pData->iMaskTexIdx), sizeof(_int));


	InputFile.read(reinterpret_cast<char*>(&_pData->bIsLoop), sizeof(_bool));
	InputFile.read(reinterpret_cast<char*>(&_pData->bIsBillboard), sizeof(_bool));
	InputFile.read(reinterpret_cast<char*>(&_pData->bIsOrthographic), sizeof(_bool));
	InputFile.read(reinterpret_cast<char*>(&_pData->bIsColorRender), sizeof(_bool));
	InputFile.read(reinterpret_cast<char*>(&_pData->bIsBloom), sizeof(_bool));

	InputFile.read(reinterpret_cast<char*>(&_pData->fRimLightThreshold), sizeof(_float));


	InputFile.read(reinterpret_cast<char*>(&_pData->iPropertyMapNum), sizeof(_uint));

	_pData->vecKeyframeInfo.resize(_pData->iPropertyMapNum);
	_pData->vecKeyframes.resize(_pData->iPropertyMapNum);

	for (_uint i = 0; i < _pData->iPropertyMapNum; ++i)
	{
		InputFile.read(reinterpret_cast<char*>(&_pData->vecKeyframeInfo[i].first), sizeof(KF_PROPERTY));
		InputFile.read(reinterpret_cast<char*>(&_pData->vecKeyframeInfo[i].second), sizeof(_uint));

		_pData->vecKeyframes[i].resize(_pData->vecKeyframeInfo[i].second);

		for (auto& KF : _pData->vecKeyframes[i])
		{
			InputFile.read(reinterpret_cast<char*>(&KF.fTimeRatio), sizeof(_float));
			InputFile.read(reinterpret_cast<char*>(&KF.vValue), sizeof(_float3));
			InputFile.read(reinterpret_cast<char*>(&KF.eEasing), sizeof(EASING));
		}
	}

	InputFile.read(reinterpret_cast<char*>(&_pData->eRenderGroup), sizeof(_uint));
	if (_pData->eRenderGroup == 5)
	{
		_int a = 5;
	}

	return S_OK;
}

HRESULT CUtils::Load_Effect(path _FilePath, PARTICLE_DATA* _pData)
{
	ifstream InputFile(_FilePath, ios::binary | ios::in);

	if (!InputFile.is_open())
		ALARM_FAIL(TEXT("망했어"));


	//이펙트 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iNameStrLen), sizeof(_uint));
	_pData->strName.resize(_pData->iNameStrLen);
	InputFile.read(&_pData->strName[0], _pData->iNameStrLen);


	//버퍼 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iBufferStrLen), sizeof(_uint));
	_pData->strBufferName.resize(_pData->iBufferStrLen);
	InputFile.read(&_pData->strBufferName[0], _pData->iBufferStrLen);


	//텍스쳐 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iTexStrLen), sizeof(_uint));
	_pData->strTexName.resize(_pData->iTexStrLen);
	InputFile.read(&_pData->strTexName[0], _pData->iTexStrLen);


	//마스크 텍스쳐 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iMaskTexStrLen), sizeof(_uint));
	_pData->strMaskTexName.resize(_pData->iMaskTexStrLen);
	InputFile.read(&_pData->strMaskTexName[0], _pData->iMaskTexStrLen);


	InputFile.read(reinterpret_cast<char*>(&_pData->fDuration), sizeof(_float));

	InputFile.read(reinterpret_cast<char*>(&_pData->fLifetime), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fLifetimeRandomOffset), sizeof(_float));

	InputFile.read(reinterpret_cast<char*>(&_pData->fStartDelay), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fStarDelayRandomOffset), sizeof(_float));

	InputFile.read(reinterpret_cast<char*>(&_pData->vCenter), sizeof(_float3));
	InputFile.read(reinterpret_cast<char*>(&_pData->vRange), sizeof(_float3));

	InputFile.read(reinterpret_cast<char*>(&_pData->vRotation), sizeof(_float3));
	InputFile.read(reinterpret_cast<char*>(&_pData->vRotationRandomOffset), sizeof(_float3));

	InputFile.read(reinterpret_cast<char*>(&_pData->vScale), sizeof(_float3));
	InputFile.read(reinterpret_cast<char*>(&_pData->vScaleRandomOffset), sizeof(_float3));

	InputFile.read(reinterpret_cast<char*>(&_pData->vDir), sizeof(_float3));
	InputFile.read(reinterpret_cast<char*>(&_pData->vDirRandomOffset), sizeof(_float3));

	InputFile.read(reinterpret_cast<char*>(&_pData->fSpeed), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fSpeedRandomOffset), sizeof(_float));

	InputFile.read(reinterpret_cast<char*>(&_pData->fAlpha), sizeof(_float3));
	InputFile.read(reinterpret_cast<char*>(&_pData->fAlphaRandomOffset), sizeof(_float3));


	InputFile.read(reinterpret_cast<char*>(&_pData->vPivot), sizeof(_float3));


	InputFile.read(reinterpret_cast<char*>(&_pData->bIsLoop), sizeof(_bool));
	InputFile.read(reinterpret_cast<char*>(&_pData->bIsBillboard), sizeof(_bool));
	//InputFile.read(reinterpret_cast<char*>(&_pData->bIsColorRender), sizeof(_bool));
	InputFile.read(reinterpret_cast<char*>(&_pData->bIsBloom), sizeof(_bool));


	InputFile.read(reinterpret_cast<char*>(&_pData->iMoveCommandsNum), sizeof(_uint));

	_pData->vecMoveCommands.resize(_pData->iMoveCommandsNum);

	for (auto& KF : _pData->vecMoveCommands)
	{
		InputFile.read(reinterpret_cast<char*>(&KF), sizeof(_bool));
	}

	InputFile.read(reinterpret_cast<char*>(&_pData->eRenderGroup), sizeof(_uint));

	return S_OK;
}

HRESULT CUtils::Load_Effect(path _FilePath, MULTI_FX_DATA* _pData)
{
	ifstream InputFile(_FilePath, ios::binary | ios::in);

	if (!InputFile.is_open())
		ALARM_FAIL(TEXT("망했어"));


	//이펙트 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iNameStrLen), sizeof(_uint));
	_pData->strName.resize(_pData->iNameStrLen);
	InputFile.read(&_pData->strName[0], _pData->iNameStrLen);


	InputFile.read(reinterpret_cast<char*>(&_pData->iFXsNum), sizeof(_uint));
	_pData->FXs.resize(_pData->iFXsNum);

	for (auto& FX : _pData->FXs)
	{
		InputFile.read(reinterpret_cast<char*>(&FX.first), sizeof(_uint));
		FX.second.resize(FX.first);
		InputFile.read(&FX.second[0], FX.first);
	}

	return S_OK;
}


void CUtils::Make_Effect(SINGLE_FX_DATA& _FXData)
{
}

void CUtils::Make_Effect(MULTI_FX_DATA& _FXData)
{
}

void CUtils::Make_Effect(PARTICLE_DATA& _FXData)
{
}

