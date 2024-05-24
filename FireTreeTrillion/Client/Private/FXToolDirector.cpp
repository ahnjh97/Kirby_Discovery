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
	if (m_bPlayingBar)
	{
		if (m_eSelected == SELECTED_SINGLE_FX)
		{
			m_FXs[m_iSelectedFXIdx]->Tick(_fTimeDelta);
			m_fCurPlayDuration = m_FXs[m_iSelectedFXIdx]->m_fDuration.first;
		}
		else if (m_eSelected == SELECTED_MULTI_FX)
		{
			m_MultiFXs[m_iSelectedFXIdx]->Tick(_fTimeDelta);
			m_fCurPlayDuration = m_MultiFXs[m_iSelectedFXIdx]->m_fDuration.first;
		}
	}

	return OBJ_NOEVENT;
}

void CFXToolDirector::Late_Tick(_float _fTimeDelta)
{
	Matrix viewMat = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW);
	Matrix projMat = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	Matrix identityMat = XMMatrixIdentity();

	m_pGameInstance->RenderGrid();

	Render_FXHierarchy();
	Render_FXPlayBar(_fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CFXToolDirector::Render()
{
	//복합 이펙트 아니면 FX에 있는 이펙트 렌더
	if (m_eSelected == SELECTED_SINGLE_FX || m_eSelected == SELECTED_PARTICLE_FX)
		m_FXs[m_iSelectedFXIdx]->Render();
	//아니면 복합 이펙트 렌더
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

		//이름 정해주기
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

		//default 이펙트 이름 뒤에 중복 존재 시 알파벳을 붙인다.
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


		//버퍼, 텍스쳐, 마스크 텍스쳐 컴포넌트 이름 떤져준다.
		singleFXDesc.strBufferTag = strComponentTag + m_FXBufferList[m_iAddingFXBufferIdx];
		singleFXDesc.strTexTag = strComponentTag + m_FXTexList[m_iAddingFXTexIdx];
		singleFXDesc.strMaskTexTag = strComponentTag + m_FXMaskTexList[m_iAddingFXMaskTexIdx];

		//기본 세팅된 키프레임들.
		vector<FX_KEYFRAME> newProperty;
		FX_KEYFRAME newStartKeyframe{0.f, Vector3::Zero, EASE_OUT };
		FX_KEYFRAME newEndKeyframe{ 1.f, {2.f, 0.f, 0.f}, EASE_OUT };

		//위치
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);

		singleFXDesc.Keyframes.emplace(KF_POS, newProperty);

		//회전
		newProperty.clear();
		newStartKeyframe.vValue = Vector3::Zero;
		newEndKeyframe.vValue = Vector3::Zero;
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_ROT, newProperty);

		//크기
		newProperty.clear();
		newStartKeyframe.vValue = Vector3::One;
		newStartKeyframe.eEasing = EASE_IN;
		newEndKeyframe.vValue = {.01f, .01f, .01f};
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_SCALE, newProperty);


		//R
		newProperty.clear();
		newStartKeyframe.vValue = { 1.f, 0.f, 0.f };
		newEndKeyframe.vValue = { 1.f, 0.f, 0.f };
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_RCOLOR, newProperty);

		//G
		newProperty.clear();
		newStartKeyframe.vValue = { 0.f, 1.f, 0.f };
		newEndKeyframe.vValue = { 0.f, 1.f, 0.f };
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_GCOLOR, newProperty);

		//B
		newProperty.clear();
		newStartKeyframe.vValue = { 0.f, 0.f, 1.f };
		newEndKeyframe.vValue = { 0.f, 0.f, 1.f };
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_BCOLOR, newProperty);

		//B
		newProperty.clear();
		newStartKeyframe.vValue = Vector3::One;
		newEndKeyframe.vValue = Vector3::One;
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_ALPHA, newProperty);

		newProperty.clear();
		newStartKeyframe.vValue = Vector3::Zero;
		newEndKeyframe.vValue = Vector3::Zero;
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_MASK, newProperty);

		CSingleEffect* pSingleFX = static_cast<CSingleEffect*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_CSingleEffect"), &singleFXDesc));
		m_FXs.emplace_back(pSingleFX);
	}


	NextColumn();

	InputInt(u8"갯수", &m_iAddingInstanceNum, 1, 500);

	if (Button(u8"파티클 생성"))
	{
		m_bOpenKeyframeEditor = false;
	}

	End();

	//이펙트 초기 값과 키프레임 값을 편집한다.
	Begin(u8"편집하기");

	SeparatorText(u8"목록");
	BeginChild(u8"목록", ImVec2(0, 200), true);
	for (_int i = 0; i < m_FXs.size(); ++i)
	{

		if (Selectable(m_FXs[i]->m_strFXName.c_str(), m_iSelectedFXIdx == i))
		{
			m_bOpenKeyframeEditor = false;
			m_iSelectedFXIdx = i;
			m_eSelected = SELECTED_SINGLE_FX;

			m_bPlayingBar = false;
			m_bLooping = m_FXs[i]->m_bIsLoop;
			m_iCurFXPassIdx = m_FXs[i]->m_iPassIdx;
			m_iCurFXTexIdx = m_FXs[i]->m_iTexIdx;
			m_iCurFXMaskTexIdx = m_FXs[i]->m_iMaskTexIdx;

			m_fTotalPlayDuration = m_FXs[i]->m_fDuration.second;
			memcpy(m_fLifeTime, &m_FXs[i]->m_fLifeTime, sizeof(_float2));
		}
	}
	EndChild();

	Render_FXProperty();

	End();
	

	//Begin("Add to MultiFX?", nullptr, ImGuiWindowFlags_NoCollapse);
	//if (Button(u8"Yes Yes Yes")/* && m_iSelectedCompositionEffectIdx != -1 && m_iSelectedEffectIdx != -1*/)
	//{
	//	//m_CompositeEffects[m_iSelectedCompositionEffectIdx]->Add_Effect(m_EditEffects[m_iSelectedEffectIdx]);
	//	//bOpenAddPopup = false;
	//}

	//End();
}

void CFXToolDirector::Render_FXProperty()
{
	if (m_iSelectedFXIdx == -1)
		return;

	//이펙트 기본 변수 세팅
	//Begin(u8"속성 편집", nullptr, ImGuiWindowFlags_NoCollapse);

	SeparatorText(u8"속성 편집");
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
	SameLine();
	Checkbox(u8"빌보딩", &pCurFX->m_bIsBillboard);
	if (Checkbox(u8"블룸", &pCurFX->m_bIsBloom))
	{
		pCurFX->m_bIsBloom ?
			pCurFX->m_bIsColorRender = false :
			pCurFX->m_bIsColorRender = true;
	}
	SameLine();
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

	if (InputInt(u8"렌더 패스",  &m_iCurFXPassIdx, 1, pCurFX->m_iMaxPassIdx))
	{
		if (m_iCurFXPassIdx < 0)
			m_iCurFXPassIdx = 0;

		if(pCurFX->m_iMaxPassIdx < m_iCurFXPassIdx)
			m_iCurFXPassIdx = pCurFX->m_iMaxPassIdx;

		pCurFX->m_iPassIdx = m_iCurFXPassIdx;
	}

	if (InputInt(u8"디퓨즈 인덱스", &pCurFX->m_iTexIdx, 1, pCurFX->m_iMaxTexIdx))
	{
		//가능한 인덱스로 한 번 보정해 주기
		if (m_iCurFXTexIdx < 0)
			m_iCurFXTexIdx = 0;

		//if(pCurFX->m_iMaskTexIdx < m_iCurFXTexIdx)
		//	m_iCurFXTexIdx = pCurFX->m_iMaskTexIdx;

		pCurFX->m_iTexIdx = m_iCurFXTexIdx;
	}

	if (InputInt(u8"마스크 인덱스", &pCurFX->m_iMaskTexIdx, 1, pCurFX->m_iMaxMaskTexIdx))
	{
		//가능한 인덱스로 한 번 보정해 주기
		if (m_iCurFXMaskTexIdx < 0)
			m_iCurFXMaskTexIdx = 0;

		//if (pCurFX->m_iMaskTexIdx < m_iCurFXMaskTexIdx)
		//	m_iCurFXMaskTexIdx = pCurFX->m_iMaskTexIdx;

		pCurFX->m_iMaskTexIdx = m_iCurFXMaskTexIdx;
	}

	if (bIsParticle)
	{
		DragFloat3(u8"범위", m_fRange, .01f, -100.f, 100.f, "%.2f");
	}

}

//이펙트의 플레이 바를 띄운다.
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

	//아무것도 선택되지 않았다면 return
	if (m_eSelected == SELECTED_END)
		return;


 	Begin(u8"Bar", nullptr, ImGuiWindowFlags_NoTitleBar);

	CEffect* pCurFX{ nullptr };
	if (m_eSelected == SELECTED_SINGLE_FX || m_eSelected == SELECTED_PARTICLE_FX)
		pCurFX = m_FXs[m_iSelectedFXIdx];
	else if (m_eSelected == SELECTED_MULTI_FX)
		pCurFX = m_MultiFXs[m_iSelectedMultiFXIdx];

	CHECK_NULLPTR(pCurFX);

	Columns(2);
	SetColumnWidth(0, 190.f);

	Text("Lifetime %.2f", pCurFX->m_fLifeRatio);
	Dummy(ImVec2(0, 58));

	//플레이 바의 첫번째 열에 이펙트의 현재 속성 값을 띄운다.

	Text(u8"위치");
	SameLine();
	Text("%.2f %.2f %.2f", pCurFX->m_vCurPos.x, pCurFX->m_vCurPos.y, pCurFX->m_vCurPos.z);
	SameLine();

	if (SmallButton(u8"위치 추가") && ( 0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
	{
		m_bOpenKeyframeEditor = false;
		FX_KEYFRAME newKeyframe{};
		newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
		newKeyframe.eEasing = EASE_OUT;
		newKeyframe.vValue = pCurFX->m_vCurPos;

		pCurFX->Add_Keyframe(newKeyframe, KF_POS);
	}

	Text(u8"회전");
	SameLine();
	Text("%.2f %.2f %.2f", pCurFX->m_vCurRot.x, pCurFX->m_vCurRot.y, pCurFX->m_vCurRot.z);
	SameLine();

	if (SmallButton(u8"회전 추가") && (0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
	{
		m_bOpenKeyframeEditor = false;
		FX_KEYFRAME newKeyframe{};
		newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
		newKeyframe.eEasing = EASE_OUT;
		newKeyframe.vValue = pCurFX->m_vCurRot;

		pCurFX->Add_Keyframe(newKeyframe, KF_ROT);
	}

	Text(u8"크기");
	SameLine();
	Text("%.2f %.2f %.2f", pCurFX->m_vCurScale.x, pCurFX->m_vCurScale.y, pCurFX->m_vCurScale.z);
	SameLine();
	if (SmallButton(u8"크기 추가") && (0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
	{
		m_bOpenKeyframeEditor = false;
		FX_KEYFRAME newKeyframe{};
		newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
		newKeyframe.eEasing = EASE_OUT;
		newKeyframe.vValue = pCurFX->m_vCurScale;

		pCurFX->Add_Keyframe(newKeyframe, KF_SCALE);
	}

	NextColumn();

	if (m_bPlayingBar)
		PushStyleColor(ImGuiCol_Button, COLOR_ORANGE);
	else
		PushStyleColor(ImGuiCol_Button, COLOR_LIGHTPINK);

	//재생
	if (ArrowButton(u8"Play", ImGuiDir_Right) || m_pGameInstance->Get_KeyState(DIK_SPACE, KEY_DOWN))
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


	NewLine();

	Text(u8"Duration");

	//재생 바 폭 맞추기
	_float fWidth = GetContentRegionAvail().x;
	PushItemWidth(fWidth);


	if (m_pGameInstance->Get_KeyState(DIK_1, KEY_DOWN))
		m_eSelected = SELECTED_SINGLE_FX;
	else if (m_pGameInstance->Get_KeyState(DIK_2, KEY_DOWN))
		m_eSelected = SELECTED_PARTICLE_FX;
	else if (m_pGameInstance->Get_KeyState(DIK_3, KEY_DOWN))
		m_eSelected = SELECTED_MULTI_FX;


	if (m_eSelected == SELECTED_SINGLE_FX /*&& m_iSelectedFXIdx != -1*/)
	{
		MakeBar_SingleFXProperty(_fTimeDelta, fWidth);
	}
	else if (m_eSelected == SELECTED_PARTICLE_FX /*&& m_iSelectedFXIdx != -1*/)
	{
		MakeBar_ParticleFXProperty(_fTimeDelta, fWidth);
	}
	else if (m_eSelected == SELECTED_MULTI_FX/* && m_iSelectedMultiFXIdx != -1*/)
	{
		MakeBar_MultiFXProperty(_fTimeDelta, fWidth);
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

	PushItemWidth(fWidth);

	End();
}

void CFXToolDirector::MakeBar_SingleFXProperty(_float _fTimeDelta, _float _fWidth)
{
	if (m_iSelectedFXIdx == -1)
		return;

	CEffect* pCurFX = m_FXs[m_iSelectedFXIdx];
	CHECK_NULLPTR(pCurFX);


	if (SliderFloat("##", &m_fCurPlayDuration, 0.f, m_fTotalPlayDuration, "%.2f"))
	{
		m_FXs[m_iSelectedFXIdx]->m_fDuration.first = m_fCurPlayDuration;
	}

	//키프레임 편집 창을 띄워주는 static 변수들
	static _bool		bMakeKFPopupToFront = { false };
	static KF_PROPERTY	eSelectedProperty = { KF_END };
	static _int			iSelectedKFIdx = { -1 };

	//키프레임 팝업 사이즈
	ImVec2 vPopupSize = { 220.f, 140.f };

	//서로 서로 띄워준다.
	Dummy(ImVec2(0, 15));

	ImVec2 vPos = GetCursorScreenPos();
	_float fInitialYPos = vPos.y - 250.f;

	_int iTempKFIdx{ 0 };
	ImDrawList* pDrawList = GetWindowDrawList();
	
	pDrawList->AddLine(vPos, ImVec2(vPos.x + _fWidth, vPos.y), IM_COL32(255, 0, 100, 255), 1.f);

	//각 키프레임 플레이 바의 위치의 상대 위치로 매칭하기.

	//위치
	for (auto& keyframe : pCurFX->m_Keyframes[KF_POS])
	{
		_float fRatio = (pCurFX->m_fLifeTime.first + (keyframe.fTimeRatio * (pCurFX->m_fLifeTime.second - pCurFX->m_fLifeTime.first)));
		_float fPosX = fRatio * _fWidth / pCurFX->m_fDuration.second;
		ImVec2 vCurPos = vPos + ImVec2{ fPosX, 2.f };

		GetWindowDrawList()->AddCircleFilled(vCurPos, 6.0f, IM_COL32(255, 255, 100, 255));

		ImVec2 mousePos = GetIO().MousePos;
		float fDistance = (_float)sqrt(pow(mousePos.x - vCurPos.x, 2) + pow(mousePos.y - vCurPos.y, 2));

		//키프레임 범위 안에서 마우스 클릭 발생 시 키프레임 위치에 ui 띄움
		if (fDistance <= 8.0f && IsMouseClicked(0))
		{
			m_bOpenKeyframeEditor = true;
			bMakeKFPopupToFront = true;
			eSelectedProperty = KF_POS;
			iSelectedKFIdx = iTempKFIdx;

			//위치, 스케일 키프레임 옆에 맞춘다
			SetNextWindowSize(vPopupSize);
			SetNextWindowPos(ImVec2(vCurPos.x, fInitialYPos));

			_float3 vValue = m_FXs[m_iSelectedFXIdx]->m_Keyframes[eSelectedProperty][iSelectedKFIdx].vValue;
			memcpy(m_vKFPopupValue, &vValue, sizeof(_float3));
			m_eKFPopupEasing = m_FXs[m_iSelectedFXIdx]->m_Keyframes[eSelectedProperty][iSelectedKFIdx].eEasing;
			//m_vKFPopupValue
		}

		++iTempKFIdx;
	}


	Dummy(ImVec2(0, 15));


	iTempKFIdx = 0;

	//회전
	vPos = GetCursorScreenPos();

	pDrawList->AddLine(vPos, ImVec2(vPos.x + _fWidth, vPos.y), IM_COL32(255, 0, 100, 255), 1.f);


	for (auto& keyframe : pCurFX->m_Keyframes[KF_ROT])
	{
		//재생 위치로 cur pos 맞추기
		_float fRatio = (pCurFX->m_fLifeTime.first + (keyframe.fTimeRatio * (pCurFX->m_fLifeTime.second - pCurFX->m_fLifeTime.first)));
		_float fPosX = fRatio * _fWidth / pCurFX->m_fDuration.second;
		ImVec2 vCurPos = vPos + ImVec2{ fPosX, 2.f };

		GetWindowDrawList()->AddCircleFilled(vCurPos, 6.0f, IM_COL32(255, 255, 100, 255));

		ImVec2 mousePos = GetIO().MousePos;
		float distance = (_float)sqrt(pow(mousePos.x - vCurPos.x, 2) + pow(mousePos.y - vCurPos.y, 2));

		//키프레임 범위 안에서 마우스 클릭 발생 시 키프레임 위치에 ui 띄움
		if (distance <= 8.0f && IsMouseClicked(0))
		{
			m_bOpenKeyframeEditor = true;
			bMakeKFPopupToFront = true;
			eSelectedProperty = KF_ROT;
			iSelectedKFIdx = iTempKFIdx;

			//위치, 스케일 키프레임 옆에 맞춘다
			
			SetNextWindowSize(vPopupSize);
			SetNextWindowPos(ImVec2(vCurPos.x, fInitialYPos));

			_float3 vValue = m_FXs[m_iSelectedFXIdx]->m_Keyframes[eSelectedProperty][iSelectedKFIdx].vValue;
			memcpy(m_vKFPopupValue, &vValue, sizeof(_float3));
			m_eKFPopupEasing = m_FXs[m_iSelectedFXIdx]->m_Keyframes[eSelectedProperty][iSelectedKFIdx].eEasing;
		}

		++iTempKFIdx;
	}


	Dummy(ImVec2(0, 15));

	iTempKFIdx = 0;
	//크기
	vPos = GetCursorScreenPos();

	pDrawList->AddLine(vPos, ImVec2(vPos.x + _fWidth, vPos.y), IM_COL32(255, 0, 100, 255), 1.f);
	for (auto& keyframe : pCurFX->m_Keyframes[KF_SCALE])
	{
		_float fRatio = (pCurFX->m_fLifeTime.first + (keyframe.fTimeRatio * (pCurFX->m_fLifeTime.second - pCurFX->m_fLifeTime.first)));
		_float fPosX = fRatio * _fWidth / pCurFX->m_fDuration.second;
		ImVec2 vCurPos = vPos + ImVec2{ fPosX, 2.f };

		GetWindowDrawList()->AddCircleFilled(vCurPos, 6.0f, IM_COL32(255, 255, 100, 255));

		ImVec2 mousePos = GetIO().MousePos;
		float distance = (_float)sqrt(pow(mousePos.x - vCurPos.x, 2) + pow(mousePos.y - vCurPos.y, 2));

		//키프레임 범위 안에서 마우스 클릭 발생 시 키프레임 위치에 ui 띄움
		if (distance <= 8.0f && IsMouseClicked(0))
		{
			m_bOpenKeyframeEditor = true;
			bMakeKFPopupToFront = true;
			eSelectedProperty = KF_SCALE;
			iSelectedKFIdx = iTempKFIdx;

			//위치, 스케일 키프레임 옆에 맞춘다
			SetNextWindowSize(vPopupSize);
			SetNextWindowPos(ImVec2(vCurPos.x, fInitialYPos));

			//띄울 키프레임의 값을 매칭해 준다.
			_float3 vValue = m_FXs[m_iSelectedFXIdx]->m_Keyframes[eSelectedProperty][iSelectedKFIdx].vValue;
			memcpy(m_vKFPopupValue, &vValue, sizeof(_float3));
			m_eKFPopupEasing = m_FXs[m_iSelectedFXIdx]->m_Keyframes[eSelectedProperty][iSelectedKFIdx].eEasing;
		}

		++iTempKFIdx;
	}
	Spacing();


 	if (m_bOpenKeyframeEditor)
	{
		if (eSelectedProperty == KF_POS)
			Begin(u8"위치", nullptr, ImGuiWindowFlags_NoCollapse);
		else if (eSelectedProperty == KF_ROT)
			Begin(u8"회전", nullptr, ImGuiWindowFlags_NoCollapse);
		else if (eSelectedProperty == KF_SCALE)
			Begin(u8"크기", nullptr, ImGuiWindowFlags_NoCollapse);


		//SetWindowFocus(u8"키프레임 편집");
		
		if (DragFloat(u8"시간", &m_FXs[m_iSelectedFXIdx]->m_Keyframes[eSelectedProperty][iSelectedKFIdx].fTimeRatio, .01f, .01f, .99f, "%.2f"))
		{

		}

		//어떤 속성을 편집하느냐에 따라 최소, 최대 범위 정한다.
		_float2 vValueRange{ 0.f, 1.f };
		if (eSelectedProperty == KF_POS)
			vValueRange = { -50.f, 50.f };
		else if (eSelectedProperty == KF_ROT)
			vValueRange = { -180.f, 180.f };
		else if (eSelectedProperty == KF_SCALE)
			vValueRange = { .001f, 50.f };


		if (DragFloat3("Value", m_vKFPopupValue, .01f, vValueRange.x, vValueRange.y, "%.2f"))
		{
			m_FXs[m_iSelectedFXIdx]->m_Keyframes[eSelectedProperty][iSelectedKFIdx].vValue = _float3{ m_vKFPopupValue[0], m_vKFPopupValue[1], m_vKFPopupValue[2]};

		}
		if (Combo(u8"Easing", &m_eKFPopupEasing, m_Easing.data(), (_int)m_Easing.size()))
		{
			m_FXs[m_iSelectedFXIdx]->m_Keyframes[eSelectedProperty][iSelectedKFIdx].eEasing = (EASING)m_eKFPopupEasing;
		}
		
		if (Button(u8"키프레임 삭제"))
		{
			m_bOpenKeyframeEditor = false;
			m_FXs[m_iSelectedFXIdx]->Delete_Keyframe(eSelectedProperty, iSelectedKFIdx);
		}
		End();
	}

	//drawline
	//Text(u8"R");
	//SameLine();
	//testKeyframes.clear();
	//testKeyframes.push_back(FX_KEYFRAME{ .5f });
	//vPos = GetCursorScreenPos();
	//vPos.y += 5.f;
	//for (auto& keyframe : pCurFX->m_Keyframes[KF_RCOLOR])
	//{
	//	_float fPosX = keyframe.fTimeRatio /*/ m_EditEffects[m_iSelectedEffectIdx]->m_fDestDuration)*/ * _fWidth;

	//	vPos.x += fPosX;
	//	GetWindowDrawList()->AddCircleFilled(vPos, 6.0f, IM_COL32(255, 255, 100, 255));
	//}

	//Text(u8"G");
	//SameLine();
	//testKeyframes.clear();
	//testKeyframes.push_back(FX_KEYFRAME{ .5f });
	//vPos = GetCursorScreenPos();
	//vPos.y += 5.f;
	//for (auto& keyframe : pCurFX->m_Keyframes[KF_GCOLOR])
	//{
	//	_float fPosX = keyframe.fTimeRatio /*/ m_EditEffects[m_iSelectedEffectIdx]->m_fDestDuration)*/ * _fWidth;

	//	vPos.x += fPosX;
	//	GetWindowDrawList()->AddCircleFilled(vPos, 6.0f, IM_COL32(255, 255, 100, 255));
	//}

	//Text(u8"B");
	//SameLine();
	//testKeyframes.clear();
	//testKeyframes.push_back(FX_KEYFRAME{ .5f });
	//vPos = GetCursorScreenPos();
	//vPos.y += 5.f;
	//for (auto& keyframe : pCurFX->m_Keyframes[KF_BCOLOR])
	//{
	//	_float fPosX = keyframe.fTimeRatio /*/ m_EditEffects[m_iSelectedEffectIdx]->m_fDestDuration)*/ * _fWidth;

	//	vPos.x += fPosX;
	//	GetWindowDrawList()->AddCircleFilled(vPos, 6.0f, IM_COL32(255, 255, 100, 255));
	//}
}




void CFXToolDirector::MakeBar_ParticleFXProperty(_float _fTimeDelta, _float _fWidth)
{
	if (m_iSelectedFXIdx == -1)
		return;


}

void CFXToolDirector::MakeBar_MultiFXProperty(_float _fTimeDelta, _float _fWidth)
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
