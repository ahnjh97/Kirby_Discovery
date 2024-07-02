#pragma once
#include "Client_Defines.h"
#include "RigidObject.h"

BEGIN(Engine)
class CModel;
END

BEGIN(Client)

class CStarBlockPiece final : public CRigidObject
{
public:
	struct PIECE_DESC : public GAMEOBJECT_DESC
	{
		_float3		vMoveDir;
		_float		fPower;
		_float		fSize;
	};

private:
	CStarBlockPiece(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStarBlockPiece(const CStarBlockPiece& rhs);
	virtual ~CStarBlockPiece() = default;

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
	HRESULT			Add_Components(_float fSize);
	HRESULT			Bind_ShaderResources();

private:
	CModel* m_pModelCom = { nullptr };
	_float			m_fLifeTime = _float();
	_float			m_fLifeMaxTime = { 0.f };

	_float			m_fScale = { 0.f };

	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

public:
	static CStarBlockPiece*	 Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;

};

END