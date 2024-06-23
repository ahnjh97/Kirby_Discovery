#pragma once
#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL CUtils
{
public:
	enum STATE { STATE_RIGHT, STATE_UP, STATE_LOOK, STATE_POSITION, STATE_END };

	static wstring  StrToWstr(const string& value);
	static wstring  StrToWstrUTF8(const string& value);
	static string   WstrToStr(const wstring& value);
	static string   WstrToStrUTF8(const wstring& value);
	static string	StrToUTF8(const string& value);

	static void		WCharToChar(const wchar_t* szWchar, char* szChar);

	static _int		Make_RandomInt(_int min, _int max);
	static _float	Make_RandomFloat(_float min, _float max);

	// 벡터의 길이만 지정해주면, 랜덤한 방향의 백터를 리턴한다.
	static _vector	Make_Random_Vector(_float fVectorLength);

	// 벡터의 방향과 랜덤 범위각도를 지정해주면, 각도 내에서 랜덤한 벡터를 리턴한다.
	static _vector	Make_RandomAngle_Vector(_float fDirAngle, _fvector vDir);
	// 벡터의 방향과 랜덤 범위각도를 지정하고, 벡터 길이의 최소값, 최대값을 입력하면 랜덤한 벡터가 나온다.
	static _vector	Make_RandomAngle_Vector(_float fDirAngle, _fvector vDir, _float fminlength, _float fmaxlength);


	//회전 관련 함수
	
	// Matrix 던져 회전 상태를 쿼터니언으로 받는다.
	static Quaternion Make_Quat_FromMatrix(const _float4x4& _mat);
	// Dir 던져 회전 상태를 쿼터니언으로 받는다.
	static Quaternion Make_Quat_FromDir(const _float4& _dir);
	static Quaternion Make_Quat_FromDir(const _float3& _dir);

	static _float3 Make_Degree_FromDir(const _float4& _dir);
	static _float3 Make_Degree_FromDir(const _float3& _dir);


	//static _float2 

	// Matrix 계산기
	static void		Set_State_Matrix(_Inout_ _float4x4& matrix, STATE eState, _fvector vState);
	static void		Set_State_Matrix(_Inout_ _float4x4& matrix, STATE eState, const _float4& vState);
	static void		Set_Scaled_Matrix(_Inout_ _float4x4& matrix, _float fScaleX, _float fScaleY, _float fScaleZ);
	static _vector	Get_State_Vector_Matrix(_Inout_ _float4x4& matrix, STATE eState);
	static _float3	Get_Scaled_Matrix(_Inout_ _float4x4& matrix);
	static void		Turn_OtherMatrix(_Inout_ _float4x4& matrix, _fvector vAxis, _float fTimeDelta, _float fAngle);
	static void		Rotation(_Inout_ _float4x4& matrix, _fvector vAxis, _float fRadian);

	//공간 이동
	static void		Make_World_ToScreen(_Inout_ _float3& vPos);
	static void		Make_Screen_ToWorld(_Inout_ _float3& vPos);


	// physX 관련 함수
	static PxMat44			To_Float4x4(const _float4x4& mat);
	static _float4x4		To_Float4x4(const PxMat44& mat);
	static PxVec3			To_PxVec3(const _fvector& vector);
	static _vector			To_Vector(const PxVec3 pxVec3);
	static PxTransform		mat44ToTransform(const PxMat44& mat);
		
	//이펙트 관련 함수
	static HRESULT Load_Effect(path _FilePath, _Out_ SINGLE_FX_DATA* _pData);
	static HRESULT Load_Effect(path _FilePath, _Out_ PARTICLE_DATA* _pData);
	static HRESULT Load_Effect(path _FilePath, _Out_ MULTI_FX_DATA* _pData);
	static void Make_Effect(SINGLE_FX_DATA& _FXData);
	static void Make_Effect(MULTI_FX_DATA& _FXData);
	static void Make_Effect(PARTICLE_DATA& _FXData);


};

template <typename T, std::size_t N>
constexpr std::size_t getArraySize(T(&)[N]) noexcept
{
	return N;
}
END