#include "stdafx.h"
#include "MapToolHelper.h"
#include "Utils.h"
#include <iostream>
#include <filesystem>

using namespace filesystem;
static _int iNonAnimIdx = -1;
static _int iAnimIdx = -1;

CMapToolHelper::CMapToolHelper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CMapToolHelper::CMapToolHelper(const CMapToolHelper& rhs)
	: CGameObject(rhs)
{
}

HRESULT CMapToolHelper::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapToolHelper::Initialize(void* pArg)
{
	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	SetUpTxtVectors(TYPE_ANIM);
	SetUpTxtVectors(TYPE_NONANIM);

	return S_OK;
}

_int CMapToolHelper::Tick(_float fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CMapToolHelper::Late_Tick(_float fTimeDelta)
{
	Menu_NonAnimModels();
}

HRESULT CMapToolHelper::Render()
{
	return S_OK;
}

void CMapToolHelper::Render_IMGUI()
{

}

void CMapToolHelper::SetUpTxtVectors(TYPE _eType)
{
	string strPath = "../../../model_txt/";
	if (TYPE_ANIM == _eType)
		strPath += "Anim/";
	else if (TYPE_NONANIM == _eType)
		strPath += "NonAnim/";

	directory_iterator end_iter;  // 디렉토리 순회의 끝을 나타내는 iterator
	directory_iterator dir_iter(strPath);  // 지정된 경로의 시작 iterator

	while (dir_iter != end_iter) {
		if (is_regular_file(*dir_iter)) {
			string strFilePath = dir_iter->path().filename().string();

			if (TYPE_ANIM == _eType)
				m_vecAnimTxts.emplace_back(strFilePath.substr(0, strFilePath.length() - 4));
			else if (TYPE_NONANIM == _eType)
				m_vecNonAnimTxts.emplace_back(strFilePath.substr(0, strFilePath.length() - 4));
		}
		++dir_iter;
	}
}

void CMapToolHelper::Menu_NonAnimModels()
{
	ImGui::Begin("Models");
	ImGui::SeparatorText("NonAnim");
	ImGui::SetNextItemWidth(200.0f);
	const _char** items2 = new const char* [m_vecNonAnimTxts.size()];
	for (size_t i = 0; i < m_vecNonAnimTxts.size(); ++i)
		items2[i] = m_vecNonAnimTxts[i].c_str();

	if (ImGui::ListBox("##NonAnim", &iNonAnimIdx, items2, static_cast<int>(m_vecNonAnimTxts.size()), 9))
	{
		m_strCurModel = m_vecNonAnimTxts[iNonAnimIdx];
		iAnimIdx = -1;
	}

	Safe_Delete_Array(items2);
	ImGui::End();
}

CMapToolHelper* CMapToolHelper::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapToolHelper* pInstance = new CMapToolHelper(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CMapToolHelper"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CMapToolHelper::Clone(void* pArg)
{
	CMapToolHelper* pInstance = new CMapToolHelper(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CMapToolHelper"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMapToolHelper::Free()
{
	__super::Free();
}

