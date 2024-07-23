#include "stdafx.h"
#include "HUD_AbilityDiscard.h"
#include "Kirby.h"
#include "FinaleKirby.h"

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

	UIOBJ_DESC DiscardUIDesc{};
	if (_pArg != nullptr)
		DiscardUIDesc = *(UIOBJ_DESC*)_pArg;
	
	m_UIObjDesc = DiscardUIDesc;

	if (FAILED(Add_Components()))
		return E_FAIL;
	
	m_pTransformCom->Set_Scaled(m_UIObjDesc.vSize.x, m_UIObjDesc.vSize.y, m_UIObjDesc.vSize.z);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_UIObjDesc.vPos.x, m_UIObjDesc.vPos.y, m_UIObjDesc.vPos.z, 1.f));

	//m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	LEVEL eLevel = (LEVEL)*m_pGameInstance->Get_CurrentLevelID();

	if (LEVEL_FINALE != *m_pCurrentLevelID)
	{
		m_pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(eLevel, TEXT("Layer_Player")));
		Safe_AddRef(m_pKirby);
	}
	
	if (LEVEL_FINALE == *m_pCurrentLevelID) //피날레 레벨에 대한 처리. 다만 현재 피날레 레벨은 어빌 덤프타임 정보가 없는 상태
	{
		m_pKirby = static_cast<CFinaleKirby*>(m_pGameInstance->Get_GameObject(eLevel, TEXT("Layer_Player")));
		Safe_AddRef(m_pKirby);
	}
	
	m_eCurState = DISCARD_HIDE;
	m_UIObjDesc.fAlpha = 0.f;

	return S_OK;
}

_int CHUD_AbilityDiscard::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (nullptr == m_pKirby)
		return OBJ_NOEVENT;

	Compute_PlayerDumpAbiliyTime(); //덤프타임 측정. (0.f ~ 1.f)
	
	if (Key_InputSystem(fTimeDelta) == false) //키입력 여부에 따라 알파 값 증감 
		return OBJ_NOEVENT;

	if (m_UIObjDesc.fAlpha <= 0.f) //알파 값 보정 및 알파 값 기준 업데이트 중지
	{
		m_UIObjDesc.fAlpha = 0.f;
		return OBJ_NOEVENT;
	}
	else
		ChaseUI_To_Player(); 	//플레이어 위치를 기준으로 UI 위치도 보정

	return OBJ_NOEVENT;
}

void CHUD_AbilityDiscard::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD_AbilityDiscard::Render()
{
	if (DISCARD_HIDE == m_eCurState && 0.f == m_UIObjDesc.fAlpha)
		return S_OK;

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

	//07.23) 카피 능력 버리기와 머금기 변형의 뱉기 상태에 대한 텍스처 세분화
	if (nullptr == m_pKirby)
		return E_FAIL;
	CKirby::BODYSTATE eKirbyState = dynamic_cast<CKirby*>(m_pKirby)->Get_KirbyInfo()->m_eBodyState;

	PASS_POSTEX ePassType = { POSTEX_ALPHABLEND_NOTEST };
	for (_uint iTexIndex = 0; iTexIndex < TEXDC_NONE; ++iTexIndex)
	{
		switch (eKirbyState)
		{
		//카피 능력 상태
		case CKirby::BODY_SWORDDEFAULT:
		case CKirby::BODY_SWORDBALLOON:
		case CKirby::BODY_BOOMDEFAULT:
		case CKirby::BODY_HAMMER:
		case CKirby::BODY_CRASHDEFAULT:
			if (TEXDC_DEFORMBASE == iTexIndex)
				continue;
			break;

		//머금기 변형 상태
		case CKirby::BODY_CARDEFAULT:
		case CKirby::BODY_CARVACUUM:
		case CKirby::BODY_BULBDEFAULT:
		case CKirby::BODY_BULBVACUUM:
			if (TEXDC_ABILITYBASE == iTexIndex)
				continue;
			break;

		//피날레 컷씬, 일반 커비 폼은 렌더x (능력 및 머금기 해제 시에 잠깐의 틱 동안 UI를 렌더하므로 처리)
		default:
			return S_OK;
			break;
		}

		if (TEXDC_GAUGE == iTexIndex)
			ePassType = POSTEX_BOSS_BARPASS_DEFAULT;  

		if (TEXDC_BTN == iTexIndex)
			ePassType = POSTEX_UIWHITEALPHA;

		hr = Bind_ShaderResources(m_pShaderCom, ePassType, m_pTexCom[TEX_DIFFUSE], iTexIndex);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

#ifdef DEBUG
void CHUD_AbilityDiscard::Render_IMGUI()
{
	switch (m_eCurState)
	{
	case DISCARD_IDLE:			ImGui::Text(u8"DISCARD_IDLE"); break;
	case DISCARD_HIDE:		ImGui::Text(u8"DISCARD_HIDE"); break;
	case DISCARD_SHOW:		ImGui::Text(u8"DISCARD_SHOW"); break;
	case DISCARD_NONE:	default: ImGui::Text(u8"DISCARD_NONE"); break;
	}
}

#endif // DEBUG

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
	//m_fGaugeRatio = m_fDumpAbilityTime / 1.f; //게이지 비율
	if (LEVEL_FINALE != *m_pCurrentLevelID)
		m_fDumpAbilityTime = dynamic_cast<CKirby*>(m_pKirby)->Get_KirbyInfo()->m_fDumpAbilityTime; //덤프타임 정보 저장

	//if (LEVEL_FINALE == *m_pCurrentLevelID)
	//	m_fDumpAbilityTime = static_cast<CFinaleKirby*>(m_pKirby)->Get_KirbyInfo()->m_fDumpAbilityTime; //피날레 커비는 해당 정보가 없음.

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
		m_eCurState = DISCARD_SHOW;
		m_UIObjDesc.fAlpha += fTimeDelta * 5.f;

		if (m_UIObjDesc.fAlpha > 1.f)
			m_UIObjDesc.fAlpha = 1.f;
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS) == false || m_fDumpAbilityTime <= 0.f)
	{
		m_eCurState = DISCARD_HIDE;
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


