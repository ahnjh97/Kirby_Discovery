#pragma once
#include "Base.h"
#include <fstream>

/* 뼈의 정보를 표현하기위한 타입 세가지 제공하낟. */
/* aiNode, aiBone, aiAnimNode */

BEGIN(Engine)

class ENGINE_DLL CBone final : public CBase
{
private:
	CBone();
	virtual ~CBone() = default;

public:
	void Set_TransformationMatrix(_fmatrix TransformationMatrix) {	XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix); }
	_float4x4* Get_EditMatrixPtr() { return &m_EditMatrx; }

public:
	const _float4x4* Get_CombinedTransformationMatrix() const {	return &m_CombinedTransformationMatrix; }

public:
	HRESULT Initialize(ifstream& fileStream);
	void Invalidate_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix TransformatrixMatrix, _bool isRatio);
	
	_bool Compare_Name(const _char* pBoneName) { return !strcmp(m_szName, pBoneName); }

	// 보간을 위한 행위
	_float4x4 Get_TransformationMatrix() { return m_TransformationMatrix; }

	void Read_BoneData(ifstream& fileStream);

private:
	_char				m_szName[MAX_PATH] = { "" };

	/* 이 뼈 만의 상태행렬 */
	_float4x4			m_TransformationMatrix;

	/* 최종 상태행렬. */
	_float4x4			m_CombinedTransformationMatrix;

	_int				m_iParentBoneIndex = { -1 };

	// 뼈를 추가적으로 돌릴 행렬
	_float4x4			m_EditMatrx = _float4x4::Identity;

public:
	static CBone* Create(ifstream& fileStream);
	CBone* Clone();
	virtual void Free() override;
};

END