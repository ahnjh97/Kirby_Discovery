#pragma once

#include "Client_Defines.h"
#include "Camera.h"

BEGIN(Client)

class CCamera_Free final : public CCamera
{
public:
	typedef struct : public CCamera::CAMERA_DESC
	{
		_float		fMouseSensor = { 0.0f };
	}CAMERA_FREE_DESC;

private:
	CCamera_Free(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera_Free(const CCamera_Free& rhs);
	virtual ~CCamera_Free() = default;

public:
	//트래킹할 타겟을 세팅한다.
	//void Set_Target(CTransform* pTarget)
	//{
	//	if (nullptr == pTarget)
	//		return;

	//	if (nullptr != m_pTarget)
	//		Safe_Release(m_pTarget);

	//	m_pTarget = pTarget;
	//	Safe_AddRef(pTarget);
	//}
	void Track_Target(_bool _bTrackTarget) { m_bTrackTarget = _bTrackTarget; }

private:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
#ifdef _DEBUG
	virtual void Render_IMGUI() override;
#endif

public:
	void Set_MatrixIndex(_int iMatrixIndex);
	void EmplaceBackCamMatrix(const _float4x4& matWorld) { m_vecCamMatrices.emplace_back(matWorld); }

	void EmplaceBackDirRadius(_int iCamType, _fvector vDir, _float fRadius);
	void LerpByTriggerInfo(_int iTriggerIndex);

	void EmplaceBackTriggerInfo(const _float4x4& matWorld, _float fScale) { 
		m_vecTriggerInfo.emplace_back(matWorld, fScale); }
	_float Compute_TriggerPosRatio(_int iTriggerIndex);

	void StartLerpByTriggerInfo(_int iTriggerIndex) { m_bLerpByTriggerInfo = true; m_iMatrixIndex = iTriggerIndex; }
	void EndLerpByTriggerInfo() { m_bLerpByTriggerInfo = false; };

	_vector SlerpDirVec(_fvector vStart, _fvector vEnd, _float fRatio);
	_float LerpRadius(_float fStart, _float fEnd, _float fRatio);

private:
	_bool			m_bWasMainCamera = { false };
	_float			m_fMouseSensor = { 0.0f };

	CTransform*		m_pTarget = { nullptr };
	_bool			m_bTrackTarget = { false };
	_float			m_fTrackDistance = { 40.f };

	_float			m_fSlerpRatio = { .2f };

	_float4			m_vDestCamPos = {};
	_float3			m_vDestCamDir = {};


	_float			m_fDestFovy = { ToRadian(30.f)};

	void			Track_Target(_float fTimeDelta);

	vector<_float4x4>	m_vecCamMatrices;
	_int				m_iMatrixIndex = { -1 };

	vector<pair<_vector, _float>>	m_vecFrontDirRadius;
	vector<pair<_vector, _float>>	m_vecRearDirRadius;
	vector<pair<_float4x4, _float>>	m_vecTriggerInfo; // Trigger InverseMatrix and Scale
	

	_float m_fTriggerRatio = {};
	_bool m_bLerpByTriggerInfo = { false };
	_bool m_bPreLerpByTriggerInfo = { false };

	_vector m_vSlerpedDir = {};
	_float m_fLerpedRadius = {};

private:
	void Control(_float fTimeDelta);

public:
	static CCamera_Free* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END