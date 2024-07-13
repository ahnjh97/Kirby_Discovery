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
	_bool IsPartialAnimFinished() { return m_Animations[m_iCurPartialAnim]->IsFinished(); }

	string Get_MeshName(_uint iMeshIndex);
	_float Get_Duration() { return m_Animations[m_iCurrentAnimIndex]->Get_Duration(); }
	_float Get_Trackposition() { return m_Animations[m_iCurrentAnimIndex]->Get_TrackPosition(); }
	_float Get_AnimRatio() { return m_Animations[m_iCurrentAnimIndex]->Get_AnimRatio(); }
	_float Get_PartialAnimRatio() { return m_Animations[m_iCurPartialAnim]->Get_AnimRatio(); }
	
	CModel* CreateModelFromMesh(_uint iMeshIndex, _float3& vOffset
		, unordered_set<string>& _setCheckedStrings, unordered_set<string>& _setExcludedMesh);

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
	void Set_Animation(_uint iAnimIndex, _float fTickPerSecond, _bool bIsLooping, _bool bInterpolation = false, _float fLerpTime = 0.1f) {
		m_iCurrentAnimIndex = iAnimIndex;	
		m_isLoop = bIsLooping;

		m_Animations[m_iCurrentAnimIndex]->Reset_TrackPosition();
		m_Animations[m_iCurrentAnimIndex]->Reset_Finished();
		m_Animations[m_iCurrentAnimIndex]->Set_TickPerSecond(fTickPerSecond);

		if (bInterpolation)
		{
			// 바뀔 애니메이션을 대상으로 선형보간 ON
			m_Animations[m_iCurrentAnimIndex]->Reset_Ratio();
			m_Animations[m_iCurrentAnimIndex]->Reset_RatioTime();
			m_Animations[m_iCurrentAnimIndex]->Set_LerpTime(fLerpTime);
		}
	}
	void Set_TrackPosition(_float fTrackPosition) { m_Animations[m_iCurrentAnimIndex]->Set_TrackPosition(fTrackPosition); }

	const _char* Get_AnimationName() const { return m_Animations[m_iCurrentAnimIndex]->Get_AnimationName(); }
	_float Get_AnimTrackPosition() { return m_Animations[m_iCurrentAnimIndex]->Get_AnimTrackPosition(); }
	_uint Get_AnimCnt() const { return m_Animations.size(); }
	vector<class CAnimation*>* const Get_Animations() { return &m_Animations; }
	_uint Get_CurAnimIndex() { return m_iCurrentAnimIndex; }
	void Reset_TrackPosition(_uint iAnimIndex) { m_Animations[iAnimIndex]->Reset_TrackPosition(); }
	
public:
	virtual HRESULT Initialize_Prototype(MODEL tModel);
	virtual HRESULT Initialize_Prototype(vector<class CMesh*>& _vecMeshes, const vector<MESH_MATERIAL>& _vecMaterials);
	virtual HRESULT Initialize(void* pArg)  override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()			override;
#endif

public:
	HRESULT Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, _uint iTextureType);
	
	HRESULT Play_Animation(_float fTimeDelta);

	HRESULT Lerp_PartialAnim(_float fTimeDelta);
	void	EmplaceBackPartialAnim(_uint iAnimIndex);
	HRESULT Play_PartialAnimation(_float fTimeDelta);
	void	Reset_PartialAnimation(_uint iAnimIndex, _float fTickPerSecond, _bool bIsLooping, _bool bInterpolation = false, _float fLerpTime = 0.1f);

	void	Stop_Animation() { m_bStop = true; }
	void	Replay_Animation() { m_bStop = false; }

	HRESULT Render(_uint iMeshIndex);
	HRESULT RenderMergedMesh();

	PxRigidStatic* ReturnStaticActor(_float4x4& matWorld);
	PxRigidStatic* ReturnStaticActor_FilterByIndex(_float4x4& matWorld, unordered_set<_uint>& _setExcludedMesh, _bool bInclude);
	PxRigidDynamic* ReturnDynamicActor(_float4x4& matWorld);
	PxRigidDynamic* ReturnDynamicActor_FilterByIndex(_float4x4& matWorld, unordered_set<_uint>& _setExcludedMesh, _bool bInclude);

	_float4 Check_Meshes(const class CTransform* pTransform, _int& iMeshIndex) const;
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
	void Set_ModelPassIndex(_uint iPassIndex) { m_iPassIndex = iPassIndex; }
	_uint Get_ModelPassIndex() { return m_iPassIndex; }
	void Set_WorldMatrixForOctree(_float4x4 _matWorld) { m_matWorld = _matWorld; }
	_float4x4 Get_WorldMatrixForOctree() { return m_matWorld; }
	HRESULT Bind_WorldMatrixForOctree(class CShader* pShader, string& strConstantName = string("g_WorldMatrix"));
	void SetUp_ModelIdleAnimForOctree(_uint iAnimIndex, _float fTickPerSec) { m_iIdleAnimIndex = iAnimIndex; m_fIdleAnimTickPerSec = fTickPerSec; }
	void ReturnToIdle() { Set_Animation(m_iIdleAnimIndex, m_fIdleAnimTickPerSec, true, true, 0.1f); }
	
	void Set_Hide(_bool bHide) { m_bHide = bHide; }
	_bool IsHidden() { return m_bHide; }
	_uint Find_MeshIndex(const string& _strMeshName);
	void RemoveNonBlendMeshes(const unordered_set<_uint>& _vecBlendingMeshIndices);
	void RemoveBlendMeshes(const unordered_set<_uint>& _vecBlendingMeshIndices);
	_bool DoesTextureExist(_uint iTextureType, _uint iMeshIndex);

	void AlignMeshMaterialIndicesWithMeshIndices();
	string ExtractDigitsAfterUnderScore(_uint iMeshIndex);

	_float Get_CurTrackPosition() { return m_Animations[m_iCurrentAnimIndex]->Get_TrackPosition(); }
	_bool Get_LerpPartialAnim() { return m_bLerpPartialAnim; }
	void Set_LerpPartialAnim(_bool bLerpPartialAnim) { m_bLerpPartialAnim = bLerpPartialAnim; }

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
	_uint						m_iIdleAnimIndex = {};
	_float						m_fIdleAnimTickPerSec = {};
	_bool						m_bHide = { false };

	unordered_map<_uint, pair<vector<_uint>, unordered_set<_uint>>>	m_mapValidBones;

	_uint m_iCurPartialAnim = {};
	_bool m_bPlayPartialAnim = { false };
	_bool m_bLerpPartialAnim = { false };
	_float m_fPartialAnimLerpTime = {};

private:
	HRESULT Ready_Meshes(_bool bOctree);
	HRESULT Ready_Materials(const _char* pModelFilePath, _bool bOctree);
	HRESULT Ready_Bones();
	HRESULT Ready_Animations();

public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL tModel);
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext
		, vector<class CMesh*>& _vecMeshes, const vector<MESH_MATERIAL>& _vecMaterials);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END