#include "stdafx.h"
#include "..\Public\Camera_Free.h"

CCamera_Free::CCamera_Free(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{

}

CCamera_Free::CCamera_Free(const CCamera_Free& rhs)
	: CCamera{ rhs }
{

}

HRESULT CCamera_Free::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera_Free::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	CAMERA_FREE_DESC* pCameraFree = (CAMERA_FREE_DESC*)pArg;
	m_fMouseSensor = pCameraFree->fMouseSensor;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pGameInstance->Add_Camera(this);

	return S_OK;
}

_int CCamera_Free::Tick(_float fTimeDelta)
{

	Control(fTimeDelta);
	Orbit_Target(fTimeDelta);

	/*
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
	*/

	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(0.f, 0.f, 0.f, 1.f));

	//m_fFovy = XMConvertToRadians(120.f);

	//__super::Bind_PipeLines();


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

	ImGui::Checkbox(u8"타겟 따라가기", &m_bTrackTarget);


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

void CCamera_Free::Track_Target(_float fTimeDelta)
{
	if (nullptr == m_pTarget)
		return;

	_float4 vTargetPos = m_pTarget->Get_State(CTransform::STATE_POSITION);
	//vTargetPos.y = m_pTransformCom->Get_State(CTransform::STATE_POSITION).y;

	_float4 vBackDir = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	vBackDir.Normalize();
	vBackDir *= m_fTrackDistance;

	_float4 vCurPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

	_float4 vDestDir = (vTargetPos + vBackDir) - vCurPos;
	vDestDir.y = 0.f;

	if (.1f <= vDestDir.Length())
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, Pos(vCurPos + (vDestDir * .06f)));
}

void CCamera_Free::Orbit_Target(_float fTimeDelta)
{
}

void CCamera_Free::Control(_float fTimeDelta)
{

	if (m_pGameInstance->Get_KeyState(DIK_TAB, KEY_DOWN))
		m_bTrackTarget = !m_bTrackTarget;

	if (m_bTrackTarget)
		Track_Target(fTimeDelta);


	if (*m_pCurrentLevelID == LEVEL_TOOL_MAP || m_pGameInstance->Get_KeyState(DIK_LSHIFT, KEY_PRESS) )
	{
		_long	MouseMove = { 0 };

		//휠 누른 채로 상하좌우 이동
		if (m_pGameInstance->Get_KeyState(DIMKS_WHEEL, KEY_PRESS))
		{
			if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_X))
				m_pTransformCom->Go_Right(fTimeDelta * -MouseMove * m_fMouseSensor);

			if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_Y))
				m_pTransformCom->Go_Up(fTimeDelta * MouseMove * m_fMouseSensor);
		}

		//전후진
		if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_WHEEL))
		{
			// 05.24) LEVEL_TOOL_UI에는 카메라 줌인/아웃 시 객체도 적용
			if (*m_pCurrentLevelID == LEVEL_TOOL_UI)
			{
				_float4x4 WorldMatrix, ViewMatrix, ProjMatrix;
				ViewMatrix = m_pTransformCom->Get_WorldMatrix_Inverse();
				m_pGameInstance->Set_Transform(CPipeLine::D3DTS_VIEW, ViewMatrix);

				//XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f);

				_float2 ViewVoulume;
				ViewVoulume.x -= g_iWinSizeX * 0.001f;
				ViewVoulume.y -= g_iWinSizeY * 0.001f;

				ProjMatrix = XMMatrixOrthographicLH(ViewVoulume.x, ViewVoulume.y, 0.0f, 300.f);
				m_pGameInstance->Set_Transform(CPipeLine::D3DTS_PROJ, ProjMatrix);

				const CTransform* pUIEditorTrans = dynamic_cast<const CTransform*>(m_pGameInstance->
					Get_Component(LEVEL_TOOL_UI, TEXT("Layer_UI"), g_strTransformTag));
				WorldMatrix = pUIEditorTrans->Get_WorldMatrix();
				
				//m_pTransformCom->Set_WorldMatrix(WorldMatrix);
			}

			m_pTransformCom->Go_Straight(fTimeDelta * MouseMove * m_fMouseSensor * .5f);

			if (m_pTarget != nullptr && m_bTrackTarget)
				m_fTrackDistance -= MouseMove*.01f ;
		}

		//일단 안씀
		//if (m_pGameInstance->Get_KeyState(DIMKS_RBUTTON, KEY_DOWN))
		//{
		//	m_vOrbitPos = 
		//}

		//우측 마우스 누른 채로 공전

		if (m_pGameInstance->Get_KeyState(DIMKS_RBUTTON, KEY_PRESS))
		{
			Vector3 vTargetPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION) + m_pTransformCom->Get_State_Float4(CTransform::STATE_LOOK) * 10.f;
			
			// 05.22) LEVEL_TOOL_UI에는 카메라 회전 기능 제외
			if (*m_pCurrentLevelID == LEVEL_TOOL_UI)
				return;

			if (*m_pCurrentLevelID == LEVEL_TOOL_FX)
				vTargetPos = Vector3::Zero;

			if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_X))
			{
				m_pTransformCom->Orbit(vTargetPos, XMVectorSet(0.f, 1.f, 0.f, 1.f), fTimeDelta * MouseMove * m_fMouseSensor);
			}
			if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_Y))
			{
				m_pTransformCom->Orbit(vTargetPos, m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), fTimeDelta * MouseMove * m_fMouseSensor);
			}
		}
	}


	//if (m_pGameInstance->Get_DIKeyState(DIK_A, KEY_PRESS))
	//{
	//	m_pTransformCom->Go_Left(fTimeDelta);

	//}

}

CCamera_Free* CCamera_Free::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Free* pInstance = new CCamera_Free(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CCamera_Free"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CCamera_Free::Clone(void* pArg)
{
	CCamera_Free* pInstance = new CCamera_Free(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CCamera_Free"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_Free::Free()
{
	Safe_Release(m_pTarget);

	__super::Free();
}
