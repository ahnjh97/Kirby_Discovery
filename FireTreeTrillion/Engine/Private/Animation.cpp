#include "Animation.h"
#include "Channel.h"
#include "Bone.h"
#include "Model.h"
CAnimation::CAnimation()
{

}

CAnimation::CAnimation(const CAnimation & rhs)
	: m_fDuration{ rhs.m_fDuration }
	, m_fTickPerSecond{ rhs.m_fTickPerSecond }
	, m_fTrackPosition{ rhs.m_fTrackPosition }
	, m_iNumChannels{ rhs.m_iNumChannels }
	, m_Channels{ rhs.m_Channels }
	, m_CurrentKeyFrameIndices{ rhs.m_CurrentKeyFrameIndices }
	, m_IsFinished{ rhs.m_IsFinished }
	, m_vecEventInfo{rhs.m_vecEventInfo }

{
	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);

	strcpy_s(m_szName, rhs.m_szName);
}

void CAnimation::Set_AnimEventData(ANIM_INFO tAnimInfo)
{
	m_fTickPerSecond = tAnimInfo.fAnimSpeed;
	m_vecEventInfo = tAnimInfo.vecEventInfo;
}

HRESULT CAnimation::Initialize(const vector<class CBone*>& Bones, ifstream& fileStream)
{
	Read_AnimationData(fileStream);
	m_CurrentKeyFrameIndices.resize(m_iNumChannels);

	for (_uint i = 0; i < m_iNumChannels; i++)
	{
		CChannel* pChannel = CChannel::Create(Bones, fileStream);
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.emplace_back(pChannel);
	}

	return S_OK;
}

void CAnimation::Invalidate_TransformationMatrix(_float fTimeDelta, const vector<CBone*>& Bones, _bool bIsLooping, CModel* model)
{
	// 선형보간 부분
	if (m_bRatio)
	{
		// 재생바를 계속 증가시킨다.
		m_fRatioTime += fTimeDelta;

		for (_uint i = 0; i < m_iNumChannels; ++i)
		{
			//Channel의 뼈 
			m_Channels[i]->Ratio_TransformationMatrix(Bones, m_fTrackPosition, &m_CurrentKeyFrameIndices[i]);
		}

		m_fTrackPosition += fTimeDelta;

		if (m_fRatioTime > 0.1f)
		{
			Reset_TrackPosition();
			m_fRatioTime = 0.f;
			m_bRatio = false;
		}
		return;
	}


	m_IsFinished = false;

	_float preTrackPosition = m_fTrackPosition;
	m_fTrackPosition += m_fTickPerSecond * fTimeDelta;

	if (!m_vecEventInfo.empty())
		int a = 3;

	// 해당 애니메이션에서, 프레임 사이에 있는 이벤트 이름을 알아온다.
	// 그 이벤트 이름에 넣어준 함수를 실행
	for (auto& eventInfo : m_vecEventInfo)
	{
		if ((_int)m_fTrackPosition == eventInfo.iStartFrame)
		{
			model->CallEvent(eventInfo.strEventName);
		}
	}

	if (m_fDuration <= m_fTrackPosition)
	{
		if (false == bIsLooping)
		{
			m_IsFinished = true;			
			return;
		}			

		m_fTrackPosition = 0.f;
	}

	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		/* 이 뼈의 상태행렬을 만들어서 CBone의 TransformationMatrix를 바꿔라. */
		m_Channels[i]->Invalidate_TransformationMatrix(Bones, m_fTrackPosition, &m_CurrentKeyFrameIndices[i]);
	}
}

void CAnimation::Read_AnimationData(ifstream& fileStream)
{
	fileStream.read(reinterpret_cast<char*>(&m_szName), sizeof(m_szName));
	string strName = m_szName;
	size_t pos = strName.find('|');
	if (pos != string::npos)
	{
		strName = strName.substr(pos + 1);
		strcpy_s(m_szName, strName.c_str());
	}

	fileStream.read(reinterpret_cast<char*>(&m_fDuration), sizeof(m_fDuration));
	fileStream.read(reinterpret_cast<char*>(&m_fTickPerSecond), sizeof(m_fTickPerSecond));
	fileStream.read(reinterpret_cast<char*>(&m_iNumChannels), sizeof(m_iNumChannels));
}

CAnimation* CAnimation::Create(const vector<class CBone*>& Bones, ifstream& fileStream)
{
	CAnimation* pInstance = new CAnimation();

	if (FAILED(pInstance->Initialize(Bones, fileStream)))
	{
		MSG_BOX(TEXT("Failed To Create : CAnimation"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimation * CAnimation::Clone()
{
	return new CAnimation(*this);
}

void CAnimation::Free()
{
	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);

	m_Channels.clear();
}

