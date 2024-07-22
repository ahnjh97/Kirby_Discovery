#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

#define DEBRISCNT 6

BEGIN(Client)

class CMeteor final : public CPhysXObject
{
public:
	struct METEOR_DESC : public CGameObject::GAMEOBJECT_DESC {
		_bool	bBig = { false };
		_float	fDelayTime = { 0.f };
		_vector vPosition = {};
		wstring strTag = {};
	};

private:
	CMeteor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeteor(const CMeteor& rhs);
	virtual ~CMeteor() = default;

public:
	void Set_BigDebris(_bool bBig) { 
		m_bBig = bBig; 
	}

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;
	virtual HRESULT Render_LightDepth()				override;
#ifdef _DEBUG
	virtual	void	Render_IMGUI()			override;
#endif
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

private:
	CModel*					m_pModelCom = { nullptr };
	CShader*				m_pShaderCom = { nullptr };
	vector<class CDebris*>	m_vecDebris = {};

	//
	_float m_fBbongTime = { 0.f };

	_float	m_fTimeDelta = { 0.f };
	_float	m_fRunTime = { 0.f };
	_float	m_fDelayTime = { 0.f };
	_float	m_fDeadTime = { 0.f };
	_float	m_fParticleDelayTime = { 0.f };
	_float	m_fOffsetTime = { 0.f };

	_float	m_fTurnSpeed = { 0.f };
	_float	m_fIncreSpeed = { 0.f };
	_float	m_fDecreSpeed = { 1.f };
	_float	m_fMeteorSpeed = { 0.f };
	_float			m_fWhiteColorDiffuse = { 0.f };

	_uint	m_iDebrisCnt = { 0 };
	_uint	m_iDebrsiMaxCnt = { 0 };

	_bool	m_bBig = { false };
	_bool	m_bShake = { false };

	_vector	m_vPosition = {};
	_vector	m_vAxis = {};
	_vector	m_vTargetPos = {};

	wstring m_strTag = {};

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	void	Compute_MotionBlur();
	_float	EaseInQuart(_float fNumber);
	_float	EaseOutCubic(_float fNumber);

public:
	static CMeteor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END