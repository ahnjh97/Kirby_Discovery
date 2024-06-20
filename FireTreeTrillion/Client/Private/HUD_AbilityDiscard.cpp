#include "stdafx.h"
#include "HUD_AbilityDiscard.h"
#include "Kirby.h"

CHUD_AbilityDiscard::CHUD_AbilityDiscard(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CHUD{ _pDevice, _pContext }
{
}

CHUD_AbilityDiscard::CHUD_AbilityDiscard(const CHUD_AbilityDiscard& _rhs)
	: CHUD{ _rhs }
{
}

HRESULT CHUD_AbilityDiscard::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHUD_AbilityDiscard::Initialize(void* _pArg)
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

	//Kirby > DumpAbility Time으로 연동 가능
#pragma region INIT DATA

	m_eCurState = DISCARD_IDLE;
	m_ePreState = DISCARD_NONE;

	//Init 초기 값 사전 저장
	m_vInitPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_vInitSize = m_pTransformCom->Get_Scaled();
	m_fInitAlpha = m_UIObjDesc.fAlpha;

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	if (pKirby == nullptr)
		return OBJ_NOEVENT;
	
	//커비 능력버리기 정보
	_float fDumpAbilityTime =  pKirby->Get_KirbyInfo()->m_fDumpAbilityTime;

	//커비 위치정보
	CTransform* pKirbyTrans = static_cast<CTransform*>(pKirby->Get_Component(g_strTransformTag));

#pragma endregion

	return S_OK;
}

_int CHUD_AbilityDiscard::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	Update_UIState(fTimeDelta);


	if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS)) //키꾹 인식
	{
		m_eCurState = DISCARD_SHOW;
	}	

	return OBJ_NOEVENT;
}

void CHUD_AbilityDiscard::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD_AbilityDiscard::Render()
{
	//대기 상태일 경우, 렌더x
	if (DISCARD_IDLE == m_eCurState) //&& KIRBYHP_NONE == m_ePreState)
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

		wstring wstrFontTag = { TEXT("Font_HUDSub_KR15") };

		m_pGameInstance->Render_Font(wstrFontTag, m_UIObjDesc.wstrText, vFontPos, vFontRGBA,
			XMConvertToRadians(m_UIObjDesc.vDegree.z), vFontOrig, vFontScale);
	}

	return S_OK;
}

HRESULT CHUD_AbilityDiscard::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_AbilityDiscard"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_AbilityDiscard_Mask"),
		TEXT("Com_TexMask"), (CComponent**)&m_pTextures[TEX_MASK])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_AbilityDiscard::Render_BindSet(CShader* _pShaderCom, CTransform* _pTransCom)
{
	if (TEXT("BtnIcon_Shadow") == m_UIObjDesc.wstrUITag) //잠시 OFF
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
	//if (TEXT("Icon") == m_UIObjDesc.wstrUITag) { ePassIndex = PS_DEFAULT; }

	if (FAILED(Bind_ShaderResources(_pShaderCom, ePassIndex, m_pTextureCom, m_iTexIndex)))
		return E_FAIL;

	//Mask
	//if (FAILED(Bind_ShaderResources(_pShaderCom, ePassIndex, m_pTextures[TEX_MASK], m_iTexIndex)))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CHUD_AbilityDiscard::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex)
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

HRESULT CHUD_AbilityDiscard::Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom)
{
	if (FAILED(_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CHUD_AbilityDiscard::Update_UIState(_float _fTimeDelta)
{
	switch (m_eCurState)
	{
	case DISCARD_IDLE: //1) 렌더x 기본 상태
		Play_Animation(m_fAccTime, DISCARD_IDLE);
		break;

	case DISCARD_HIDE: //시간경과 후 사라짐
		if (DISCARD_SHOW == m_ePreState) //이전 상태 확인
		{
			m_fAccTime += _fTimeDelta;
			if (m_fAccTime > 0.16f)
			{
				m_fAccTime = 0.f;
				m_eCurState = DISCARD_IDLE;
			}
			else
				Play_Animation(m_fAccTime, DISCARD_HIDE);
		}
		break;

	case DISCARD_SHOW: //키입력
		m_ePreState = DISCARD_SHOW;
		break;

	case DISCARD_NONE:
		break;
	default:
		break;
	}
}

void CHUD_AbilityDiscard::Play_Animation(_float _fAccTime, ABILITYDISCARD_STATE _eCurState)
{
	switch (m_eCurState)
	{
	case DISCARD_IDLE:

		break;
	case DISCARD_HIDE:
		break;
	case DISCARD_SHOW:
		break;
	case DISCARD_NONE:
		break;
	default:
		break;
	}
 }

CHUD_AbilityDiscard* CHUD_AbilityDiscard::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHUD_AbilityDiscard* pInstance = new CHUD_AbilityDiscard(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CHUD_AbilityDiscard"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHUD_AbilityDiscard::Clone(void* pArg)
{
	CHUD_AbilityDiscard* pInstance = new CHUD_AbilityDiscard(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CHUD_AbilityDiscard"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHUD_AbilityDiscard::Free()
{
	__super::Free();
	Safe_Release(m_pTextures[TEX_NONE]);
}


