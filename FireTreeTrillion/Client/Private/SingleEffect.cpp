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

HRESULT CSingleEffect::Initialize_Prototype(FX_DESC& FXDesc)
{
	m_strFXName = FXDesc.strFXName;
	m_FXDesc = FXDesc;

	return S_OK;
}

HRESULT CSingleEffect::Initialize(void* pArg)
{
	FX_DESC FXDesc{};

	//미리 세팅된 값이 있다면
	if (m_FXDesc.strFXName != "NONE")
	{
		FXDesc = m_FXDesc;

		//clone 하면서 새로 들어오는 값!
		if (pArg != nullptr)
		{
			FXDesc.vInitPos = (*(FX_DESC*)pArg).vInitPos;
			FXDesc.vInitRot = (*(FX_DESC*)pArg).vInitRot;
			FXDesc.vInitScale = (*(FX_DESC*)pArg).vInitScale;
			FXDesc.pSocketMatrix = (*(FX_DESC*)pArg).pSocketMatrix;
			FXDesc.fStartDelay = (*(FX_DESC*)pArg).fStartDelay;
		}
	}
	else if (pArg != nullptr)
	{
		FXDesc = *(FX_DESC*)pArg;
	}

	HRESULT hr;

	FXDesc.bIsSingleFX = true;

	hr = __super::Initialize(&FXDesc);
	CHECK_FAILED(hr);


	hr = Add_Components(FXDesc);
	CHECK_FAILED(hr);


	//초기 크자이 세팅
	m_vCurPos = Calculate_CurValue_Lerp(0.f, KF_POS);
	Quaternion vCurQuat = Calculate_CurValue_Slerp(0.f, KF_ROT);
	_float3 vRadianEuler = vCurQuat.ToEuler();
	m_vCurRot = { ToDegree(vRadianEuler.x), ToDegree(vRadianEuler.y), ToDegree(vRadianEuler.z) };
	m_vCurScale = Calculate_CurValue_Lerp(0.f, KF_SCALE);


	//초기 회전 세팅
	_float3 vInitRadianRot = { ToRadian(m_vInitRot.x), ToRadian(m_vInitRot.y) , ToRadian(m_vInitRot.z) };
	_float4x4 RotMat = _float4x4::CreateFromYawPitchRoll(vInitRadianRot);
	_float3 vDir = _float3::TransformNormal(m_vCurPos, RotMat);


	m_pTransformCom->Set_State(CTransform::STATE_POSITION, Pos(m_vInitPos) + Dir(vDir));
	Quaternion vInitQuat = Quaternion::CreateFromYawPitchRoll(vInitRadianRot);
	Quaternion vResultQuat = vCurQuat * vInitQuat;
	m_pTransformCom->Turn_Absolute(vResultQuat);
	m_pTransformCom->Set_Scaled(m_vInitScale * m_vCurScale);

	m_fStartDelay = FXDesc.fStartDelay;
	return S_OK;
}

_int CSingleEffect::Tick(_float _fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CSingleEffect::Late_Tick(_float _fTimeDelta)
{
	//현재 설정 값으로 적용할 타임델타 값을 바꾼다.
	_float fMyTimeDelta = _fTimeDelta;
	switch (m_eTimer)
	{
	case TIMER_FIRST:
		fMyTimeDelta = m_pGameInstance->Get_FirstTimer();
		break;
	case TIMER_SECOND:
		fMyTimeDelta = m_pGameInstance->Get_SecondTimer();
		break;
	default:
		break;
	}

	if (0.f < m_fStartDelay)
	{
		m_fStartDelay -= fMyTimeDelta;

		if (m_fStartDelay <= 0.f)
			m_fStartDelay = 0.f;

		return;
	}


	//true 반환하면 duration 끝난 것.
	if (Calculate_Duration(fMyTimeDelta))
	{
		//툴에서는 다시 시작하기
		if (*m_pCurrentLevelID != LEVEL_TOOL_FX && m_fDuration.second != FX_MAXDURATION)
		{
			m_bDead = true;
		}
	}


	//true 반환하면 lifetime 끝난 것.
	if (Calculate_Lifetime(fMyTimeDelta))
	{
		if (*m_pCurrentLevelID != LEVEL_TOOL_FX)
			m_bNoRender = true;
	}

	Compute_ViewZ();

	m_vCurPos = Calculate_CurValue_Lerp(fMyTimeDelta, KF_POS);

	Quaternion vCurQuat = Calculate_CurValue_Slerp(fMyTimeDelta, KF_ROT);
	_float3 vRadianEuler = vCurQuat.ToEuler();
	m_vCurRot = { ToDegree(vRadianEuler.x), ToDegree(vRadianEuler.y), ToDegree(vRadianEuler.z) };


	m_vCurScale = Calculate_CurValue_Lerp(fMyTimeDelta, KF_SCALE);

	m_vCurRColor = Calculate_CurValue_Lerp(fMyTimeDelta, KF_RCOLOR);
	m_vCurGColor = Calculate_CurValue_Lerp(fMyTimeDelta, KF_GCOLOR);
	m_vCurBColor = Calculate_CurValue_Lerp(fMyTimeDelta, KF_BCOLOR);

	m_fCurAlpha = Calculate_CurValue_Lerp(fMyTimeDelta, KF_ALPHA).x;
	m_fCurMaskThreshold = Calculate_CurValue_Lerp(fMyTimeDelta, KF_MASK).x;

	_float3 vUVOffset = Calculate_CurValue_Lerp(fMyTimeDelta, KF_UVOFFSET);
	m_vCurUVOffset = { vUVOffset.x, vUVOffset.y };


	//뒤 세 변수는 이후 추가했으므로, vector 크기 기존과 달라 넘어가 버릴 수 있다.
	if (KF_MASKUVOFFSET < m_Keyframes.size())
	{
		_float3 vMaskUVOffset = Calculate_CurValue_Lerp(fMyTimeDelta, KF_MASKUVOFFSET);
		m_vCurMaskUVOffset = { vMaskUVOffset.x, vMaskUVOffset.y };
	}

	if (KF_MASKUVANGLE < m_Keyframes.size())
		m_vCurMaskUVAngle = Calculate_CurValue_Lerp(fMyTimeDelta, KF_MASKUVANGLE).x;


	//초기 회전 세팅
	_float3 vInitRadianRot = { ToRadian(m_vInitRot.x), ToRadian(m_vInitRot.y) , ToRadian(m_vInitRot.z) };
	_float4x4 RotMat = _float4x4::CreateFromYawPitchRoll(vInitRadianRot);
	_float3 vDir = _float3::TransformNormal(m_vCurPos, RotMat);


	//위치 갱신
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, Pos(m_vInitPos) + Dir(vDir * m_vInitScale));

	//회전 갱신
	Quaternion vInitQuat = Quaternion::CreateFromYawPitchRoll(vInitRadianRot);

	_float3 vCurContinueRot = { ToRadian(m_vContinuousRotation.x), ToRadian(m_vContinuousRotation.y) , ToRadian(m_vContinuousRotation.z) };
	vCurContinueRot *= m_fDuration.first;
	Quaternion vContinuousQuat = Quaternion::CreateFromYawPitchRoll(vCurContinueRot);
	Quaternion vResultQuat = vContinuousQuat * vCurQuat * vInitQuat;

	m_pTransformCom->Turn_Absolute(vResultQuat);

	//크기 갱신
	m_pTransformCom->Set_Scaled(m_vInitScale * m_vCurScale);

	//소켓 매트릭스
	if (m_pSoketMatrix != nullptr)
	{
		_float4x4 socketMatrix = *m_pSoketMatrix;
		socketMatrix.Right().Normalize();
		socketMatrix.Up().Normalize();
		socketMatrix.Backward().Normalize();

		_float3 vScale = CUtils::Get_Scaled_Matrix(socketMatrix);
		if (vScale.x < .001f)
			CUtils::Set_Scaled_Matrix(socketMatrix, .001f, vScale.y, vScale.z);
		if (vScale.y < .001f)
			CUtils::Set_Scaled_Matrix(socketMatrix, vScale.x, .001f, vScale.z);
		if (vScale.z < .001f)
			CUtils::Set_Scaled_Matrix(socketMatrix, vScale.x, vScale.y, .001f);
		m_pTransformCom->Set_WorldMatrix(m_pTransformCom->Get_WorldMatrix() * socketMatrix);
	}


	if (m_bIsBillboard)
		Billboard_Effect();

	if ((CRenderer::RENDERGROUP)m_eRenderGroup != CRenderer::RENDER_END)
		m_pGameInstance->Add_RenderGroup((CRenderer::RENDERGROUP)m_eRenderGroup, this);

	if (m_bIsBloom)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);

}

HRESULT CSingleEffect::Render()
{
	if (m_bNoRender || 0.f < m_fStartDelay)
		return S_OK;

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

	m_iMaxTexIdx = m_pTextureCom[TEX_DIFFUSE]->Get_TextureNum() - 1;

	hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(FXDesc.strMaskTexTag),
		TEXT("Com_MaskTexture"), (CComponent**)&m_pTextureCom[TEX_MASK]);
	CHECK_FAILED(hr);

	m_iMaxMaskTexIdx = m_pTextureCom[TEX_MASK]->Get_TextureNum() - 1;

	if (FXDesc.strBufferTag == "Prototype_Component_VIBuffer_Rect"
		|| FXDesc.strBufferTag == "Prototype_Component_VIBuffer_UpperRect")
	{
		hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(FXDesc.strBufferTag),
			TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
		CHECK_FAILED(hr);


		hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
		CHECK_FAILED(hr);

		//현재 VtxPosTex Shader Pass 6까지
		m_iMaxPassIdx = POSTEX_END - 1;
		m_bBindShaderVars = false;
	}
	else
	{
		hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(FXDesc.strBufferTag),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom);
		CHECK_FAILED(hr);


		hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
		CHECK_FAILED(hr);

		//현재 VtxModel Shader Pass 10까지
		m_iMaxPassIdx = MODEL_END - 1;
		m_bBindShaderVars = true;
	}

	return S_OK;
}

HRESULT CSingleEffect::Bind_ShaderResources(_int iTexIdx, _int iMaskTexIdx)
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	HRESULT hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);

	if (true == m_bBindShaderVars) {
		_bool bStencil = true;
		_bool bRimLight = false;
		_bool bMotionBlur = false;
		m_pShaderCom->Bind_RawValue("g_bStencil", &bStencil, sizeof(_bool));
		m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool));
		m_pShaderCom->Bind_RawValue("g_bMotionBlur", &bMotionBlur, sizeof(_bool));
	}
	
	//직교일 경우, 직교 행렬 바인딩
	if (!m_bIsOrthographic)
	{
		hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW));
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ));
		CHECK_FAILED(hr);
	}
	else
	{
		_float4x4 ViewMatrix = _float4x4::Identity;
		_float4x4 ProjMatrix = XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1000.f);

		if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjMatrix)))
			return E_FAIL;
	}

	hr = m_pTextureCom[TEX_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", iTexIdx);
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", iMaskTexIdx);
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fCurAlpha, sizeof(_float));
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_RawValue("g_fMaskThreshold", &m_fCurMaskThreshold, sizeof(_float));
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_RawValue("g_vRColor", &m_vCurRColor, sizeof(_float3));
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_RawValue("g_vUVOffset", &m_vCurUVOffset, sizeof(_float2));
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_RawValue("g_vMaskUVOffset", &m_vCurMaskUVOffset, sizeof(_float2));
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_RawValue("g_fMaskUVAngle", &m_vCurMaskUVAngle, sizeof(_float));
	CHECK_FAILED(hr);

	hr = m_pGameInstance->Bind_RTShaderResource(m_pShaderCom, TEXT("Target_Depth"), "g_DepthTexture");
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
