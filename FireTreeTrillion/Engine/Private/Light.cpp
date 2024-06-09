#include "..\Public\Light.h"

#include "GameInstance.h"

CLight::CLight()
{
}

HRESULT CLight::Initialize(const LIGHT_DESC & LightDesc)
{
	m_LightDesc = LightDesc;

	return S_OK;
}

HRESULT CLight::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer, _bool bForTool)
{
	_uint		iPassIndex = { 0 };

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
	else if (LIGHT_DESC::TYPE_FLASH == m_LightDesc.eType)
	{
		m_LightDesc.fRange -= (m_LightDesc.fRange / 20.f);
		_vector vLightAmbient = XMLoadFloat4(&m_LightDesc.vAmbient);
		vLightAmbient -= (vLightAmbient / 30.f);
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
	//갓 레이 용 패스로 틀어준다.
	else if (LIGHT_DESC::TYPE_GODRAY == m_LightDesc.eType)
	{
		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
			return E_FAIL;

		
		iPassIndex = DEFERRED_GODRAY;
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

