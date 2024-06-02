#include "stdafx.h"
#include "AnimToolHelper.h"

#include "AnimToolObject.h"
#include "Model.h"
#include "Utils.h"
#include "ImSequencer.h"
#include "tinyxml2.h"


static string	g_strProtoObjTag = "";
//static string	g_strModelTag = "";
//static _int	g_iActiveModelNum = -1;

static const char* SequencerItemTypeNames[] = { "notify"/*, "Collision Event", "Sound Event" */ };
struct AnimSequence : public ImSequencer::SequenceInterface
{
	//struct AnimSequenceItem
	//{
	//	string	strEventName;
	//	_int	iStartFrame;
	//	_int	iEndFrame;
	//};

	// 구조체 내 멤버 변수, vector
	_int m_iFrameMin, m_iFrameMax;
	std::vector<EVENT_INFO> m_vecSequenceItems;

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

	virtual const char* GetItemLabel(int _index, const char* _eventName) const
	{
		static char tmps[512];
		snprintf(tmps, 512, "[%d] %s", _index/*m_vecSequenceItems.size()*/, _eventName/*SequencerItemTypeNames[m_vecSequenceItems[0].iEventType]*/);
		return tmps;
	}

	virtual void Get(int index, int** start, int** end, char** eventName, unsigned int* color) override
	{
		EVENT_INFO& sequenceItem = m_vecSequenceItems[index];
		if (color)
			*color = 0xFFAA8080; // same color for everyone, return color based on type
		if (start)
			*start = &sequenceItem.iStartFrame;
		if (end)
			*end = &sequenceItem.iEndFrame;
		if (eventName)
			*eventName = const_cast<char*>(sequenceItem.strEventName.c_str());
	}

	virtual void Add(int type, const char* strName) override
	{
		m_vecSequenceItems.push_back(EVENT_INFO{ strName, 0, 1 });
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

//========================================== AnimToolHelper ==========================================
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

	// 테스트 객체 생성
	m_pAnimToolObj = static_cast<CAnimToolObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_AnimToolObject")));
	CHECK_NULLPTR(m_pAnimToolObj);

	// 테스트 애님 모델들 가져오기
	Ready_AnimModels();

	Load();

	return S_OK;
}

_int CAnimToolHelper::Tick(_float fTimeDelta)
{
	m_pAnimToolObj->Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CAnimToolHelper::Late_Tick(_float fTimeDelta)
{
	m_pAnimToolObj->Late_Tick(fTimeDelta);

	Render_ObjectList();
}

HRESULT CAnimToolHelper::Render()
{
	return S_OK;
}

void CAnimToolHelper::Ready_AnimModels()
{
	string strPath = "../../../model_txt/Anim/";
	directory_iterator end_iter;			// 디렉토리 순회의 끝을 나타내는 iterator
	directory_iterator dir_iter(strPath);   // 지정된 경로의 시작 iterator

	while (dir_iter != end_iter) 
	{
		if (is_regular_file(*dir_iter)) 
		{
			string strFilePath = dir_iter->path().filename().string();
			m_vecAnimModels.emplace_back(strFilePath.substr(0, strFilePath.length() - 4));
		}
		++dir_iter;
	}
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
		for (int n = 0; n < m_vecAnimModels.size(); n++)
		{
			const bool is_selected = (item_current_idx == n);
			string strModelName = m_vecAnimModels[n];
			if (strModelName.empty()) continue;

			if (filter.PassFilter(strModelName.c_str()))
			{
				if (ImGui::Selectable(strModelName.c_str(), is_selected))
				{
					item_current_idx = n;
					
					m_strModelName = m_vecAnimModels[n];
					wstring wstrPrototypeTag = TEXT("Prototype_Component_Model_") + CUtils::StrToWstr(m_strModelName);
					m_pAnimToolObj->Change_ModelCom(wstrPrototypeTag);
				}
			}
			
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
				
				Render_AnimationList();
			}
		}
		ImGui::EndListBox();
	}
	ImGui::End();
}

void CAnimToolHelper::Render_AnimationList()
{
	ImGui::Begin("ANIMATION LIST");

	float windowHeight = ImGui::GetWindowHeight();
	float listBoxHeight = windowHeight - 100;
	ImGui::SetNextWindowSizeConstraints(ImVec2(-1, windowHeight), ImVec2(-1, windowHeight));


	static ImGuiTextFilter filter;
	ImGui::Text("Search Animation");
	filter.Draw();

	m_pModel = static_cast<CModel*>(m_pAnimToolObj->Get_Component(TEXT("Com_Model")));
	/*if (obj->Get_PrototypeTag() == wstrObjectTag)
	{*/
	//CModel** pModel = obj->Get_ModelAddress();
	//_uint uModelCnt = obj->Get_ModelCnt();

	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	//for (size_t w = 0; w < uModelCnt; ++w) // 모델 개수 만큼 Tab을 형성한다.
	//{
		// 현재 활성화된 탭 인덱스
	if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
	{
		//MODEL tModelInfo = pModel[w]->Get_ModelInfo();
		if (ImGui::BeginTabItem(m_strModelName.c_str())) // 모델이름으로 Tab Name 형성
		{
			//g_iActiveModelNum = w;
			_float childwindowHeight = ImGui::GetWindowHeight();
			_float childlistBoxHeight = windowHeight - 110;
			ImGui::SetNextWindowSizeConstraints(ImVec2(-1, childlistBoxHeight), ImVec2(-1, childlistBoxHeight));

			static _int item_current_idx = -1;
			static _int item_previous_idx = -1;

			if (ImGui::BeginListBox(" ", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
			{
				// 해당 모델이 가지고 있는 애니메이션 개수
				_uint uAnimCnt = m_pModel->Get_AnimCnt();
				vector<CAnimation*>* pVecAnims = m_pModel->Get_Animations();

				if (m_pGameInstance->Get_KeyState(DIK_UP, KEY_DOWN))
				{
					item_current_idx--;
					if (item_current_idx < 0)
						item_current_idx = uAnimCnt - 1;
				}
				if (m_pGameInstance->Get_KeyState(DIK_DOWN, KEY_DOWN))
				{
					item_current_idx++;
					if (item_current_idx >= uAnimCnt)
						item_current_idx = 0;
				}

				// 인덱스가 변경되었을 때 애니메이션 설정 호출
				if (item_previous_idx != item_current_idx)
				{
					m_pModel->Set_Animation(item_current_idx);
					item_previous_idx = item_current_idx;
				}

				for (_int n = 0; n < uAnimCnt; n++)
				{
					const _bool is_selected = (item_current_idx == n);
					const _char* animName = (*pVecAnims)[n]->Get_AnimationName();
					m_strAnimationName = animName;
					if (animName == nullptr) continue;

					if (filter.PassFilter(animName))
					{
						if (ImGui::Selectable(animName, is_selected))
						{
							item_current_idx = n;
							m_pModel->Set_Animation(n);
						}

						if (is_selected)
						{
							// 해당 애니메이션 ListBox 포커싱
							ImGui::SetItemDefaultFocus();
							ImGui::SetScrollHereY();
							// 애니메이션 창 띄우기
							Render_FrameLine(&(*pVecAnims)[n], animName);
						}
					}
				}
				ImGui::EndListBox();
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

// 가져온 애니메이션의 정보를 가져오는 함수
void CAnimToolHelper::Render_FrameLine(CAnimation** ppAnimation, const string& strAnimationTag)
{
	static string previousAnimation = (*ppAnimation)->Get_AnimationName();

	// Sequence에서 해당 animation frame data긁어와서 처리
	static AnimSequence mySequence;
	mySequence.m_iFrameMin = 0;
	mySequence.m_iFrameMax = (_int)(*ppAnimation)->Get_Duration();

	// 벡터 초기화 세팅
	if (mySequence.m_vecSequenceItems.empty())
	{
		// QZR : 추후 animation정보를 가져오는 Load()를 여기서 처리
		//Load();
		// Load해서 가져온 data들 중 Sequence에 띄워야하는
		// (1) 프레임 처음/끝, (2) 이벤트 종류, (3) 이벤트 이름을 가져와서 아래에 push_back한다.

		//for (auto item : m_vecSequence)
		//{
		//	mySequence.m_vecSequenceItems.push_back(AnimSequence::AnimSequenceItem{item.strEventName, item.iStartFrame, item.iEndFrame });
		//}
		mySequence.m_vecSequenceItems.push_back(EVENT_INFO{"Notify", 0,1 });
	}

	 // 고정할 위치와 크기
	ImVec2 windowPos  = ImVec2(368, 583);  // 창의 위치
	ImVec2 windowSize = ImVec2(1221, 299); // 창의 크기

	// 창의 위치와 크기를 설정
	ImGui::SetNextWindowPos(windowPos,	 ImGuiCond_Always);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

	// 애니메이션 관련 창 생성
	ImGui::Begin(strAnimationTag.c_str());

	static _int selectedEntry = -1;
	static _int firstFrame = 0;
	static _bool expanded = true;

	// 아이템 간격
	ImGui::PushItemWidth(100);
	
	// 현재 프레임 띄우기
	static _int currentFrame = 0;
	ImGui::Text("Current Frame"); ImGui::SameLine();
	ImGui::InputInt("  ", &currentFrame); ImGui::SameLine();
	
	static _bool bIsLoop = true;
	ImGui::Text("LOOP"); ImGui::SameLine();
	ImGui::Checkbox("   ", &bIsLoop);
	if (!bIsLoop)
		(*ppAnimation)->Set_TrackPosition((_float)currentFrame);
	else
		currentFrame = (*ppAnimation)->Get_TrackPosition();
	ImGui::SameLine();
	
	// 애니메이션 스피드
	m_fAnimationSpeed = (*ppAnimation)->Get_TickPerSecond();
	if (previousAnimation != (*ppAnimation)->Get_AnimationName())
		m_fAnimationSpeed = (*ppAnimation)->Get_TickPerSecond();
	ImGui::Text("Animation Speed"); ImGui::SameLine();
	ImGui::InputFloat("    ", &m_fAnimationSpeed); ImGui::SameLine();
	(*ppAnimation)->Set_TickPerSecond(m_fAnimationSpeed);

	// 미리보기
	if (ImGui::Button("PREVIEW"))
	{
		//auto& ModelIter = m_mapSequence[m_strModelName];
		//auto& AnimIter = ModelIter[m_strAnimationName];

		////Event 정보를 덮어씌운다.
		//AnimIter.fAnimSpeed = m_fAnimationSpeed;
		//AnimIter.vecEventInfo = mySequence.m_vecSequenceItems;
		//Save();
		//Load();
	}

	// 현 애니메이션 데이터 저장
	ImGui::Text("IS IT DONE? >> "); ImGui::SameLine();
	if (ImGui::Button("ANIMATION DATA SAVE")) 
	{
		auto& ModelIter = m_mapSequence[m_strModelName];
		auto& AnimIter = ModelIter[m_strAnimationName];

		//Event 정보를 덮어씌운다.
		AnimIter.fAnimSpeed = m_fAnimationSpeed;
		AnimIter.vecEventInfo = mySequence.m_vecSequenceItems;
		Save();
	}



	ImGui::Text("Frame Min: %d", mySequence.m_iFrameMin); ImGui::SameLine();
	ImGui::Text("Frame Max: %d", mySequence.m_iFrameMax);
	ImGui::PopItemWidth();

	// 프레임단위 띄우기
	ImSequencer::Sequencer(&mySequence, &currentFrame, &expanded, &selectedEntry, &firstFrame,
		ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_CHANGE_FRAME | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL);
	
	if (selectedEntry != -1)
	{
		ImGui::OpenPopup("Notify");

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Notify", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Register Event Name");

			static char charEventName[128] = ""; 
			ImGui::InputText(" ", charEventName, sizeof(charEventName));
			if (ImGui::Button("OK", ImVec2(120, 0)) || m_pGameInstance->Get_DIKeyState(DIK_RETURN, KEY_DOWN)) 
			{
				_bool bCheckSame = false;
				for (auto& p : mySequence.m_vecSequenceItems)
				{
					if (p.strEventName == charEventName)
					{
						bCheckSame = true;
						break;
					}
				}
				
				if (!bCheckSame)
				{
					mySequence.m_vecSequenceItems.push_back(EVENT_INFO{ charEventName, 0, 1 });
					memset(charEventName, 0, sizeof(charEventName));
					selectedEntry = -1;
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SetItemDefaultFocus(); ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				selectedEntry = -1;
				ImGui::CloseCurrentPopup(); 
			}
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

//void CAnimToolHelper::Save() 
//{
//	tinyxml2::XMLDocument m_xmlDocument;
//	tinyxml2::XMLNode* m_pRootNode = m_xmlDocument.NewElement("Root");
//	m_xmlDocument.InsertFirstChild(m_pRootNode);
//
//	tinyxml2::XMLElement* m_pElement = m_xmlDocument.NewElement("Version");
//	m_pElement->SetText(240602);
//	m_pRootNode->InsertEndChild(m_pElement);
//
//	for (const auto& [ModelName, AnimMap] : m_mapSequence) 
//	{
//		tinyxml2::XMLElement* m_pModelElement = m_xmlDocument.NewElement("ModelName");
//		m_pModelElement->SetText(ModelName.c_str());
//		m_pRootNode->InsertEndChild(m_pModelElement);
//
//		for (const auto& [AnimName, AnimInfo] : AnimMap) 
//		{
//			tinyxml2::XMLElement* m_pAnimElement = m_xmlDocument.NewElement("Animation");
//			m_pAnimElement->SetText(AnimName.c_str());
//			m_pModelElement->InsertEndChild(m_pAnimElement);
//
//			tinyxml2::XMLElement* m_pAnimSpeedElement = m_xmlDocument.NewElement("AnimSpeed");
//			m_pAnimSpeedElement->SetText(AnimInfo.fAnimSpeed);
//			m_pAnimElement->InsertEndChild(m_pAnimSpeedElement);
//
//			tinyxml2::XMLElement* m_pCountElement = m_xmlDocument.NewElement("Count");
//			unsigned int iCnt = AnimInfo.vecEventInfo.size();
//			m_pCountElement->SetText(iCnt);
//			m_pAnimElement->InsertEndChild(m_pCountElement);
//
//			for (unsigned int i = 0; i < iCnt; ++i) {
//				string strData = "Data" + to_string(i);
//				tinyxml2::XMLElement* m_pDataElement = m_xmlDocument.NewElement(strData.c_str());
//
//				m_pDataElement->SetAttribute("EventName", AnimInfo.vecEventInfo[i].strEventName.c_str());
//				m_pDataElement->SetAttribute("StartFrame", AnimInfo.vecEventInfo[i].iStartFrame);
//				m_pDataElement->SetAttribute("EndFrame", AnimInfo.vecEventInfo[i].iEndFrame);
//
//				m_pAnimElement->InsertEndChild(m_pDataElement);
//			}
//		}
//	}
//
//	tinyxml2::XMLPrinter printer;
//	m_xmlDocument.Print(&printer);
//
//	std::ofstream outFile("../Bin/Resources/Data/AnimationData.xml");
//	outFile << printer.CStr();
//	outFile.close();
//}

//void CAnimToolHelper::Save()
//{
//	tinyxml2::XMLDocument m_xmlDocument;
//	tinyxml2::XMLNode* m_pRootNode;
//
//	// 루트 요소 생성
//	m_pRootNode = m_xmlDocument.NewElement("Root");
//	m_xmlDocument.InsertFirstChild(m_pRootNode);
//
//	// 버전 요소 생성 및 추가
//	tinyxml2::XMLElement* m_pElement = m_xmlDocument.NewElement("Version");
//	m_pElement->SetText(240602);
//	m_pRootNode->InsertEndChild(m_pElement);
//
//	// 맵 시퀀스를 순회
//	for (const auto& [ModelName, AnimMap] : m_mapSequence)
//	{
//		// ModelName 요소 생성 및 추가
//		tinyxml2::XMLElement* m_pModelElement = m_xmlDocument.NewElement("ModelName");
//		m_pModelElement->SetText(ModelName.c_str());
//		m_pRootNode->InsertEndChild(m_pModelElement);
//
//		// 현재 모델의 애니메이션 맵을 순회
//		for (const auto& [AnimName, AnimInfo] : AnimMap)
//		{
//			// ModelName 하위에 Animation 요소 생성 및 추가
//			tinyxml2::XMLElement* m_pAnimElement = m_xmlDocument.NewElement("Animation");
//			m_pAnimElement->SetText(AnimName.c_str());
//			m_pModelElement->InsertEndChild(m_pAnimElement);
//
//			// ModelName 하위에 AnimSpeed 요소 생성 및 추가
//			m_pElement = m_xmlDocument.NewElement("AnimSpeed");
//			m_pElement->SetText(AnimInfo.fAnimSpeed);
//			m_pModelElement->InsertEndChild(m_pElement);
//
//			// ModelName 하위에 Count 요소 생성 및 추가
//			m_pElement = m_xmlDocument.NewElement("Count");
//			_uint iCnt = AnimInfo.vecEventInfo.size();
//			m_pElement->SetText(iCnt);
//			m_pModelElement->InsertEndChild(m_pElement);
//
//			// 현재 애니메이션의 이벤트 정보를 순회
//			for (_uint i = 0; i < iCnt; ++i)
//			{
//				string strData = "Data" + to_string(i);
//				m_pElement = m_xmlDocument.NewElement(strData.c_str());
//
//				// Data 요소에 속성 추가
//				string strTemp = AnimInfo.vecEventInfo[i].strEventName;
//				m_pElement->SetAttribute("EventName", strTemp.c_str());
//
//				string strStartFrame = to_string(AnimInfo.vecEventInfo[i].iStartFrame);
//				m_pElement->SetAttribute("StartFrame", strStartFrame.c_str());
//
//				string strEndFrame = to_string(AnimInfo.vecEventInfo[i].iEndFrame);
//				m_pElement->SetAttribute("EndFrame", strEndFrame.c_str());
//
//				// Data 요소를 ModelName 하위에 추가
//				m_pModelElement->InsertEndChild(m_pElement);
//			}
//		}
//	}
//
//	// XML 문서를 파일로 저장
//	tinyxml2::XMLError error = m_xmlDocument.SaveFile("../Bin/Resources/Data/AnimationData.xml");
//}


void CAnimToolHelper::Save()
{
	tinyxml2::XMLDocument m_xmlDocument;
	tinyxml2::XMLNode* m_pRootNode;

	// 루트 요소 생성
	m_pRootNode = m_xmlDocument.NewElement("Root");
	m_xmlDocument.InsertFirstChild(m_pRootNode);

	// 버전 요소 생성 및 추가
	tinyxml2::XMLElement* m_pElement = m_xmlDocument.NewElement("Version");
	m_pElement->SetText(240602);
	m_pRootNode->InsertEndChild(m_pElement);

	// 맵 시퀀스를 순회
	for (const auto& [ModelName, AnimMap] : m_mapSequence)
	{
		// ModelName 요소 생성 및 추가
		tinyxml2::XMLElement* m_pModelElement = m_xmlDocument.NewElement("ModelName");

		// 개행 문자 추가
		string strModelName = ModelName + "\n";
		m_pModelElement->SetText(strModelName.c_str());
		m_pRootNode->InsertEndChild(m_pModelElement);

		// 현재 모델의 애니메이션 맵을 순회
		for (const auto& [AnimName, AnimInfo] : AnimMap)
		{
			// ModelName 하위에 Animation 요소 생성 및 추가
			tinyxml2::XMLElement* m_pAnimElement = m_xmlDocument.NewElement("Animation");
			m_pAnimElement->SetText(AnimName.c_str());
			m_pModelElement->InsertEndChild(m_pAnimElement);

			// ModelName 하위에 AnimSpeed 요소 생성 및 추가
			m_pElement = m_xmlDocument.NewElement("AnimSpeed");
			m_pElement->SetText(AnimInfo.fAnimSpeed);
			m_pModelElement->InsertEndChild(m_pElement);

			// ModelName 하위에 Count 요소 생성 및 추가
			m_pElement = m_xmlDocument.NewElement("Count");
			_uint iCnt = AnimInfo.vecEventInfo.size();
			m_pElement->SetText(iCnt);
			m_pModelElement->InsertEndChild(m_pElement);

			// 현재 애니메이션의 이벤트 정보를 순회
			for (_uint i = 0; i < iCnt; ++i)
			{
				string strData = "Data" + to_string(i);
				m_pElement = m_xmlDocument.NewElement(strData.c_str());

				// Data 요소에 속성 추가
				string strTemp = AnimInfo.vecEventInfo[i].strEventName;
				m_pElement->SetAttribute("EventName", strTemp.c_str());

				string strStartFrame = to_string(AnimInfo.vecEventInfo[i].iStartFrame);
				m_pElement->SetAttribute("StartFrame", strStartFrame.c_str());

				string strEndFrame = to_string(AnimInfo.vecEventInfo[i].iEndFrame);
				m_pElement->SetAttribute("EndFrame", strEndFrame.c_str());

				// Data 요소를 ModelName 하위에 추가
				m_pModelElement->InsertEndChild(m_pElement);
			}
		}
	}

	// XML 문서를 파일로 저장
	tinyxml2::XMLError error = m_xmlDocument.SaveFile("../Bin/Resources/Data/AnimationData.xml");
}

//void CAnimToolHelper::Load() 
//{   
//	// XML 파일을 읽어올 경로 설정
//	const char* filePath = "../Bin/Resources/Data/AnimationData.xml";
//
//	// XMLDocument 객체 생성
//	tinyxml2::XMLDocument m_xmlDocument;
//
//	// XML 파일 로드
//	if (m_xmlDocument.LoadFile(filePath) != tinyxml2::XML_SUCCESS) 
//	{
//		MSG_BOX(TEXT("Failed to load XML file"));
//		return;
//	}
//
//	// 루트 요소 가져오기
//	tinyxml2::XMLElement* pRoot = m_xmlDocument.RootElement();
//
//	// ModelName 요소 순회
//	for (tinyxml2::XMLElement* pModelNameElement = pRoot->FirstChildElement("ModelName");
//		pModelNameElement != nullptr;
//		pModelNameElement = pModelNameElement->NextSiblingElement("ModelName")) 
//	{
//
//		// ModelName 가져오기
//		const char* modelName = pModelNameElement->GetText();
//		// 문자열 끝 부분의 공백 제거
//		string modelNameStr(modelName);
//		modelNameStr.erase(std::find_if(modelNameStr.rbegin(), modelNameStr.rend(), [](unsigned char ch) {
//			return !std::isspace(ch);
//			}).base(), modelNameStr.end());
//		
//		//if (modelName)
//		if (!modelNameStr.empty())
//		{
//			// Animation 정보를 담을 임시 맵
//			unordered_map<string, ANIM_INFO> animMap;
//
//			// Animation 요소 순회
//			for (tinyxml2::XMLElement* pAnimationElement = pModelNameElement->FirstChildElement("Animation");
//				pAnimationElement != nullptr;
//				pAnimationElement = pAnimationElement->NextSiblingElement("Animation")) {
//
//				// Animation 가져오기
//				const char* animationName = pAnimationElement->GetText();
//				if (animationName) 
//				{
//					// AnimSpeed 가져오기
//					tinyxml2::XMLElement* pAnimSpeedElement = pAnimationElement->FirstChildElement("AnimSpeed");
//					float animSpeed = 0.0f;
//					if (pAnimSpeedElement) 
//					{
//						pAnimSpeedElement->QueryFloatText(&animSpeed);
//					}
//
//					// Count 가져오기
//					tinyxml2::XMLElement* pCountElement = pAnimationElement->FirstChildElement("Count");
//					int count = 0;
//					if (pCountElement) 
//					{
//						pCountElement->QueryIntText(&count);
//					}
//
//					// Event 정보를 담을 벡터
//					vector<EVENT_INFO> eventInfoVec;
//
//					// Data 요소 순회
//					for (tinyxml2::XMLElement* pDataElement = pAnimationElement->FirstChildElement();
//						pDataElement != nullptr;
//						pDataElement = pDataElement->NextSiblingElement()) 
//					{
//
//						const char* eventName = pDataElement->Attribute("EventName");
//						int startFrame = 0, endFrame = 0;
//						pDataElement->QueryIntAttribute("StartFrame", &startFrame);
//						pDataElement->QueryIntAttribute("EndFrame", &endFrame);
//
//						if (eventName) 
//						{
//							eventInfoVec.push_back({ eventName, startFrame, endFrame });
//						}
//					}
//
//					// Animation 정보를 맵에 추가
//					animMap.emplace(animationName, ANIM_INFO{ animSpeed, eventInfoVec });
//				}
//			}
//
//			// ModelName과 해당 Animation 정보를 m_mapSequence에 추가
//			m_mapSequence.emplace(modelNameStr, std::move(animMap));
//		}
//	}
//}

void CAnimToolHelper::Load()
{
	// XML 파일을 읽어올 경로 설정
	const char* filePath = "../Bin/Resources/Data/AnimationData.xml";

	// XMLDocument 객체 생성
	tinyxml2::XMLDocument m_xmlDocument;

	// XML 파일 로드
	if (m_xmlDocument.LoadFile(filePath) != tinyxml2::XML_SUCCESS)
	{
		MSG_BOX(TEXT("Failed to load XML file"));
		return;
	}

	// 루트 요소 가져오기
	tinyxml2::XMLElement* pRoot = m_xmlDocument.RootElement();

	// ModelName 및 Animation 정보를 읽어옴
	for (tinyxml2::XMLElement* pModelElement = pRoot->FirstChildElement("ModelName");
		pModelElement != nullptr;
		pModelElement = pModelElement->NextSiblingElement("ModelName"))
	{

		// 문자열 끝 부분의 공백 제거
		string modelNameStr(pModelElement->GetText());
		modelNameStr.erase(std::find_if(modelNameStr.rbegin(), modelNameStr.rend(), [](_ubyte ch) {
			return !std::isspace(ch);
			}).base(), modelNameStr.end());
		
		if (!modelNameStr.empty())
		{
			// ModelName에 해당하는 AnimMap을 생성
			AnimToolMap::mapped_type& animMap = m_mapSequence[string(modelNameStr)];

			// Animation 정보 읽기
			for (tinyxml2::XMLElement* pAnimElement = pModelElement->FirstChildElement("Animation");
				pAnimElement != nullptr;
				pAnimElement = pAnimElement->NextSiblingElement("Animation"))
			{
				const char* animName = pAnimElement->GetText();
				if (animName)
				{
					// ANIM_INFO 객체 생성 및 초기화
					ANIM_INFO animInfo;

					// AnimSpeed 읽기
					tinyxml2::XMLElement* pAnimSpeedElement = pAnimElement->NextSiblingElement("AnimSpeed");
					if (pAnimSpeedElement)
					{
						_float animSpeed;
						pAnimSpeedElement->QueryFloatText(&animSpeed);
						animInfo.fAnimSpeed = animSpeed;
					}

					// Count 값 읽기
					tinyxml2::XMLElement* pCountElement = pAnimElement->NextSiblingElement("Count");
					if (pCountElement)
					{
						_uint count;
						pCountElement->QueryUnsignedText(&count);

						// Event 정보 읽기
						for (unsigned int i = 0; i < count; ++i)
						{
							std::string dataName = "Data" + std::to_string(i);
							tinyxml2::XMLElement* pDataElement = pCountElement->NextSiblingElement(dataName.c_str());
							if (pDataElement)
							{
								EVENT_INFO eventInfo;

								// EventName, StartFrame, EndFrame 읽기
								const char* eventName = pDataElement->Attribute("EventName");
								int startFrame, endFrame;
								pDataElement->QueryIntAttribute("StartFrame", &startFrame);
								pDataElement->QueryIntAttribute("EndFrame", &endFrame);

								eventInfo.strEventName = eventName ? eventName : "";
								eventInfo.iStartFrame = startFrame;
								eventInfo.iEndFrame = endFrame;

								// ANIM_INFO의 vecEventInfo에 추가
								animInfo.vecEventInfo.push_back(eventInfo);
							}
						}
					}

					// ANIM_INFO 객체를 AnimMap에 추가
					animMap[string(animName)] = animInfo;
				}
			}
		}
	}
}

//void CAnimToolHelper::Load(const string& FileName)
//{
//	tinyxml2::XMLDocument	m_xmlDocument;
//	tinyxml2::XMLNode* m_pNode;
//	tinyxml2::XMLElement* m_pElement;
//
//	tinyxml2::XMLError error = m_xmlDocument.LoadFile("../Bin/Resources/Data/AnimationData.xml");
//	m_pNode = m_xmlDocument.FirstChild();
//
//	_int	iRead, iCnt, iEA;
//
//	m_pElement = m_pNode->FirstChildElement("Version");
//	m_pElement->QueryIntText(&iRead);
//	m_pElement = m_pNode->FirstChildElement("Count");
//	m_pElement->QueryIntText(&iCnt);
//
//	//모델 이름 읽어오기
//	string ModelName;
//
//	//모델에 있는 애니메이션 배열 들고오기
//	//애니메이션 이름 읽어오기
//	string AnimName;
//
//	for (_uint i = 0; i < iCnt; ++i)
//	{
//		string strData = "Data" + to_string(i);
//		m_pElement = m_pNode->FirstChildElement(strData.c_str());
//
//		EVENT_INFO Info{};
//		Info.strEventName = m_pElement->Attribute("EventName");
//		Info.iStartFrame = stoi(m_pElement->Attribute("StartFrame"));
//		Info.iEndFrame = stoi(m_pElement->Attribute("EndFrame"));
//		m_pElement->QueryIntText(&iEA);
//
//		//m_vecSequence.push_back(Info);
//	}
//}

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

	Safe_Release(m_pAnimToolObj);
	Safe_Release(m_pModel);
	m_vecAnimModels.clear();
}

