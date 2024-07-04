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
		XMVectorSet(m_UIObjDesc.vPos.x, m_UIObjDesc.vPos.y, m_UIObjDesc.vPos.z, 1.f));

	//m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	//m_eTexState = DISCARD_IDLE;

	m_pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	Safe_AddRef(m_pKirby);


	return S_OK;
}

_int CHUD_AbilityDiscard::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (m_pKirby == nullptr)
	{
		m_pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
		Safe_AddRef(m_pKirby);
	}


	// 여기에서 항상 플레이어의 덤프 타임을 측정한다.
	// 0.f ~ 1.f 수가 계산될 것이다.
	Compute_PlayerDumpAbiliyTime();
	
	// 단순히, 내가 키를 입력하면 알파값이 증가, 키를 입력하지 않으면 알파값이 감소하는 로직이다.
	if (Key_InputSystem(fTimeDelta) == false)
		return OBJ_NOEVENT;

	if (m_UIObjDesc.fAlpha <= 0.f) //알파 값 보정 및 알파 값 기준 업데이트 중지
	{
		m_UIObjDesc.fAlpha = 0.f;
		return OBJ_NOEVENT;
	}
	else
	{
		if (m_pKirby == nullptr) return OBJ_NOEVENT;

		// UI 가 플레이어를 따라가는 로직이다.
		ChaseUI_To_Player();
	}
	return OBJ_NOEVENT;

#pragma region EASING 1 SCOOP
	/*
	if (m_IsGaugeBLINK) //게이지UI 애니메이션
	{
		if (m_fBLINKAnimTime < 0.5f)
			m_fBLINKAnimTime += fTimeDelta;

		else if (m_fBLINKAnimTime > 0.5f)
			m_fBLINKAnimTime -= fTimeDelta;

		_float3 vScale = { 1.1f, 1.1f, 1.f };
		vScale.x += EASE_OUT(m_fBLINKAnimTime * 2.f); //그래프 MAX값은 1이어야하며, 범위는 0 ~ 1로 설정되어야함
		vScale.y += EASE_OUT(m_fBLINKAnimTime * 2.f);
		m_pTransformCom->Set_Scaled(vScale);
	}
	*/
#pragma endregion
	
	//버튼 입력X
	//m_IsGaugeBLINK = FALSE;
	//m_fHIDEAnimTime += fTimeDelta;

	//m_UIObjDesc.fAlpha -= fTimeDelta * 5.f;
	//if (m_fHIDEAnimTime > 5.f) //시간 경과할 경우 idle 상태로 변경
	//	m_fHIDEAnimTime = 0.f;
}

void CHUD_AbilityDiscard::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD_AbilityDiscard::Render()
{
	HRESULT hr;

	//For.Mask
	if (FAILED(m_pTexCom[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_fDumpAbilityTime, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	//셰이더 파일의 매트릭스 정보를 가져와 바인딩
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	PASS_POSTEX ePassType = { POSTEX_ALPHABLEND_NOTEST };

	for (_uint iTexIndex = 0; iTexIndex < TEXDC_NONE; ++iTexIndex)
	{
		if (TEXDC_GAUGE == iTexIndex)
			ePassType = POSTEX_BOSS_BARPASS_DEFAULT;

		if (TEXDC_BTN == iTexIndex)
			ePassType = POSTEX_UIWHITEALPHA;

		hr = Bind_ShaderResources(m_pShaderCom, ePassType, m_pTexCom[TEX_DIFFUSE], iTexIndex);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

void CHUD_AbilityDiscard::ChaseUI_To_Player()
{
	//커비 위치정보
	CTransform* pKirbyTrans = static_cast<CTransform*>(m_pKirby->Get_Component(g_strTransformTag));
	_float4 vKirbyPos = pKirbyTrans->Get_State(CTransform::STATE_POSITION);

	//뷰포트 공간 상의 X, Y를 정보를 구함
	_matrix VPMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vViewportPos = XMVector3TransformCoord(vKirbyPos, VPMatrix);
	_float fViewX = XMVectorGetX(vViewportPos);
	_float fViewY = XMVectorGetY(vViewportPos);
	fViewY -= 0.1f;

	//뷰포트 기준 위치 재설정
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_UIObjDesc.vPos.x * fViewX, m_UIObjDesc.vPos.y * fViewY, m_UIObjDesc.vPos.z, 1.f));
}

void CHUD_AbilityDiscard::Compute_PlayerDumpAbiliyTime()
{
	//if (m_pKirby == nullptr)
	//	return;

	//m_fGaugeRatio = m_fDumpAbilityTime / 1.f; //게이지 비율
	m_fDumpAbilityTime = m_pKirby->Get_KirbyInfo()->m_fDumpAbilityTime; //덤프시간 체크

	if (m_fDumpAbilityTime > 1.f)
		m_fDumpAbilityTime = 1.f;

	else if (m_fDumpAbilityTime < 0.f)
		m_fDumpAbilityTime = 0.f;
}

_bool CHUD_AbilityDiscard::Key_InputSystem(_float fTimeDelta)
{
	//보유 어빌이 없을 경우를 덤프시간으로 체크 (어떤 상황에서나 V키를 입력할 경우에 대한 예외처리)
	//이는 덤프시간 MAX치 오버될 경우 (value >= 1.f)와 이어지며, 값이 1로 넘어가자마자 0으로 변경됨
	if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS) && m_fDumpAbilityTime > 0.f)
	{
		if (m_pKirby == nullptr)
			return false;

		m_UIObjDesc.fAlpha += fTimeDelta * 5.f;

		if (m_UIObjDesc.fAlpha > 1.f)
			m_UIObjDesc.fAlpha = 1.f;
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS) == false || m_fDumpAbilityTime <= 0.f)
	{
		m_UIObjDesc.fAlpha -= fTimeDelta * 5.f;

		if (m_UIObjDesc.fAlpha < 0.f) //시간 경과할 경우 idle 상태로 변경
			m_UIObjDesc.fAlpha = 0.f;
	}

	return true;
}

HRESULT CHUD_AbilityDiscard::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_HUD_AbilityDiscard"),
		TEXT("Com_TexDiffuse"), (CComponent**)&m_pTexCom[TEX_DIFFUSE])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_HUD_AbilityDiscard_Mask"),
		TEXT("Com_TexMask"), (CComponent**)&m_pTexCom[TEX_MASK])))
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

	for (auto& iTex : m_pTexCom)
		Safe_Release(iTex);

	Safe_Release(m_pKirby);
}


