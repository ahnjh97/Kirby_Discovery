#include "Model.h"
#include "GameInstance.h"
#include "MergedMesh.h"
#include "OcTree.h"
//#include "Channel.h"
#include "Texture.h"
#include "Shader.h"
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
	, m_pMergedMesh{ rhs.m_pMergedMesh }
	, m_vecTextureArraySRVs{ rhs.m_vecTextureArraySRVs }
	, m_pSamplerState{ rhs.m_pSamplerState }
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

	Safe_AddRef(m_pSamplerState);

	for (auto& texArraySRV : m_vecTextureArraySRVs)
		Safe_AddRef(texArraySRV);
}

CBone * CModel::Get_BonePtr(const _char * pBoneName) const
{
	auto	iter = find_if(m_Bones.begin(), m_Bones.end(), [&](CBone* pBone)->_bool
	{		
		return pBone->Compare_Name(pBoneName);
	});

	return *iter;
}

string CModel::Get_MeshName(_uint iMeshIndex)
{
	string str = m_Meshes[iMeshIndex]->Get_Name();
	return str;
}

HRESULT CModel::Initialize_Prototype(MODEL tModel)
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

	_float4x4 TransformMatrix = XMMatrixIdentity();
	TransformMatrix = XMMatrixScaling(tModel.fScale, tModel.fScale, tModel.fScale) * XMMatrixRotationY(XMConvertToRadians(tModel.fDegree));

	::XMStoreFloat4x4(&m_TransformMatrix, TransformMatrix);

	/* 읽은 정보를 바탕으로해서 내가 사용하기 좋게 정리한다.  */
	if (m_tModel.eType == TYPE_ANIM)
	{
		if (FAILED(Ready_Bones()))
			return E_FAIL;
	}

	/* 모델을 구성하는 메시들을 생성한다. */
	/* 모델 = 메시 + 메시 + ... */
	if (FAILED(Ready_Meshes(tModel.bOctree)))
		return E_FAIL;

 	if (FAILED(Ready_Materials(m_tModel.strModelName.c_str(), tModel.bOctree)))
		return E_FAIL;

	if (m_tModel.eType == TYPE_ANIM)
	{
		if (FAILED(Ready_Animations()))
			return E_FAIL;
	}

	m_InputFile.close();

	if (true == tModel.bOctree) {
		/*Create_MergedMesh(TransformMatrix);
		CreateSamplerState();

		vector<wstring> vecDiffuse;
		for (auto& vecPaths : m_vecTexturePaths)
			vecDiffuse.push_back(vecPaths[TextureType_DIFFUSE]);
		m_vecTextureArraySRVs.emplace_back(CreateTexture2DArraySRV(vecDiffuse));

		vector<wstring> vecNormal;
		for (auto& vecPaths : m_vecTexturePaths)
			vecNormal.push_back(vecPaths[TextureType_NORMALS]);
		m_vecTextureArraySRVs.emplace_back(CreateTexture2DArraySRV(vecNormal));

		vector<wstring> vecMRA;
		for (auto& vecPaths : m_vecTexturePaths)
			vecMRA.push_back(vecPaths[TextureType_METALNESS]);
		m_vecTextureArraySRVs.emplace_back(CreateTexture2DArraySRV(vecMRA));*/

		//wstring wstrFullPath = L"../../../Resources/Models/NonAnim/Level1Stage1Step01/GsAllBuildingCeilingConcreteC_MRA._622887136.dds";
		//vecTexPath.emplace_back(wstrFullPath);
	}
		
	return S_OK;
}

HRESULT CModel::Initialize(void * pArg)
{
	/* 읽은 정보를 바탕으로해서 내가 사용하기 좋게 정리한다.  */
	

	return S_OK;
}

#ifdef _DEBUG
void CModel::Render_IMGUI()
{
	__super::Render_IMGUI();
	

	if (m_Animations.empty())
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
#endif

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
	if (m_bStop) return S_OK;

	/* 현재 애니메이션에 맞는 뼈의 상태(m_TransformationMatrix)를 갱신해준다. */
	m_Animations[m_iCurrentAnimIndex]->Invalidate_TransformationMatrix(fTimeDelta, m_Bones, m_isLoop, this);

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

HRESULT CModel::Render()
{
	if (nullptr == m_pMergedMesh)
		return S_OK;

	m_pMergedMesh->Bind_Buffers();
	m_pMergedMesh->Render();

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

_float4 CModel::Check_Meshes(const class CTransform* pTransform, _Out_ _int& iMeshIndex) const
{
	if (m_Meshes.empty())
		return _float4();

	vector<pair<_float4, _int>> vecPickPosAndMeshIdx;
	for (_int i = 0; i < static_cast<_int>(m_iNumMeshes); i++)
	{
		_float4 vTemp = m_Meshes[i]->Get_PickPos(pTransform);
		if (vTemp.w != 0)
			vecPickPosAndMeshIdx.emplace_back(pair<_float4, _int>(vTemp, i));
	}

	if (vecPickPosAndMeshIdx.empty())
		return _float4();

	_float fShortest = { FLT_MAX };
	_float4 fResult = {};
	for (auto& iter : vecPickPosAndMeshIdx)
	{
		if (iter.first.w < fShortest) {
			fShortest = iter.first.w;
			fResult = iter.first;
			iMeshIndex = iter.second;
		}
	}
	return fResult;
}

void CModel::Add_Event(const string& EventName, function<void()>&& Callback)
{
	m_AnimEvents.emplace(EventName, move(Callback));
}

void CModel::CallEvent(const string& EventName)
{
	auto EventIter = m_AnimEvents.find(EventName);
	if (EventIter != m_AnimEvents.end())
		EventIter->second();
}

HRESULT CModel::Ready_Meshes()
void CModel::Find_MinMax(_float3& vMin, _float3& vMax)
{
	for (auto& mesh : m_Meshes)
		mesh->Find_MinMax(vMin, vMax);
}

COcTree* CModel::Create_OcTree(_float3 vMin, _float3 vMax, vector<_uint>& _vecPassIndices, vector<_float>& _vecSamplingFactors)
{
	if (vMin.x == 0 || false == m_tModel.bOctree)
		return nullptr;

	m_vMin = vMin;
	m_vMax = vMax;
	_float3 vCenter = _float3((m_vMin.x + m_vMax.x) * 0.5f, (m_vMin.y + m_vMax.y) * 0.5f, (m_vMin.z + m_vMax.z) * 0.5f);
	_float3 vHalfExtents = _float3((m_vMax.x - m_vMin.x) * 0.5f, (m_vMax.y - m_vMin.y) * 0.5f, (m_vMax.z - m_vMin.z) * 0.5f);

	vector<_float3*> vecVerticesPtrs;
	vector<_uint> vecNumVertices;
	vector<_float3*> vecNormalsPtrs;
	vector<_float2*> vecTexCoordsPtrs;
	vector<_float3*> vecTangentsPtrs;
	vector<_uint*> vecIndicesPtrs;
	vector<_uint> vecNumIndices;

	for (auto& mesh : m_Meshes)
	{
		vecVerticesPtrs.push_back(mesh->Get_VerticesPtr());
		vecNumVertices.push_back(mesh->Get_NumVertices());
		vecNormalsPtrs.push_back(mesh->Get_NormalsPtr());
		vecTexCoordsPtrs.push_back(mesh->Get_TexCoordsPtr());
		vecTangentsPtrs.push_back(mesh->Get_TangentsPtr());
		vecIndicesPtrs.push_back(mesh->Get_IndicesPtr());
		vecNumIndices.push_back(mesh->Get_NumIndices());
	}

	//vector<FACE> vecFaces;

	//_int iCount{};
	//for (_uint i = 0; i < iNumIndices / 3; i++)
	//{
	//	vecFaces.emplace_back(pIndicesPtr[iCount], pIndicesPtr[iCount + 1], pIndicesPtr[iCount + 2]);
	//	iCount += 3;
	//}

	string strFilePath = "../../../objects_txt/" + m_tModel.strModelName + "_Octree.txt";
	ifstream fileInput(strFilePath, ios::in | ios::binary);

	COcTree* pOctree = COcTree::Create(m_pDevice, m_pContext, vCenter, vHalfExtents, vecVerticesPtrs, vecNumVertices
		, vecNormalsPtrs, vecTexCoordsPtrs, vecTangentsPtrs,vecIndicesPtrs, vecNumIndices
		, fileInput, m_Meshes, m_Materials, _vecPassIndices, _vecSamplingFactors);

	fileInput.close();

	return pOctree;
}

void CModel::Create_MergedMesh(_fmatrix TransformMatrix)
{
	vector<_float3*> vecMeshVerticesPtrs;
	vector<_uint> vecMeshNumVertices;
	vector<_float3*> vecMeshNormalsPtrs;
	vector<_uint*> vecMeshIndicesPtrs;
	vector<_uint> vecMeshNumIndices;
	vector<_uint> vecMaterialIndices;

	for (auto& mesh : m_Meshes)
	{
		vecMeshVerticesPtrs.emplace_back(mesh->Get_VerticesPtr());
		vecMeshNumVertices.emplace_back(mesh->Get_NumVertices());
		vecMeshNormalsPtrs.emplace_back(mesh->Get_NormalsPtr());
		vecMeshIndicesPtrs.emplace_back(mesh->Get_IndicesPtr());
		vecMeshNumIndices.emplace_back(mesh->Get_NumIndices());
		vecMaterialIndices.emplace_back(mesh->Get_MaterialIndex());
	}

	m_pMergedMesh = CMergedMesh::Create(m_pDevice, m_pContext, vecMeshVerticesPtrs, vecMeshNumVertices,
		vecMeshNormalsPtrs, vecMeshIndicesPtrs, vecMeshNumIndices, vecMaterialIndices, m_tModel.strModelName, TransformMatrix);
}

void CModel::Bind_TextureArrays()
{
	//if(nullptr != m_pSamplerState)
	//	m_pContext->PSSetSamplers(5, 1, &m_pSamplerState);
	//for (_uint i = 4; i < 4 + m_vecTextureArraySRVs.size(); i++)
	//	m_pContext->PSSetShaderResources(i, 1, &m_vecTextureArraySRVs[i-4]);
}

//ID3D11ShaderResourceView* CModel::CreateTexture2DArraySRV(const vector<wstring>& filePaths) {
//	ID3D10Multithread* pMultithread = nullptr;
//	if (FAILED(m_pDevice->QueryInterface(__uuidof(ID3D10Multithread), reinterpret_cast<void**>(&pMultithread)))) {
//		MSG_BOX(TEXT("ID3D10Multithread Failed."));
//		return nullptr;
//	}
//	pMultithread->SetMultithreadProtected(TRUE);
//	pMultithread->Enter();
//
//	vector<ID3D11Texture2D*> textures;
//	D3D11_TEXTURE2D_DESC textureDesc = {};
//	bool firstTexture = true;
//
//	for (const auto& filePath : filePaths) {
//		ID3D11ShaderResourceView* pSRV = nullptr;
//		ID3D11Resource* pResource = nullptr;
//		HRESULT hr = DirectX::CreateDDSTextureFromFile(m_pDevice, filePath.c_str(), &pResource, &pSRV);
//		if (FAILED(hr)) {
//			return nullptr;
//		}
//
//		ID3D11Texture2D* texture = nullptr;
//		hr = pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture);
//		if (FAILED(hr)) {
//			pSRV->Release();
//			pResource->Release();
//			return nullptr;
//		}
//
//		textures.push_back(texture);
//
//		if (firstTexture) {
//			texture->GetDesc(&textureDesc);
//			firstTexture = false;
//		}
//
//		pSRV->Release();
//		pResource->Release();
//	}
//
//	textureDesc.ArraySize = static_cast<_uint>(textures.size());
//	textureDesc.Usage = D3D11_USAGE_DEFAULT;
//	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
//	textureDesc.CPUAccessFlags = 0;
//	textureDesc.MiscFlags = 0;
//
//	ID3D11Texture2D* textureArray = nullptr;
//	HRESULT hr = m_pDevice->CreateTexture2D(&textureDesc, nullptr, &textureArray);
//	if (FAILED(hr)) {
//		for (auto& texture : textures) {
//			texture->Release();
//		}
//		return nullptr;
//	}
//
//	for (_uint i = 0; i < textures.size(); ++i) {
//		for (_uint mipLevel = 0; mipLevel < textureDesc.MipLevels; ++mipLevel) {
//			m_pContext->CopySubresourceRegion(textureArray, D3D11CalcSubresource(mipLevel, i, textureDesc.MipLevels), 0, 0, 0, textures[i], mipLevel, nullptr);
//		}
//		textures[i]->Release();
//	}
//
//	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//	srvDesc.Format = textureDesc.Format;
//	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
//	srvDesc.Texture2DArray.MostDetailedMip = 0;
//	srvDesc.Texture2DArray.MipLevels = textureDesc.MipLevels;
//	srvDesc.Texture2DArray.FirstArraySlice = 0;
//	srvDesc.Texture2DArray.ArraySize = textureDesc.ArraySize;
//
//	ID3D11ShaderResourceView* srv = nullptr;
//	hr = m_pDevice->CreateShaderResourceView(textureArray, &srvDesc, &srv);
//	textureArray->Release();
//	pMultithread->Leave();
//	pMultithread->Release();
//	if (FAILED(hr))
//		return nullptr;
//
//	return srv;
//}

void CModel::CreateSamplerState()
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 필터링 모드 설정
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // 가로 축 주소 모드 설정
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; // 세로 축 주소 모드 설정
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP; // 깊이 축 주소 모드 설정
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; // 비교 함수 설정
	samplerDesc.MinLOD = -D3D11_FLOAT32_MAX; // 최소 LOD 설정
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // 최대 LOD 설정

	HRESULT hr = m_pDevice->CreateSamplerState(&samplerDesc, &m_pSamplerState);
	if (FAILED(hr)) {
		TEXT("Failed to Create SamplerState");
		Safe_Release(m_pSamplerState);
		return;
	}
}

HRESULT CModel::Ready_Meshes(_bool bOctree)
{
	m_InputFile.read(reinterpret_cast<char*>(&m_iNumMeshes), sizeof(m_iNumMeshes));

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		CMesh* pMesh = CMesh::Create(m_pDevice, m_pContext, m_tModel.eType, m_strDirectory, m_InputFile, m_Bones, XMLoadFloat4x4(&m_TransformMatrix), bOctree);
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel::Ready_Materials(const _char* pModelFilePath, _bool bOctree)
{
	m_InputFile.read(reinterpret_cast<char*>(&m_iNumMaterials), sizeof(m_iNumMaterials));

	if (false == bOctree)
	{
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
	}
	else
	{
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

			// 텍스처 경로 저장
			if (m_vecTexturePaths.size() <= iTemp) {
				m_vecTexturePaths.resize(iTemp + 1); // 벡터 크기 조정
			}
			if (m_vecTexturePaths[iTemp].size() <= jTemp) {
				m_vecTexturePaths[iTemp].resize(jTemp + 1); // 벡터 크기 조정
			}
			m_vecTexturePaths[iTemp][jTemp] = wstring(szPerfectPath); // 경로 저장
		}
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

		// 로더에서 읽어온 애님툴에서 수정된 애니메이션 unordered_map을 여기서 사용
		for (auto& pair : m_tModel.umapAnimInfo)
		{
			// 애니메이션 map을 돌면서 현재 접근한 애니메이션과 이름이 같은 지 확인
			if (pAnimation->Get_AnimationName() == pair.first)
			{
				// 같은 경우 해당 애니메이션이 갈취(?)해야하는 정보를 set해준다.
				pAnimation->Set_AnimEventData(pair.second);
			}
		}
		m_Animations.push_back(pAnimation);
	}

	m_iNumAnimations = m_Animations.size();

	return S_OK;
}

CModel * CModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL tModel)
{
	CModel* pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(tModel)))
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
		MSG_BOX(TEXT("Failed To Clone : CModel"));

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

	Safe_Release(m_pSamplerState);

	for (auto& pTexArr : m_vecTextureArraySRVs)
		Safe_Release(pTexArr);
	m_vecTextureArraySRVs.clear();
		
	for (auto& Material : m_Materials)
	{
		for (size_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
			Safe_Release(Material.MaterialTextures[i]);
	}
	m_Materials.clear();

	Safe_Release(m_pMergedMesh);

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);
	m_Meshes.clear();
}
