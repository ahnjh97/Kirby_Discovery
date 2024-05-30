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
	void Set_Target(CTransform* pTarget)
	{
		if (nullptr == pTarget)
			return;

		if (nullptr != m_pTarget)
			Safe_Release(m_pTarget);

		m_pTarget = pTarget;
		Safe_AddRef(pTarget);
	}
	void Track_Target(_bool _bTrackTarget) { m_bTrackTarget = _bTrackTarget; }

private:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Render_IMGUI() override;

public:
	void Set_MatrixIndex(_int iMatrixIndex);
	void EmplaceBackCamMatrix(const _float4x4& matWorld);

private:
	_float			m_fMouseSensor = { 0.0f };

	CTransform*		m_pTarget = { nullptr };
	_bool			m_bTrackTarget = { false };
	_float			m_fTrackDistance = { 8.f };
	void			Track_Target(_float fTimeDelta);

	vector<_float4x4>	m_vecCamMatrices;
	_int			m_iMatrixIndex = { -1 };

	//Vector3			m_vOrbitPos = { 0.f, 0.f, 0.f };
private:
	void Orbit_Target(_float fTimeDelta);
	void Control(_float fTimeDelta);

public:
	static CCamera_Free* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END