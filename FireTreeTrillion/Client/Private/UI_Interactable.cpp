#include "stdafx.h"
#include "UI_Interactable.h"

CUI_Interactable::CUI_Interactable(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CUI_Interactable::CUI_Interactable(const CUI_Interactable& rhs)
	: CUIObject{ rhs }
{
}

HRESULT CUI_Interactable::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Interactable::Initialize(void* pArg)
{
	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	if (FAILED(Add_Components()))
		return E_FAIL;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));
	
	// 사이즈, 위치 디폴트값 지정
	//m_InitialSize = _float2(116.f * 0.5f, 88.f * 0.5f);
	m_InitialSize = _float2(300.f * 0.3f, 200.f * 0.3f);
	m_pTransformCom->Set_Scaled(m_InitialSize.x, m_InitialSize.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	
	m_bIsRender = false;

	return S_OK;
}

_int CUI_Interactable::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	_float fOwnTimeDelta = fTimeDelta * 0.5f;

	// 위치 UPDATE
	CTransform* pTransform = m_pOwner->Get_TransformCom();
	_float4 vPos = pTransform->Get_State_Float4(CTransform::STATE_POSITION);

	//07.22) 대상이 DEEDEEDEE일 경우, ZOffset 값을 별도로 세팅
	_float fZOffset = { 0.f };
	if (TEXT("Prototype_GameObject_DeeDeeDee") == m_pOwner->Get_PrototypeTag())
		fZOffset = -1.f;

	//Update_Pos(_float3(vPos.x, vPos.y + m_fOffset, vPos.z - 1.f));
	Update_Pos(_float3(vPos.x, vPos.y + m_fOffset, vPos.z + fZOffset));
	
	// 사이즈 UPDATE
	static _float fAccTime = 0.f;
	if (m_eState == PLUS)
	{
		fAccTime += fOwnTimeDelta;
		if (fAccTime >= 0.5f)
		{
			fAccTime = 0.5f;
			m_eState = MINUS;
		}
	} 
	else
	{
		fAccTime -= fOwnTimeDelta;
		if (fAccTime < 0.f)
		{
			fAccTime = 0.f;
			m_eState = PLUS;
		}			
	}

	_float fRatio = sin((fAccTime * 3.1415f) / 2);
	m_pTransformCom->Set_Scaled(m_InitialSize.x + m_InitialSize.x * fAccTime,
							    m_InitialSize.y + m_InitialSize.y * fAccTime, 1.f);

	return OBJ_NOEVENT;
}

void CUI_Interactable::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CUI_Interactable::Render()
{
	if (m_bIsRender == false) return S_OK;

	HRESULT hr;
	hr = Bind_ShaderResources();
	CHECK_FAILED(hr);

	hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);

	hr = m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
	CHECK_FAILED(hr);
	
	//hr = m_pShaderCom->Begin(POSTEX_DEFAULT); // 사이즈 변화 POSTEX_SOLIDBLEND
	hr = m_pShaderCom->Begin(POSTEX_ALPHATEST_COLOR); // 사이즈 변화 POSTEX_SOLIDBLEND
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Bind_Buffers();
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Render(); 
	CHECK_FAILED(hr);

	return S_OK;
}

#ifdef _DEBUG
void CUI_Interactable::Render_IMGUI()
{
	//char ratio[16];
	////ImGui::DragFloat(ratio, (_float*)&m_fRatioTimeBar, 0.01f, 0.01f, 1.f);
	//ImGui::Separator(); ImGui::NewLine();

	//for (_int i = 0; i < m_arrPosition.size(); ++i)
	//{
	//	char name[16], size[16], color[16];
	//	sprintf_s(name,  "pos%d",   i);
	//	//sprintf_s(size,  "size%d",  i);

	//	ImGui::DragFloat2(name,  (_float*)&m_arrPosition[i]);
	//	//ImGui::DragFloat(size,   (_float*)&m_arrSizeRatio[i], 0.05f, 0.1f, 2.f);

	//	m_arrSize[i].x = m_arrOriginalSize[i].x * m_arrSizeRatio[i];
	//	m_arrSize[i].y = m_arrOriginalSize[i].y * m_arrSizeRatio[i];

	//	m_pTransformCom->Set_Scaled(m_arrSize[i].x, m_arrSize[i].y, 1.f);
	//	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
	//		XMVectorSet(m_arrPosition[i].x   - g_iWinSizeX * 0.5f,
	//					- m_arrPosition[i].y + g_iWinSizeY * 0.5f,
	//					0.f,
	//					1.f));

	//	ImGui::NewLine();
	//}

	//char test[16], test2[16];
	//ImGui::DragFloat3(test, (_float*)&m_vTESTCOLOR, 0.01f, 0.f, 1.f);
	//ImGui::DragFloat3(test2, (_float*)&m_vTESTCOLOR2, 0.01f, 0.f, 1.f);
}
#endif

HRESULT CUI_Interactable::Add_Components()
{
	HRESULT hr(S_OK);

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
											  TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_Interacting"),
											  TEXT("Com_Texture"), (CComponent**)&m_pTextureCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
											  TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CUI_Interactable::Bind_ShaderResources()
{
	HRESULT hr(S_OK);
	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	return S_OK;
}

void CUI_Interactable::Update_Pos(_float3 _vPosition)
{
	_float4 vNewPosition = _float4{ _vPosition.x, _vPosition.y, _vPosition.z, 1.f };

	_matrix ViewMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW);
	_matrix ProjMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_matrix ComMatrix = ViewMatrix * ProjMatrix;

	_float4 vFinPos = XMVector3TransformCoord(XMLoadFloat4(&vNewPosition), ComMatrix);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet((vFinPos.x * g_iWinSizeX) * 0.5f,
					(vFinPos.y * g_iWinSizeY) * 0.5f,
					0.f,
					1.f));
}

CUI_Interactable* CUI_Interactable::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Interactable* pInstance = new CUI_Interactable(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CUI_Interactable"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Interactable::Clone(void* pArg)
{
	CUI_Interactable* pInstance = new CUI_Interactable(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CUI_Interactable"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Interactable::Free()
{
	__super::Free();

	Safe_Release(m_pTextureCom);
}

