#pragma once
#include "GameObject.h"
#include "Renderer.h"

BEGIN(Engine)

#define FX_MAXDURATION 99.f

class ENGINE_DLL CEffect : public CGameObject
{
#ifdef _DEBUG
	friend class CFXToolDirector;
#endif

public:
	enum TEX {TEX_DIFFUSE, TEX_MASK, TEX_END};

	typedef struct : public GAMEOBJECT_DESC
	{

		//이펙트 이름
		string strFXName = { "NONE" };

		//필요한 버퍼, 텍스쳐, 마스크 텍스쳐 이름
		string strBufferTag = { "NONE" };
		string strTexTag = { "NONE" };
		string strMaskTexTag = { "NONE" };

		_float					fDuration = { 0.f };
		pair<_float, _float>	fLifetime = { 0.f, 1.f };

		//쉐이더 패스 인덱스, 텍스쳐 인덱스, 마스크 텍스쳐 인덱스
		_int iPassIdx = { 0 };
		_int iTexIdx = { 0 };
		_int iMaskTexIdx = { 0 };

		//루프하는가?
		_bool bIsLoop = { false };
		//빌보딩하는가?
		_bool bIsBillboard = { false };
		//직교하는가?
		_bool bIsOrthographic = { false };
		_bool bIsColorRender = { false };
		_bool bIsBloom = { false };

		_float		fRimLightThreshold = { 0.f };
		_float3		vContinuousRotation = { 0.f, 0.f, 0.f };

		map<KF_PROPERTY, vector<FX_KEYFRAME>> Keyframes;


		//clone 시 전달되는 변수들. clone할 때만 전달할 것!!
		
		//위치를 계속 맞춰주는 소켓. 
		const _float4x4* pSocketMatrix = { nullptr };

		//이펙트 재생을 시작하는 딜레이
		_float fStartDelay = { 0.f };

		//이펙트의 기본 시작 크자이 offset
		_float3 vInitPos = { 0.f, 0.f, 0.f };
		_float3 vInitRot = { 0.f, 0.f, 0.f };
		_float3 vInitScale = { 1.f, 1.f, 1.f };

		_uint eRenderGroup = { 0 };
		TIMER eTimer = { TIMER_END };

		_bool bIsSingleFX = { false };
	}FX_DESC;

protected:
	CEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffect(const CEffect& rhs);
	virtual ~CEffect() = default;

public:
	_float Get_BiggerDuration(_float fDuration)
	{
		m_fDuration.second = (m_fDuration.second < fDuration) ? fDuration : m_fDuration.second;
		return m_fDuration.second;
	}

	virtual _bool	IsEnded() { return m_bDone; }
	void			Set_Multi(_bool bSingle = false) { m_bSingle = bSingle; }

	virtual _int	Get_Size() { return 1; }
	string			Get_Name() { return m_strFXName; }

	virtual void	Reset_Duration();
	virtual void	Fill_SaveData(_Out_ SINGLE_FX_DATA* pFXData);
	virtual void	Fill_SaveData(PARTICLE_DATA* pFXData) {}
	virtual void	Fill_SaveData(MULTI_FX_DATA* pFXData) {}

	virtual void	Set_NoRender() { m_bRender = false; }

	//키프레임 추가(단일 이펙트 용)
	void			Add_Keyframe(FX_KEYFRAME& newKeyframe, KF_PROPERTY eProperty);
	void			Delete_Keyframe(KF_PROPERTY eProperty, _uint iKeyframeIdx);
	//이펙트 추가(묶음 이펙트 용)
	virtual void	Add_Effect(CEffect* pEffect) {}
	virtual void	Add_RenderGroup();
	virtual HRESULT Initialize(void* pArg) override;

protected:
	//재생 시간이 다 되었는가?
	_bool			m_bDone = { false };
	//단독 이펙트인가?
	_bool			m_bSingle = { true };
	/*이펙트 시작 세팅*/

	//이펙트 재생을 시작하는 딜레이
	_float			m_fStartDelay = { 0.f };

	//필요한 버퍼, 텍스쳐, 마스크 텍스쳐 이름
	string			m_strFXName = { "NONE" };
	wstring			m_strBufferTag = { TEXT("NONE") };
	wstring			m_strTexTag = { TEXT("NONE") };
	wstring			m_strMaskTexTag = { TEXT("NONE") };

	//렌더 여부
	_bool			m_bRender = { false };

	//패스, 텍스쳐, 마스크 텍스쳐 인덱스
	//최대 인덱스를 한번 제한해 주는데, 이후 고치기

	_int			m_iPassIdx = { 0 };
	_int			m_iMaxPassIdx = { 0 };

	_int			m_iTexIdx = { 0 };
	_int			m_iMaxTexIdx = { 1 };

	_int			m_iMaskTexIdx = { 0 };
	_int			m_iMaxMaskTexIdx = { 1 };

	//재생 시 속성
	_bool			m_bIsLoop = { false };
	_bool			m_bIsBillboard = { false };
	_bool			m_bIsOrthographic = { false };
	_bool			m_bIsColorRender = { true };
	_bool			m_bIsBloom = { false };

	_uint			m_eRenderGroup = { (_uint)CRenderer::RENDER_NONBLEND };
	TIMER			m_eTimer = { TIMER_NONE };

	_float			m_fRimLightThreshold = { 0.f };

	//지속하여 회전을 시켜주는 속도
	_float3			m_vContinuousRotation = { 0.f, 0.f, 0.f };

	//기본 시작 크자이
	_float3 m_vInitPos = { 0.f, 0.f, 0.f };
	_float3 m_vInitRot = { 0.f, 0.f, 0.f };
	_float3 m_vInitScale = { 1.f, 1.f, 1.f };

	//특정 대상 중심에 붙어있어야 할 경우 계속 따라가며 유지되는 월드 상태
	const _float4x4* m_pSoketMatrix = {nullptr};

	/*Tick 돌리며 계속 값 바뀌는 부분*/

	//이펙트의 지속 시간.(first: 현재 duration, second: 목표 duration)
	pair<_float, _float>	m_fDuration = { 0.f, 1.f };

	//수명 지속 시간. duration이 사라져야 이펙트 사라짐
	//루프하는 경우 life time 처음부터 다시 시작한다. 
	pair<_float, _float>	m_fLifetime = { 0.f, 1.f };
	_float					m_fLifeRatio = { 0.f };

	//보간 키프레임
	map<KF_PROPERTY, vector<FX_KEYFRAME>>	m_Keyframes;
	_uint* m_iCurKeyframeIdxs = { nullptr };
	//float3 키프레임
	
	//크자이
	_float3			m_vCurPos = {};
	_float3			m_vCurRot = {};
	_float3			m_vCurScale = {};

	//색상
	_float3			m_vCurRColor = { 0.f, 0.f, 0.f };
	_float3			m_vCurGColor = { 0.f, 0.f, 0.f };
	_float3			m_vCurBColor = { 0.f, 0.f, 0.f };

	//float1 키프레임
	
	//알파
	_float			m_fCurAlpha = { 1.f };
	//마스크 임계
	_float			m_fCurMaskThreshold = { 0.f };

	//uv 오프셋들
	_float2			m_vCurUVOffset = { 0.f, 0.f };
	_float2			m_vCurMaskUVOffset = { 0.f, 0.f };
	_float			m_vCurMaskUVAngle = { 0.f };

	_bool			m_bIsSingleFX = { false };
protected:
	_bool			Update_Duration(_float fTimeDelta);
	_bool			Update_LifeTime(_float fTimeDelta);

	_bool			Calculate_Duration(_float _fTimeDelta);
	_bool			Calculate_Lifetime(_float _fTimeDelta);
	_float3			Calculate_CurValue_Lerp(_float fTimeDelta, KF_PROPERTY eProperty, _bool bIsInEditor = false);
	_float4			Calculate_CurValue_Slerp(_float fTimeDelta,  KF_PROPERTY eProperty, _bool bIsInEditor = false);
	void			Billboard_Effect();

public:
	virtual void Free() override;

};

END