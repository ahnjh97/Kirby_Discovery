#pragma once
#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CEffect : public CGameObject
{
public:
	typedef struct : public GAMEOBJECT_DESC
	{
		//이펙트 재생을 시작하는 딜레이
		_float fStartDelay = { 0.f };
		
		//루프하는가?
		_bool bIsLoop = { false };
		//빌보딩하는가?
		_bool bIsBillboard = { false };
		//직교하는가?
		_bool bIsOrthographic = { false };
		_bool bIsNonLight = { false };
		_bool bIsBloom = { false };

		//이펙트의 기본 시작 크자이
		_float3 vInitPos = { 0.f, 0.f, 0.f };
		_float3 vInitRot = { 0.f, 0.f, 0.f };
		_float3 vInitScale = { 1.f, 1.f, 1.f };

		//텍스쳐가 흑백 or RGB로 이루어져 있는 경우, 설정할 색상 값
		
		//흑백일 경우 흰색, RGB일 경우 R 색상에 해당
		_float3 vRColor = { 0.f, 0.f, 0.f };
		_float3 vGColor = { 0.f, 0.f, 0.f };
		_float3 vBColor = { 0.f, 0.f, 0.f };

		//이펙트 이름
		string strFXName = { "NONE" };

		//필요한 버퍼, 텍스쳐, 마스크 텍스쳐 이름
		string strBufferTag = { "NONE" };
		string strTexTag = { "NONE" };
		string strMaskTexTag = { "NONE" };

		//쉐이더 패스 인덱스, 텍스쳐 인덱스, 마스크 텍스쳐 인덱스
		_int iPassIdx = { 0 };
		_int iTexIdx = { 0 };
		_int iMaskTexIdx = { 0 };


		map<_uint, vector<FX_KEYFRAME>> Keyframes;

	}FX_DESC;

protected:
	CEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffect(const CEffect& rhs);
	virtual ~CEffect() = default;

public:
	_float Get_TotalDuration(_float fDuration)
	{
		m_fDuration.second = (m_fDuration.second < fDuration) ? fDuration : m_fDuration.second;
		return m_fDuration.second;
	}

	virtual _int Get_Size() { return 1; }


	virtual void Reset_Duration() { m_fDuration.first = 0.f; }
	virtual void Fill_SaveData(_Out_ FX_DATA* pEffectData);


	//키프레임 추가(단일 이펙트 용)
	void			Add_Keyframe(FX_KEYFRAME& newKeyframe);

	//이펙트 추가(묶음 이펙트 용)
	virtual void	Add_Effect(CEffect& pEffect) {}

	virtual HRESULT Initialize(void* pArg) override;

protected:

	/*이펙트 시작 세팅*/

	//이펙트 재생을 시작하는 딜레이
	_float			m_fStartDelay = { 0.f };

	//필요한 버퍼, 텍스쳐, 마스크 텍스쳐 이름
	wstring			m_strFXName = { TEXT("NONE") };
	wstring			m_strBufferTag = { TEXT("NONE") };
	wstring			m_strTexTag = { TEXT("NONE") };
	wstring			m_strMaskTexTag = { TEXT("NONE") };

	//렌더 여부
	_bool			m_bRender = { false };

	//패스, 텍스쳐, 마스크 텍스쳐 인덱스
	_uint			m_iPassIdx = { 0 };
	_uint			m_iTexIdx = { 0 };
	_uint			m_iMaskTexIdx = { 0 };

	//재생 시 속성
	_bool			m_bIsLoop = { false };
	_bool			m_bIsBillboard = { false };
	_bool			m_bIsOrthographic = { false };
	_bool			m_bIsNonLight = { true };
	_bool			m_bIsBloom = { false };

	//기본 시작 크자이
	_float3 m_vInitPos = { 0.f, 0.f, 0.f };
	_float3 m_vInitRot = { 0.f, 0.f, 0.f };
	_float3 m_vInitScale = { 1.f, 1.f, 1.f };

	/*Tick 돌리며 계속 값 바뀌는 부분*/

	//이펙트의 지속 시간.(first: 현재 duration, second: 목표 duration)
	pair<_float, _float>	m_fDuration = { 0.f, 1.f };

	//수명 지속 시간. duration이 사라져야 이펙트 사라짐. 
	pair<_float, _float>	m_fLifeTime = { 0.f, 1.f };


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


protected:
	_bool			Update_Duration(_float fTimeDelta);
	_bool			Update_LifeTime(_float fTimeDelta);
	void			Calculate_CurValue(_float fTimeDelta, _bool bIsInEditor = false);

public:
	virtual void Free() override;

};

END