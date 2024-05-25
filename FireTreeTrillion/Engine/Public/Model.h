#pragma once

#include "Component.h"
#include "Animation.h"
#include <fstream>

BEGIN(Engine)

class ENGINE_DLL CModel final : public CComponent
{
private:
	CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModel(const CModel& rhs);
	virtual ~CModel() = default;

public:
	_uint Get_NumMeshes() const { return m_iNumMeshes; }
	class CBone* Get_BonePtr(const _char* pBoneName) const;

	MODEL Get_ModelInfo() { return m_tModel; }

	_bool IsFinished() { return m_Animations[m_iCurrentAnimIndex]->IsFinished(); }

public:
	void Set_TickPerSecond(_float _fTickPerSecond) {
		m_Animations[m_iCurrentAnimIndex]->Set_TickPerSecond(_fTickPerSecond);
	}


	void Set_Animation(_uint iAnimIndex, _float fTickPerSecond, _bool bIsLooping, _bool bInterpolation = false) {
		m_iCurrentAnimIndex = iAnimIndex;	
		m_isLoop = bIsLooping;

		m_Animations[m_iCurrentAnimIndex]->Reset_TrackPosition();
		m_Animations[m_iCurrentAnimIndex]->Reset_Finished();
		m_Animations[m_iCurrentAnimIndex]->Set_TickPerSecond(fTickPerSecond);

		if (bInterpolation)
		{
			// 바뀔 애니메이션을 대상으로 선형보간 ON
			m_Animations[m_iCurrentAnimIndex]->Reset_Ratio();
		}
	}

	const _char* Get_AnimationName() const {
		return m_Animations[m_iCurrentAnimIndex]->Get_AnimationName();
	}

public:
	virtual HRESULT Initialize_Prototype(MODEL tModel);
	virtual HRESULT Initialize(void* pArg)  override;
	virtual void	Render_IMGUI()			override;

public:
	HRESULT Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, _uint iTextureType);
	
	HRESULT Play_Animation(_float fTimeDelta);
	HRESULT Render(_uint iMeshIndex);

	HRESULT CreateDynamicActor(_float4 vPos);
	HRESULT CreateStaticActor(_float4 vPos);

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
	MODEL						m_tModel;

private:
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const _char* pModelFilePath);
	HRESULT Ready_Bones();
	HRESULT Ready_Animations();

public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL tModel);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END