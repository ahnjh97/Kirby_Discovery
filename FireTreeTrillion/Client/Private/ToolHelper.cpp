#include "stdafx.h"
#include "ToolHelper.h"

#include "UIObject.h"
#include "Utils.h"

CToolHelper::CToolHelper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CToolHelper::CToolHelper(const CToolHelper& rhs)
	: CGameObject(rhs)
{
}

HRESULT CToolHelper::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CToolHelper::Initialize(void* pArg)
{
	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	// ************************** UI 처음 띄우고 나서 SAVE 한 뒤에 삭제 될 소스 **************************
	//CUIObject::UIOBJ_DESC		UIObjDesc{};
	//UIObjDesc.Size2D			= _float2(167.f, 232.f);
	//UIObjDesc.Position2D		= _float2(1172.f, 60.f);
	//UIObjDesc.WindowSize2D	= _float2(g_iWinSizeX, g_iWinSizeY);
	//CUIObject* pUIObject = static_cast<CUIObject*>(m_pGameInstance->Add_CloneReturn(m_iLevelIndex, LAYER_UI, TEXT("Prototype_GameObject_Wallet"), &UIObjDesc));
	//CHECK_NULLPTR(pUIObject);
	//m_vecUIObjects.push_back(pUIObject);

	//pUIObject = static_cast<CUIObject*>(m_pGameInstance->Add_CloneReturn(m_iLevelIndex, LAYER_UI, TEXT("Prototype_GameObject_UI_DayNightLine"), &UIObjDesc));
	//CHECK_NULLPTR(pUIObject);
	//m_vecUIObjects.push_back(pUIObject);

	// ************************************************************************************************
	Load("../Bin/Resources/Data/UIData/UI_240418.ui");

	return S_OK;
}

_int CToolHelper::Tick(_float fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CToolHelper::Late_Tick(_float fTimeDelta)
{
}

HRESULT CToolHelper::Render()
{
	return S_OK;
}

void CToolHelper::Save()
{
	string FileName = "../Bin/Resources/Data/UIData/UI_240418.ui";
	std::ofstream File(FileName, std::ios::binary);

	if (!File.is_open()) {
		MSG_BOX(TEXT("UIHelper Save Failed"));
		return;
	}

	// PrototypeTag, size2D, position2D
	size_t size = m_vecUIObjects.size();
	File.write(reinterpret_cast<const char*>(&size), sizeof(size));

	for (auto iter : m_vecUIObjects)
	{
		wstring PrototypeTag = iter->Get_PrototypeTag();
		_float2 size2D = _float2();//iter->Get_size2D();
		_float2 position2D = _float2();//iter->Get_pos2D();

		string strPrototypeTag = CUtils::WstrToStr(PrototypeTag);
		size_t PrototypeTagLen = strPrototypeTag.length() + 1;
		File.write(reinterpret_cast<const char*>(&PrototypeTagLen), sizeof(PrototypeTagLen));
		File.write(strPrototypeTag.c_str(), sizeof(char) * PrototypeTagLen);
		
		File.write(reinterpret_cast<const char*>(&size2D), sizeof(size2D));
		File.write(reinterpret_cast<const char*>(&position2D), sizeof(position2D));
	}

	File.close();
}

void CToolHelper::Load(const string& FileName)
{
	std::ifstream File(FileName, std::ios::binary);

	if (!File.is_open()) {
		MSG_BOX(TEXT("UIHelper Load Failed"));
		return;
	}

	// PrototypeTag, Worldmatrix
	size_t size = 0;
	File.read(reinterpret_cast<char*>(&size), sizeof(size));
	m_vecUIObjects.reserve(size);

	for (size_t i = 0; i < size; ++i)
	{
		// PrototypeTag, size2D, position2D
		char PrototypeTag[256];
		size_t PrototypeTagLen;
		File.read(reinterpret_cast<char*>(&PrototypeTagLen), sizeof(PrototypeTagLen));
		File.read(PrototypeTag, sizeof(char) * PrototypeTagLen);

		_float2 size2D, pos2D;
		File.read(reinterpret_cast<char*>(&size2D), sizeof(size2D));
		File.read(reinterpret_cast<char*>(&pos2D), sizeof(pos2D));

		CUIObject::UIOBJ_DESC	UIObjDesc{};
		/*UIObjDesc.Size2D		= size2D;
		UIObjDesc.Position2D	= pos2D;
		UIObjDesc.WindowSize2D	= _float2(g_iWinSizeX, g_iWinSizeY);*/
		CUIObject* pUIObject = static_cast<CUIObject*>(m_pGameInstance->Clone_GameObject(CUtils::StrToWstr(PrototypeTag), &UIObjDesc));
		m_vecUIObjects.push_back(pUIObject);
	}

	File.close();
}

void CToolHelper::Render_IMGUI()
{
	if (ImGui::Button("Save"))
	{
		Save();
	}

	ImGui::SameLine();

	if (ImGui::Button("Load"))
	{
		Load("../Bin/Resources/Data/UIData/TEST.ui");
	}

	ImGui::Separator();
	ImGui::NewLine();
}

CToolHelper* CToolHelper::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CToolHelper* pInstance = new CToolHelper(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CToolHelper"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CToolHelper::Clone(void* pArg)
{
	CToolHelper* pInstance = new CToolHelper(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CToolHelper"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CToolHelper::Free()
{
	__super::Free();

	for (auto& p : m_vecUIObjects)
		Safe_Release(p);
	m_vecUIObjects.clear();
}

