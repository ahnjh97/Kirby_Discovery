#include "Bone.h"

CBone::CBone()
{
}

HRESULT CBone::Initialize(ifstream& fileStream)
{
	Read_BoneData(fileStream);

	XMStoreFloat4x4(&m_TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_TransformationMatrix)));
	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());

	return S_OK;
}

void CBone::Invalidate_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix TransformatrixMatrix, _bool isRatio)
{
	if (m_EditMatrx != _float4x4::Identity && isRatio == false)
	{
		m_TransformationMatrix *= m_EditMatrx;
		//_matrix			TransformationMatrix = { XMLoadFloat4x4(&m_TransformationMatrix) };
		//_matrix			EditMatrix = { XMLoadFloat4x4(&m_EditMatrx) };
		//_vector			vScaleSrc, vQuaternionSrc, vTranslationSrc;
		//_vector			vScaleDst, vQuaternionDst, vTranslationDst;
		//XMMatrixDecompose(&vScaleSrc, &vQuaternionSrc, &vTranslationSrc, TransformationMatrix);
		//XMMatrixDecompose(&vScaleDst, &vQuaternionDst, &vTranslationDst, EditMatrix);
		//_vector			vResultQuaternion = { XMQuaternionMultiply(XMQuaternionNormalize(vQuaternionSrc), XMQuaternionNormalize(vQuaternionDst)) };
		//_matrix			ResultMatrix = { XMMatrixAffineTransformation(vScaleSrc, XMVectorSet(0.f, 0.f, 0.f, 1.f), vResultQuaternion, vTranslationSrc) };
		//XMStoreFloat4x4(&m_TransformationMatrix, ResultMatrix);
	}

	if (-1 == m_iParentBoneIndex)
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * TransformatrixMatrix);
	else
	{
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, 
			XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
	}
}

void CBone::Read_BoneData(ifstream& fileStream)
{
	fileStream.read(reinterpret_cast<char*>(&m_iParentBoneIndex), sizeof(m_iParentBoneIndex));
	fileStream.read(reinterpret_cast<char*>(&m_szName), sizeof(m_szName));
	fileStream.read(reinterpret_cast<char*>(&m_TransformationMatrix), sizeof(m_TransformationMatrix));
}

CBone* CBone::Create(ifstream& fileStream)
{
	CBone* pInstance = new CBone();

	if (FAILED(pInstance->Initialize(fileStream)))
	{
		MSG_BOX(TEXT("Failed To Create : CBone"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CBone * CBone::Clone()
{
	return new CBone(*this);
}

void CBone::Free()
{
	
}
