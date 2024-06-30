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

	m_eTexState = DISCARD_IDLE;

	//Init 초기 값 사전 저장
	//m_vInitPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	//m_vInitSize = m_pTransformCom->Get_Scaled();

	return S_OK;
}

_int CHUD_AbilityDiscard::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS))
	{
		//매 틱마다 정보를 갱신할 필요는 없으므로, 해당 상태일 때 커비 정보를 체크
		CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
		if (pKirby == nullptr)
			return OBJ_NOEVENT;

		//커비 능력버리기 시간
		m_fDumpAbilityTime = pKirby->Get_KirbyInfo()->m_fDumpAbilityTime;

		if (m_fDumpAbilityTime <= 0) //사전에 능력이 없을 경우를 덤프시간으로 체크
			m_eTexState = DISCARD_IDLE;

		else
			m_IsGaugeUP = TRUE;
	}

	//게이지 인디케이터 시작
	if (m_IsGaugeUP)
	{
#pragma region SET VIEWPORT MATRIX

		//매 틱마다 정보를 갱신할 필요는 없으므로, 해당 상태일 때 커비 정보를 체크
		CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
		if (pKirby == nullptr)
			return OBJ_NOEVENT;

		//커비 위치정보
		CTransform* pKirbyTrans = static_cast<CTransform*>(pKirby->Get_Component(g_strTransformTag));
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
		
#pragma endregion

		m_eTexState = DISCARD_SHOW;

		//m_fDumpAbilityTime += fTimeDelta;
		m_fGaugeRatio = m_fDumpAbilityTime; //게이지 비율 :: 
		m_UIObjDesc.fAlpha = 1.f;

		if (m_fDumpAbilityTime <= 1.f) //현재 게이지 대비 버리는시간 값 비교
		{
			m_IsGaugeUP = FALSE;
			m_fDumpAbilityTime = 0.f;
		}
	}
	else //FALSE == m_IsGaugeUP
	{ 
		m_eTexState = DISCARD_HIDE;
		m_fHideAnimTime += fTimeDelta;
		m_UIObjDesc.fAlpha -= fTimeDelta * 5.f;

		if (m_UIObjDesc.fAlpha <= 0) //알파 값 보정
			m_UIObjDesc.fAlpha = 0.f;

		if (m_fHideAnimTime > 3.f) //시간 경과할 경우 idle 상태로 변경
		{
			m_eTexState = DISCARD_IDLE;
			m_fHideAnimTime = 0.f;
		}
	}
	
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
	if (DISCARD_IDLE == m_eTexState)
		return S_OK;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	//셰이더 파일의 매트릭스 정보를 가져와 바인딩
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	for (_uint iTexType = 0; iTexType < TEX_NONE; ++iTexType)
	{
		PASS_POSTEX ePassType = { POSTEX_ALPHABLEND_NOTEST };
		if (TEX_MASK == iTexType) //마스크 텍스처에 대한 설정
		{
			ePassType = POSTEX_UI_MASK;
			m_pTextureCom[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0);
			m_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_fGaugeRatio, sizeof(_float3));
		}
		else
		{
			for (_uint iTexIndex = 0; iTexIndex < 3; ++iTexIndex)
			{
				hr = Bind_ShaderResources(m_pShaderCom, ePassType, m_pTextureCom[iTexType], iTexIndex);
				CHECK_FAILED(hr);
			}
		}
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
	for (auto& iTex : m_pTextureCom)
		Safe_Release(iTex);

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);

}


