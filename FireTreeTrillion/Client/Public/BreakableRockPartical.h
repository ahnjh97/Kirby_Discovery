#pragma once
#include "Client_Defines.h"
#include "RigidObject.h"

BEGIN(Engine)
class CModel;
END

class CBreakableRockPartical final : public CRigidObject
{
public:
	struct BREAKABLEPARTICALDESC
	{
		_float4x4 matrix;
		_float3 vMoveDir;
		_float	fPower;
		wstring wstrModelName;
	};

private:
	CBreakableRockPartical(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBreakableRockPartical(const CBreakableRockPartical& rhs);
	virtual ~CBreakableRockPartical() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual HRESULT Render_LightDepth()							override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()								override;
#endif

private:
	HRESULT			Add_Components(const wstring& _wstrModelName);
	HRESULT			Bind_ShaderResources();


private:
	CModel*			m_pModelCom = { nullptr };
	_float			m_fLifeTime = _float();
	_float			m_fLifeMaxTime = { 0.f };


	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };


public:
	static CBreakableRockPartical* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;


};

