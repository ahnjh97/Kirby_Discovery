#include "stdafx.h"
#include "HUD_StarPoint.h"
#include "Kirby.h"

CHUD_StarPoint::CHUD_StarPoint(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CHUD{ _pDevice, _pContext }
{
}

CHUD_StarPoint::CHUD_StarPoint(const CHUD_StarPoint& _rhs)
	: CHUD{ _rhs }
{
}

HRESULT CHUD_StarPoint::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHUD_StarPoint::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	UIOBJ_DESC* HUDStarPoint_Desc{};
	if (nullptr != _pArg)
		HUDStarPoint_Desc = (UIOBJ_DESC*)_pArg;

	if (FAILED(Add_Components()))
		return E_FAIL;
	
	m_UIObjDesc = (*HUDStarPoint_Desc);
	m_UIObjDesc.eUIType = (*HUDStarPoint_Desc).eUIType;
	m_UIObjDesc.vColorRGB = (*HUDStarPoint_Desc).vColorRGB;
	m_UIObjDesc.fAlpha = (*HUDStarPoint_Desc).fAlpha;
	//.vDegree = (*HUDStarPoint_Desc).vDegree;

	if (UI_TEXTURE == m_UIObjDesc.eUIType)
		m_iTexIndex = (*HUDStarPoint_Desc).iTexIndex;

	if (UI_FONT == m_UIObjDesc.eUIType)
		m_UIObjDesc.wstrText = (*HUDStarPoint_Desc).wstrText;


	m_pTransformCom->Set_Scaled(m_UIObjDesc.vSize.x, m_UIObjDesc.vSize.y, m_UIObjDesc.vSize.z);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_UIObjDesc.vPos.x - m_UIObjDesc.vCenter.x + m_UIObjDesc.vCenter.x,
					m_UIObjDesc.vPos.y - m_UIObjDesc.vCenter.y + m_UIObjDesc.vCenter.y,
					m_UIObjDesc.vPos.z, 1.f));

#pragma region SET_PROJ

	if (PROJ_ORTHO == m_UIObjDesc.eUIProj)
	{
		m_UIObjDesc.vDegree.z = (*HUDStarPoint_Desc).vDegree.z;
		m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
		XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));
	}

	if (PROJ_PERSPEC == m_UIObjDesc.eUIProj)
	{
		m_UIObjDesc.vDegree = (*HUDStarPoint_Desc).vDegree;

		_float fRadianX = XMConvertToRadians(m_UIObjDesc.vDegree.x);
		_float fRadianY = XMConvertToRadians(m_UIObjDesc.vDegree.y);
		_float fRadianZ = XMConvertToRadians(m_UIObjDesc.vDegree.z);
		m_pTransformCom->Rotation(fRadianX, fRadianY, fRadianZ);
	}

#pragma endregion

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());

#pragma region INIT DATA

	m_eCurState = STARPOINT_WAIT;
	m_ePreState = STARPOINT_HIDE;

	//Init 초기 값 사전 저장
	m_vInitPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_vInitSize = m_pTransformCom->Get_Scaled();
	m_fInitAlpha = m_UIObjDesc.fAlpha;

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	if (pKirby == nullptr)
		return S_OK;

	m_iPreCoin = pKirby->Get_Coin();

#pragma endregion

	return S_OK;
}

_int CHUD_StarPoint::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	Update_UIState(fTimeDelta);

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	if (pKirby == nullptr)
		return OBJ_NOEVENT;

	else
	{
		//이전 재화량 대비 현재 재화량 비교
		_uint iCurCoin = pKirby->Get_Coin();
		if (iCurCoin > m_iPreCoin)
		{
			m_IsLootTrigger = TRUE;
			m_iPreCoin = iCurCoin;
			pKirby->Set_Coin(m_iPreCoin);

			if (m_IsLootTrigger) //== TRUE;
			{
				m_eCurState = STARPOINT_LOOT;
				m_IsLootTrigger = FALSE;
			}
		}
	}

	return OBJ_NOEVENT;
}

void CHUD_StarPoint::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD_StarPoint::Render()
{
	//해당 상태의 경우에는 렌더x
	if (STARPOINT_WAIT == m_eCurState && STARPOINT_HIDE == m_ePreState)
		return S_OK;

	if (UI_TEXTURE == m_UIObjDesc.eUIType)
		Render_BindSet(m_pShaderCom, m_pTransformCom);

	if (UI_FONT == m_UIObjDesc.eUIType)
	{
		_float2 vFontPos = { m_UIObjDesc.vPos.x + m_UIObjDesc.vCenter.x,
							-m_UIObjDesc.vPos.y + m_UIObjDesc.vCenter.y };

		_float4 vFontRGBA = { m_UIObjDesc.vColorRGB.x, m_UIObjDesc.vColorRGB.y, m_UIObjDesc.vColorRGB.z, m_UIObjDesc.fAlpha };
		_float2 vFontOrig = { 1.f, 1.f };
		_float2 vFontScale = { m_UIObjDesc.vSize.x, m_UIObjDesc.vSize.y };

		wstring wstrFontTag = { TEXT("Font_HUD_StarPoint_NUM30") };
		
		//_uint iCurCoin = m_pKirby->Get_Coin();
		_uint iFont100 = m_iPreCoin / 100;
		_uint iFont10 = (m_iPreCoin % 100) / 10;
		_uint iFont1 = m_iPreCoin % 10;

		if (TEXT("Font100") == m_UIObjDesc.wstrUITag || TEXT("Font100_Shadow") == m_UIObjDesc.wstrUITag)
			return S_OK;
			//m_UIObjDesc.wstrText = to_wstring(iFont100);

		if (TEXT("Font10") == m_UIObjDesc.wstrUITag || TEXT("Font10_Shadow") == m_UIObjDesc.wstrUITag)
		{
			if (iFont10 > 0)
			{
				m_UIObjDesc.wstrText = to_wstring(iFont10);

				m_UIObjDesc.vPos.x = m_vInitPos.x + 12.f;
				m_UIObjDesc.vPos.y = m_vInitPos.y;
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_UIObjDesc.vPos);
			}
			
			else			
				return S_OK;
		}

		if (TEXT("Font1") == m_UIObjDesc.wstrUITag || TEXT("Font1_Shadow") == m_UIObjDesc.wstrUITag)
		{
			m_UIObjDesc.wstrText = to_wstring(iFont1);

			if (iFont10 > 0)
			{
				m_UIObjDesc.vPos.x = m_vInitPos.x + 12.f;
				m_UIObjDesc.vPos.y = m_vInitPos.y;
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_UIObjDesc.vPos);
			}
			else
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_UIObjDesc.vPos);
		}

		m_pGameInstance->Render_Font(wstrFontTag, m_UIObjDesc.wstrText, vFontPos, vFontRGBA,
			XMConvertToRadians(m_UIObjDesc.vDegree.z), vFontOrig, vFontScale);
	}

	return S_OK;
}

HRESULT CHUD_StarPoint::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_StarPoint"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_StarPoint::Render_BindSet(CShader* _pShaderCom, CTransform* _pTransCom)
{
	//마스크도 어색해서 잠시 OFF 처리. 추후 디벨롭 필요
	if (TEXT("Effect_Mask") == m_UIObjDesc.wstrUITag)
		return S_OK;

	if (TEXT("Effect") == m_UIObjDesc.wstrUITag && m_bIsRender == FALSE)
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

	SHADER_PS ePassIndex = { PS_APBLEND_NOZTEST }; //셰이더 패스 기본값
	if (TEXT("Icon") == m_UIObjDesc.wstrUITag) { ePassIndex = PS_DEFAULT; }
	//if (TEXT("Effect_Mask") == m_UIObjDesc.wstrUITag) { ePassIndex = PS_MASK_HP; }

	if (FAILED(Bind_ShaderResources(_pShaderCom, ePassIndex, m_pTextureCom, m_iTexIndex)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_StarPoint::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex)
{
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

HRESULT CHUD_StarPoint::Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom)
{
	if (FAILED(_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CHUD_StarPoint::Update_UIState(_float _fTimeDelta)
{
	switch (m_eCurState)
	{
	case CHUD::STARPOINT_IDLE: // 1) 렌더X 기본 상태
		if (STARPOINT_HIDE == m_ePreState)	//이전 상태가 HIDE인 경우, 기본값으로 세팅
			m_eCurState = STARPOINT_WAIT;
		break;

	case CHUD::STARPOINT_WAIT: //3) 특정 이벤트 이후 대기 상태
		if (STARPOINT_LOOT == m_ePreState)
		{
			m_fAccTime += _fTimeDelta;
			if (m_fAccTime >= 5.f)
			{
				m_eCurState = STARPOINT_HIDE; //3-A) 시간 경과 후 숨김 상태로 변경
				m_fAccTime = 0.f;
			}
		}
		else
			Play_Animation(m_fAccTime, STARPOINT_WAIT);
		break;

	//Frame 344 > 354
	case CHUD::STARPOINT_HIDE: // 4) 숨김 상태
		m_fAccTime += _fTimeDelta;
		if (m_fAccTime >= 0.16f)
		{
			m_fAccTime = 0.f;
			m_eCurState = STARPOINT_IDLE; //4-A) 시간 경과 후 대기 상태로 변경 (렌더X)
			m_ePreState = STARPOINT_HIDE;
		}
		else
			Play_Animation(m_fAccTime, STARPOINT_HIDE);
		break;

	//Frame 50 > 64 
	case CHUD::STARPOINT_LOOT: // 2) 획득 상태
		m_fAccTime += _fTimeDelta;
		if (m_fAccTime >= 0.125f)
		{
			m_fAccTime = 0.f;
			m_eCurState = STARPOINT_WAIT;
			m_ePreState = STARPOINT_LOOT;
			m_IsLootTrigger = FALSE;
		}
		else
			Play_Animation(m_fAccTime, STARPOINT_LOOT);
		break;

	case CHUD::STARPOINT_NONE:
	default:	break;
	}
}

void CHUD_StarPoint::Play_Animation(_float _fAccTime, STARPOINT_STATE _eCurState)
{
	_float4 vCurPos{};
	//if (!m_pKirby) //==nullptr
	//	return;

	//_uint iCurCoin = m_pKirby->Get_Coin();
	//_uint iFont10 = (iCurCoin % 100) / 10;

	switch (_eCurState)
	{
	case CHUD::STARPOINT_IDLE:
	break;
	
	case CHUD::STARPOINT_WAIT:
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vInitPos);

		if (TEXT("Font10_Shadow") == m_UIObjDesc.wstrUITag || TEXT("Font10") == m_UIObjDesc.wstrUITag
		|| TEXT("Font1_Shadow") == m_UIObjDesc.wstrUITag || TEXT("Font1") == m_UIObjDesc.wstrUITag)
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_UIObjDesc.vPos);

		break;

	case CHUD::STARPOINT_HIDE: 
		vCurPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		vCurPos.x += 0.05f;//vCurPos.x += 0.005f;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vCurPos);

		if (TEXT("Font100_Shadow") == m_UIObjDesc.wstrUITag || TEXT("Font10_Shadow") == m_UIObjDesc.wstrUITag || TEXT("Font1_Shadow") == m_UIObjDesc.wstrUITag
			|| TEXT("Font100") == m_UIObjDesc.wstrUITag || TEXT("Font10") == m_UIObjDesc.wstrUITag || TEXT("Font1") == m_UIObjDesc.wstrUITag)
		{
			m_UIObjDesc.vPos.x = vCurPos.x;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_UIObjDesc.vPos);
		}

		break;

	case CHUD::STARPOINT_LOOT:
	
		if (TEXT("Font100_Shadow") == m_UIObjDesc.wstrUITag || TEXT("Font10_Shadow") == m_UIObjDesc.wstrUITag || TEXT("Font1_Shadow") == m_UIObjDesc.wstrUITag
			|| TEXT("Font100") == m_UIObjDesc.wstrUITag || TEXT("Font10") == m_UIObjDesc.wstrUITag || TEXT("Font1") == m_UIObjDesc.wstrUITag)
		{
			if (m_IsMovingUP)
			{
				vCurPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
				vCurPos.y += 5.f;
				m_UIObjDesc.vPos.y = vCurPos.y;
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_UIObjDesc.vPos);
				
				if (_fAccTime > 0.03f)
					m_IsMovingUP = FALSE;
			}
			else //m_IsMovingUP == FALSE;
			{
				vCurPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
				vCurPos.y -= 5.f;
				m_UIObjDesc.vPos.y = vCurPos.y;
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_UIObjDesc.vPos);

				if (m_vInitPos.y >= m_UIObjDesc.vPos.y)
					m_UIObjDesc.vPos.y = m_vInitPos.y;

				m_IsMovingUP = TRUE;
				_fAccTime = 0.f;
			}
		}

	break;

	}
 }

CHUD_StarPoint* CHUD_StarPoint::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHUD_StarPoint* pInstance = new CHUD_StarPoint(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CHUD_StarPoint"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHUD_StarPoint::Clone(void* pArg)
{
	CHUD_StarPoint* pInstance = new CHUD_StarPoint(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CHUD_StarPoint"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHUD_StarPoint::Free()
{
	__super::Free();
}


