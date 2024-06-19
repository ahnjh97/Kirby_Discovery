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
	struct PIECE_DESC
	{
		_float4		vInitialPos;
		_float3		vDir;
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
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

private:
	CModel*			m_pModelCom = nullptr;
	_float			m_fLifeTime = _float();
	_float			m_fLifeTimeMax = _float();
	_float			m_fTurnSpeed = _float();

public:
	static CStarBlockPiece*	 Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;

};

END