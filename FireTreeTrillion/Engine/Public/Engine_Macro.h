#pragma once

#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

// For Key_Manager
#define VK_MAX	0xff

#define GRAVITY 9.81f

//FOR COLLISION GROUP
#define	PASSING_GROUP 50


// assert로 확인하고자하는 코드 체크
// ※ DEBUG용으로 RELEASE모드에서 동작하는 소스엔 사용하지 마십시오.
#define CHECK_FAILED(p)					assert(SUCCEEDED(p))
#define CHECK_FAILED_MSG(p, message)	assert(SUCCEEDED(p) && message)
#define CHECK_NULLPTR(p)				assert(p != nullptr && "This pointer" #p "is a nullptr.")
#define ALARM_FAIL(message)				assert(0 && message)
#define Assert(expression)				assert(expression)

// switch-case문에서 default로 빠지면 안되는 경우 error처리
#ifdef DEBUG
# define NODEFAULT   assert(0)
#else
# define NODEFAULT   __assume(0)
#endif


//빠름 - 느림 ease
#define EASE_OUT(t) (_float)(1 - pow(1 - t, 3))
//빠름 - 느림 ease(빠름)
#define EASE_OUT_FAST(t) (_float)(1 - pow(1 - t, 6))

//느림 - 빠름 ease
#define EASE_IN(t)	(_float)pow(t, 4)
//느림 - 빠름 ease(빠름)
#define EASE_IN_FAST(t)	(_float)pow(t, 6)

//느림 - 빠름 - 느림 ease+
#define EASE_INOUT(t) (_float)(-(cos(3.14159f * t) - 1) / 2)
//느림 - 빠름 - 느림 ease(빠름)
#define EASE_INOUT_FAST(t) (_float)((t < 0.5) ? (4 * t * t * t) : 1 - pow(-2 * t + 2, 3) / 2)

#define EASE_IN_SINE(t) cos((t * 3.14159f) / 2)

#define EASE_OUT_CIRC(t) sqrt(1 - pow(t - 1, 2))

//일반 각도를 라디안 각도로 변환한다.
#define ToRadian(degree)			(XMConvertToRadians(degree))
//라디안 각도를 일반 각도로 변환한다.
#define ToDegree(radian)			(XMConvertToDegrees(radian))

//float3를 주면 w에 0이 박힌 float4로 반환한다.
#define Dir(vec3)					_float4{(vec3).x,( vec3).y, (vec3).z, 0.f}
//float3를 주면 w에 1이 박힌 float4로 반환한다.
#define Pos(vec3)					_float4{(vec3).x, (vec3).y, (vec3).z, 1.f}
//float3에서 y값을 0으로 만들고 반환한다.
#define XZVec(vec3)				_float3{vec3.x, 0.f, vec3.z}

//float4를 float3로 만듭니다.
#define F4toF3(vec4)				_float3{vec4.x, vec4.y, vec4.z}

//w가 1인 제로 벡터를 반환한다.
#define ZeroVecPos					_float4{0.f, 0.f, 0.f, 1.f}
//w가 0인 제로 벡터를 반환한다.
#define ZeroVecDir					_float4{0.f, 0.f, 0.f, 0.f}

#define GET_POS							m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION)
#define GET_ANOTHER_POS(transformCom)	(transformCom->Get_State_Vector(CTransform::STATE_POSITION))

#define SET_POS(pos)		m_pTransformCom->Set_State(CTransform::STATE_POSITION, pos)

//float ratio를 기준으로, start 값과 dest 값 사이의 보간 값을 제공한다. 
#define LERP(start, dest, curRatio) _float(start + curRatio * (dest - start))
//어떤 범위 안의 값을 다른 범위로 매핑한다.
#define MAPVALUE(floatValue,  minfloat1,  maxfloat1,  minfloat2,  maxfloat2) (minfloat2 + (floatValue - minfloat1) * (maxfloat2 - minfloat2) / (maxfloat1 - minfloat1))
//어떤 범위 안의 값을 0에서 1 사이의 값으로 매핑한다.
#define SATURATE(floatValue) (clamp(floatValue, 0.f, 1.f))

//해당 float 값이 -1인지를 판단한다.
#define ISDEFAULTFLOAT(floatValue) _bool(floatValue == -1.f)
#define ISDEFAULTFLOAT3(float3Value) _bool(float3Value.x == -1.f && float3Value.y == -1.f && float3Value.z == -1.f)



#define BEGIN(NAMESPACE)	namespace NAMESPACE {
#define END					}

#ifdef ENGINE_EXPORTS
#define ENGINE_DLL _declspec(dllexport)
#else
#define ENGINE_DLL _declspec(dllimport)
#endif

#define MSG_BOX(message)										\
::MessageBox(nullptr, message, L"error", MB_OK)

#define MB(text, caption)										\
::MessageBox(nullptr, text, caption, MB_OK)					

#define NO_COPY(ClassName)										\
ClassName(const ClassName&) = delete;							\
ClassName& operator=(const ClassName&) = delete;

#define DECLARE_SINGLETON(ClassName)							\
		NO_COPY(ClassName)										\
public :														\
	static ClassName* Get_Instance();							\
	static unsigned int Destroy_Instance();						\
private:														\
	static ClassName* m_pInstance;

#define IMPLEMENT_SINGLETON(ClassName)							\
ClassName* ClassName::m_pInstance = nullptr;					\
ClassName* ClassName::Get_Instance()							\
{																\
	if (nullptr == m_pInstance)									\
		m_pInstance = new ClassName;							\
	return m_pInstance;											\
}																\
unsigned int  ClassName::Destroy_Instance()						\
{																\
	unsigned int	iRefCnt = 0;								\
	if(nullptr != m_pInstance)									\
	{															\
		iRefCnt = m_pInstance->Release();						\
		if(0 == iRefCnt)										\
			m_pInstance = nullptr;								\
	}															\
	return iRefCnt;												\
}

