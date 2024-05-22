#include "stdafx.h"
#include "FXToolDirector.h"
#include "GameInstance.h"

#include "SingleEffect.h"
#include "MultiEffect.h"
#include "Camera_Free.h"


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

	//ImGuizmo::DrawGrid(viewMat.m[0], projMat.m[0], identityMat.m[0], 100.f);

	m_pGameInstance->RenderGrid();

	Render_FXHierarchy();
	Render_FXProperty(_fTimeDelta);
	Render_FXPlayBar(_fTimeDelta);
	//m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CFXToolDirector::Render()
{
	return S_OK;
}

void CFXToolDirector::Render_IMGUI()
{
	__super::Render_IMGUI();
}

void CFXToolDirector::Render_FXHierarchy()
{
	Begin("Make");

	Combo("Buffer", &m_iAddingFXBufferIdx, m_FXBufferList.data(), (_int)m_FXBufferList.size());
	Combo("Texture", &m_iAddingFXTexIdx, m_FXTexList.data(), (_int)m_FXTexList.size());
	Combo("Mask", &m_iAddingFXMaskTexIdx, m_FXMaskTexList.data(), (_int)m_FXMaskTexList.size());

	if (Button("Make FX"))
	{
		//m_SingleFXs.emplace_back();
	}
	SameLine();
	if (Button("Make Particle"))
	{

	}

	End();

	/*
	BeginChild("FX List", ImVec2(0, 200), true);

	for (_int i = 0; i < m_SingleFXs.size(); ++i)
	{
		if (Selectable(CUtils::WstrToStr(m_SingleFXs[i]->m_strFXName).c_str(), m_iSelectedSingleFXIdx == i))
		{

		}
	}

	EndChild();
	*/

	Begin("Add to MultiFX?", nullptr, ImGuiWindowFlags_NoCollapse);
	if (Button(u8"Yes Yes Yes")/* && m_iSelectedCompositionEffectIdx != -1 && m_iSelectedEffectIdx != -1*/)
	{
		//m_CompositeEffects[m_iSelectedCompositionEffectIdx]->Add_Effect(m_EditEffects[m_iSelectedEffectIdx]);
		//bOpenAddPopup = false;
	}
	End();
}

void CFXToolDirector::Render_FXProperty(_float _fTimeDelta)
{
	if (m_iSelectedSingleFXIdx == -1)
		return;

	//이펙트 기본 변수 세팅
	Begin("FX Property", nullptr, ImGuiWindowFlags_NoCollapse);


	CEffect* pCurFX = m_SingleFXs[m_iSelectedSingleFXIdx];

	char tempBuf[256];
	strncpy_s(tempBuf, m_curFXName.c_str(), sizeof(tempBuf));
	tempBuf[sizeof(tempBuf) - 1] = 0;

	//이름
	if (InputText("Name", tempBuf, sizeof(tempBuf)))
	{
		if (tempBuf[0] == '\0')
			strcpy_s(tempBuf, "Default");

		m_curFXName = string(tempBuf);
		pCurFX->m_strFXName = m_curFXName;
	}


	Checkbox("Loop", &pCurFX->m_bIsLoop);
	Checkbox(u8"Billboard", &pCurFX->m_bIsBillboard);
	Checkbox(u8"Orthographic", &pCurFX->m_bIsOrthographic);

	if (Checkbox(u8"Bloom", &pCurFX->m_bIsBloom))
	{
		pCurFX->m_bIsBloom ?
			pCurFX->m_bIsNonLight = false :
			pCurFX->m_bIsNonLight = true;
	}


	DragFloat(u8"Duration", &pCurFX->m_fDuration.second, .1f, 0.f, 30.f, "%.2f");
	DragFloat2(u8"Lifetime", m_fLifeTime, .1f, 0.f, pCurFX->m_fDuration.second, "%.2f");

	//만약 파티클이라면
	//DragFloat3(u8"Range", m_fRange, .01f, 0.f, 100.f, "%.2f");

	DragInt(u8"Render Pass", &pCurFX->m_iPassIdx, 1.f, 0, 10);
	DragInt(u8"Texture Index", &pCurFX->m_iTexIdx, 1.f, 0, pCurFX->m_iMaxTexIdx);
	DragInt(u8"Mask Texture Index", &pCurFX->m_iMaskTexIdx, 1.f, 0, pCurFX->m_iMaxMaskTexIdx);


	if (DragFloat3(u8"Init Pos", m_vEditPos, .01f, -50.f, 50.f, "%.2f"))
	{
		memcpy(&pCurFX->m_vInitPos, m_vEditPos, sizeof(_float3));
	}
	if (DragFloat3(u8"Init Rot", m_vEditRot, .1f, -180.f, 180.f, "%.2f"))
	{
		memcpy(&pCurFX->m_vInitRot, m_vEditRot, sizeof(_float3));
	}
	if (DragFloat3(u8"Init Scale", m_vEditScale, .05f, .01f, 100.f, "%.2f"))
	{
		memcpy(&pCurFX->m_vInitScale, m_vEditScale, sizeof(_float3));
	}

	End();
}

void CFXToolDirector::Render_FXPlayBar(_float _fTimeDelta)
{
}

void CFXToolDirector::Render_MultiFXHierarchy()
{
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
	__super::Free();
}
