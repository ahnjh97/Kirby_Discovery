#include "stdafx.h"
#include "SingleEffect.h"

CSingleEffect::CSingleEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CEffect{ pDevice, pContext }
{
}

CSingleEffect::CSingleEffect(const CSingleEffect& rhs)
	:CEffect{ rhs }
	, m_FXDesc{ rhs.m_FXDesc }
{
}

HRESULT CSingleEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSingleEffect::Initialize_Prototype(FX_DESC FXDesc)
{
	m_strFXName = FXDesc.strFXName;
	m_FXDesc = FXDesc;

	return S_OK;
}

HRESULT CSingleEffect::Initialize(void* pArg)
{
	FX_DESC FXDesc{};

	if (m_FXDesc.strFXName != "NONE")
	{
		FXDesc = m_FXDesc;

		if (pArg != nullptr)
		{
			FXDesc.vInitPos = (*(FX_DESC*)pArg).vInitPos;
			FXDesc.vInitRot = (*(FX_DESC*)pArg).vInitRot;
			FXDesc.vInitScale = (*(FX_DESC*)pArg).vInitScale;
			FXDesc.bIsColorRender = (*(FX_DESC*)pArg).bIsColorRender;

		}
	}
	else if (pArg != nullptr)
	{
		FXDesc = *(FX_DESC*)pArg;
	}

	HRESULT hr;

	hr = __super::Initialize(&FXDesc);
	CHECK_FAILED(hr);

	hr = Add_Components(FXDesc);
	CHECK_FAILED(hr);

	return S_OK;
}

_int CSingleEffect::Tick(_float _fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;


	if (Calculate_Duration(_fTimeDelta))
	{
		//툴에서는 다시 시작하기
		if (*m_pCurrentLevelID == LEVEL_TOOL_FX)
		{
			m_fDuration.first = 0.f;
		}
		else
			m_bDead = true;
	}

	if (Calculate_Lifetime(_fTimeDelta))
	{

	}


	m_vCurPos = Calculate_CurValue_Lerp(_fTimeDelta, KF_POS);

	Quaternion vCurQuat = Calculate_CurValue_Slerp(_fTimeDelta, KF_ROT);
	_float3 vRadianEuler = vCurQuat.ToEuler();
	m_vCurRot = { ToDegree(vRadianEuler.x), ToDegree(vRadianEuler.y), ToDegree(vRadianEuler.z) };
	m_vCurScale = Calculate_CurValue_Lerp(_fTimeDelta, KF_SCALE);





	m_vCurRColor = Calculate_CurValue_Lerp(_fTimeDelta, KF_RCOLOR);
	m_vCurGColor = Calculate_CurValue_Lerp(_fTimeDelta, KF_GCOLOR);
	m_vCurBColor = Calculate_CurValue_Lerp(_fTimeDelta, KF_BCOLOR);
	m_fCurAlpha = Calculate_CurValue_Lerp(_fTimeDelta, KF_ALPHA).x;
	m_fCurMaskThreshold = Calculate_CurValue_Lerp(_fTimeDelta, KF_MASK).x;

	_float3 vUVOffset = Calculate_CurValue_Lerp(_fTimeDelta, KF_UVOFFSET);
	m_vCurUVOffset = { vUVOffset.x, vUVOffset.y };

	//초기 회전 세팅
	_float3 vInitRadianRot = { ToRadian(m_vInitRot.x), ToRadian(m_vInitRot.y) , ToRadian(m_vInitRot.z) };


	_float4x4 RotMat = _float4x4::CreateFromYawPitchRoll(vInitRadianRot);
	_float3 vDir = _float3::TransformNormal(m_vCurPos, RotMat);


	m_pTransformCom->Set_State(CTransform::STATE_POSITION, Pos(m_vInitPos) + Dir(vDir));

	Quaternion vInitQuat = Quaternion::CreateFromYawPitchRoll(vInitRadianRot);

	Quaternion vResultQuat = vCurQuat * vInitQuat;
	m_pTransformCom->Turn_Absolute(vResultQuat);


	m_pTransformCom->Set_Scaled(m_vInitScale * m_vCurScale);
	//m_pTransformCom->Set_Scaled({ 1.f, 1.f, 1.f });



	return OBJ_NOEVENT;
}

void CSingleEffect::Late_Tick(_float _fTimeDelta)
{
	if (m_bIsColorRender)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

	if (m_bIsBloom)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);

}

HRESULT CSingleEffect::Render()
{
	HRESULT hr;

	if (m_pModelCom == nullptr)
	{
		hr = Bind_ShaderResources(m_iTexIdx, m_iMaskTexIdx);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(m_iPassIdx);
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Bind_Buffers();
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Render();
		CHECK_FAILED(hr);
	}
	else
	{
		hr = Bind_ShaderResources(m_iTexIdx, m_iMaskTexIdx);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(m_iPassIdx);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Render(0);
		CHECK_FAILED(hr);
	}
	return S_OK;
}

HRESULT CSingleEffect::Add_Components(FX_DESC& FXDesc)
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(FXDesc.strTexTag),
		TEXT("Com_DiffuseTexture"), (CComponent**)&m_pTextureCom[TEX_DIFFUSE]);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(FXDesc.strMaskTexTag),
		TEXT("Com_MaskTexture"), (CComponent**)&m_pTextureCom[TEX_MASK]);
	CHECK_FAILED(hr);


	if (FXDesc.strBufferTag == "Prototype_Component_VIBuffer_Rect")
	{
		hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(FXDesc.strBufferTag),
			TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
		CHECK_FAILED(hr);


		hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
		CHECK_FAILED(hr);

		//현재 VtxPosTex Shader Pass 3개
		m_iMaxPassIdx = 2;
	}
	else
	{
		hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(FXDesc.strBufferTag),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom);

		CHECK_FAILED(hr);


		hr = __super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Shader_VtxModel"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
		CHECK_FAILED(hr);

		//현재 VtxModel Shader Pass 4개
		m_iMaxPassIdx = 3;
	}

	return S_OK;
}

HRESULT CSingleEffect::Bind_ShaderResources(_int iTexIdx, _int iMaskTexIdx)
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	HRESULT hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);

	//직교일 경우, 직교 행렬 바인딩
	if (!m_bIsOrthographic)
	{
		hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW));
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ));
		CHECK_FAILED(hr);
	}


	hr = m_pTextureCom[TEX_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", iTexIdx);
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", iMaskTexIdx);
	CHECK_FAILED(hr);


	return S_OK;
}

CSingleEffect* CSingleEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSingleEffect* pInstance = new CSingleEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CSingleEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CSingleEffect* CSingleEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, FX_DESC FXDesc)
{
	CSingleEffect* pInstance = new CSingleEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(FXDesc)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CSingleEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSingleEffect::Clone(void* pArg)
{
	CSingleEffect* pInstance = new CSingleEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : CSingleEffect"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSingleEffect::Free()
{
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pModelCom);

	for (auto& texture : m_pTextureCom)
		Safe_Release(texture);

	Safe_Release(m_pShaderCom);

	__super::Free();
}
