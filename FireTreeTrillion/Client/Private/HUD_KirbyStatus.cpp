#include "stdafx.h"
#include "HUD_KirbyStatus.h"
#include "Kirby.h"

CHUD_KirbyStatus::CHUD_KirbyStatus(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CHUD{ _pDevice, _pContext }
{
}

CHUD_KirbyStatus::CHUD_KirbyStatus(const CHUD_KirbyStatus& _rhs)
	: CHUD{ _rhs }
{
}

HRESULT CHUD_KirbyStatus::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHUD_KirbyStatus::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	UIOBJ_DESC* HUDKirby_Desc{};
	if (nullptr != _pArg)
		HUDKirby_Desc = (UIOBJ_DESC*)_pArg;

	if (FAILED(Add_Components()))
		return E_FAIL;
	
	m_UIObjDesc = (*HUDKirby_Desc);
	m_UIObjDesc.eUIType = (*HUDKirby_Desc).eUIType;
	m_UIObjDesc.vColorRGB = (*HUDKirby_Desc).vColorRGB;
	m_UIObjDesc.fAlpha = (*HUDKirby_Desc).fAlpha;
	//m_UIObjDesc.vDegree = (*HUDKirby_Desc).vDegree;

	if (UI_TEXTURE == m_UIObjDesc.eUIType)
		m_iTexIndex = (*HUDKirby_Desc).iTexIndex;

	if (UI_FONT == m_UIObjDesc.eUIType)
		m_UIObjDesc.wstrText = (*HUDKirby_Desc).wstrText;


	m_pTransformCom->Set_Scaled(m_UIObjDesc.vSize.x, m_UIObjDesc.vSize.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_UIObjDesc.vPos.x - m_UIObjDesc.vCenter.x + m_UIObjDesc.vCenter.x,
					m_UIObjDesc.vPos.y - m_UIObjDesc.vCenter.y + m_UIObjDesc.vCenter.y, 
					m_UIObjDesc.vPos.z, 1.f));

#pragma region SET_PROJ

	if (PROJ_ORTHO == m_UIObjDesc.eUIProj)
	{
		m_UIObjDesc.vDegree.z = (*HUDKirby_Desc).vDegree.z;
		m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
		XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));
	}

	if (PROJ_PERSPEC == m_UIObjDesc.eUIProj)
	{
		m_UIObjDesc.vDegree = (*HUDKirby_Desc).vDegree;

		_float fRadianX = XMConvertToRadians(m_UIObjDesc.vDegree.x);
		_float fRadianY = XMConvertToRadians(m_UIObjDesc.vDegree.y);
		_float fRadianZ = XMConvertToRadians(m_UIObjDesc.vDegree.z);
		m_pTransformCom->Rotation(fRadianX, fRadianY, fRadianZ);
	}

#pragma endregion

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());

	m_fSaveMyY = m_UIObjDesc.vPos.y - m_UIObjDesc.vCenter.y + m_UIObjDesc.vCenter.y;

	// 쉐이킹 진폭 초기화
	m_fAmplitude = 0.001f;

	m_eCurState = KIRBYHP_WAIT;
	m_ePreState = KIRBYHP_HIDE;

	return S_OK;
}

_int CHUD_KirbyStatus::Tick(_float fTimeDelta)
{	
	__super::Tick(fTimeDelta);

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));

	if (pKirby == nullptr)
		return OBJ_NOEVENT;

	Compute_Player_Hp(fTimeDelta, pKirby);
	Update_UIState(fTimeDelta);

	return OBJ_NOEVENT;
}

void CHUD_KirbyStatus::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD_KirbyStatus::Render()
{
	if (UI_TEXTURE == m_UIObjDesc.eUIType)
		Render_BindSet(m_pShaderCom, m_pTransformCom);

	if (UI_FONT == m_UIObjDesc.eUIType)
	{
		_float2 vFontPos = { m_UIObjDesc.vPos.x + m_UIObjDesc.vCenter.x,
							-m_UIObjDesc.vPos.y + m_UIObjDesc.vCenter.y };

		_float4 vFontRGBA = { m_UIObjDesc.vColorRGB.x, m_UIObjDesc.vColorRGB.y, m_UIObjDesc.vColorRGB.z, m_UIObjDesc.fAlpha };
		_float2 vFontOrig = { 1.f, 1.f };
		_float2 vFontScale = { 1.2f, 1.2f };

		wstring wstrFontTag = { TEXT("Font_HUDSub_KR15") };

		if (KIRBYHP_WAIT == m_eCurState && KIRBYHP_HIDE == m_ePreState)
			return S_OK;

		m_pGameInstance->Render_Font(wstrFontTag, m_UIObjDesc.wstrText, vFontPos, vFontRGBA,
			XMConvertToRadians(m_UIObjDesc.vDegree.z), vFontOrig, vFontScale);
	}

	return S_OK;
}

HRESULT CHUD_KirbyStatus::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_StatusBar_Kirby"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_StatusBar_Kirby_Mask"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTextureMask)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_KirbyStatus::Render_BindSet(CShader* _pShaderCom, CTransform* _pTransCom)
{
	if (KIRBYHP_WAIT == m_eCurState && KIRBYHP_HIDE == m_ePreState)
		return S_OK;

	CHECK_NULLPTR(_pShaderCom);

	if (FAILED(_pTransCom->Bind_ShaderResource(_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (PROJ_PERSPEC == m_UIObjDesc.eUIProj)
	{
		//m_ViewMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
		XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
		m_ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);
	}

	//셰이더 파일의 매트릭스 정보를 가져와 바인딩
	if (FAILED(_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	SHADER_PS ePassIndex = { PS_ALPHABLEND }; //셰이더 패스 기본값
	if (TEXT("Gauge") == m_UIObjDesc.wstrUITag){ ePassIndex = PS_MASK_HP;	}
	if (TEXT("Gauge_Damage") == m_UIObjDesc.wstrUITag){ ePassIndex = PS_MASK_HPDAMAGE;	}

	if (FAILED(Bind_ShaderResources(_pShaderCom, ePassIndex, m_pTextureCom, m_iTexIndex)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_KirbyStatus::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex)
{
	if (TEXT("Gauge") == m_UIObjDesc.wstrUITag)
	{
		m_pTextureMask->Bind_ShaderResource(_pShaderCom, "g_MaskTexture", 0);
		_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_fHpRatio, sizeof(_float));
		_pShaderCom->Bind_RawValue("g_fAlarmColor", &m_fAlarmColor, sizeof(_float));
	}
	if (TEXT("Gauge_Damage") == m_UIObjDesc.wstrUITag)
	{
		m_pTextureMask->Bind_ShaderResource(_pShaderCom, "g_MaskTexture", 0);
		_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_fHpSlowRatio, sizeof(_float));
		_pShaderCom->Bind_RawValue("g_fAlarmColor", &m_fAlarmColor, sizeof(_float));
	}

	//셰이더 파일의 텍스처 정보를 가져와 바인딩
	_pTextureCom->Bind_ShaderResource(_pShaderCom, "g_DiffuseTexture", _iTexIndex);

	//셰이더의 원시데이터 가져와 저장
	_pShaderCom->Bind_RawValue("g_vRColor", &m_UIObjDesc.vColorRGB, sizeof(_float3));
	_pShaderCom->Bind_RawValue("g_fAlpha", &m_UIObjDesc.fAlpha, sizeof(_float));

	//Begin() > Apply() 함수 호출 전 셰이더 전역 데이터를 저장해야함
	if (FAILED(_pShaderCom->Begin(_iPassIndex)))
		return E_FAIL;

	if (FAILED(Bind_VIBuffer(m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_KirbyStatus::Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom)
{
	if (FAILED(_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CHUD_KirbyStatus::Update_UIState(_float _fTimeDelta)
{
	switch (m_eCurState)
	{
	case CHUD::KIRBYHP_IDLE: // 1) 렌더x 기본 상태
		if (KIRBYHP_HIDE == m_ePreState)	//이전 상태가 HIDE인 경우, 기본값으로 세팅
			m_eCurState = KIRBYHP_WAIT;
		break;

	case CHUD::KIRBYHP_WAIT: // 3) 특정 이벤트 이후 대기 상태
		if (KIRBYHP_DAMAGE == m_ePreState)	//이전 피격받았을 경우,
		{
			m_fAccTime += _fTimeDelta;
			if (m_fAccTime > 5.f)
			{
				m_eCurState = KIRBYHP_HIDE; //3-A) 이후 숨김 상태로 변경
				m_fAccTime = 0.f;
			}
		}
		else
			Play_Animation(m_fAccTime, KIRBYHP_WAIT);
		break;

	case CHUD::KIRBYHP_HIDE: // 4) 숨김 상태
		m_fAccTime += _fTimeDelta;
		if (m_fAccTime > 0.16f)
		{
 			m_fAccTime = 0.f;
			m_eCurState = KIRBYHP_IDLE;	//4-A) 시간 경과 후 대기 상태로 변경 (렌더X)
			m_ePreState = KIRBYHP_HIDE;
		}
		else
			Play_Animation(m_fAccTime, KIRBYHP_HIDE);
		break;

	//Frame 52 > 77
	case CHUD::KIRBYHP_DAMAGE: // 2) 피격 상태
		m_fAccTime += _fTimeDelta;
		if (m_fAccTime >= 25.f / 144.f)
		{
			m_fAccTime = 0.f;
			m_eCurState = KIRBYHP_WAIT;
			m_ePreState = KIRBYHP_DAMAGE;
		}
		else
			Play_Animation(m_fAccTime, KIRBYHP_DAMAGE);
		break;

	case CHUD::KIRBYHP_HEAL: //
		break;

	case CHUD::KIRBYHP_NONE:
	default:	break;
	}
}

void CHUD_KirbyStatus::Play_Animation(_float _fAccTime, HUD_KIRBYHP _eCurState)
{
	_float4 vWAITPos = { 0.f, 0.f, 0.f, 0.f };

	switch (m_eCurState)
	{
	case CHUD::KIRBYHP_IDLE: //기본 상태에서 위치 값을 저장
		break;

	case CHUD::KIRBYHP_WAIT:
		if (TEXT("Name") == m_UIObjDesc.wstrUITag)
			m_UIObjDesc.vPos = { -750.f, 413.f, 0.f };

		if (TEXT("Base") == m_UIObjDesc.wstrUITag || TEXT("Shadow") == m_UIObjDesc.wstrUITag || (TEXT("Blur") == m_UIObjDesc.wstrUITag))
			m_UIObjDesc.vPos = { -0.39f, 0.21f, 1.f };

		if (TEXT("Gauge_Base") == m_UIObjDesc.wstrUITag || (TEXT("Gauge_Damage") == m_UIObjDesc.wstrUITag || TEXT("Gauge") == m_UIObjDesc.wstrUITag))
			m_UIObjDesc.vPos = { -0.39f, 0.21f, 1.f };

		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
			XMVectorSet(m_UIObjDesc.vPos.x - m_UIObjDesc.vCenter.x + m_UIObjDesc.vCenter.x,
				m_UIObjDesc.vPos.y - m_UIObjDesc.vCenter.y + m_UIObjDesc.vCenter.y,
				m_UIObjDesc.vPos.z, 1.f));

		break;

	case CHUD::KIRBYHP_HIDE: //X값 좌측 이동, 알파 값 죽이기
		if (m_UIObjDesc.wstrUITag == TEXT("Name"))
			m_UIObjDesc.vPos.x -= 40.f;

		m_UIObjDesc.vPos.x -= 0.05f;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
			XMVectorSet(m_UIObjDesc.vPos.x - m_UIObjDesc.vCenter.x + m_UIObjDesc.vCenter.x,
				m_UIObjDesc.vPos.y - m_UIObjDesc.vCenter.y + m_UIObjDesc.vCenter.y,
				m_UIObjDesc.vPos.z, 1.f));

		m_UIObjDesc.fAlpha -= 1.f / 255.f * _fAccTime;

		if (m_UIObjDesc.fAlpha < 1.f / 255.f)
			m_UIObjDesc.fAlpha = 1.f / 255.f;
		break;

	case CHUD::KIRBYHP_DAMAGE:
		//m_pTransformCom->Set_State(CTransform::STATE_POSITION, vStateWAITPos);
		break;

	case CHUD::KIRBYHP_HEAL:
		break;

	case CHUD::KIRBYHP_NONE:
	default:	break;
	}
}

void CHUD_KirbyStatus::Compute_Player_Hp(_float fTimeDelta, CKirby* pKirby)
{
#pragma region 분홍색 게이지 공식
	// 현재 커비의 HP 맥스치
	_float fKirbyHpMax = pKirby->Get_MaxHp();
	_float fKirbyHp = pKirby->Get_Hp();

	// 이 비율은 0 ~ 1 사이에 있어야 한다.
	m_fHpRatio = (fKirbyHp / fKirbyHpMax);

#pragma endregion

#pragma region 노란색 게이지 공식
	// 피가 닳았다는 신호이다.
	if (m_fHpRatio < m_fHpSlowRatio)
	{
		m_eCurState = KIRBYHP_DAMAGE;

		// 만약, 현재 피통과 느리게 따라오는 피통의 비율이 다를경우 가산하기 시작한다.
		m_fAccDamageTime += fTimeDelta;

		// 여기에서부터 반짝이 시작
		m_bAlarm = true;

		// 한번만 발동시켜주는 트리거
		if (m_bShakingTrigger == true)
		{
			// 여기서부터 쉐이킹 시작
			m_bShaking = true;
			m_bShakingTrigger = false;
		}
	}
	// 회복 되었을 경우
	else if (m_fHpRatio > m_fHpSlowRatio)
	{
		// 이곳은 피가 차는 곳이다.
		m_fAccHealTime += fTimeDelta;
		m_bAlarm = TRUE;
	}

	// 만약, 0.8초가 지났으면 그제서야 m_fHpSlowRatio 가 HpRatio 를 따라간다.
	// 이곳은 피가 닳았을 때
	if (m_fAccDamageTime > 0.8f)
	{
		// 한번 계산을 위해 키는 불 값
		if (m_bComputeDeltaGauge == true)
		{
			// 0.8초가 지났을 때, 내가 가야하는 거리를 계산한 것이다.
			m_fDistanceGauge = m_fHpSlowRatio - m_fHpRatio;

			m_bComputeDeltaGauge = false;
		}
		else
		{

			_float fOffSet = 2.f;
			m_fHpSlowRatio -= fTimeDelta * m_fDistanceGauge * fOffSet;

			// Slow비율이 만약, 현재 HP보다 작아졌다면? (따라왔다는 뜻)
			if (m_fHpSlowRatio < m_fHpRatio)
			{
				m_fHpSlowRatio = m_fHpRatio;
				m_bComputeDeltaGauge = true;
				m_fAccDamageTime = 0.f;

				// 다시 흔들 준비를 한다.
				m_bShakingTrigger = true;

				// 여기에서 반짝이 끝
				m_bAlarm = false;
			}
		}
	}

	// 피 회복 (셰이킹 x)
	if (m_fAccHealTime > 0.8f)
	{
		// 한번 계산을 위해 키는 불 값
		if (m_bComputeDeltaGauge == true)
		{
			// 0.8초가 지났을 때, 내가 가야하는 거리를 계산한 것이다.
			m_fDistanceGauge = m_fHpSlowRatio - m_fHpRatio;

			m_bComputeDeltaGauge = false;
		}
		else
		{
			_float fOffSet = 2.f;
			m_fHpSlowRatio -= fTimeDelta * m_fDistanceGauge * fOffSet;

			// Slow비율이 만약, 현재 HP보다 작아졌다면? (따라왔다는 뜻)
			if (m_fHpSlowRatio > m_fHpRatio)
			{
				m_fHpSlowRatio = m_fHpRatio;
				m_bComputeDeltaGauge = true;
				m_fAccDamageTime = 0.f;

				// 여기에서 반짝이 끝
				m_bAlarm = false;
			}
		}
	}

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

#pragma region 피통 UI 쉐이킹 코드

	if (m_bShaking == TRUE)
	{
		// 진동 주기
		_float fCycle = 50.f;

		m_fShakingTime += fTimeDelta;
		m_fShakingAcc += fTimeDelta * fCycle;
		_float fShakePosY = sin(m_fShakingAcc) * m_fAmplitude;

		m_fAmplitude -= fTimeDelta * 0.005f;

		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		vPos.y = m_fSaveMyY + fShakePosY;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

		if (m_fShakingTime > 0.4f)
		{
			m_fShakingTime = 0.f;
			m_bShaking = FALSE;
			m_fAmplitude = 0.005f;

			vPos.y = m_fSaveMyY;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		}
	}
#pragma endregion

}

CHUD_KirbyStatus* CHUD_KirbyStatus::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHUD_KirbyStatus* pInstance = new CHUD_KirbyStatus(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CHUD_KirbyStatus"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHUD_KirbyStatus::Clone(void* pArg)
{
	CHUD_KirbyStatus* pInstance = new CHUD_KirbyStatus(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CHUD_KirbyStatus"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHUD_KirbyStatus::Free()
{
	Safe_Release(m_pTextureMask);
	__super::Free();
}


