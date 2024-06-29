#include "stdafx.h"
#include "HUD_AbilityDiscard.h"
#include "Kirby.h"

CHUD_AbilityDiscard::CHUD_AbilityDiscard(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject { _pDevice, _pContext }
{
}

CHUD_AbilityDiscard::CHUD_AbilityDiscard(const CHUD_AbilityDiscard& _rhs)
	: CUIObject{ _rhs }
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

	UIOBJ_DESC* DiscardUIDesc{};
	if (_pArg != nullptr)
		DiscardUIDesc = (UIOBJ_DESC*)_pArg;

	m_UIObjDesc = *DiscardUIDesc;
	
	if (FAILED(Add_Components()))
		return E_FAIL;
	
	m_pTransformCom->Set_Scaled(m_UIObjDesc.vSize.x, m_UIObjDesc.vSize.y, m_UIObjDesc.vSize.z);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_UIObjDesc.vPos.x,	m_UIObjDesc.vPos.y,	m_UIObjDesc.vPos.z, 1.f));

	//m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	m_eTexState = DISCARD_HIDE;

	//Init 초기 값 사전 저장
	m_vInitPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_vInitSize = m_pTransformCom->Get_Scaled();

#pragma endregion

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	if (pKirby == nullptr)
		return OBJ_NOEVENT;

	//커비 위치정보
	//CTransform* pKirbyTrans = static_cast<CTransform*>(pKirby->Get_Component(g_strTransformTag));
	//_float4 vKirbyPos = pKirbyTrans->Get_State(CTransform::STATE_POSITION);
	//m_vInitPos.y = vKirbyPos.y;
	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vInitPos);

	//커비 능력버리기 정보
	m_fDumpAbilityTime = pKirby->Get_KirbyInfo()->m_fDumpAbilityTime;

	return S_OK;
}

_int CHUD_AbilityDiscard::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS))
		m_IsGaugeUP = TRUE;
	
	//게이지 인디케이터 시작
	if (m_IsGaugeUP)
	{
		m_eTexState = DISCARD_SHOW;
		m_fDumpAbilityTime += fTimeDelta;

		if (m_fDumpAbilityTime > 3.f)
		{
			m_IsGaugeUP = FALSE;
		}
	}
	else //FALSE == m_IsGaugeUP
		m_eTexState = DISCARD_HIDE;

	return OBJ_NOEVENT;
}

void CHUD_AbilityDiscard::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD_AbilityDiscard::Render()
{
	HRESULT hr;

	//숨김 상태에는 렌더x
	if (DISCARD_HIDE == m_eTexState)
		return S_OK;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	//셰이더 파일의 매트릭스 정보를 가져와 바인딩
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	for (_uint iTex = 0; iTex < TEX_NONE; ++iTex)
	{
		SHADER_PS ePassType = { PS_DEFAULT };
		if (TEX_MASK == iTex) //마스크 텍스처에 대한 설정
		{
			ePassType = PS_MASK_HP;
			m_UIObjDesc.fAlpha = 0.1f;

			m_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_UIObjDesc.vColorRGB, sizeof(_float3));

		}

		hr = Bind_ShaderResources(m_pShaderCom, ePassType, m_pTextureCom[iTex], 0);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CHUD_AbilityDiscard::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_HUD_AbilityDiscard"),
		TEXT("Com_TexDiffuse"), (CComponent**)&m_pTextureCom[TEX_DIFFUSE])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_HUD_AbilityDiscard_Mask"),
		TEXT("Com_TexMask"), (CComponent**)&m_pTextureCom[TEX_MASK])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

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
	//Safe_Release(m_pTextures[TEX_NONE]);
}


