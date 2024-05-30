#include "stdafx.h"
#include "AnimToolHelper.h"

#include "ImSequencer.h"
#include "Utils.h"


static string strProtoTag = "";
static _int		iModelNum = 0;

static const char* SequencerItemTypeNames[] = { "Effect Event", "Collision Event", "Sound Event" };
struct AnimSequence : public ImSequencer::SequenceInterface
{
	struct AnimSequenceItem
	{
		_int  iType;
		_int  iStart;
		_int  iEnd;
		_bool bExpanded;
	};

	// 구조체 내 멤버 변수, vector
	_int m_iFrameMin, m_iFrameMax;
	std::vector<AnimSequenceItem> m_vecSequenceItems;

	// 구조체 생성자
	AnimSequence() {}

	// 프레임 최소 최대값
	virtual int GetFrameMin() const override { return m_iFrameMin; }
	virtual int GetFrameMax() const override { return m_iFrameMax; }

	// 아이템 개수/종류
	virtual int GetItemCount() const override { return static_cast<int>(m_vecSequenceItems.size()); }

	// 추가할 이벤트 개수/종류
	virtual int GetItemTypeCount() const { return sizeof(SequencerItemTypeNames) / sizeof(char*); }
	virtual const char* GetItemTypeName(_int typeIndex) const override
	{
		return SequencerItemTypeNames[typeIndex];
	}

	virtual const char* GetItemLabel(int index) const
	{
		static char tmps[512];
		snprintf(tmps, 512, "[%02d] %s", index, SequencerItemTypeNames[m_vecSequenceItems[index].iType]);
		return tmps;
	}

	virtual void Get(int index, int** start, int** end, int* type, unsigned int* color) override
	{
		AnimSequenceItem& sequenceItem = m_vecSequenceItems[index];
		if (color)
			*color = 0xFFAA8080; // same color for everyone, return color based on type
		if (start)
			*start = &sequenceItem.iStart;
		if (end)
			*end = &sequenceItem.iEnd;
		if (type)
			*type = sequenceItem.iType;
	}

	virtual void Add(int type) override
	{
		m_vecSequenceItems.push_back(AnimSequenceItem{ type, m_iFrameMin, m_iFrameMax, false });
	}

	virtual void Del(int index) override
	{
		if (index < m_vecSequenceItems.size())
			m_vecSequenceItems.erase(m_vecSequenceItems.begin() + index);
	}

	virtual void Duplicate(int index) override
	{
		if (index < m_vecSequenceItems.size())
			m_vecSequenceItems.push_back(m_vecSequenceItems[index]);
	}
};


CAnimToolHelper::CAnimToolHelper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CAnimToolHelper::CAnimToolHelper(const CAnimToolHelper& rhs)
	: CGameObject(rhs)
{
}

HRESULT CAnimToolHelper::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAnimToolHelper::Initialize(void* pArg)
{
	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	Ready_AnimObjects(L"Layer_AnimObjects");

	//Load("../Bin/Resources/Data/UIData/UI_240418.ui");

	return S_OK;
}

_int CAnimToolHelper::Tick(_float fTimeDelta)
{
	for (auto& obj : m_vecAnimObjects)
		obj->Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CAnimToolHelper::Late_Tick(_float fTimeDelta)
{
	for (auto& obj : m_vecAnimObjects)
		obj->Late_Tick(fTimeDelta);

	Render_ObjectList();
}

HRESULT CAnimToolHelper::Render()
{
	for (auto& obj : m_vecAnimObjects)
		obj->Render();


	return S_OK;
}

void CAnimToolHelper::Ready_AnimObjects(const wstring& strLayerTag)
{
	CGameObject* pGameObj = nullptr;

	// Kirby
	pGameObj = m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Kirby"));
	CHECK_NULLPTR(pGameObj);
	m_vecAnimObjects.push_back(pGameObj);

	// 다른 애님 오브젝트들 이어서

}

void CAnimToolHelper::Render_ObjectList()
{
	ImGui::Begin("OBJECT LIST");

	// ListBox 사이즈 조정
	float windowHeight = ImGui::GetWindowHeight();
	float listBoxHeight = windowHeight - 100;
	ImGui::SetNextWindowSizeConstraints(ImVec2(-1, listBoxHeight), ImVec2(-1, listBoxHeight));

	// 검색 기능 추가
	static ImGuiTextFilter filter;
	ImGui::Text("Search Object");
	filter.Draw();

	static int item_current_idx = -1;
	if (ImGui::BeginListBox(" ", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
	{
		for (int n = 0; n < m_vecAnimObjects.size(); n++)
		{
			const bool is_selected = (item_current_idx == n);
			wstring wstrTag = m_vecAnimObjects[n]->Get_PrototypeTag();
			_char szName[256];
			CUtils::WCharToChar(wstrTag.c_str(), szName);
			if (szName == nullptr) continue;
			if (filter.PassFilter(szName))
				if (ImGui::Selectable(szName, is_selected))
					item_current_idx = n;
			
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
				Render_AnimationList(wstrTag);
			}
		}
		ImGui::EndListBox();
	}
	ImGui::End();
}

void CAnimToolHelper::Render_AnimationList(const wstring& wstrObjectTag)
{
	ImGui::Begin("ANIMATION LIST");
	
	float windowHeight = ImGui::GetWindowHeight();
	float listBoxHeight = windowHeight - 100;
	ImGui::SetNextWindowSizeConstraints(ImVec2(-1, windowHeight), ImVec2(-1, windowHeight));

	static ImGuiTextFilter filter;
	ImGui::Text("Search Animation");
	filter.Draw();

	for (auto& obj : m_vecAnimObjects)
	{
		if (obj->Get_PrototypeTag() == wstrObjectTag)
		{
			CModel** pModel = obj->Get_ModelAddress();
			_uint uModelCnt = obj->Get_ModelCnt();

			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			for (size_t w = 0; w < uModelCnt; ++w) // 모델 개수 만큼 Tab을 형성한다.
			{
				if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
				{
					MODEL tModelInfo = pModel[w]->Get_ModelInfo();
					if (ImGui::BeginTabItem(tModelInfo.strModelName.c_str())) // 모델이름으로 Tab Name 형성
					{
						float childwindowHeight = ImGui::GetWindowHeight();
						float childlistBoxHeight = windowHeight - 110;
						ImGui::SetNextWindowSizeConstraints(ImVec2(-1, childlistBoxHeight), ImVec2(-1, childlistBoxHeight));

						static int item_current_idx = -1;
						if (ImGui::BeginListBox(" ", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
						{
								// 해당 모델이 가지고 있는 애니메이션 개수
								_uint uAnimCnt = pModel[w]->Get_AnimCnt();
								vector<CAnimation*>* pVecAnims = pModel[w]->Get_Animations();
								for (int n = 0; n < uAnimCnt; n++)
								{
									const bool is_selected = (item_current_idx == n);

									const char* animName = (*pVecAnims)[n]->Get_AnimationName();
									if (animName == nullptr) continue;

									if (filter.PassFilter(animName))
										if (ImGui::Selectable(animName, is_selected))
											item_current_idx = n;

									if (is_selected)
									{
										ImGui::SetItemDefaultFocus();
										Render_FrameLine(&(*pVecAnims)[n], animName);
									}
								}
								ImGui::EndListBox();
						}
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
			}
		}
		ImGui::End();
	}
}

void CAnimToolHelper::Render_FrameLine(CAnimation** ppAnimation, const string& strAnimationTag)
{
	static AnimSequence mySequence;
	mySequence.m_iFrameMin = 0;
	mySequence.m_iFrameMax = (_int)(*ppAnimation)->Get_Duration();

	// 벡터에 SEQUENCE ITEM 여러개 넣기
	if (mySequence.m_vecSequenceItems.empty())
	{
		mySequence.m_vecSequenceItems.push_back(AnimSequence::AnimSequenceItem{ 0, 11, 12, false });
		mySequence.m_vecSequenceItems.push_back(AnimSequence::AnimSequenceItem{ 1, 10, 11, false });
		mySequence.m_vecSequenceItems.push_back(AnimSequence::AnimSequenceItem{ 2, 11, 12, false });
	}

	 // 고정할 위치와 크기
	ImVec2 windowPos = ImVec2(368, 583); // 창의 위치
	ImVec2 windowSize = ImVec2(1221, 299); // 창의 크기

	// 창의 위치와 크기를 설정
	ImGui::SetNextWindowPos(windowPos,	 ImGuiCond_Always);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

	// 애니메이션 관련 창 생성
	ImGui::Begin(strAnimationTag.c_str());

	static int selectedEntry = -1;
	static int firstFrame = 0;
	static bool expanded = true;

	// 아이템 간격
	ImGui::PushItemWidth(100);
	
	// 현재 프레임 띄우기
	static _int currentFrame = 0;
	ImGui::InputInt("Current Frame", &currentFrame); ImGui::SameLine();

	// 애니메이션 스피드
	static _float animationSpeed = 0;
	ImGui::InputFloat("Animation Speed", &animationSpeed); ImGui::SameLine();

	// 현 애니메이션 데이터 저장
	if (ImGui::Button("ANIMATION DATA SAVE")) 
	{
		Save();
	}

	ImGui::Text("Frame Min: %d", mySequence.m_iFrameMin); ImGui::SameLine();
	ImGui::Text("Frame Max: %d", mySequence.m_iFrameMax);
	ImGui::PopItemWidth();

	// 프레임단위 띄우기
	ImSequencer::Sequencer(&mySequence, &currentFrame, &expanded, &selectedEntry, &firstFrame,
		ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | /*ImSequencer::SEQUENCER_COPYPASTE | */ImSequencer::SEQUENCER_CHANGE_FRAME);

	ImGui::End();
}

//void CAnimToolHelper::Render_EventList()
//{
//}

void CAnimToolHelper::Save()
{
	//string FileName = "../Bin/Resources/Data/UIData/UI_240418.ui";
	//std::ofstream File(FileName, std::ios::binary);

	//if (!File.is_open()) {
	//	MSG_BOX(TEXT("UIHelper Save Failed"));
	//	return;
	//}

	//// PrototypeTag, size2D, position2D
	//size_t size = m_vecUIObjects.size();
	//File.write(reinterpret_cast<const char*>(&size), sizeof(size));

	//for (auto iter : m_vecUIObjects)
	//{
	//	wstring PrototypeTag = iter->Get_PrototypeTag();
	//	_float2 size2D = _float2();//iter->Get_size2D();
	//	_float2 position2D = _float2();//iter->Get_pos2D();

	//	string strPrototypeTag = CUtils::WstrToStr(PrototypeTag);
	//	size_t PrototypeTagLen = strPrototypeTag.length() + 1;
	//	File.write(reinterpret_cast<const char*>(&PrototypeTagLen), sizeof(PrototypeTagLen));
	//	File.write(strPrototypeTag.c_str(), sizeof(char) * PrototypeTagLen);

	//	File.write(reinterpret_cast<const char*>(&size2D), sizeof(size2D));
	//	File.write(reinterpret_cast<const char*>(&position2D), sizeof(position2D));
	//}

	//File.close();
}

void CAnimToolHelper::Load(const string& FileName)
{
	//std::ifstream File(FileName, std::ios::binary);

	//if (!File.is_open()) {
	//	MSG_BOX(TEXT("UIHelper Load Failed"));
	//	return;
	//}

	//// PrototypeTag, Worldmatrix
	//size_t size = 0;
	//File.read(reinterpret_cast<char*>(&size), sizeof(size));
	//m_vecUIObjects.reserve(size);

	//for (size_t i = 0; i < size; ++i)
	//{
	//	// PrototypeTag, size2D, position2D
	//	char PrototypeTag[256];
	//	size_t PrototypeTagLen;
	//	File.read(reinterpret_cast<char*>(&PrototypeTagLen), sizeof(PrototypeTagLen));
	//	File.read(PrototypeTag, sizeof(char) * PrototypeTagLen);

	//	_float2 size2D, pos2D;
	//	File.read(reinterpret_cast<char*>(&size2D), sizeof(size2D));
	//	File.read(reinterpret_cast<char*>(&pos2D), sizeof(pos2D));

	//	CUIObject::UIOBJ_DESC	UIObjDesc{};
	//	/*UIObjDesc.Size2D		= size2D;
	//	UIObjDesc.Position2D	= pos2D;
	//	UIObjDesc.WindowSize2D	= _float2(g_iWinSizeX, g_iWinSizeY);*/
	//	CUIObject* pUIObject = static_cast<CUIObject*>(m_pGameInstance->Clone_GameObject(CUtils::StrToWstr(PrototypeTag), &UIObjDesc));
	//	m_vecUIObjects.push_back(pUIObject);
	//}

	//File.close();
}

void CAnimToolHelper::Render_IMGUI()
{
	if (ImGui::Button("Save"))
		//Save();
		ImGui::SameLine();

	if (ImGui::Button("Load"))
		//Load("../Bin/Resources/Data/UIData/TEST.ui");

		ImGui::Separator(); ImGui::NewLine();
}

CAnimToolHelper* CAnimToolHelper::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAnimToolHelper* pInstance = new CAnimToolHelper(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CAnimToolHelper"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CAnimToolHelper::Clone(void* pArg)
{
	CAnimToolHelper* pInstance = new CAnimToolHelper(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CAnimToolHelper"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAnimToolHelper::Free()
{
	__super::Free();

	for (auto& p : m_vecAnimObjects)
		Safe_Release(p);
	m_vecAnimObjects.clear();
}

