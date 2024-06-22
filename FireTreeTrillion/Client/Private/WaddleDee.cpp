#include "stdafx.h"
#include "WaddleDee.h"

CWaddleDee::CWaddleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CCharacter{ pDevice, pContext }
{
}

CWaddleDee::CWaddleDee(const CWaddleDee& rhs)
	:CCharacter{ rhs }
{
}

HRESULT CWaddleDee::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWaddleDee::Initialize(void* pArg)
{
	/*DEE_DESC pDeeDesc{};

	if (nullptr != pArg)
		pDeeDesc = *(DEE_DESC*)pArg;

	pDeeDesc.fSpeedPerSec = 5.f;
	pDeeDesc.fRotationPerSec = XMConvertToRadians(90.0f);*/

	HRESULT hr;

	hr = __super::Initialize(pArg);
	CHECK_FAILED_MSG(hr, "와들디 생성 망했어");


	/*hr = Add_Components();
	CHECK_FAILED_MSG(hr, "와들디 생성 망했어");

	m_pModelCom->Set_Animation(0, 50.f, true, true);*/

	return S_OK;
}

_int CWaddleDee::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CWaddleDee::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

HRESULT CWaddleDee::Render()
{
	__super::Render();
	return S_OK;
}

HRESULT CWaddleDee::Render_LightDepth()
{
	return S_OK;
}

#ifdef _DEBUG
void CWaddleDee::Render_IMGUI()
{
	__super::Render_IMGUI();
}
#endif
/*
HRESULT CWaddleDee::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		ALARM_FAIL("쉐이더가 읍서");

	HRESULT hr;

	hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED_MSG(hr, "바인딩 망함");
	hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW));
	CHECK_FAILED_MSG(hr, "바인딩 망함");
	hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ));
	CHECK_FAILED_MSG(hr, "바인딩 망함");


	hr = m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
	CHECK_FAILED_MSG(hr, "바인딩 망함");
	hr = m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool));
	CHECK_FAILED_MSG(hr, "바인딩 망함");
	hr = m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float));
	CHECK_FAILED_MSG(hr, "바인딩 망함");
	hr = m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));
	CHECK_FAILED_MSG(hr, "바인딩 망함");
	hr = m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4));
	CHECK_FAILED_MSG(hr, "바인딩 망함");
	hr = m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float));
	CHECK_FAILED_MSG(hr, "바인딩 망함");


	return S_OK;
}
*/

_bool CWaddleDee::Custom_Face(_uint iMeshIndex)
{
	return _bool();
}

void CWaddleDee::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	//Safe_Release(m_pEyeTextureCom);
	//Safe_Release(m_pTestTextureCom);
}
