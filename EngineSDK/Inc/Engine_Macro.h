#pragma once

#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

// For Key_Manager
#define VK_MAX	0xff

// assert로 확인하고자하는 코드 체크
// ※ DEBUG용으로 RELEASE모드에서 동작하는 소스엔 사용하지 마십시오.
#define CHECK_FAILED(p)					assert(SUCCEEDED(p))
#define CHECK_FAILED_MSG(p, message)	assert(SUCCEEDED(p && message))
#define CHECK_NULLPTR(p)				assert(p != nullptr && "This pointer" #p "is a nullptr.")
#define ALARM_FAIL(message)				assert(0 && message)


#define BEGIN(NAMESPACE)	namespace NAMESPACE {
#define END					}

#ifdef ENGINE_EXPORTS
#define ENGINE_DLL _declspec(dllexport)
#else
#define ENGINE_DLL _declspec(dllimport)
#endif

#define MSG_BOX(message)										\
::MessageBox(nullptr, message, L"error", MB_OK)

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

