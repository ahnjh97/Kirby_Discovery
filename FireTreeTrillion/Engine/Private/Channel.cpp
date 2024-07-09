#include "Channel.h"
#include "Bone.h"

CChannel::CChannel()
{
}

HRESULT CChannel::Initialize(const vector<class CBone*>& Bones, ifstream& fileStream)
{
	fileStream.read(reinterpret_cast<char*>(&m_szName), sizeof(m_szName));
	fileStream.read(reinterpret_cast<char*>(&m_iNumKeyFrames), sizeof(m_iNumKeyFrames));

	auto	iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool
		{
			++m_iBoneIndex;
			return pBone->Compare_Name(m_szName);
		});

	m_KeyFrames.reserve(m_iNumKeyFrames);

	for (_uint i = 0; i < m_iNumKeyFrames; i++)
	{
		KEYFRAME tempKeyFrame = {};
		fileStream.read(reinterpret_cast<char*>(&tempKeyFrame), sizeof(tempKeyFrame));
		m_KeyFrames.push_back(tempKeyFrame);
	}

	return S_OK;
}

void CChannel::Invalidate_TransformationMatrix(const vector<class CBone*>& Bones, _float fTrackPosition, _uint* pCurrentKeyFrameIndex)
{
	// 재생 바가 0.0f면, 초기상태이므로 현재 프레임을 완전 초기상태로 돌려놓는다.
	if (0.0f == fTrackPosition)
		(*pCurrentKeyFrameIndex) = 0;

	// 밖에서 가장 마지막 프레임의 정보를 받아둔다.
	KEYFRAME		KeyFrame = m_KeyFrames.back();

	_float3			vScale;
	_float4			vRotation;
	_float3			vTranslation;

	// 만약, 매개변수로 받은 재생바가 가장 마지막의 프레임의 정보의 Time보다 넘어갔을 경우, 마지막 정보로 유지한다.
	if (KeyFrame.fTime <= fTrackPosition)
	{
		vScale = KeyFrame.vScale;
		vRotation = KeyFrame.vRotation;
		vTranslation = KeyFrame.vTranslation;
	}
	// 현재 재생바의 위치에 따라 모든 움직임을 선형보간한다.
	else
	{
		// 만약 재생 바가 다음 프레임을 넘어가버렸을 경우 현재 프레임을 증가시킨다. 
		// 프레임드랍이 발생했을 경우 2칸넘어가도 while문에 의해 한번 더 발동된다.
		while (fTrackPosition >= m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].fTime)
			++(*pCurrentKeyFrameIndex);

		// 선형보간 비율이다. (재생바 - 현재 프레임) / (다음프레임 - 현재프레임) 이면 전 프레임과 다음프레임 사이의 비율이 나올 것이다.
		_float		fRatio = (fTrackPosition - m_KeyFrames[(*pCurrentKeyFrameIndex)].fTime)
			/ (m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].fTime - m_KeyFrames[(*pCurrentKeyFrameIndex)].fTime);

		// 위에서 구한 비율로 선형보간을 진행하기 위해 XMVectorLerp 함수를 사용한다. 현 프레임과 다음 프레임 사이를 선형보간한다.
		XMStoreFloat3(&vScale, XMVectorLerp(XMLoadFloat3(&m_KeyFrames[(*pCurrentKeyFrameIndex)].vScale), XMLoadFloat3(&m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].vScale), fRatio));
		XMStoreFloat4(&vRotation, XMQuaternionSlerp(XMLoadFloat4(&m_KeyFrames[(*pCurrentKeyFrameIndex)].vRotation), XMLoadFloat4(&m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].vRotation), fRatio));
		XMStoreFloat3(&vTranslation, XMVectorLerp(XMLoadFloat3(&m_KeyFrames[(*pCurrentKeyFrameIndex)].vTranslation), XMLoadFloat3(&m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].vTranslation), fRatio));
	}

	// matrix를 선언하고, XMMatrixAffineTransformation를 사용하여 행렬을 만들어준다. 아핀 변환 매트릭스를 생성하는데,
	// 아핀 변환은 변환이 평행이동, 회전, 크기조절로 이루어진 선형 변환인 경우를 나타낸다.
	_matrix		TransformationMatrix = XMMatrixAffineTransformation(
		// 1인자 : 크기조절
		XMLoadFloat3(&vScale),
		// 2인자 : 회전 중심 지점 벡터. Channel 의 중심을 0.f, 0.f, 0.f 으로 잡아주어야 뒤틀리지 않는다.
		XMVectorSet(0.f, 0.f, 0.f, 1.f),
		// 3인자 : 회전을 나타내는 쿼터니언
		XMLoadFloat4(&vRotation),
		// 4인자 : 평행 이동 벡터
		XMVectorSetW(XMLoadFloat3(&vTranslation), 1.f));


	Bones[m_iBoneIndex]->Set_TransformationMatrix(TransformationMatrix);
}

void CChannel::Ratio_TransformationMatrix(const vector<class CBone*>& Bones, _float fTrackPosition, _uint* pCurrentKeyFrameIndex)
{

	if (fTrackPosition == 0.f)
	{
		// 현재 상태를 그대로 받아옴.
		_float4x4 RatioTransform = Bones[m_iBoneIndex]->Get_TransformationMatrix();

		_vector vScale;
		_vector vRotation;
		_vector vTranslation;

		XMMatrixDecompose(&vScale, &vRotation, &vTranslation, XMLoadFloat4x4(&RatioTransform));

		// 그래서 각 Channel의 멤버변수에 박아넣는다.
		XMStoreFloat3(&m_vScale, vScale);
		XMStoreFloat4(&m_vRotation, vRotation);
		XMStoreFloat3(&m_vTranslation, vTranslation);

	}

	// 선형보간 비율이다. (재생바 - 현재 프레임) / (다음프레임 - 현재프레임) 이면 전 프레임과 다음프레임 사이의 비율이 나올 것이다.
	_float		fRatio = fTrackPosition / 0.1f;

	_float3 vScale;
	_float4 vRotation;
	_float3 vTranslation;

	// 위에서 구한 비율로 선형보간을 진행하기 위해 XMVectorLerp 함수를 사용한다. 현 프레임과 다음 프레임 사이를 선형보간한다.
	XMStoreFloat3(&vScale, XMVectorLerp(XMLoadFloat3(&m_vScale), XMLoadFloat3(&m_KeyFrames[0].vScale), fRatio));
	XMStoreFloat4(&vRotation, XMQuaternionSlerp(XMLoadFloat4(&m_vRotation), XMLoadFloat4(&m_KeyFrames[0].vRotation), fRatio));
	XMStoreFloat3(&vTranslation, XMVectorLerp(XMLoadFloat3(&m_vTranslation), XMLoadFloat3(&m_KeyFrames[0].vTranslation), fRatio));

	// matrix를 선언하고, XMMatrixAffineTransformation를 사용하여 행렬을 만들어준다. 아핀 변환 매트릭스를 생성하는데,
	// 아핀 변환은 변환이 평행이동, 회전, 크기조절로 이루어진 선형 변환인 경우를 나타낸다.
	_matrix		TransformationMatrix = XMMatrixAffineTransformation(
		// 1인자 : 크기조절
		XMLoadFloat3(&vScale),
		// 2인자 : 회전 중심 지점 벡터. Channel 의 중심을 0.f, 0.f, 0.f 으로 잡아주어야 뒤틀리지 않는다.
		XMVectorSet(0.f, 0.f, 0.f, 1.f),
		// 3인자 : 회전을 나타내는 쿼터니언
		XMLoadFloat4(&vRotation),
		// 4인자 : 평행 이동 벡터
		XMVectorSetW(XMLoadFloat3(&vTranslation), 1.f));


	Bones[m_iBoneIndex]->Set_TransformationMatrix(TransformationMatrix);
}

_bool CChannel::IsValid()
{
	_bool bValid = false;

	for (_uint i = 1; i < m_iNumKeyFrames; i++) {
		if (m_KeyFrames[i].vScale.x != m_KeyFrames[i - 1].vScale.x)
			return true;
		if (m_KeyFrames[i].vScale.y != m_KeyFrames[i - 1].vScale.y)
			return true;
		if (m_KeyFrames[i].vScale.z != m_KeyFrames[i - 1].vScale.z)
			return true;
		if (m_KeyFrames[i].vRotation.x != m_KeyFrames[i - 1].vRotation.x)
			return true;   
		if (m_KeyFrames[i].vRotation.y != m_KeyFrames[i - 1].vRotation.y)
			return true;   
		if (m_KeyFrames[i].vRotation.z != m_KeyFrames[i - 1].vRotation.z)
			return true;  
		if (m_KeyFrames[i].vRotation.w != m_KeyFrames[i - 1].vRotation.w)
			return true;
		if (m_KeyFrames[i].vTranslation.x != m_KeyFrames[i - 1].vTranslation.x)
			return true;
		if (m_KeyFrames[i].vTranslation.y != m_KeyFrames[i - 1].vTranslation.y)
			return true;
		if (m_KeyFrames[i].vTranslation.z != m_KeyFrames[i - 1].vTranslation.z)
			return true;
	}

	return _bool();
}

CChannel* CChannel::Create(const vector<class CBone*>& Bones, ifstream& fileStream)
{
	CChannel* pInstance = new CChannel();

	if (FAILED(pInstance->Initialize(Bones, fileStream)))
	{
		MSG_BOX(TEXT("Failed To Create : CChannel"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CChannel::Free()
{
	m_KeyFrames.clear();
}
