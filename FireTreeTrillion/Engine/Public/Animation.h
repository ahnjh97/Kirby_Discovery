#pragma once
#include "Base.h"

/* 하나의 애니메이션이 몇개의 뼈를 컨트롤해야하는지. 그 뼈들은 무엇인지.  */
/* 이 애니메이션을 재생하는데 걸리는 총 거리.. */
/* 이 애니메이션을 구동하는 속도. */

BEGIN(Engine)

class CAnimation final : public CBase
{
private:
	CAnimation();
	CAnimation(const CAnimation& rhs);
	virtual ~CAnimation() = default;

public:
	_bool IsFinished() const {	return m_IsFinished; }

	void Reset_Finished(_bool isFinished = false) {	m_IsFinished = isFinished; }

	void Reset_TrackPosition() {
		m_fTrackPosition = 0.f;
		m_preTrackPosition = 0.f;
		for (_uint i = 0; i < m_iNumChannels; ++i)
		{
			m_CurrentKeyFrameIndices[i] = 0;
		}
	}

	void Reset_Ratio() { m_bRatio = true; }
	void Remove_Ratio() { m_bRatio = false; }

	_float Get_Duration() {	return m_fDuration; }
	_float Get_TrackPosition() { return m_fTrackPosition; }
	_float Get_AnimRatio() { return m_fTrackPosition / m_fDuration;	}
	_float Get_AnimTrackPosition() { return m_fTrackPosition; }

public:
	_float		 Get_TickPerSecond() const { return m_fTickPerSecond; }
	void		 Set_TickPerSecond(_float _fTickPerSecond) { m_fTickPerSecond = _fTickPerSecond; }
				 
	_float		 Get_TrackPosition() const { return m_fTrackPosition; }
	void		 Set_TrackPosition(_float _fTrackPosition) { m_fTrackPosition = _fTrackPosition; }

	const _char* Get_AnimationName() const { return m_szName; }
	_float		 Get_Duration() const { return m_fDuration; }

	void		Set_LerpTime(_float fLerpTime) { m_fLerpTime = fLerpTime; }
	void		Reset_RatioTime() { m_fRatioTime = 0; }

	// 애님툴에서 작업한 데이터 가져오기
	void		 Set_AnimEventData(ANIM_INFO tAnimInfo);

public:
	HRESULT		Initialize(const vector<class CBone*>& Bones, ifstream& fileStream);
	void		Invalidate_TransformationMatrix(_float fTimeDelta, const vector<class CBone*>& Bones, _bool bIsLooping, class CModel* model);

	void		Read_AnimationData(ifstream& fileStream);

private:
	_char								m_szName[MAX_PATH] = { "" };
	
	_float								m_fDuration = { 0.0f };		  /* 전체 재생 길이. */
	_float								m_fTickPerSecond = { 0.0f };  /* 초당 얼마나 재생을 해야하는가 (속도) */
	_float								m_fTrackPosition = { 0.0f };  /* 현재 애니메이션이 어디까지 재생되었는지"? */
	_float								m_preTrackPosition = { 0.f }; // 애니메이션이 이전 틱에서 지나간 키프레임

	_uint								m_iNumChannels = { 0 };
	vector<class CChannel*>				m_Channels;
	vector<_uint>						m_CurrentKeyFrameIndices;

	_bool								m_IsFinished = { false };	
	_float								m_fRatioTime = { 0.f };
	_bool								m_bRatio = { false };
	_float								m_fLerpTime = { 0.1f };

	vector<EVENT_INFO>					m_vecEventInfo;
	
public:
	static CAnimation*	Create(const vector<class CBone*>& Bones, ifstream& fileStream);
	CAnimation*			Clone();
	virtual void		Free() override;

};

END