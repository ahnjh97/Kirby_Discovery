#include "stdafx.h"
#include "LevelChangeStar.h"
#include "Kirby.h"

CLevelChangeStar::CLevelChangeStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject{ pDevice, pContext }
{
}

CLevelChangeStar::CLevelChangeStar(const CLevelChangeStar& rhs)
	: CUIObject{ rhs }
{
}

HRESULT CLevelChangeStar::Initialize_Prototype()
{
	fill(m_arrTextures.begin(), m_arrTextures.end(), nullptr);

	return S_OK;
}

HRESULT CLevelChangeStar::Initialize(void* pArg)
{
	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	hr = Add_Components();
	CHECK_FAILED(hr);

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	m_pPlayer = static_cast<CKirby*>(m_pGameInstance->Get_GameObject_ByTag(LEVEL_GAMEPLAY, L"Layer_Player", TEXT("Prototype_GameObject_Kirby")));

	return S_OK;
}

//그래서 스태틱으로 되ㅏ어잇는 유아이가 레벨이동되고도 살아잇게끔하기!
_int CLevelChangeStar::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD1, KEY_DOWN))
	{
		CTransform* pTransform = m_pPlayer->Get_TransformCom();
		m_vPlayerPos = pTransform->Get_State(CTransform::STATE_POSITION);
	}

	if(m_vPlayerPos != _float3())
	{
		_float3 vPos = _float3(m_vPlayerPos);
		Update_Pos(vPos);
	}

	return OBJ_NOEVENT;
}

void CLevelChangeStar::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CLevelChangeStar::Render()
{
	HRESULT hr;
	hr = Bind_ShaderResources();
	CHECK_FAILED(hr);

	//QZR : 렌더 테스트 끝나고 사이즈값 함수로 빼기
	m_pTransformCom->Set_Scaled(200.f, 200.f, 1.f);

	hr = m_pShaderCom->Begin(16); //머지할때 17로 바꾸시오.
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Bind_Buffers();
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Render();
	CHECK_FAILED(hr);

	return S_OK;
}

#ifdef _DEBUG
void CLevelChangeStar::Render_IMGUI()
{
}
#endif

HRESULT CLevelChangeStar::Add_Components()
{
	HRESULT hr(S_OK);

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

#pragma region 텍스쳐 컴포넌트
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_SimpleStar"),
		TEXT("Com_Texture_AlphaStar"), (CComponent**)&m_arrTextures[0]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_SimpleStar"),
		TEXT("Com_Texture_SubStar"), (CComponent**)&m_arrTextures[1]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_SimpleStar"),
		TEXT("Com_Texture_LastStar"), (CComponent**)&m_arrTextures[2]);
	CHECK_FAILED(hr);
#pragma endregion

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CLevelChangeStar::Bind_ShaderResources()
{
	HRESULT hr(S_OK);

	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	//for (size_t i = 0; i < m_arrTextures.size(); ++i)
	//{
		//hr = m_arrTextures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
		hr = m_arrTextures[0]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
		CHECK_FAILED(hr);
	//}

	return S_OK;
}

void CLevelChangeStar::Update_Pos(_float3 _vPosition)
{
	_float4 vNewPosition = _float4{ _vPosition.x, _vPosition.y, _vPosition.z, 1.f };

	// 뷰-투영
	_matrix ViewMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW);
	_matrix ProjMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_matrix ComMatrix = ViewMatrix * ProjMatrix;

	// 뿅
	m_vFinPos = XMVector3TransformCoord(XMLoadFloat4(&vNewPosition), ComMatrix);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet((m_vFinPos.x * g_iWinSizeX) * 0.5f,
					(m_vFinPos.y * g_iWinSizeY) * 0.5f + 20.f,
					 0.f,
					 1.f));
}

CLevelChangeStar* CLevelChangeStar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelChangeStar* pInstance = new CLevelChangeStar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CLevelChangeStar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelChangeStar::Clone(void* pArg)
{
	CLevelChangeStar* pInstance = new CLevelChangeStar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CLevelChangeStar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelChangeStar::Free()
{
	for (auto& texure : m_arrTextures)
		Safe_Release(texure);

	/// 태그로 기존에 있는 친구를 가져온거라 safe_release안해줘도 될것같은디
	Safe_Release(m_pPlayer);
	__super::Free();
}

