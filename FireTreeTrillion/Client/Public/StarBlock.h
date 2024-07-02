#pragma once
#include "Client_Defines.h"
#include "MapObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CStarBlock final : public CPhysXObject
{
private:
	CStarBlock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStarBlock(const CStarBlock& rhs);
	virtual ~CStarBlock() = default;

public:
	virtual HRESULT		Initialize_Prototype()			override;
	virtual HRESULT		Initialize(void* pArg)			override;
	virtual _int		Tick(_float fTimeDelta)			override;
	virtual void		Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT		Render()						override;
	virtual HRESULT		Render_LightDepth()				override;
#ifdef _DEBUG
	virtual void		Render_IMGUI()					override;
#endif
	virtual void		Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

	void				Break_From_Car();

private:
	HRESULT				Add_Components(wstring wstrModelProtoTag);
	HRESULT				Bind_ShaderResources();

	_int				Make_Partical();


	_bool				RayCast_Terrain(const _float3 vMoveDir);
	void				Compute_MotionBlur();
	_float2				m_vPreScreenPos = { 0.f, 0.f };
	_float4				m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };


private:
	CShader*				m_pShaderCom = { nullptr };
	CModel*					m_pModelCom = { nullptr };
	vector<PxRigidActor*>	m_vecStaticActors;

	_float					m_fFlyTime = { 0.f };
	_float					m_fHitPower = { 0.f };

	_bool					m_bStaticOffTrigger = { true };

	_float					m_fSize = { 0.f };
	
public:
	static CStarBlock*	 Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;

};

END