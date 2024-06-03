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
	_bool IsFinished(_uint iCurrentAnimIndex) { return m_Animations[iCurrentAnimIndex]->IsFinished(); }

	string Get_MeshName(_uint iMeshIndex);

	_float Get_Duration() { return m_Animations[m_iCurrentAnimIndex]->Get_Duration(); }

	_float Get_Trackposition() { return m_Animations[m_iCurrentAnimIndex]->Get_TrackPosition(); }

	_float Get_AnimRatio() {
		return m_Animations[m_iCurrentAnimIndex]->Get_AnimRatio();
	}

public:
	void Set_TickPerSecond(_float _fTickPerSecond) { m_Animations[m_iCurrentAnimIndex]->Set_TickPerSecond(_fTickPerSecond); }
	
	void Set_Animation(_int iAnimIndex)
	{
		m_isLoop = true;
		m_iCurrentAnimIndex = iAnimIndex;
		m_Animations[m_iCurrentAnimIndex]->Reset_TrackPosition();
		m_Animations[m_iCurrentAnimIndex]->Reset_Finished();
		m_Animations[m_iCurrentAnimIndex]->Remove_Ratio();
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

	const _char* Get_AnimationName() const { return m_Animations[m_iCurrentAnimIndex]->Get_AnimationName(); }
	_uint Get_AnimCnt() const { return m_Animations.size(); }
	vector<class CAnimation*>* const Get_Animations() { return &m_Animations; }
	
public:
	virtual HRESULT Initialize_Prototype(MODEL tModel);
	virtual HRESULT Initialize(void* pArg)  override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()			override;
#endif

public:
	HRESULT Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, _uint iTextureType);
	
	HRESULT Play_Animation(_float fTimeDelta);
	void	Stop_Animation() { m_bStop = true; }
	void	Replay_Animation() { m_bStop = false; }
	HRESULT Render(_uint iMeshIndex);

	HRESULT CreateDynamicActor(_float4 vPos);
	HRESULT CreateStaticActor(_float4 vPos);

	_float4 Check_Meshes(const class CTransform* pTransform, _Out_ _int& iMeshIndex) const;

	void Find_MinMax(_float3& vMin, _float3& vMax);
	void Create_OcTree(_float3 vMin, _float3 vMax);
	void Culling(_fmatrix matWorldInverse);
	void Save_OctreeData();

private:
	_uint						m_iNumMeshes = { 0 };
	vector<class CMesh*>		m_Meshes;

	_uint						m_iNumMaterials = { 0 };
	vector<MESH_MATERIAL>		m_Materials;

	_float4x4					m_TransformMatrix;

	vector<class CBone*>		m_Bones;

	class COcTree*				m_pOctree = { nullptr };
	_float3						m_vMin = {};
	_float3						m_vMax = {};

	// 애니메이션
	_uint						m_iNumAnimations = { 0 };
	_uint						m_iCurrentAnimIndex = { 0 };
	_bool						m_isLoop = { false };
	_bool						m_bStop = false;
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