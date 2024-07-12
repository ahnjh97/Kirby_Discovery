#include "Animation.h"

#include "GameInstance.h"

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

vector<_uint> CAnimation::Get_ValidBoneIndices()
{
	vector<_uint> vecValidChannelIndices;
	for (auto& channel : m_Channels)
		if (true == channel->IsValid())
			vecValidChannelIndices.push_back(channel->Get_ChannelBoneIndex());

	return vecValidChannelIndices;
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

		for (_uint i = 0; i < m_iNumChannels; ++i)  //Channel의 뼈 
			m_Channels[i]->Ratio_TransformationMatrix(Bones, m_fTrackPosition, &m_CurrentKeyFrameIndices[i]);

		if (m_fRatioTime > m_fLerpTime)
		{
			Reset_TrackPosition();
			m_fRatioTime = 0.f;
			m_bRatio = false;
		}

		m_fTrackPosition += fTimeDelta;

		return;
	}

	m_IsFinished = false;
	m_fTrackPosition += m_fTickPerSecond * fTimeDelta;

	// 해당 애니메이션에서, 프레임 사이에 있는 이벤트 이름을 알아온다.
	// 그 이벤트 이름에 넣어준 함수를 실행
	if (*CGameInstance::Get_Instance()->Get_CurrentLevelID() != 6)
	{
		for (auto& eventInfo : m_vecEventInfo)
		{
			if (m_preTrackPosition < eventInfo.iStartFrame &&
				eventInfo.iStartFrame <= m_fTrackPosition)
			{
				model->CallEvent(eventInfo.strEventName);
			}
		}
		m_preTrackPosition = m_fTrackPosition;
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

	/* 이 뼈의 상태행렬을 만들어서 CBone의 TransformationMatrix를 바꿔라. */
	for (_uint i = 0; i < m_iNumChannels; ++i) 
		m_Channels[i]->Invalidate_TransformationMatrix(Bones, m_fTrackPosition, &m_CurrentKeyFrameIndices[i]);
}

void CAnimation::Lerp_TransformMatrix(_float fTimeDelta, const vector<class CBone*>& Bones, _float& fPartialAnimLerpTime, CModel* pModel, unordered_set<_uint>& _setValidBones)
{
	// 재생바를 계속 증가시킨다.
	fPartialAnimLerpTime += fTimeDelta;

	for (_uint i = 0; i < m_iNumChannels; ++i) { //Channel의 뼈

		_uint iChannelBoneIdx = m_Channels[i]->Get_ChannelBoneIndex();
		if(_setValidBones.end() != _setValidBones.find(iChannelBoneIdx))
			m_Channels[i]->Ratio_TransformationMatrix(Bones, m_fTrackPosition, &m_CurrentKeyFrameIndices[i]);
	}

	if (fPartialAnimLerpTime > m_fLerpTime)
	{
		fPartialAnimLerpTime = 0.f;
		pModel->Set_LerpPartialAnim(false);
	}
}

void CAnimation::Read_AnimationData(ifstream& fileStream)
{
	//fileStream.read(reinterpret_cast<char*>(&m_szName), sizeof(m_szName));
	//string strName = m_szName;
	//size_t pos = strName.find('|');
	//if (pos != string::npos)
	//{
	//	strName = strName.substr(pos + 1);
	//	strcpy_s(m_szName, strName.c_str());
	//}
	
	fileStream.read(reinterpret_cast<char*>(m_szName), sizeof(m_szName));
	m_szName[sizeof(m_szName) - 1] = '\0';
	string strName = m_szName;
	size_t pos = strName.find_last_of('|');
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

