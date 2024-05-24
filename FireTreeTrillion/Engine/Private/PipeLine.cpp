#include "PipeLine.h"
#include "GameInstance.h"
//#include "Shader.h"
#include "Camera.h"


CPipeLine::CPipeLine()
{
}

HRESULT CPipeLine::Add_Camera(CCamera* pCamera)
{
	m_pCameras.emplace_back(pCamera);
	Safe_AddRef(pCamera);
	return S_OK;
}

void CPipeLine::Clear_Camera()
{
	if (!m_pCameras.empty())
	{
		for (auto& pCam : m_pCameras)
			Safe_Release(pCam);
	}

	m_pCameras.clear();
	m_iCurCameraIdx = 0;
}

HRESULT CPipeLine::Switch_CurCamera(_int iIdx)
{
	if (m_pCameras.size() <= iIdx)
		return E_FAIL;

	m_iCurCameraIdx = iIdx;

	return S_OK;
}

void CPipeLine::Clear_Camera()
{
	if (!m_pCameras.empty())
	{
		for (auto& pCam : m_pCameras)
			Safe_Release(pCam);
	}
	m_pCameras.clear();
}

void CPipeLine::Bind_Pipeline()
{
	if (m_pCameras.empty())
		return;

	m_pCameras[m_iCurCameraIdx]->Bind_PipeLines();


	for (size_t i = 0; i < D3DTS_END; i++)
	{
		//XMStoreFloat4x4(&m_TransformMatrices[i], XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_TransformMatrices[i])));
		m_TransformInverseMatrices[i] = m_TransformMatrices[i].Invert();
	}

	//memcpy(&m_vCamPos, &m_PipelineInvMatrices[D3DPS_VIEW].m[3][0], sizeof(_float4));
	m_vCamPosition = Pos(m_TransformInverseMatrices->Translation());
}

HRESULT CPipeLine::Initialize()
{
	for (size_t i = 0; i < D3DTS_END; i++)
	{
		m_TransformMatrices[i] = Matrix::Identity;
		m_TransformInverseMatrices[i] = _float4x4::Identity;
		//XMStoreFloat4x4(&m_TransformMatrices[i], XMMatrixIdentity());
		//XMStoreFloat4x4(&m_TransformInverseMatrices[i], XMMatrixIdentity());
	}

	m_vCamPosition = _float4::Zero;

	return S_OK;
}

void CPipeLine::Tick()
{
	Bind_Pipeline();
}

CPipeLine* CPipeLine::Create()
{
	CPipeLine* pInstance = new CPipeLine();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CPipeLine"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPipeLine::Free()
{
	if (!m_pCameras.empty())
	{
		for (auto& pCam : m_pCameras)
			Safe_Release(pCam);
	}
	m_pCameras.clear();
}
