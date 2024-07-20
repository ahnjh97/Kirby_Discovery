#include "..\Public\Light.h"

#include "GameInstance.h"

CLight::CLight()
{
}

HRESULT CLight::Initialize(const LIGHT_DESC & LightDesc)
{
	m_LightDesc = LightDesc;

	if (LIGHT_DESC::TYPE_HORONG == m_LightDesc.eType)
	{
		m_vOriginHorongPower = m_LightDesc.vDiffuse;
		m_fOriginRange = m_LightDesc.fRange;
	}


	return S_OK;
}

HRESULT CLight::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer, _bool bForTool)
{
	_uint		iPassIndex = { 0 };
	_float		fTimeDelta = CGameInstance::Get_Instance()->Get_OriginalTimer();

	if (m_bInterpolate == true)
	{
		m_fInterpolateTime -= fTimeDelta;
		_float fInverseRatio = 1.f - (m_fInterpolateTime * m_fRatioTime);

		_float Easing = EASE_OUT(fInverseRatio);
		_float SaturateEasing = SATURATE(Easing);

		_float4 vDiffuse = m_vCurDiffuse + (m_vTargetDiffuse - m_vCurDiffuse) * SaturateEasing;
		_float fRange = m_fCurRange + (m_fTargetRange - m_fCurRange) * SaturateEasing;

		if (LIGHT_DESC::TYPE_HORONG == m_LightDesc.eType)
		{
			m_vOriginHorongPower = vDiffuse;
			m_fOriginRange = fRange;
		}
		else
		{
			m_LightDesc.vDiffuse = vDiffuse;
			m_LightDesc.fRange = fRange;
		}


		if (m_fInterpolateTime <= 0.f)
		{
			m_bInterpolate = false;
			if (LIGHT_DESC::TYPE_HORONG == m_LightDesc.eType)
			{
				m_vOriginHorongPower = m_vTargetDiffuse;
				m_fOriginRange = m_fTargetRange;
			}
			else
			{
				m_LightDesc.vDiffuse = m_vTargetDiffuse;
				m_LightDesc.fRange = m_fTargetRange;
			}

		}
	}

	if (LIGHT_DESC::TYPE_DIRECTIONAL == m_LightDesc.eType)
	{
		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;

		if (FAILED(pShader->Bind_RawValue("g_vLightDir", &m_LightDesc.vDirection, sizeof(_float4))))
			return E_FAIL;

		
		iPassIndex = bForTool == true ? DEFERRED_DIRECTLIGHT_TOOL : DEFERRED_DIRECTLIGHT;
	}
	else if (LIGHT_DESC::TYPE_POINT == m_LightDesc.eType)
	{

		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;

		if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
			return E_FAIL;

		iPassIndex = DEFERRED_POINTLIGHT;

	}
	else if (LIGHT_DESC::TYPE_HORONG == m_LightDesc.eType)
	{
		_float fRandom = CUtils::Make_RandomFloat(0.8f, 1.f);

		_vector vLightAmbient = XMLoadFloat4(&m_vOriginHorongPower);
		vLightAmbient *= fRandom;
		XMStoreFloat4(&m_LightDesc.vDiffuse, vLightAmbient);

		_float fRange = m_fOriginRange;
		fRange *= fRandom;
		m_LightDesc.fRange = fRange;

		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
			return E_FAIL;

		iPassIndex = DEFERRED_POINTLIGHT;
	}

	else if (LIGHT_DESC::TYPE_FLASH == m_LightDesc.eType)
	{
		m_LightDesc.fRange -= (m_LightDesc.fRange / (fTimeDelta * 700.f));
		_vector vLightAmbient = XMLoadFloat4(&m_LightDesc.vAmbient);
		vLightAmbient -= (vLightAmbient / (fTimeDelta * 800.f));
		XMStoreFloat4(&m_LightDesc.vAmbient, vLightAmbient);

		if (m_LightDesc.fRange < 0.1f)
		{
			m_bDead = true;
		}

		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;

		if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
			return E_FAIL;

		iPassIndex = DEFERRED_POINTLIGHT;
	}
	else if (LIGHT_DESC::TYPE_SUPERFLASH == m_LightDesc.eType)
	{
		m_iLifeTime++;

		if (m_iLifeTime > 2)
			m_bDead = true;

		if (m_iLifeTime == 3)
		{
			m_LightDesc.fRange *= 0.4f;
			m_LightDesc.vAmbient.x *= 0.5f;
			m_LightDesc.vAmbient.y *= 0.5f;
			m_LightDesc.vAmbient.z *= 0.5f;
		}

		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
			return E_FAIL;

		iPassIndex = DEFERRED_POINTLIGHT;
	}

	if (FAILED(pShader->Bind_RawValue("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vLightAmbient", &m_LightDesc.vAmbient, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vLightSpecular", &m_LightDesc.vSpecular, sizeof(_float4))))
		return E_FAIL;

	pShader->Begin(iPassIndex);

	pVIBuffer->Render();

	return S_OK;
}

void CLight::Update_LightPos(_fvector vPos)
{
	XMStoreFloat4(&m_LightDesc.vPosition, vPos);
}

void CLight::Interpolate_Light(_float4 vTargetDiffuse, _float fTargetRange, _float fInterpolateTime)
{
	m_vTargetDiffuse = vTargetDiffuse;
	m_vCurDiffuse = m_LightDesc.vDiffuse;
	m_fTargetRange = fTargetRange;
	m_fCurRange = m_LightDesc.fRange;
	m_fInterpolateTime = fInterpolateTime;
	m_fRatioTime = 1.f / m_fInterpolateTime;
	m_bInterpolate = true;
}

_bool CLight::Compute_RenderCull()
{
	if (m_LightDesc.eType != m_LightDesc.TYPE_POINT)
		return true;


	_vector vLightPos = XMLoadFloat4(&m_LightDesc.vPosition);
	_vector vCamPos = CGameInstance::Get_Instance()->Get_CamPosition();

	if (m_LightDesc.fRange > XMVectorGetX(XMVector3Length(vLightPos - vCamPos)))
		return true;

	_matrix ViewProjectMatrix = CGameInstance::Get_Instance()->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * CGameInstance::Get_Instance()->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vLightScreenPos = XMVector3TransformCoord(vLightPos, ViewProjectMatrix);

	_float X = XMVectorGetX(vLightScreenPos);
	_float Y = XMVectorGetY(vLightScreenPos);

	_float fLightRange = m_LightDesc.fRange;
	_float fScreenRange = fLightRange / XMVectorGetW(vLightScreenPos);

	if (X - fScreenRange > 1.f || X + fScreenRange < -1.f
		|| Y - fScreenRange > 1.f || Y + fScreenRange < -1.f)
		return false;

	return true;
}


CLight * CLight::Create(const LIGHT_DESC & LightDesc)
{
	CLight*		pInstance = new CLight();

	if (FAILED(pInstance->Initialize(LightDesc)))
	{
		MSG_BOX(TEXT("Failed To Created : CLight"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLight::Free()
{

}

