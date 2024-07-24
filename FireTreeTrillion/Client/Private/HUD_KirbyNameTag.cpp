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
	_float3 vScale = { 300.f * 0.5f, 100.f * 0.5f, 1.f };
	m_pTransformCom->Set_Scaled(vScale);

	//_float4	vTrans = { -690.f, 420.f, 1.f, 1.f };
	_float4	vTrans = { -690.f, 417.5f, 1.f, 1.f };
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vTrans);
	m_vInitPos = vTrans;

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

	_float4 vPos = {};
	switch (m_eCurState)
	{
	case NAMETAG_HIDE:
		m_UIObjDesc.fAlpha -= fTimeDelta * 5.f;

		vPos = GET_POS;
		vPos.x -= fTimeDelta * 20.f;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		break;

	case NAMETAG_SHOW:
		m_UIObjDesc.fAlpha = 1.f;

		vPos = GET_POS;
		vPos.x = m_vInitPos.x;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		break;

	case NAMETAG_DAMAGE:
		/*
		m_fStartShake += fTimeDelta;

		_float fShakeValue = _fShakeIntensity * std::sin(m_fShakeTime * 70.0f); //지진 강도(진폭) 설정
		if (fShakeValue > 0)

		else
			m_fShakeTime = 0.f;

		if (m_fStartShake > 0.4f)
		{
			m_fStartShake = 0.f;
					vPos = GET_POS;
			vPos.y = m_vInitPos.y;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		}
		*/
		break;
	}

	if (m_UIObjDesc.fAlpha >= 1.f)
		m_UIObjDesc.fAlpha = 1.f;

	if (m_UIObjDesc.fAlpha <= 0.f) //알파 값 보정 및 업데이트 중지
	{
		m_UIObjDesc.fAlpha = 0.f;
		return OBJ_NOEVENT;
	}

	return OBJ_NOEVENT;
}

void CHUD_KirbyNameTag::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD_KirbyNameTag::Render()
{
	HRESULT hr = S_OK;
	if (nullptr == m_pKirby)
		return E_FAIL;

#pragma region RENDER_BINDSET

	//렌더 OFF
	if (NAMETAG_HIDE == m_eCurState && 0.f == m_UIObjDesc.fAlpha)
		return S_OK;

#pragma region KIRBY_NAMETAG

	CKirby::BODYSTATE eVacuumState = dynamic_cast<CKirby*>(m_pKirby)->Get_KirbyInfo()->m_eBodyState; //머금기 변형 정보
	TEX_NAMETAG eTexIndex = Check_TexIndex();

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	//POSTEX_SOLIDBLEND_NOZTEST, POSTEX_ALPHABLEND_NOTEST
	hr = Bind_ShaderResources(m_pShaderCom, POSTEX_ALPHABLEND_NOTEST, m_pTextureCom, eTexIndex);
	CHECK_FAILED(hr);

	return S_OK;
}

#ifdef _DEBUG
void CHUD_KirbyNameTag::Render_IMGUI()
{
}
#endif

CHUD_KirbyNameTag::TEX_NAMETAG CHUD_KirbyNameTag::Check_TexIndex()
{
	//07.24) 카피 능력 / 머금기 변경 정보 분리
	CKirby::BODYSTATE eVacuumState = dynamic_cast<CKirby*>(m_pKirby)->Get_KirbyInfo()->m_eBodyState; //머금기 변형 정보

	if (CKirby::BODY_CARDEFAULT == eVacuumState)
		return TEXNT_DEFORMCAR;

	if (CKirby::BODY_BULBDEFAULT == eVacuumState)
		return TEXNT_DEFORMBULB;

	ABILITYTYPE eAbilityType = dynamic_cast<CKirby*>(m_pKirby)->Get_AbilityType(); //카피 능력 정보
	switch (eAbilityType)
	{
	case ABILITY_DEFAULT:	return TEXNT_KIRBY;		break;
	case ABILITY_SWORD:		return TEXNT_SWORD;		break;
	case ABILITY_HAMMER:	return TEXNT_TOYHAMMER;	break;
	case ABILITY_BOMB:		return TEXNT_BOMB;	break;
	case ABILITY_CRASH:		return TEXNT_CRASH;	break;
	case ABILITY_END:	default:
		break;
	}

	return TEXNT_KIRBY;
}

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

	_pShaderCom->Bind_RawValue("g_fAlpha", &m_UIObjDesc.fAlpha, sizeof(_float));

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


