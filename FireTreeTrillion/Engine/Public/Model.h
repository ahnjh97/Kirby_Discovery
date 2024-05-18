#pragma once

#include "Component.h"
#include "Animation.h"
#include <fstream>

BEGIN(Engine)

class ENGINE_DLL CModel final : public CComponent
{
public:
	enum TYPE { TYPE_NONANIM, TYPE_ANIM, TYPE_END };

private:
	CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModel(const CModel& rhs);
	virtual ~CModel() = default;

public:
	_uint Get_NumMeshes() const { return m_iNumMeshes; }
	class CBone* Get_BonePtr(const _char* pBoneName) const;

	_bool IsFinished() { return m_Animations[m_iCurrentAnimIndex]->IsFinished(); }

public:
	void Set_Animation(_uint iAnimIndex, _bool isLoop) { m_iCurrentAnimIndex = iAnimIndex;	m_isLoop = isLoop; }

public:
	virtual HRESULT Initialize_Prototype(TYPE eType, const string& strModelName, _fmatrix TransformMatrix, MODEL tModel, _bool bNonAnimVersion);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, _uint iTextureType);
	
	HRESULT Play_Animation(_float fTimeDelta);
	HRESULT Render(_uint iMeshIndex);

private:
	TYPE						m_eModelType = { TYPE_END };

private:
	_uint						m_iNumMeshes = { 0 };
	vector<class CMesh*>		m_Meshes;

	_uint						m_iNumMaterials = { 0 };
	vector<MESH_MATERIAL>		m_Materials;

	_float4x4					m_TransformMatrix;

	vector<class CBone*>		m_Bones;

	_uint						m_iNumAnimations = { 0 };
	_uint						m_iCurrentAnimIndex = { 0 };
	_bool						m_isLoop = { false };
	vector<class CAnimation*>	m_Animations;

	_float4x4					m_MeshBoneMatrices[512];

	// 파일입출력 변수
	string						m_strDirectory;
	ifstream					m_InputFile;

	// 모델 정보
	string						m_strModelName;
	MODEL						m_tModel;
	_bool						m_bIsNonAnimVersion = { false };

private:
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const _char* pModelFilePath);
	HRESULT Ready_Bones();
	HRESULT Ready_Animations();

public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eType, const string& strModelName, _fmatrix TransformMatrix
						,MODEL tModel, _bool bNonAnimVersion = false);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END