#include "stdafx.h"
#include "..\Public\Camera_Free.h"

CCamera_Free::CCamera_Free(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CCamera{ pDevice, pContext }
{

}

CCamera_Free::CCamera_Free(const CCamera_Free & rhs)
	: CCamera{ rhs }
{

}

HRESULT CCamera_Free::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera_Free::Initialize(void * pArg)
{
	if (nullptr == pArg)
		return E_FAIL;
	
	CAMERA_FREE_DESC*	pCameraFree = (CAMERA_FREE_DESC*)pArg;
	m_fMouseSensor = pCameraFree->fMouseSensor;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

_int CCamera_Free::Tick(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_A, KEY_PRESS))
	{
		m_pTransformCom->Go_Left(fTimeDelta);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_D, KEY_PRESS))
	{
		m_pTransformCom->Go_Right(fTimeDelta);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_W, KEY_PRESS))
	{
		m_pTransformCom->Go_Straight(fTimeDelta);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_S, KEY_PRESS))
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}

	_long	MouseMove = { 0 };
	if (m_pGameInstance->Get_DIKeyState(DIK_LSHIFT, KEY_PRESS))
	{
		if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_X))
		{
			m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * MouseMove * m_fMouseSensor);
		}
		if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_Y))
		{
			m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), fTimeDelta * MouseMove * m_fMouseSensor);
		}
	}

	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(0.f, 0.f, 0.f, 1.f));

	//m_fFovy = XMConvertToRadians(120.f);

	__super::Bind_PipeLines();


	return OBJ_NOEVENT;
}

void CCamera_Free::Late_Tick(_float fTimeDelta)
{
}

HRESULT CCamera_Free::Render()
{
	return S_OK;
}

void CCamera_Free::Render_IMGUI()
{
	static _float fSpeed = 10.f;
	_float4 fPosition = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);

	ImGui::SliderFloat("CameraFree Speed", &fSpeed, 0.f, 50.f);
	//m_pTransformCom->Set_Speed(fSpeed);
	/*
	ImGui::SliderFloat("CameraFree Smooth Speed", &m_fSmoothSpeed, 0.f, 0.3f);
	m_pTransformCom->Set_Speed(fSpeed);

	ImGui::Separator();
	ImGui::DragFloat3("CameraFree Position", &fPosition.x);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, fPosition);

	ImGui::Separator();
	ImGui::DragFloat3("CameraFree Offset", &m_vOffset.x);*/
}

CCamera_Free * CCamera_Free::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CCamera_Free*		pInstance = new CCamera_Free(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CCamera_Free"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject * CCamera_Free::Clone(void * pArg)
{
	CCamera_Free*		pInstance = new CCamera_Free(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CCamera_Free"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_Free::Free()
{
	__super::Free();

}
