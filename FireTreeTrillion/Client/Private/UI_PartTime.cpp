#include "stdafx.h"
#include "UI_PartTime.h"

CUI_PartTime::CUI_PartTime(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CUI_PartTime::CUI_PartTime(const CUI_PartTime& _rhs)
	: CUIObject{ _rhs }
{
}

HRESULT CUI_PartTime::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_PartTime::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	fill(m_arrTexures.begin(), m_arrTexures.end(), nullptr);
	if (FAILED(Add_Components()))
		return E_FAIL;

	m_position2D = _float2(50.f, 50.f);
	m_WindowSize2D = _float2(g_iWinSizeX, g_iWinSizeY);
	m_pTransformCom->Set_Scaled(m_SizeBar2D.x, m_SizeBar2D.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_position2D.x	 - m_WindowSize2D.x * 0.5f,
					- m_position2D.y + m_WindowSize2D.y * 0.5f,
					0.f,
					1.f));

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(m_WindowSize2D.x, m_WindowSize2D.y, 0.f, 1.f));

	m_bRender = true;

	return S_OK;
}

_int CUI_PartTime::Tick(_float fTimeDelta)
{	
	__super::Tick(fTimeDelta);

	//Compute_Timer(fTimeDelta);

	return OBJ_NOEVENT;
}

void CUI_PartTime::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CUI_PartTime::Render()
{
	if (m_bRender == false) return S_OK;

	//for (_int i = 0; i < m_arrTexures.size();++i)
	//{
		_int i = 0;
		HRESULT hr;
		hr = Bind_ShaderResources(i);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(7);
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Bind_Buffers();
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Render();
		CHECK_FAILED(hr);
	//}

	return S_OK;
}

#ifdef _DEBUG
void CUI_PartTime::Render_IMGUI()
{
	char name[12], name2[12], name3[12], name4[12];
	sprintf_s(name, "pos");
	ImGui::DragFloat2(name, (_float*)&m_position2D);
	//sprintf_s(name2, "size");
	//ImGui::DragFloat2(name2, (_float*)&m_size2D);
	//_float fSpeed = m_pTransformCom->Get_Speed();
	//sprintf_s(name3, "speed");
	//ImGui::DragFloat(name3, &fSpeed);
	//sprintf_s(name4, "rotate");
	//ImGui::DragFloat(name4, &m_fRotate);

	m_pTransformCom->Set_Scaled(m_SizeBar2D.x, m_SizeBar2D.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_position2D.x - m_WindowSize2D.x * 0.5f,
			-m_position2D.y + m_WindowSize2D.y * 0.5f,
			0.f,
			1.f));
	//m_pTransformCom->Set_Speed(fSpeed);
	//m_pTransformCom->Rotate(XMVectorSet(0.f, 0.f, 1.f, 0.f), m_fRotate);
}
#endif

HRESULT CUI_PartTime::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_BaseBar"),
		TEXT("Com_Texture_BaseBar"), (CComponent**)&m_arrTexures[0])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_TimeBarBlank"),
		TEXT("Com_Texture_TimeBarBlank"), (CComponent**)&m_arrTexures[1])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_TimeBar"),
		TEXT("Com_Texture_TimeBar"), (CComponent**)&m_arrTexures[2])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_TimeBarBW"),
		TEXT("Com_Texture_TimeBarBW"), (CComponent**)&m_arrTexures[3])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_DeeBG"),
		TEXT("Com_Texture_DeeBG"), (CComponent**)&m_arrTexures[4])))
		return E_FAIL;
	
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_DeeBGBW"),
		TEXT("Com_Texture_DeeBGBW"), (CComponent**)&m_arrTexures[5])))
		return E_FAIL;

	//if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_StatusBar_Kirby_Mask"),
	//	TEXT("Com_Texture_Mask"), (CComponent**)&m_pTexMask)))
	//	return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_PartTime::Bind_ShaderResources(_int _iTextureNum)
{
	HRESULT hr;
	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	hr = m_arrTexures[_iTextureNum]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
	CHECK_FAILED(hr);

	return S_OK;
}

void CUI_PartTime::Compute_Timer(_float fTimeDelta)
{

#pragma region 분홍색 게이지 공식

	// 현재 커비의 HP 맥스치
	_float fTimeMax = 50.f;
	_float fCurrentTime = fTimeMax - fTimeDelta;

	// 이 비율은 0 ~ 1 사이에 있어야 한다.
	m_fCurHpRatio = (fCurrentTime / fTimeMax);

	// 처음에 동기화 작업을 1회 한다. 만약, 레벨이 넘어간다면 이 불값은 다시 true로 만들어야 한다.
	if (m_bInitializeHp == true)
	{
		m_fPreHpRatio = m_fSlowHpRatio = m_fHpRatio = m_fCurHpRatio;
		m_bInitializeHp = false;
	}

#pragma endregion

#pragma region 노란색 게이지 공식

	// 피가 닳았다는 뜻이다.
	if (m_fCurHpRatio < m_fPreHpRatio)
	{
		if (m_isHealing == true)
		{
			m_isHealing = false;
			m_fHealHoleTime = 0.f;
			m_fDeltaRatio = 0.f;
			m_bDeltaRatio = true;
		}

		m_bShaking = TRUE;
		m_bAlarm = TRUE;
		// 피가 찼으니까 분홍이에게 대입을 한다.
		m_fHpRatio = m_fCurHpRatio;
		m_isDamage = true;

		m_fIdleTime = 0.f;
	}
	// 피가 찼다는 뜻이다.
	else if (m_fCurHpRatio > m_fPreHpRatio)
	{
		if (m_isDamage == true)
		{
			m_isDamage = false;
			m_fDamageHoleTime = 0.f;
			m_fDeltaRatio = 0.f;
			m_bDeltaRatio = true;
			m_bAlarm = false;
		}

		// 피가 찼으니까 노랑이한테 대입을 한다.
		m_fSlowHpRatio = m_fCurHpRatio;
		m_isHealing = true;

		m_fIdleTime = 0.f;
	}
	else
	{
		m_fIdleTime += fTimeDelta;
	}


	// 만약, 커비가 데미지를 입었다면?
	if (m_isDamage == true)
	{
		m_fDamageHoleTime += fTimeDelta;

		// 만약, 피가 닳고 0.8초가 넘어갔다면?
		if (m_fDamageHoleTime > 0.8f)
		{
			if (m_bDeltaRatio == true)
			{
				// 차이값을 구했다.
				m_fDeltaRatio = m_fSlowHpRatio - m_fHpRatio;
				m_bDeltaRatio = false;
			}
			// 차이값을 한번 구했다면, 그만큼 피를 틱당 깎아준다.
			m_fSlowHpRatio -= m_fDeltaRatio * fTimeDelta * 2.f;

			if (m_fSlowHpRatio < m_fHpRatio)
			{
				m_bDeltaRatio = true;
				m_isDamage = false;
				m_fDamageHoleTime = 0.f;
				m_fDeltaRatio = 0.f;
				m_fSlowHpRatio = m_fHpRatio;
				m_bAlarm = false;
			}
		}
	}
	// 만약, 커비가 힐을 했다면?
	else if (m_isHealing == true)
	{
		m_fHealHoleTime += fTimeDelta;

		// 만약, 피가 회복되고 0.8초가 넘어갔다면?
		if (m_fHealHoleTime > 0.8f)
		{
			if (m_bDeltaRatio == true)
			{
				// 차이값을 구했다.
				m_fDeltaRatio = m_fSlowHpRatio - m_fHpRatio;
				m_bDeltaRatio = false;
			}
			// 차이값을 한번 구했다면, 그만큼 피를 틱당 올려준다.
			m_fHpRatio += m_fDeltaRatio * fTimeDelta * 2.f;

			if (m_fSlowHpRatio < m_fHpRatio)
			{
				m_bDeltaRatio = true;
				m_isHealing = false;
				m_fHealHoleTime = 0.f;
				m_fDeltaRatio = 0.f;
				m_fHpRatio = m_fSlowHpRatio;
			}
		}
	}
	m_fPreHpRatio = m_fCurHpRatio;
#pragma endregion

#pragma region 피통 UI 반짝반짝 코드

	// 노란 게이지가 반짝이가 되는 중
	if (m_bAlarm == true)
	{
		m_fAlarmTime += fTimeDelta * 40.f;
		//-1 ~ 1 사이의 범위 -> -0.5 ~ 0.5 사이의 범위
		m_fAlarmColor = (sin(m_fAlarmTime)) * 0.5f;
	}
	// 노란 게이지가 반짝이지 않는 중
	else
	{
		m_fAlarmColor = 0.f;
		m_fAlarmTime = 0.f;
	}

#pragma endregion

}


CUI_PartTime* CUI_PartTime::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_PartTime* pInstance = new CUI_PartTime(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CUI_PartTime"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_PartTime::Clone(void* pArg)
{
	CUI_PartTime* pInstance = new CUI_PartTime(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CUI_PartTime"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_PartTime::Free()
{
	Safe_Release(m_pTexMask);
	__super::Free();
}

