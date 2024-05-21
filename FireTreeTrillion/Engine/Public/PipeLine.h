#pragma once

#include "Base.h"

/* 렌더링파이프라인에 필요한 ViewMatrix, ProjMatrix보존하낟. */
/* 기타 클라이언트 로직에 필요한 데이터들을 만들고 저장ㅎ나다. (View, Proj Inv, CamPosition */

BEGIN(Engine)

class CPipeLine final : public CBase
{
public:
	enum TRANSFORMSTATE { D3DTS_VIEW, D3DTS_PROJ, D3DTS_END };

private:
	CPipeLine();
	virtual ~CPipeLine() = default;

public:
	void		Set_Transform(TRANSFORMSTATE _eState, _fmatrix _TransformMatrix) { m_TransformMatrices[_eState] = _TransformMatrix; }

	_float4x4	Get_Transform(TRANSFORMSTATE _eState) const { return m_TransformMatrices[_eState]; }
	_float4x4	Get_Transform_Inv(TRANSFORMSTATE _eState) const { return m_TransformInverseMatrices[_eState]; }

	_float4		Get_CamPosition /*_Float4*/() const { return m_vCamPosition; }


public:
	_matrix Get_Transform_Matrix(TRANSFORMSTATE eState) const {
		return XMLoadFloat4x4(&m_TransformMatrices[eState]);
	}
	_float4x4 Get_Transform_Float4x4(TRANSFORMSTATE eState) const {
		return m_TransformMatrices[eState];
	}
	_matrix Get_Transform_Matrix_Inverse(TRANSFORMSTATE eState) const {
		return XMLoadFloat4x4(&m_TransformInverseMatrices[eState]);
	}
	_float4x4 Get_Transform_Float4x4_Inverse(TRANSFORMSTATE eState) const {
		return m_TransformInverseMatrices[eState];
	}

	//_vector Get_CamPosition_Vector() const {
	//	return XMLoadFloat4(&m_vCamPosition);
	//}


public:
	HRESULT Add_Camera(class CCamera* pCamera);
	HRESULT Switch_CurCamera(_int iIdx);


	HRESULT Initialize();
	void Bind_Pipeline();
	void Tick();

private:
	_int m_iCurCameraIdx = { 0 };
	vector<class CCamera*> m_pCameras;


	_float4x4			m_TransformMatrices[D3DTS_END];
	_float4x4			m_TransformInverseMatrices[D3DTS_END];
	_float4				m_vCamPosition;


public:
	static CPipeLine* Create();
	virtual void Free() override;	
};

END