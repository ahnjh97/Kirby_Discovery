#include "stdafx.h"
#include "HUD_StarPoint.h"

CHUD_StarPoint::CHUD_StarPoint(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CHUD{ _pDevice, _pContext }
{
}

CHUD_StarPoint::CHUD_StarPoint(const CHUD_StarPoint& _rhs)
	: CHUD{ _rhs }
{
}

HRESULT CHUD_StarPoint::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHUD_StarPoint::Initialize(void* _pArg)
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
	m_UIObjDesc.vDegree = (*HUDStarPoint_Desc).vDegree;

	if (UI_TEXTURE == m_UIObjDesc.eUIType)
		m_iTexIndex = (*HUDStarPoint_Desc).iTexIndex;

	if (UI_FONT == m_UIObjDesc.eUIType)
		m_UIObjDesc.wstrText = (*HUDStarPoint_Desc).wstrText;


	m_pTransformCom->Set_Scaled(m_UIObjDesc.vSize.x, m_UIObjDesc.vSize.y, m_UIObjDesc.vSize.z);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_UIObjDesc.vPos.x - m_UIObjDesc.vCenter.x + m_UIObjDesc.vCenter.x,
			m_UIObjDesc.vPos.y - m_UIObjDesc.vCenter.y + m_UIObjDesc.vCenter.y, m_UIObjDesc.vPos.z, 1.f));

#pragma region SET_PROJ

	if (PROJ_ORTHO == m_UIObjDesc.eUIProj)
	{
		m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
		XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));
	}

	if (PROJ_PERSPEC == m_UIObjDesc.eUIProj)
	{
		m_pTransformCom->Rotation(XMVectorSet(AXIS_X), XMConvertToRadians(m_UIObjDesc.vDegree.x));
		m_pTransformCom->Rotation(XMVectorSet(AXIS_Y), XMConvertToRadians(m_UIObjDesc.vDegree.y));
		m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
	}

#pragma endregion

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());

	m_eSPstate = STARPOINT_IDLE;

	return S_OK;
}

_int CHUD_StarPoint::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	
	if (m_pGameInstance->Get_DIKeyState(DIK_1, KEY_DOWN))
	{
		m_fAccTime = 0.f;
		m_eSPstate = STARPOINT_LOOT;
		if (m_UIObjDesc.wstrUITag == TEXT("Font100") || m_UIObjDesc.wstrUITag == TEXT("Font10") || m_UIObjDesc.wstrUITag == TEXT("Font1")
			|| m_UIObjDesc.wstrUITag == TEXT("Font100_Shadow") || m_UIObjDesc.wstrUITag == TEXT("Font10_Shadow") || m_UIObjDesc.wstrUITag == TEXT("Font1_Shadow"))
		{
			_uint iCount = stoi(m_UIObjDesc.wstrText);
			iCount += 1;

			m_UIObjDesc.wstrText = to_wstring(iCount);
		}
	}

	switch (m_eSPstate)
	{
	case CHUD::STARPOINT_IDLE:
		m_fAccTime = 0.f;
		Play_Animation(fTimeDelta, STARPOINT_IDLE);

		if (STARPOINT_LOOT == m_eSPrePstate)
		{
			if (m_fAccTime >= 100.f / 144.f)
				m_eSPstate = STARPOINT_HIDE;
		}		
		break;

	case CHUD::STARPOINT_HIDE: //Frame 344 > 354
		Play_Animation(fTimeDelta, STARPOINT_HIDE);
		break;

	case CHUD::STARPOINT_SHOW: //Frame 213 > 223
		Play_Animation(fTimeDelta, STARPOINT_SHOW);
		break;

	case CHUD::STARPOINT_LOOT: //Frame 50 > 64
		m_fAccTime += fTimeDelta;
		if (m_fAccTime >= 14.f / 144.f)
		{
			m_eSPstate = STARPOINT_IDLE;
			m_eSPrePstate = STARPOINT_HIDE;
		}
		Play_Animation(m_fAccTime, STARPOINT_LOOT);
		break;

	case CHUD::STARPOINT_DROP:
		break;

	case CHUD::STARPOINT_NONE:
	default:	break;
	}

	return OBJ_NOEVENT;
}

void CHUD_StarPoint::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD_StarPoint::Render()
{
	if (UI_TEXTURE == m_UIObjDesc.eUIType)
	{
		if (PROJ_ORTHO == m_UIObjDesc.eUIProj)
			Render_OrthoProj(m_pShaderCom, m_pTransformCom);

		if (PROJ_PERSPEC == m_UIObjDesc.eUIProj)
			Render_PerspecProj(m_pShaderCom, m_pTransformCom);
	}

	if (UI_FONT == m_UIObjDesc.eUIType)
	{
		_float2 vFontPos = { m_UIObjDesc.vPos.x + m_UIObjDesc.vCenter.x,
							-m_UIObjDesc.vPos.y + m_UIObjDesc.vCenter.y };

		_float4 vFontRGBA = { m_UIObjDesc.vColorRGB.x, m_UIObjDesc.vColorRGB.y, m_UIObjDesc.vColorRGB.z, m_UIObjDesc.fAlpha };
		_float2 vFontOrig = { 1.f, 1.f };
		_float2 vFontScale = { 1.f, 1.f };

		if (m_UIObjDesc.wstrUITag == TEXT("Font_Shadow"))
			vFontScale = { 1.f, 1.f };

		wstring wstrFontTag = { TEXT("Font_HUD_StarPoint_NUM30") };

		if (m_bIsRender == FALSE)
		{
			if (m_UIObjDesc.wstrUITag == TEXT("Font100") || m_UIObjDesc.wstrUITag == TEXT("Font10"))
				return S_OK;
		
			if (m_UIObjDesc.wstrUITag == TEXT("Font100_Shadow") || m_UIObjDesc.wstrUITag == TEXT("Font10_Shadow"))
				return S_OK;
		}

		m_pGameInstance->Render_Font(wstrFontTag, m_UIObjDesc.wstrText, vFontPos, vFontRGBA,
			XMConvertToRadians(m_UIObjDesc.vDegree.z), vFontOrig, vFontScale);
	}

	return S_OK;
}

HRESULT CHUD_StarPoint::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_StarPoint"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_StarPoint::Render_OrthoProj(CShader* _pShaderCom, CTransform* _pTransCom)
{
	if (m_bIsRender == FALSE)
	{
		if (m_UIObjDesc.wstrUITag == TEXT("Effect"))
			return S_OK;

		if (m_UIObjDesc.wstrUITag == TEXT("Effect_Mask"))
			return S_OK;
	}

	CHECK_NULLPTR(_pShaderCom);

	if (FAILED(_pTransCom->Bind_ShaderResource(_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	//셰이더 파일의 매트릭스 정보를 가져와 바인딩
	if (FAILED(_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	//아이콘의 경우, 기본값으로 렌더
	SHADER_PS ePSIndex = { PS_ALPHABLEND };
	if (m_UIObjDesc.wstrUITag == TEXT("Icon")) //|| m_UIObjDesc.wstrUITag == TEXT("Effect_Mask"))
		ePSIndex = PS_DEFAULT;

	if (FAILED(Bind_ShaderResources(_pShaderCom, ePSIndex, m_pTextureCom, m_iTexIndex)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_StarPoint::Render_PerspecProj(CShader* _pShaderCom, CTransform* _pTransCom)
{
	CHECK_NULLPTR(_pShaderCom);

	if (FAILED(_pTransCom->Bind_ShaderResource(_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	_float4x4 ViewMatrix{}; //= m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	XMStoreFloat4x4(&ViewMatrix, XMMatrixIdentity());
	_float4x4 ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);

	//셰이더 파일의 매트릭스 정보를 가져와 바인딩
	if (FAILED(_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;

	if (FAILED(_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjMatrix)))
		return E_FAIL;

	if (FAILED(Bind_ShaderResources(_pShaderCom, PS_ALPHABLEND, m_pTextureCom, m_iTexIndex)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_StarPoint::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex)
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

HRESULT CHUD_StarPoint::Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom)
{
	if (FAILED(_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CHUD_StarPoint::Play_Animation(_float _fAccTime, HUD_STARPOINT _eSPstate)
{
	switch (_eSPstate)
	{
	case CHUD::STARPOINT_IDLE:
		if (m_UIObjDesc.wstrUITag == TEXT("Effect"))
		{
			m_bIsRender = FALSE;
			m_iTexIndex = 3;
		}

		if (m_UIObjDesc.wstrUITag == TEXT("Effect_Mask"))
		{
			m_bIsRender = FALSE;

			m_UIObjDesc.vDegree.z = 0.f;
			m_UIObjDesc.vSize.x = 128.f;
			m_UIObjDesc.vSize.y = 128.f;

			m_UIObjDesc.fAlpha = 10.f / 255.f;
		}

		if (m_UIObjDesc.wstrUITag == TEXT("Font100") || m_UIObjDesc.wstrUITag == TEXT("Font10") || m_UIObjDesc.wstrUITag == TEXT("Font1"))
			m_UIObjDesc.vPos.y = 417.f;

		if (m_UIObjDesc.wstrUITag == TEXT("Font100_Shadow") || m_UIObjDesc.wstrUITag == TEXT("Font10_Shadow") || m_UIObjDesc.wstrUITag == TEXT("Font1_Shadow"))
			m_UIObjDesc.vPos.y = 413.f;

	break;

	case CHUD::STARPOINT_HIDE: //X값 우측 이동, 알파 값 죽이기
		m_UIObjDesc.vPos.x += m_fAccTime * 10.f;

		m_UIObjDesc.fAlpha -= 1.f / 255.f * _fAccTime;

		if (m_UIObjDesc.fAlpha < 1.f / 255.f)
			m_UIObjDesc.fAlpha = 1.f / 255.f;
	break;

	case CHUD::STARPOINT_SHOW:
	break;

	case CHUD::STARPOINT_LOOT:
		if (m_UIObjDesc.wstrUITag == TEXT("Effect"))
		{
			m_bIsRender = TRUE;

			if (m_iTexIndex >= 8)	m_iTexIndex = 2;
			++m_iTexIndex;
		}

		if (m_UIObjDesc.wstrUITag == TEXT("Effect_Mask")) //스케일, 회전 및 알파 값 변화
		{
			m_bIsRender = TRUE;

			m_UIObjDesc.vDegree.z += _fAccTime * 10.f;
			m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));

			m_UIObjDesc.vSize.x += _fAccTime * 100.f;
			m_UIObjDesc.vSize.y += _fAccTime * 100.f;
			m_pTransformCom->Set_Scaled(m_UIObjDesc.vSize.x, m_UIObjDesc.vSize.y, m_UIObjDesc.vSize.z);

			m_UIObjDesc.fAlpha -= 1.f / 255.f * _fAccTime;

			if (m_UIObjDesc.fAlpha < 1.f / 255.f)
				m_UIObjDesc.fAlpha = 1.f / 255.f;
		}

		if (m_UIObjDesc.wstrUITag == TEXT("Font100") || m_UIObjDesc.wstrUITag == TEXT("Font10")
			|| m_UIObjDesc.wstrUITag == TEXT("Font1") || m_UIObjDesc.wstrUITag == TEXT("Font100_Shadow")
			|| m_UIObjDesc.wstrUITag == TEXT("Font10_Shadow") || m_UIObjDesc.wstrUITag == TEXT("Font1_Shadow"))
		{
			_uint iCount = stoi(m_UIObjDesc.wstrText);

			_uint iFont1 = iCount % 10;
			_uint iFont10 = (iCount / 10) % 10;
			_uint iFont100 = (iCount / 100) % 10;

			if (m_UIObjDesc.wstrUITag == TEXT("Font100") || m_UIObjDesc.wstrUITag == TEXT("Font100_Shadow"))
			{
				if (iFont100 > 0)
				{
					m_bIsRender = TRUE;
					m_UIObjDesc.wstrText = to_wstring(iFont100);
				}
				
				m_bIsRender = FALSE;
			}

			if (m_UIObjDesc.wstrUITag == TEXT("Font10") || m_UIObjDesc.wstrUITag == TEXT("Font10_Shadow"))
			{
				if (iFont10 > 0)
				{
					m_bIsRender = TRUE;
					m_UIObjDesc.wstrText = to_wstring(iFont10);
				}

				m_bIsRender = FALSE;
			}

			if (m_UIObjDesc.wstrUITag == TEXT("Font1") || m_UIObjDesc.wstrUITag == TEXT("Font1_Shadow"))
				m_UIObjDesc.wstrText = to_wstring(iFont1);

			
			if (m_IsMovingUP)
			{
				m_UIObjDesc.vPos.y += 1.f;

				if (m_UIObjDesc.wstrUITag == TEXT("Font100") || m_UIObjDesc.wstrUITag == TEXT("Font10")
					|| m_UIObjDesc.wstrUITag == TEXT("Font1"))
				{
					if (m_UIObjDesc.vPos.y >= 430.f)
					{
						m_UIObjDesc.vPos.y = 430.f;
						m_IsMovingUP = FALSE;
					}
				}

				if (m_UIObjDesc.wstrUITag == TEXT("Font100_Shadow") || m_UIObjDesc.wstrUITag == TEXT("Font10_Shadow")
					|| m_UIObjDesc.wstrUITag == TEXT("Font1_Shadow"))
				{
					if (m_UIObjDesc.vPos.y >= 434.f)
					{
						m_UIObjDesc.vPos.y = 434.f;
						m_IsMovingUP = FALSE;
					}
				}
			}
			else
			{
				m_UIObjDesc.vPos.y -= 1.f;

				if (m_UIObjDesc.wstrUITag == TEXT("Font100") || m_UIObjDesc.wstrUITag == TEXT("Font10")
					|| m_UIObjDesc.wstrUITag == TEXT("Font1"))
				{
					if (m_UIObjDesc.vPos.y <= 417.f)
					{
						m_UIObjDesc.vPos.y = 417.f;
						m_IsMovingUP = TRUE;
					}
				}

				if (m_UIObjDesc.wstrUITag == TEXT("Font100_Shadow") || m_UIObjDesc.wstrUITag == TEXT("Font10_Shadow")
					|| m_UIObjDesc.wstrUITag == TEXT("Font1_Shadow"))
				{
					if (m_UIObjDesc.vPos.y <= 421.f)
					{
						m_UIObjDesc.vPos.y = 421.f; 
						m_IsMovingUP = TRUE;
					}
				}
			}
		}
	break;

	case CHUD::STARPOINT_DROP:
	break;
	}
}

CHUD_StarPoint* CHUD_StarPoint::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHUD_StarPoint* pInstance = new CHUD_StarPoint(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CHUD_StarPoint"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHUD_StarPoint::Clone(void* pArg)
{
	CHUD_StarPoint* pInstance = new CHUD_StarPoint(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CHUD_StarPoint"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHUD_StarPoint::Free()
{
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);

	__super::Free();
}


