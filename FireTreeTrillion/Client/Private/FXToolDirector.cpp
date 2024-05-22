#include "stdafx.h"
#include "FXToolDirector.h"
#include "GameInstance.h"

#include "SingleEffect.h"
#include "MultiEffect.h"
#include "Camera_Free.h"

#define COLOR_BLUE				ImVec4(0.26f, 0.59f, 0.98f, 0.40f)
#define COLOR_ORANGE			ImVec4(1.0f, 0.5f, 0.0f, 1.0f)
#define COLOR_LIGHTYELLOW		ImVec4(1.0f, .8f, 0.0f, 1.0f)

#define COLOR_DARKPINK			ImVec4(0.6f, 0.18f, 0.37f, 1.0f)
#define COLOR_PINK				ImVec4(0.8f, 0.18f, 0.37f, 1.0f)
#define COLOR_LIGHTPINK			ImVec4(1.0f, 0.18f, 0.37f, 1.0f)

CFXToolDirector::CFXToolDirector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{ pDevice, pContext }
{
}

CFXToolDirector::CFXToolDirector(const CFXToolDirector& rhs)
	:CGameObject{ rhs }
{
}

void CFXToolDirector::Make_Effect(SINGLE_FX_DATA& EffectData)
{
}

void CFXToolDirector::Make_Effect(MULTI_FX_DATA& EffectData)
{
}

CEffect* CFXToolDirector::Find_Effect(string strEffectName)
{
	return nullptr;
}

HRESULT CFXToolDirector::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFXToolDirector::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};

	if (nullptr == pArg)
	{
		GameObjectDesc.fSpeedPerSec = 5.f;
		GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	}
	else
	{
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;
	}

	//if (FAILED(__super::Initialize(&GameObjectDesc)))
	//	return E_FAIL;


	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED_MSG(hr, "Failed To Initialize : CFXToolDirector");


	hr = Add_Components();
	CHECK_FAILED_MSG(hr, "Failed To Add Components : CFXToolDirector");

	hr = Ready_FXPrototypeVector();
	CHECK_FAILED_MSG(hr, "Failed To Add Components : CFXToolDirector");

	//SetupImGuiStyle(true, .8f);

	return S_OK;
}

_int CFXToolDirector::Tick(_float _fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CFXToolDirector::Late_Tick(_float _fTimeDelta)
{
	Matrix viewMat = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW);
	Matrix projMat = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	Matrix identityMat = XMMatrixIdentity();

	m_pGameInstance->RenderGrid();

	Render_FXHierarchy();
	Render_FXProperty(_fTimeDelta);
	//Render_FXPlayBar(_fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CFXToolDirector::Render()
{

	if (m_eSelected == SELECTED_SINGLE_FX || m_eSelected == SELECTED_PARTICLE_FX)
		m_FXs[m_iSelectedFXIdx]->Render();
	else if (m_eSelected == SELECTED_MULTI_FX)
		m_MultiFXs[m_iSelectedMultiFXIdx]->Render();

	return S_OK;
}

void CFXToolDirector::Render_IMGUI()
{
	__super::Render_IMGUI();
}

void CFXToolDirector::Render_FXHierarchy()
{
	Begin(u8"만들기");

	Combo(u8"디퓨즈 텍스쳐", &m_iAddingFXTexIdx, m_FXTexList.data(), (_int)m_FXTexList.size());
	Combo(u8"마스크 텍스쳐", &m_iAddingFXMaskTexIdx, m_FXMaskTexList.data(), (_int)m_FXMaskTexList.size());

	Separator();
	Columns(2);

	Combo(u8"버퍼", &m_iAddingFXBufferIdx, m_FXBufferList.data(), (_int)m_FXBufferList.size());

	if (Button(u8"이펙트 생성"))
	{
		//m_FXs.emplace_back();

		CSingleEffect::FX_DESC singleFXDesc{};
		string strComponentTag = "Prototype_Component_";

		string strBaseName{"Default FX"};
		switch (m_iAddingFXBufferIdx)
		{
		case 1:
			break;
		default:
			strBaseName = "Rect FX ";
			//singleFXDesc.strBufferTag = "Rect";
			break;
		}

		//default 이펙트 이름 뒤에 알파벳을 붙인다.
		char szSuffix = 'A';
		while (true)
		{
			_bool bDoesExistSameName{ false };
			singleFXDesc.strFXName = strBaseName + szSuffix;

			//중복 이름 있으면 안됨
			for (const auto& fx : m_FXs)
			{
				if (fx->m_strFXName == singleFXDesc.strFXName)
				{
					bDoesExistSameName = true;
					break;
				}
			}

			//중복 이름이 없거나, 알파벳이 초과하면 반복 끝
			if (!bDoesExistSameName || 'Z' <= szSuffix)
				break;

			++szSuffix;
		}


		singleFXDesc.strBufferTag = strComponentTag + m_FXBufferList[m_iAddingFXBufferIdx];
		singleFXDesc.strTexTag = strComponentTag + m_FXTexList[m_iAddingFXTexIdx];
		singleFXDesc.strMaskTexTag = strComponentTag + m_FXMaskTexList[m_iAddingFXMaskTexIdx];

		CSingleEffect* pSingleFX = static_cast<CSingleEffect*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_CSingleEffect"), &singleFXDesc));
		m_FXs.emplace_back(pSingleFX);
	}


	NextColumn();

	InputInt(u8"갯수", &m_iAddingInstanceNum, 1, 500);

	if (Button(u8"파티클 생성"))
	{

	}

	End();

	
	Begin(u8"편집하기");

	BeginChild(u8"목록");
	for (_int i = 0; i < m_FXs.size(); ++i)
	{
		//if (m_iSelectedFXIdx == i)
		//	PushStyleColor(ImGuiCol_Text, COLOR_ORANGE);
		//else
		//	PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
		

		if (Selectable(m_FXs[i]->m_strFXName.c_str(), m_iSelectedFXIdx == i))
		{
			m_iSelectedFXIdx = i;
			m_eSelected = SELECTED_SINGLE_FX;
		}

		//PopStyleColor();
	}
	EndChild();

	End();
	

	//Begin("Add to MultiFX?", nullptr, ImGuiWindowFlags_NoCollapse);
	//if (Button(u8"Yes Yes Yes")/* && m_iSelectedCompositionEffectIdx != -1 && m_iSelectedEffectIdx != -1*/)
	//{
	//	//m_CompositeEffects[m_iSelectedCompositionEffectIdx]->Add_Effect(m_EditEffects[m_iSelectedEffectIdx]);
	//	//bOpenAddPopup = false;
	//}

	//End();
}

void CFXToolDirector::Render_FXProperty(_float _fTimeDelta)
{
	if (m_iSelectedFXIdx == -1)
		return;

	//이펙트 기본 변수 세팅
	Begin(u8"속성 편집", nullptr, ImGuiWindowFlags_NoCollapse);


	CEffect* pCurFX = m_FXs[m_iSelectedFXIdx];
	_bool bIsParticle = _bool{ dynamic_cast<CSingleEffect*>(pCurFX) == nullptr };

	char tempBuf[256];
	strncpy_s(tempBuf, pCurFX->m_strFXName.c_str(), sizeof(tempBuf));
	tempBuf[sizeof(tempBuf) - 1] = 0;

	//이름
	if (InputText("Name", tempBuf, sizeof(tempBuf)))
	{
		if (tempBuf[0] == '\0')
			strcpy_s(tempBuf, "Default");

		m_curFXName = string(tempBuf);
		pCurFX->m_strFXName = m_curFXName;
	}

	//루프, 빌보드
	Checkbox(u8"루프", &pCurFX->m_bIsLoop);
	Checkbox(u8"빌보딩", &pCurFX->m_bIsBillboard);
	if (Checkbox(u8"블룸", &pCurFX->m_bIsBloom))
	{
		pCurFX->m_bIsBloom ?
			pCurFX->m_bIsColorRender = false :
			pCurFX->m_bIsColorRender = true;
	}

	if(!bIsParticle)
		Checkbox(u8"직교", &pCurFX->m_bIsOrthographic);


	//전체 시간
	if (DragFloat(u8"재생 시간", &pCurFX->m_fDuration.second, .1f, 0.f, 30.f, "%.2f"))
	{
		m_fTotalPlayDuration = pCurFX->m_fDuration.second;
	}

	//이펙트의 재생 수명
	if (DragFloat2(u8"수명", m_fLifeTime, .1f, 0.f, pCurFX->m_fDuration.second, "%.2f"))
	{
		memcpy(&pCurFX->m_fLifeTime, m_fLifeTime, sizeof(_float2));
		//pCurFX->m_fLifeTime = m_fLifeTime;
	}

	InputInt(u8"렌더 패스", &pCurFX->m_iPassIdx, 0, 10);
	InputInt(u8"디퓨즈 인덱스", &pCurFX->m_iTexIdx, 0, pCurFX->m_iMaxTexIdx);
	InputInt(u8"마스크 인덱스", &pCurFX->m_iMaskTexIdx, 0, pCurFX->m_iMaxMaskTexIdx);

	
	if (DragFloat3(u8"시작 위치", m_vEditPos, .01f, -50.f, 50.f, "%.2f"))
	{
		memcpy(&pCurFX->m_vInitPos, m_vEditPos, sizeof(_float3));
	}
	if (DragFloat3(u8"시작 회전", m_vEditRot, .1f, -180.f, 180.f, "%.2f"))
	{
		memcpy(&pCurFX->m_vInitRot, m_vEditRot, sizeof(_float3));
	}
	if (DragFloat3(u8"시작 크기", m_vEditScale, .05f, .01f, 100.f, "%.2f"))
	{
		memcpy(&pCurFX->m_vInitScale, m_vEditScale, sizeof(_float3));
	}

	if (bIsParticle)
	{
		DragFloat3(u8"범위", m_fRange, .01f, -100.f, 100.f, "%.2f");
	}



	End();
}

void CFXToolDirector::Render_FXPlayBar(_float _fTimeDelta)
{
	/*
	ImGuiIO& io = GetIO();
	ImVec2 windowSize = ImVec2(500, 100);
	ImVec2 windowPos = ImVec2(
		(io.DisplaySize.x - windowSize.x) * 0.5f,
		(io.DisplaySize.y - windowSize.y) - windowSize.y);
	SetNextWindowPos(windowPos);
	*/

	Begin(u8"Bar", nullptr, ImGuiWindowFlags_NoTitleBar);

	if (m_bPlayingBar)
		PushStyleColor(ImGuiCol_Button, COLOR_ORANGE);
	else
		PushStyleColor(ImGuiCol_Button, COLOR_LIGHTPINK);

	//재생
	if (Button(u8"Play") || m_pGameInstance->Get_KeyState(DIK_SPACE, KEY_DOWN))
	{
		m_bPlayingBar = !m_bPlayingBar;

		//if(m_iSelectedMultiFXIdx != -1)

		//재생 다 끝났을 시 duration 0
		if (m_bPlayingBar && m_fCurPlayDuration >= m_fTotalPlayDuration)
			m_fCurPlayDuration = 0.f;

		SameLine();
	}

	PopStyleColor();

	SameLine();

	if (Button(u8"Restart"))
	{
		m_bPlayingBar = true;
		m_fCurPlayDuration = 0.f;
	}

	//재생 바 폭 맞추기

	Text(u8"Duration");
	//SameLine();

	_float fWidth = GetContentRegionAvail().x;
	PushItemWidth(fWidth);

	if (SliderFloat("##", &m_fCurPlayDuration, 0.f, m_fTotalPlayDuration, "%.2f"))
	{

	}

	PopItemWidth();

	vector<FX_KEYFRAME> testKeyframes;
	testKeyframes.push_back(FX_KEYFRAME{ .2f });
	testKeyframes.push_back(FX_KEYFRAME{ .6f });

	//Separator();

	Dummy(ImVec2(0, 10));
	Text(u8"위치");
	SameLine();

		ImVec2 vPos = GetCursorScreenPos();
		vPos.y += 2.f;
	for (auto& keyframe : testKeyframes)
	{
		_float fPosX = keyframe.fTimeRatio /*/ m_EditEffects[m_iSelectedEffectIdx]->m_fDestDuration)*/ * fWidth;

		vPos.x += fPosX;
		GetWindowDrawList()->AddCircleFilled(vPos, 6.0f, IM_COL32(255, 255, 100, 255));
	}

	Spacing();

	Text(u8"회전");
	SameLine();
	testKeyframes.clear();
	testKeyframes.push_back(FX_KEYFRAME{ .4f });
	testKeyframes.push_back(FX_KEYFRAME{ .9f });
	vPos = GetCursorScreenPos();
	vPos.y += 2.f;
	for (auto& keyframe : testKeyframes)
	{
		_float fPosX = keyframe.fTimeRatio /*/ m_EditEffects[m_iSelectedEffectIdx]->m_fDestDuration)*/ * fWidth;

		ImVec2 vPos = GetCursorScreenPos();
		vPos.x += fPosX;
		GetWindowDrawList()->AddCircleFilled(vPos, 6.0f, IM_COL32(255, 255, 100, 255));
	}

	Spacing();

	Text(u8"크기");
	SameLine();
	testKeyframes.clear();
	testKeyframes.push_back(FX_KEYFRAME{ .5f });
	vPos = GetCursorScreenPos();
	vPos.y += 2.f;
	for (auto& keyframe : testKeyframes)
	{
		_float fPosX = keyframe.fTimeRatio /*/ m_EditEffects[m_iSelectedEffectIdx]->m_fDestDuration)*/ * fWidth;

		vPos.x += fPosX;
		GetWindowDrawList()->AddCircleFilled(vPos, 6.0f, IM_COL32(255, 255, 100, 255));
	}



	if (m_pGameInstance->Get_KeyState(DIK_1, KEY_DOWN))
		m_eSelected = SELECTED_SINGLE_FX;
	else if (m_pGameInstance->Get_KeyState(DIK_2, KEY_DOWN))
		m_eSelected = SELECTED_PARTICLE_FX;
	else if (m_pGameInstance->Get_KeyState(DIK_3, KEY_DOWN))
		m_eSelected = SELECTED_MULTI_FX;


	if (m_eSelected == SELECTED_SINGLE_FX /*&& m_iSelectedFXIdx != -1*/)
	{
		MakeBar_SingleFXProperty(_fTimeDelta);
	}
	else if (m_eSelected == SELECTED_PARTICLE_FX /*&& m_iSelectedFXIdx != -1*/)
	{
		MakeBar_ParticleFXProperty(_fTimeDelta);
	}
	else if (m_eSelected == SELECTED_MULTI_FX/* && m_iSelectedMultiFXIdx != -1*/)
	{
		MakeBar_MultiFXProperty(_fTimeDelta);
	}

	if (m_bPlayingBar)
	{
		m_fCurPlayDuration += _fTimeDelta;

		if (m_fTotalPlayDuration <= m_fCurPlayDuration)
		{
			m_fCurPlayDuration = m_bLoopingBar ? 0.f : m_fTotalPlayDuration;
			m_bPlayingBar = m_bLoopingBar ? true : false;
		}
	}

	End();
}

void CFXToolDirector::MakeBar_SingleFXProperty(_float _fTimeDelta)
{
	//Text
	if (SliderFloat("##", &m_fCurPlayDuration, 0.f, m_fTotalPlayDuration, "%.2f"))
	{

	}
}




void CFXToolDirector::MakeBar_ParticleFXProperty(_float _fTimeDelta)
{
}

void CFXToolDirector::MakeBar_MultiFXProperty(_float _fTimeDelta)
{
}

void CFXToolDirector::Render_MultiFXHierarchy()
{
}


HRESULT CFXToolDirector::Ready_FXPrototypeVector()
{
	CComponent_Manager::PROTOTYPES* pStaticProtoMap{ nullptr };

	pStaticProtoMap = m_pGameInstance->Get_ComMap(LEVEL_STATIC);
	if (nullptr == pStaticProtoMap)
		return E_FAIL;

	Ready_Ingredient(TEXT("FXTexture"), &m_FXTexList, pStaticProtoMap);
	Ready_Ingredient(TEXT("FXTexture"), &m_FXMaskTexList, pStaticProtoMap);

	Ready_Ingredient(TEXT("VIBuffer"), &m_FXBufferList, pStaticProtoMap);

	return S_OK;
}

void CFXToolDirector::Ready_Ingredient(wstring wstrSearchTag, vector<char*>* vecCombo, CComponent_Manager::PROTOTYPES* comMap)
{
	for (auto& comPair : *comMap)
	{
		if (comPair.first.find(wstrSearchTag) != wstring::npos)
		{
			string strComName = CUtils::WstrToStr(comPair.first);
			strComName = string(strComName.begin() + 20, strComName.end());
			char* tempStr = new char[strComName.size() + 1];
			strcpy_s(tempStr, strComName.size() + 1, strComName.c_str());
			vecCombo->push_back(tempStr);
		}
	}
}

HRESULT CFXToolDirector::Add_Components()
{
	return S_OK;
}

CFXToolDirector* CFXToolDirector::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFXToolDirector* pInstance = new CFXToolDirector(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CFXToolDirector"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFXToolDirector::Clone(void* pArg)
{
	CFXToolDirector* pInstance = new CFXToolDirector(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : CFXToolDirector"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFXToolDirector::Free()
{
	for (auto& pFX : m_FXs)
		Safe_Release(pFX);

	m_FXs.clear();

	for (auto& pFX : m_MultiFXs)
		Safe_Release(pFX);

	m_MultiFXs.clear();

	__super::Free();
}
