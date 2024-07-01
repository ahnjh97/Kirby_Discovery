#include "stdafx.h"
#include "LevelChangeStar.h"

#include "Kirby.h"
#include "Utils.h"

const _float	g_fStarSpeed = 0.01f;
const _float	g_fPosOffset = 18.f;

CLevelChangeStar::CLevelChangeStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject{ pDevice, pContext }
{
}

CLevelChangeStar::CLevelChangeStar(const CLevelChangeStar& rhs)
	: CUIObject{ rhs }
	, m_arrTextures(rhs.m_arrTextures)
	, m_arrayStarMatrix(rhs.m_arrayStarMatrix)
{
}

HRESULT CLevelChangeStar::Initialize_Prototype()
{
	fill(m_arrTextures.begin(), m_arrTextures.end(), nullptr);
	fill(m_arrayStarMatrix.begin(), m_arrayStarMatrix.end(), _float4x4());
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

	m_pPlayer = static_cast<CKirby*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, L"Layer_Player", TEXT("Prototype_GameObject_Kirby")));
	m_bIsRender = false;

	return S_OK;
}

_int CLevelChangeStar::Tick(_float fTimeDelta)
{
	m_fTimeDelta = fTimeDelta;//m_pGameInstance->Get_SecondTimer();
	__super::Tick(m_fTimeDelta);

	// for test
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD1, KEY_DOWN))
		Activate(m_fTimeDelta);
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD2, KEY_DOWN))
		Deactivate();

	if (m_bIsRender)
	{
		Update_Pos(m_vFocusingPos);

		// 1초동안 돌아요
		m_fTurningTime += fTimeDelta;
		if (m_fTurningTime >= 1.f)
		{
			CUtils::Rotation(m_arrayStarMatrix[1], _float4(0.f, 0.f, 1.f, 0.f), 0.f);
			// 다 돌면 도는 것을 멈추는 스위치를 켜줘요
			m_bRemove = true;
		}

		// 비율은 줄어들되, 0 이하로 내려가지 않아요.
		m_fRatio -= m_fTimeDelta * 0.9f;
		if (m_fRatio <= 0.f)
			m_fRatio = 0.f;

		// 바뀐 비율을 받아서 별들의 사이즈 값을 처리하요.
		for (_uint i = 0; i < m_arrTextures.size(); ++i)
			Change_Size(i);
	}

	return OBJ_NOEVENT;
}

void CLevelChangeStar::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SUPERUI, this);
}

HRESULT CLevelChangeStar::Render()
{
	if (false == m_bIsRender) return S_OK;

	HRESULT hr;
	hr = Bind_ShaderResources();
	CHECK_FAILED(hr);

	for (_uint i = 0; i < m_arrTextures.size(); ++i)
	{
		//if (i != 1) continue;
		m_pShaderCom->Bind_RawValue("g_iMasking", &i, sizeof(_int));

		hr = m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_arrayStarMatrix[i]);
		CHECK_FAILED(hr);

		// 텍스쳐를 넘긴다.
		hr = m_arrTextures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(16); //머지할때 17로 바꾸시오.
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Bind_Buffers();
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Render();
		CHECK_FAILED(hr);
	}

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
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_Star"),
		TEXT("Com_Texture_AlphaStar"), (CComponent**)&m_arrTextures[0]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_Star"),
		TEXT("Com_Texture_SubStar"), (CComponent**)&m_arrTextures[1]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_Star"),
		TEXT("Com_Texture_LastStar"), (CComponent**)&m_arrTextures[2]);
	CHECK_FAILED(hr);
#pragma endregion

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
	CHECK_FAILED(hr);

	return S_OK;
}

// 고정값들을 셰이더에 바인딩한다.
HRESULT CLevelChangeStar::Bind_ShaderResources()
{
	HRESULT hr(S_OK);

	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelChangeStar::Change_Size(_uint iNum)
{
	//_float fEaseRatio = m_fRatio; // EASE 사용하려면 여기서 조정
	switch (iNum)
	{
	case 0: // 뚫린 투명한 별
	{
		m_DecSize = _float2(m_InitialSize.x * m_fRatio, m_InitialSize.y * m_fRatio);
		if (m_DecSize.x <= m_FinalSize.x || m_DecSize.y <= m_FinalSize.y)
		{
			//if (m_bRemove)	// alpha star meets green star.
			//{
			//	_float2 InitialSizeTemp = m_InitialSize * 2.f;
			//	_float2 DecreSizeTemp = _float2(InitialSizeTemp.x * m_fRatio, InitialSizeTemp.y * m_fRatio);
			//	CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[iNum], DecreSizeTemp.x, DecreSizeTemp.y, 1.f);
			//}
			//else
				CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[iNum], m_FinalSize.x, m_FinalSize.y, 1.f);
		}
		else
			CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[iNum], m_DecSize.x, m_DecSize.y, 1.f);

		_float4 vPos = CUtils::Get_State_Vector_Matrix(m_arrayStarMatrix[iNum], CUtils::STATE_POSITION);
		CUtils::Set_State_Matrix(m_arrayStarMatrix[iNum], CUtils::STATE_POSITION, _float4(vPos.x, vPos.y, vPos.z, 1.f));
		//CUtils::Rotation(m_arrayStarMatrix[iNum], _float4(0.f, 0.f, 1.f, 0.f), 0.f);
	}
	break;
	case 1: // 연두 별                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
	{
		_float2 InitialSize = m_InitialSize * 2.f;
		if (false == m_bRemove)	// 투명별과 마주치면 그만 돌아요
		{
			m_DecGreenSize = _float2(InitialSize.x * m_fRatio, InitialSize.y * m_fRatio);
			CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[iNum], m_DecGreenSize.x, m_DecGreenSize.y, 1.f);
			CUtils::Turn_OtherMatrix(m_arrayStarMatrix[iNum], _float4(0.f, 0.f, 1.f, 0.f), m_fTimeDelta *0.5f, 720.f);
		}
		else
			CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[iNum], m_DecGreenSize.x, m_DecGreenSize.y, 1.f);

		_float4 vPos = CUtils::Get_State_Vector_Matrix(m_arrayStarMatrix[iNum], CUtils::STATE_POSITION);
		CUtils::Set_State_Matrix(m_arrayStarMatrix[iNum], CUtils::STATE_POSITION, _float4(vPos.x, vPos.y, vPos.z + 0.1f, 1.f));
	}
	break; 
	case 2: // 찐 연두 별
	{
		CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[iNum], m_InitialSize.x, m_InitialSize.y, 1.f);
		_float4 vPos = CUtils::Get_State_Vector_Matrix(m_arrayStarMatrix[iNum], CUtils::STATE_POSITION);
		CUtils::Set_State_Matrix(m_arrayStarMatrix[iNum], CUtils::STATE_POSITION, _float4(vPos.x, vPos.y, vPos.z + 0.2f, 1.f));
	}
	break;
	}

	return S_OK;
}

void CLevelChangeStar::Update_Pos(_float4 _vPosition)
{
	_float4 vNewPosition = _float4{ _vPosition.x, _vPosition.y, _vPosition.z, 1.f };

	// 뷰-투영
	_matrix ViewMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW);
	_matrix ProjMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_matrix ComMatrix = ViewMatrix * ProjMatrix;

	// 뿅
	m_vFinPos = XMVector3TransformCoord(XMLoadFloat4(&vNewPosition), ComMatrix);
	_float4 vPos = _float4((m_vFinPos.x * g_iWinSizeX) * 0.5f,
						   (m_vFinPos.y * g_iWinSizeY) * 0.5f + g_fPosOffset,
							0.f, 1.f);

	for (_int i = 0; i < 3 ; ++i)
		CUtils::Set_State_Matrix(m_arrayStarMatrix[i], CUtils::STATE_POSITION, vPos);
}

void CLevelChangeStar::Activate(_float fTimeDelta)
{
	m_bIsRender = true;
	CTransform* pTransform = m_pPlayer->Get_TransformCom();
	m_vFocusingPos = pTransform->Get_State(CTransform::STATE_POSITION);
}

void CLevelChangeStar::Deactivate()
{
	m_bIsRender = false;
	m_bRemove = false;
	m_fRatio = 1.f;
	m_fRatioRemove = 1.f;
	m_fTurningTime = 0.f;
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

	Safe_Release(m_pPlayer);
	__super::Free();
}

