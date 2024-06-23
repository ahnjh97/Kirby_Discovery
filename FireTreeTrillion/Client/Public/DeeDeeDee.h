#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CDeeDeeDee final : public CMonster
{
public:
	enum STATE_TYPE {



		STATE_END
	};



private:
	CDeeDeeDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDeeDeeDee(const CDeeDeeDee& rhs);
	virtual ~CDeeDeeDee() = default;

public:
	virtual HRESULT Initialize_Prototype()	override;
	virtual HRESULT Initialize(void* pArg)	override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render()				override;
	virtual HRESULT Render_LightDepth()		override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif
	virtual void	Add_AnimEvent()			override;
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

public:
	void Change_State(STATE_TYPE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	void Look_Player(_float fTimeDelta);
	void Bone_Turn_Interpolate(_float4& vMoveDir, const _float4& vTargetDir, _float fTimeDelta);
	_bool IsAnimFinished();

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	// FSM
	void SetUp_FSM();
	_float4 m_vBoneLook = {};

public:
	static CDeeDeeDee* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END