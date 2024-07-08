#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CCharacterController;
END

BEGIN(Client)


class CBaum final : public CPhysXObject
{
public:
	struct BAUMDESC : public GAMEOBJECT_DESC
	{
		// 디제스터마스터가 스피드와 방향을 입력해주고, 무조건 그곳을 향해 간다.
		_float	fBaumSpeed = { 0.f };
		_float4 vBaumMoveDir = { 0.f, 0.f, 0.f, 0.f };
		_float4 vPos = { 0.f, 0.f, 0.f, 0.f };
	};


private:
	CBaum(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBaum(const CBaum& rhs);
	virtual ~CBaum() = default;

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
	HRESULT			Add_Components(wstring wstrModelProtoTag);
	HRESULT			Bind_ShaderResources();
	_int			Make_Partical();
	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

	_float4			m_vBaumMoveDir = { 0.f, 0.f, 0.f, 0.f };
	_float			m_fBaumSpeed = { 0.f };
	_float			m_fScale = { 0.01f };
	_float			m_fBbongTime = { 0.f };


	enum BAUMTYPE { BAUM_BAUM, BAUM_STARPIECE, BAUM_END };
	BAUMTYPE		m_eBaumType = { BAUM_END };
	_bool			m_bOnTerrain = { false };
	// 붙어먹을 로드 클래스
	class CFinaleRoad* m_pMyRoad = { nullptr };
	_float4x4		m_HitWorld = {};
	void			Find_MyRoad();


private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CCharacterController* m_pControllerCom = { nullptr };

	class CLight* m_pLight = { nullptr };

public:
	static CBaum* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END