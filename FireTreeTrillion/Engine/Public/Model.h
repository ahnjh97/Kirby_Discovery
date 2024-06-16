#pragma once

#include "Component.h"
#include "Animation.h"

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

	MODEL Get_ModelInfo() const { return m_tModel; }
	string Get_ModelName() const { return m_tModel.strModelName; }

	_bool IsFinished() { return m_Animations[m_iCurrentAnimIndex]->IsFinished(); }
	_bool IsFinished(_uint iCurrentAnimIndex) { return m_Animations[iCurrentAnimIndex]->IsFinished(); }

	string Get_MeshName(_uint iMeshIndex);
	_float Get_Duration() { return m_Animations[m_iCurrentAnimIndex]->Get_Duration(); }
	_float Get_Trackposition() { return m_Animations[m_iCurrentAnimIndex]->Get_TrackPosition(); }
	_float Get_AnimRatio() { return m_Animations[m_iCurrentAnimIndex]->Get_AnimRatio(); }

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
	void Set_TrackPosition(_float fTrackPosition) { m_Animations[m_iCurrentAnimIndex]->Set_TrackPosition(fTrackPosition); }

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
	HRESULT RenderMergedMesh();

	HRESULT CreateDynamicActor(_float4x4& matWorld);
	HRESULT CreateStaticActor(_float4x4& matWorld);

	_float4 Check_Meshes(const class CTransform* pTransform, _Out_ _int& iMeshIndex) const;
	void	Add_Event(const string& EventName, function<void()>&& Callback);
	void	CallEvent(const string& EventName);


	void Find_MinMax(_float3& vMin, _float3& vMax);
	void Find_MinMax_WorldPos(_float3& vMin, _float3& vMax);
	class COcTree* Create_OcTree(_float3 vMin, _float3 vMax, vector<_uint>& _vecPassIndices, vector<_float>& _vecSamplingFactors
		, vector<string>& _vecConstantNames);

	void Create_MergedMesh(_fmatrix TransformMatrix);
	void Bind_TextureArrays();
	/*ID3D11ShaderResourceView* CreateTexture2DArraySRV(const vector<wstring>& filePaths);*/
	void CreateSamplerState();

	HRESULT Bind_StencilRimLightMotionBlur(class CShader* pShader, vector<string>& _vecConstantNames); // For Binding at Octree
	void SetUpStencilRimLightMotionBlurPassIndex(_uint iShaderVars, _float fRimWidth, _uint iPassIndex); // For Binding at Octree
	_uint Get_ModelPassIndex() { return m_iPassIndex; }
	void Set_WorldMatrixForOctree(_float4x4 _matWorld) { m_matWorld = _matWorld; }
	HRESULT Bind_WorldMatrixForOctree(class CShader* pShader, string& strConstantName = string("g_WorldMatrix"));

private:
	_uint						m_iNumMeshes = { 0 };
	vector<class CMesh*>		m_Meshes;

	_uint						m_iNumMaterials = { 0 };
	vector<MESH_MATERIAL>		m_Materials;

	_float4x4					m_TransformMatrix;

	vector<class CBone*>		m_Bones;

	_float3						m_vMin = {};
	_float3						m_vMax = {};

	// 애니메이션
	_uint						m_iNumAnimations = { 0 };
	_uint						m_iCurrentAnimIndex = { 0 };
	_bool						m_isLoop = { false };
	_bool						m_bStop = false;
	vector<class CAnimation*>	m_Animations;

	_float4x4					m_MeshBoneMatrices[512];

	unordered_map<string, function<void()>>	m_AnimEvents;

	// 파일입출력 변수
	string						m_strDirectory;
	ifstream					m_InputFile;

	// 모델 정보
	MODEL						m_tModel;

	class CMergedMesh*			m_pMergedMesh = { nullptr };
	vector<vector<wstring>>		m_vecTexturePaths;
	vector<ID3D11ShaderResourceView*>	m_vecTextureArraySRVs;
	ID3D11SamplerState*			m_pSamplerState = { nullptr };

	_float						m_fRimWidth = { 0.2f }; // For Binding at Octree
	_bool						m_bStencil = { true };
	_bool						m_bRimLight = { true };
	_bool						m_bMotionBlur = { false };
	_uint						m_iPassIndex = {};
	_float4x4					m_matWorld = {};


private:
	HRESULT Ready_Meshes(_bool bOctree);
	HRESULT Ready_Materials(const _char* pModelFilePath, _bool bOctree);
	HRESULT Ready_Bones();
	HRESULT Ready_Animations();

public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL tModel);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END