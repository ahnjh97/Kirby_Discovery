#include "Model.h"
#include "Channel.h"
#include "Texture.h"
#include "Shader.h"
#include "Model.h"
#include "Bone.h"
#include "Mesh.h"
#include "Utils.h"

CModel::CModel(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent{ pDevice, pContext }
{
}

CModel::CModel(const CModel & rhs)
	: CComponent{ rhs }
	, m_iNumMeshes { rhs.m_iNumMeshes } 
	, m_Meshes { rhs.m_Meshes }
	, m_iNumMaterials { rhs.m_iNumMaterials }
	, m_Materials { rhs.m_Materials }
	, m_TransformMatrix { rhs.m_TransformMatrix }	
	, m_iNumAnimations { rhs.m_iNumAnimations }	
	, m_tModel{ rhs.m_tModel }
{
	for (auto& pPrototypeAnimation : rhs.m_Animations)	
		m_Animations.push_back(pPrototypeAnimation->Clone());

	for (auto& pPrototypeBone : rhs.m_Bones)
		m_Bones.push_back(pPrototypeBone->Clone());
	
	for (auto& pMesh : m_Meshes)	
		Safe_AddRef(pMesh);

	for (auto& Material : m_Materials)
	{
		for (size_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++)		
			Safe_AddRef(Material.MaterialTextures[i]);	
	}
}

CBone * CModel::Get_BonePtr(const _char * pBoneName) const
{
	auto	iter = find_if(m_Bones.begin(), m_Bones.end(), [&](CBone* pBone)->_bool
	{		
		return pBone->Compare_Name(pBoneName);
	});

	return *iter;
}

HRESULT CModel::Initialize_Prototype(_fmatrix TransformMatrix, MODEL tModel)
{
	m_tModel = tModel;

	string strFolderName = "Anim/";
	string strTxt = ".txt";

	if (m_tModel.eType == TYPE_NONANIM)
		strFolderName = "Non" + strFolderName;

	m_strDirectory = "../../../model_txt/" + strFolderName + m_tModel.strModelName + strTxt;

	m_InputFile.open(m_strDirectory.c_str(), ios::in | ios::binary);
	if (!m_InputFile.is_open())
	{	// StrToWstr 필요
		wstring tempstr = L"Failed To Open : " + CUtils::StrToWstr(m_strDirectory);
		MSG_BOX(tempstr.c_str());
		return E_FAIL;
	}

	::XMStoreFloat4x4(&m_TransformMatrix, TransformMatrix);

	/* 읽은 정보를 바탕으로해서 내가 사용하기 좋게 정리한다.  */
	if (m_tModel.eType == TYPE_ANIM)
	{
		if (FAILED(Ready_Bones()))
			return E_FAIL;
	}

	/* 모델을 구성하는 메시들을 생성한다. */
	/* 모델 = 메시 + 메시 + ... */
	if (FAILED(Ready_Meshes()))
		return E_FAIL;

 	if (FAILED(Ready_Materials(m_tModel.strModelName.c_str())))
		return E_FAIL;

	if (m_tModel.eType == TYPE_ANIM)
	{
		if (FAILED(Ready_Animations()))
			return E_FAIL;
	}

	m_InputFile.close();

	return S_OK;
}

HRESULT CModel::Initialize(void * pArg)
{
	/* 읽은 정보를 바탕으로해서 내가 사용하기 좋게 정리한다.  */
	

	return S_OK;
}

void CModel::Render_IMGUI()
{
	__super::Render_IMGUI();
	

	if (m_Animations.size() >= m_iCurrentAnimIndex)
		return;

	// Animation의 end까지
	ImGui::Text("AnimationIndex: %d", m_iCurrentAnimIndex);

	const _char* animationName = m_Animations[m_iCurrentAnimIndex]->Get_AnimationName();
	ImGui::Text("Animation Name: %s", animationName);

	_float fTickPerSecond = m_Animations[m_iCurrentAnimIndex]->Get_TickPerSecond();
	ImGui::DragFloat("TickPerSecond : ", &fTickPerSecond);
	m_Animations[m_iCurrentAnimIndex]->Set_TickPerSecond(fTickPerSecond);

	_float fTrackPosition = m_Animations[m_iCurrentAnimIndex]->Get_TrackPosition();
	ImGui::DragFloat("Track_Position : ", &fTrackPosition);
	m_Animations[m_iCurrentAnimIndex]->Set_TrackPosition(fTrackPosition);
}

HRESULT CModel::Bind_BoneMatrices(CShader * pShader, const _char * pConstantName, _uint iMeshIndex)
{
	ZeroMemory(m_MeshBoneMatrices, sizeof(_float4x4) * 512);

	m_Meshes[iMeshIndex]->Stock_Matrices(m_Bones, m_MeshBoneMatrices);

	return pShader->Bind_Matrices(pConstantName, m_MeshBoneMatrices, 512);	
}

HRESULT CModel::Bind_ShaderResource(CShader * pShader, const _char * pConstantName, _uint iMeshIndex, _uint iTextureType)
{
	if (iMeshIndex >= m_iNumMeshes)
		return E_FAIL;

	_uint		iMeshMaterialIndex = { m_Meshes[iMeshIndex]->Get_MaterialIndex() };

	if (iMeshMaterialIndex >= m_iNumMaterials)
		return E_FAIL;

	if (nullptr != m_Materials[iMeshMaterialIndex].MaterialTextures[iTextureType])
	{
		if (FAILED(m_Materials[iMeshMaterialIndex].MaterialTextures[iTextureType]->Bind_ShaderResource(pShader, pConstantName)))
			return E_FAIL;
	}
	
	return S_OK;
}

HRESULT CModel::Play_Animation(_float fTimeDelta)
{
	/* 현재 애니메이션에 맞는 뼈의 상태(m_TransformationMatrix)를 갱신해준다. */
	m_Animations[m_iCurrentAnimIndex]->Invalidate_TransformationMatrix(fTimeDelta, m_Bones, m_isLoop);

	for (auto& pBone : m_Bones)
		pBone->Invalidate_CombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_TransformMatrix));
	
	return S_OK;
}

HRESULT CModel::Render(_uint iMeshIndex)
{
	m_Meshes[iMeshIndex]->Bind_Buffers();
	m_Meshes[iMeshIndex]->Render();

	return S_OK;
}

HRESULT CModel::CreateDynamicActor(_float4 vPos)
{
	for (auto& mesh : m_Meshes)
		mesh->CreateDynamicActor(vPos);

	return S_OK;
}

HRESULT CModel::CreateStaticActor(_float4 vPos)
{
	for (auto& mesh : m_Meshes)
		mesh->CreateStaticActor(vPos);

	return S_OK;
}

HRESULT CModel::Ready_Meshes()
{
	m_InputFile.read(reinterpret_cast<char*>(&m_iNumMeshes), sizeof(m_iNumMeshes));

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		CMesh* pMesh = CMesh::Create(m_pDevice, m_pContext, m_tModel.eType, m_strDirectory, m_InputFile, m_Bones, XMLoadFloat4x4(&m_TransformMatrix));
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel::Ready_Materials(const _char * pModelFilePath)
{
	m_InputFile.read(reinterpret_cast<char*>(&m_iNumMaterials), sizeof(m_iNumMaterials));

	while (true)
	{
		_uint iTemp;
		_uint jTemp;
		MESH_MATERIAL MeshMaterial{};
		_char			szFullPath[MAX_PATH] = { "" };
		_tchar			szPerfectPath[MAX_PATH] = { L"" };
		m_InputFile.read(reinterpret_cast<char*>(&iTemp), sizeof(iTemp));
		if (iTemp == m_iNumMaterials)
			break;
		m_InputFile.read(reinterpret_cast<char*>(&jTemp), sizeof(jTemp));
		m_InputFile.read(reinterpret_cast<char*>(&szFullPath), sizeof(szFullPath));

		MultiByteToWideChar(CP_ACP, 0, szFullPath, strlen(szFullPath), szPerfectPath, MAX_PATH);
		MeshMaterial.MaterialTextures[jTemp] = CTexture::Create(m_pDevice, m_pContext, szPerfectPath);
		if (nullptr == MeshMaterial.MaterialTextures[jTemp])
			return E_FAIL;

		if (m_Materials.size() <= iTemp) // 새로운 배열 삽입
			m_Materials.push_back(MeshMaterial);
		else // 벡터 안에있는 배열의 원소 수정
			m_Materials[iTemp].MaterialTextures[jTemp] = MeshMaterial.MaterialTextures[jTemp];
	}

	return S_OK;
}

HRESULT CModel::Ready_Bones()
{
	_uint iNumBones = {};

	m_InputFile.read(reinterpret_cast<char*>(&iNumBones), sizeof(iNumBones));
	m_Bones.reserve(iNumBones);

	for (_uint i = 0; i < iNumBones; i++)
	{
		CBone* pBone = CBone::Create(m_InputFile);
		if (nullptr == pBone)
			return E_FAIL;
		m_Bones.push_back(pBone);
	}

	return S_OK;
}

HRESULT CModel::Ready_Animations()
{
	m_InputFile.read(reinterpret_cast<char*>(&m_iNumAnimations), sizeof(m_iNumAnimations));
	m_Animations.reserve(m_iNumAnimations);

	for (size_t i = 0; i < m_iNumAnimations; i++)
	{
		CAnimation* pAnimation = CAnimation::Create(m_Bones, m_InputFile);
		if (nullptr == pAnimation)
			return E_FAIL;

		m_Animations.push_back(pAnimation);
	}

	m_iNumAnimations = m_Animations.size();

	return S_OK;
}

CModel * CModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix TransformMatrix, MODEL tModel)
{
	CModel* pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(TransformMatrix, tModel)))
	{
		MSG_BOX(TEXT("Failed To Create : CModel"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CModel::Clone(void * pArg)
{
	CModel*		pInstance = new CModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CModel"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModel::Free()
{
	__super::Free();

	for (auto& pAnimation : m_Animations)
		Safe_Release(pAnimation);
	m_Animations.clear();

	for (auto& pBone : m_Bones)
		Safe_Release(pBone);
	m_Bones.clear();

	for (auto& Material : m_Materials)
	{
		for (size_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
			Safe_Release(Material.MaterialTextures[i]);
	}
	m_Materials.clear();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);
	m_Meshes.clear();
}
