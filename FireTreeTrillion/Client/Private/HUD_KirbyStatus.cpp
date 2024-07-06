#include "stdafx.h"
#include "HUD_KirbyStatus.h"
#include "Kirby.h"
#include "FinaleKirby.h"

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

	if (UI_TEXTURE == m_UIObjDesc.eUIType)
		m_iTexIndex = (*HUDKirby_Desc).iTexIndex;

	if (UI_FONT == m_UIObjDesc.eUIType)
		m_UIObjDesc.wstrText = (*HUDKirby_Desc).wstrText;

	m_pTransformCom->Set_Scaled(m_UIObjDesc.vSize.x, m_UIObjDesc.vSize.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_UIObjDesc.vPos.x,
					m_UIObjDesc.vPos.y,
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

	m_fSaveMyX = m_UIObjDesc.vPos.x;
	m_fSaveMyY = m_UIObjDesc.vPos.y;

	// 쉐이킹 진폭 초기화
	m_fAmplitude = 0.001f;

	if (UI_FONT == m_UIObjDesc.eUIType)
	{
		m_vFontPos = { m_UIObjDesc.vPos.x + m_UIObjDesc.vCenter.x,
						-m_UIObjDesc.vPos.y + m_UIObjDesc.vCenter.y };

		m_fFontSavePosX = m_vFontPos.x;
	}

	m_eCurState = KIRBYHP_WAIT;
	m_ePreState = KIRBYHP_HIDE;

	return S_OK;
}

_int CHUD_KirbyStatus::Tick(_float fTimeDelta)
{	
	__super::Tick(fTimeDelta);

	m_fTimeDelta = fTimeDelta;

	_float fHpMax = { 0.f };
	_float fHp = { 0.f };

	if (*m_pCurrentLevelID == LEVEL_FINALE)
	{
		CFinaleKirby* pKirby = static_cast<CFinaleKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
		if (pKirby == nullptr)
			return OBJ_NOEVENT;

		fHp = pKirby->Get_Hp();
		fHpMax = pKirby->Get_MaxHp();
	}
	else
	{
		CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
		if (pKirby == nullptr)
			return OBJ_NOEVENT;

		fHp = pKirby->Get_Hp();
		fHpMax = pKirby->Get_MaxHp();
	}

	// 피를 닳게 하는 기능을 가진 함수
	Compute_Player_Hp(fTimeDelta, fHpMax, fHp);

	// 아무 이상이 없을 때, 자동으로 사라지게 하는 기능을 가진 함수
	Disappear_HpBar(fTimeDelta);

	return OBJ_NOEVENT;
}

void CHUD_KirbyStatus::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD_KirbyStatus::Render()
{
	//if (KIRBYHP_WAIT == m_eCurState && KIRBYHP_HIDE == m_ePreState)
	//	return S_OK;

	// 만약, 클래스 내부에서 랜더되지 않아도 되는 상황으로 판단한다면, 랜더를 하지 않는다.
	if (m_bRenderHpbar == false)
		return S_OK;

	// 알파값이 1보다 낮아지려고 하는 것은 사라지려고 하는 것이다.
	if (m_fAlpha < 1.f)
	{
		if (m_UIObjDesc.wstrUITag == TEXT("Gauge_Damage"))
			return S_OK;
	}


	if (UI_TEXTURE == m_UIObjDesc.eUIType)
	{
		Render_BindSet(m_pShaderCom, m_pTransformCom);
	}

	if (UI_FONT == m_UIObjDesc.eUIType)
	{
		_float4 vFontRGBA = { m_UIObjDesc.vColorRGB.x, m_UIObjDesc.vColorRGB.y, m_UIObjDesc.vColorRGB.z, m_UIObjDesc.fAlpha * m_fAlpha };
		_float2 vFontOrig = { 1.f, 1.f };
		_float2 vFontScale = { 1.2f, 1.2f };
		wstring wstrFontTag = { TEXT("Font_HUDSub_KR15") };

		m_pGameInstance->Render_Font(wstrFontTag, m_UIObjDesc.wstrText, m_vFontPos, vFontRGBA,
			XMConvertToRadians(m_UIObjDesc.vDegree.z), vFontOrig, vFontScale);
	}


	return S_OK;
}

HRESULT CHUD_KirbyStatus::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_HUD_StatusBar_Kirby"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_HUD_StatusBar_Kirby_Mask"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTexMask)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_KirbyStatus::Render_BindSet(CShader* _pShaderCom, CTransform* _pTransCom)
{
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

	SHADER_PS ePassIndex = { PS_APBLEND_NOZTEST }; //셰이더 패스 기본값
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
		m_pTexMask->Bind_ShaderResource(_pShaderCom, "g_MaskTexture", 0);
		_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_fHpRatio, sizeof(_float));
		_pShaderCom->Bind_RawValue("g_fAlarmColor", &m_fAlarmColor, sizeof(_float));
	}
	if (TEXT("Gauge_Damage") == m_UIObjDesc.wstrUITag)
	{
		m_pTexMask->Bind_ShaderResource(_pShaderCom, "g_MaskTexture", 0);
		_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_fSlowHpRatio, sizeof(_float));
		_pShaderCom->Bind_RawValue("g_fAlarmColor", &m_fAlarmColor, sizeof(_float));

		if (m_isHealing == true)
		{
			_float3 vHealingColor = { 255.f, 180.f, 207.f}; 
			vHealingColor /= 255.f;
			_pShaderCom->Bind_RawValue("g_vRColor", &vHealingColor, sizeof(_float3));
		}
		else
			_pShaderCom->Bind_RawValue("g_vRColor", &m_UIObjDesc.vColorRGB, sizeof(_float3));
	}

	//셰이더 파일의 텍스처 정보를 가져와 바인딩
	_pTextureCom->Bind_ShaderResource(_pShaderCom, "g_DiffuseTexture", _iTexIndex);

	//셰이더의 원시데이터 가져와 저장
	if (TEXT("Gauge_Damage") != m_UIObjDesc.wstrUITag)
	{
		_pShaderCom->Bind_RawValue("g_vRColor", &m_UIObjDesc.vColorRGB, sizeof(_float3));
	}

	_float fFinalAlpha = m_UIObjDesc.fAlpha * m_fAlpha;
	_pShaderCom->Bind_RawValue("g_fAlpha", &fFinalAlpha, sizeof(_float));

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
	case KIRBYHP_IDLE: // 1) 렌더x 기본 상태
		if (KIRBYHP_HIDE == m_ePreState)	//이전 상태가 HIDE인 경우, 기본값으로 세팅
			m_eCurState = KIRBYHP_WAIT;
		break;

	case KIRBYHP_WAIT: // 3) 특정 이벤트 이후 대기 상태
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

	case KIRBYHP_HIDE: // 4) 숨김 상태
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
	case KIRBYHP_DAMAGE: // 2) 피격 상태
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

	case KIRBYHP_HEAL: 
		break;

	case KIRBYHP_NONE:
	default:	break;
	}
}

void CHUD_KirbyStatus::Play_Animation(_float _fAccTime, KIRBYHP_STATE _eCurState)
{
	_float4 vWAITPos = {};

	switch (m_eCurState)
	{
	case CHUD::KIRBYHP_IDLE: //기본 상태에서 위치 값을 저장
		break;

	case CHUD::KIRBYHP_WAIT:
		//if (TEXT("Name") == m_UIObjDesc.wstrUITag)
		//	m_UIObjDesc.vPos = { -750.f, 413.f, 0.f };

		//if (TEXT("Base") == m_UIObjDesc.wstrUITag || TEXT("Shadow") == m_UIObjDesc.wstrUITag || (TEXT("Blur") == m_UIObjDesc.wstrUITag))
		//	m_UIObjDesc.vPos = { -0.39f, 0.21f, 1.f };

		//if (TEXT("Gauge_Base") == m_UIObjDesc.wstrUITag || (TEXT("Gauge_Damage") == m_UIObjDesc.wstrUITag || TEXT("Gauge") == m_UIObjDesc.wstrUITag))
		//	m_UIObjDesc.vPos = { -0.39f, 0.21f, 1.f };

		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
			XMVectorSet(m_UIObjDesc.vPos.x - m_UIObjDesc.vCenter.x + m_UIObjDesc.vCenter.x,
				m_UIObjDesc.vPos.y - m_UIObjDesc.vCenter.y + m_UIObjDesc.vCenter.y,
				m_UIObjDesc.vPos.z, 1.f));

		break;

	case CHUD::KIRBYHP_HIDE: //X값 좌측 이동, 알파 값 죽이기
		if (m_UIObjDesc.wstrUITag == TEXT("Name"))
		{
			m_UIObjDesc.vPos.x -= 40.f;
			//m_UIObjDesc.vPos.x = m_vOriginalPos - (내가 이동하고 싶은 값) (50.f * _fAccTime * (1.f / 0.16f))
		}

		m_UIObjDesc.vPos.x -= 0.05f;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
			XMVectorSet(m_UIObjDesc.vPos.x - m_UIObjDesc.vCenter.x + m_UIObjDesc.vCenter.x,
				m_UIObjDesc.vPos.y - m_UIObjDesc.vCenter.y + m_UIObjDesc.vCenter.y,
				m_UIObjDesc.vPos.z, 1.f));

		// 1 ~ 0  -> 0.16초
		m_UIObjDesc.fAlpha = 1.f - (_fAccTime * (1.f / 0.16f));

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

void CHUD_KirbyStatus::Compute_Player_Hp(_float fTimeDelta, _float _fKirbyHpMax, _float _fKirbyHp)
{

#pragma region 분홍색 게이지 공식

	// 현재 커비의 HP 맥스치
	_float fKirbyHpMax = _fKirbyHpMax;
	_float fKirbyHp = _fKirbyHp;

	// 이 비율은 0 ~ 1 사이에 있어야 한다.
	m_fCurHpRatio = (fKirbyHp / fKirbyHpMax);

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

void CHUD_KirbyStatus::Disappear_HpBar(_float fTimeDelta)
{
	// 피격 또는 회복이 된 이후로 7초가 지났으면 사라지는 애니메이션이 재생이 된다.
	if (m_fIdleTime > 5.f)
	{
		// 투명해지기 시작하는 시점이다.
		if (m_fAlpha > 0.f)
		{
			m_fAlpha -= fTimeDelta * 5.5f;

			_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
			vPos.x -= fTimeDelta * 0.1f;
			if (UI_FONT == m_UIObjDesc.eUIType)
			{
				m_vFontPos.x -= m_fTimeDelta * 170.f;
			}
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		}

		if (m_fAlpha < 0.f)
		{
			m_fAlpha = 0.f;
			m_bRenderHpbar = false;
		}
	}
	// 반대로, 7초 이하이거나 (피격 또는 회복이 되었다는 뜻임) 
	// 또는 우리가 밖에서 RenderHpbar 라는 불 변수를 true로 만들어주면 강제적으로 Render한다.
	else if (m_fIdleTime <= 5.f || m_bCustomRenderHpbar == true)
	{
		m_bRenderHpbar = true;
		m_fAlpha = 1.f;
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		vPos.x = m_fSaveMyX;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		if (UI_FONT == m_UIObjDesc.eUIType)
		{
			m_vFontPos.x = m_fFontSavePosX;
		}
	}
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
	Safe_Release(m_pTexMask);
	__super::Free();
}


