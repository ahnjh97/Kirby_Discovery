#include "stdafx.h"
#include "HUD_KirbyNameTag.h"
#include "Kirby.h"
#include "FinaleKirby.h"

CHUD_KirbyNameTag::CHUD_KirbyNameTag(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject { _pDevice, _pContext }
{
}

CHUD_KirbyNameTag::CHUD_KirbyNameTag(const CHUD_KirbyNameTag& _rhs)
	: CUIObject{ _rhs }
{
}

HRESULT CHUD_KirbyNameTag::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHUD_KirbyNameTag::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	UIOBJ_DESC* BtnIconDesc{};

	if (_pArg != nullptr)
		BtnIconDesc = (UIOBJ_DESC*)_pArg;

	if (FAILED(Add_Components()))
		return E_FAIL;

	//_float3 vOffset = { 0.65f, 0.65f, 1.f };
	//m_vOrigScale = vScale * vOffset;
	_float3 vScale = { 300.f * 0.5f, 50.f * 0.5f, 1.f };
	m_pTransformCom->Set_Scaled(vScale);

	_float4	vTrans = { -700.f, 400.f, 1.f, 1.f };
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vTrans);

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	//m_eCurState = BTN_IDLE;
	m_pCurrentLevelID = m_pGameInstance->Get_CurrentLevelID();

	if (LEVEL_FINALE != *m_pCurrentLevelID)
	{
		m_pKirby = dynamic_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
		Safe_AddRef(m_pKirby);
	}

	if (LEVEL_FINALE == *m_pCurrentLevelID)
	{
		m_pKirby = dynamic_cast<CFinaleKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
		Safe_AddRef(m_pKirby);
	}
	return S_OK;
}

_int CHUD_KirbyNameTag::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	_float3 vScale = { 300.f * 0.75f, 50.f * 0.75f, 1.f };
	m_pTransformCom->Set_Scaled(vScale);

	_float4	vTrans = { -700.f, 400.f, 1.f, 1.f };
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vTrans);


	return OBJ_NOEVENT;
}

void CHUD_KirbyNameTag::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD_KirbyNameTag::Render()
{
	HRESULT hr = S_OK;

#pragma region RENDER_BINDSET

	//렌더 OFF
	//if (BTN_HIDE == m_eCurState && 0.f == m_fBlinkAlpha && 0.f == m_fBtnAlpha)
	//	return S_OK;

#pragma region KIRBY_NAMETAG

	if (nullptr == m_pKirby)
		return E_FAIL;

	CKirby::BODYSTATE eKirbyState = dynamic_cast<CKirby*>(m_pKirby)->Get_KirbyInfo()->m_eBodyState;

	_uint iTexIndex = { TEXNT_NONE };
	switch (eKirbyState)
	{
		//카피 능력 상태
	case CKirby::BODY_SWORDDEFAULT:
	case CKirby::BODY_SWORDBALLOON:
		iTexIndex = TEXNT_SWORD;
		break;

	case CKirby::BODY_BOOMDEFAULT:	iTexIndex = TEXNT_BOMB;	break;
	case CKirby::BODY_HAMMER:		iTexIndex = TEXNT_TOYHAMMER;	break;
	case CKirby::BODY_CRASHDEFAULT:	iTexIndex = TEXNT_CRASH;	break;

		//머금기 변형 상태
	case CKirby::BODY_CARDEFAULT:
	case CKirby::BODY_CARVACUUM:
		iTexIndex = TEXNT_DEFORMCAR;
		break;

	case CKirby::BODY_BULBDEFAULT:
	case CKirby::BODY_BULBVACUUM:
		iTexIndex = TEXNT_DEFORMBULB;
		break;

	case CKirby::BODY_FINALCUT:
		break;

	default:
		iTexIndex = TEXNT_KIRBY;
		break;
	}

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	//POSTEX_SOLIDBLEND_NOZTEST, POSTEX_ALPHABLEND_NOTEST
	hr = Bind_ShaderResources(m_pShaderCom, POSTEX_ALPHABLEND_NOTEST, m_pTextureCom, iTexIndex);
	CHECK_FAILED(hr);

	return S_OK;
}

#ifdef _DEBUG
void CHUD_KirbyNameTag::Render_IMGUI()
{
}
#endif

HRESULT CHUD_KirbyNameTag::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_HUD_StatusBar_NameTag"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_KirbyNameTag::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex)
{
	//셰이더 파일의 텍스처 정보를 가져와 바인딩
	_pTextureCom->Bind_ShaderResource(_pShaderCom, "g_DiffuseTexture", _iTexIndex);

	//셰이더의 원시데이터 가져와 저장
	_pShaderCom->Bind_RawValue("g_vRColor", &m_UIObjDesc.vColorRGB, sizeof(_float3));

	//Begin() > Apply() 함수 호출 전 셰이더 전역 데이터를 저장해야함
	if (FAILED(_pShaderCom->Begin(_iPassIndex)))
		return E_FAIL;

	if (FAILED(Bind_VIBuffer(m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_KirbyNameTag::Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom)
{
	if (FAILED(_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CHUD_KirbyNameTag* CHUD_KirbyNameTag::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHUD_KirbyNameTag* pInstance = new CHUD_KirbyNameTag(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CHUD_KirbyNameTag"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHUD_KirbyNameTag::Clone(void* pArg)
{
	CHUD_KirbyNameTag* pInstance = new CHUD_KirbyNameTag(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CHUD_KirbyNameTag"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHUD_KirbyNameTag::Free()
{
	__super::Free();	
	Safe_Release(m_pKirby);
}


