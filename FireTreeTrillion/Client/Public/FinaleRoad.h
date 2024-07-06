#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"
#include "FinaleRoadGrouper.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CFinaleRoad final : public CPhysXObject
{
public:
	//enum MOVECMD { MOVECMD_STOP, MOVECMD_ROTATE, MOVECMD_END};
	enum COLLIDETYPE
	{
		CTYPE_NONE,
		CTYPE_DOWN,
		CTYPE_BREAK,
		CTYPE_END
	};

	struct ROAD_DESC : public GAMEOBJECT_DESC
	{
		COLLIDETYPE			eCollideType = { CTYPE_END };
		_bool				bIsAnimModel = { false };
		const _float4x4*	pSocketMat = { nullptr };
	};

private:
	CFinaleRoad(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinaleRoad(const CFinaleRoad& rhs);
	virtual ~CFinaleRoad() = default;

public:
	_float4x4		Get_WorldMatrix() { return m_WorldMatrix; }
	_float4			Get_WorldPos() { return CUtils::Get_State_Vector_Matrix(m_WorldMatrix, CUtils::STATE_POSITION); }


	void			Make_CollisionEvent(/*CFinaleRoadGrouper::MOVECMD eMove*/);

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
	HRESULT			Add_Components(wstring _strModelTag, _bool _bIsAnimModel);
	HRESULT			Bind_ShaderResources();
	void			Make_Particles();


	COLLIDETYPE		m_eCollideType = { CTYPE_END };
	_bool			m_bIsAnimModel = { false };

	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

	_float			m_fWhiteColorDiffuse = {};
	wstring			m_wstrModelName = { L"NONE" };

	_float4x4			m_WorldMatrix;
	const _float4x4*	m_pSocketMatrix;

	PxRigidDynamic*		m_pDynamicActor = { nullptr };
	CModel*				m_pModelCom = { nullptr };
	CShader*			m_pShaderCom = { nullptr };

public:
	static CFinaleRoad* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END
