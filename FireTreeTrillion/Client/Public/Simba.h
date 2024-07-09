#pragma once
#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CSimba final : public CMonster
{
public:
	enum SIMBA_ANIM {
		SIMBA_END
	};

	enum SIMBA_EYETEX { EYETEX_DIFFUSE, EYETEX_NORMAL, EYETEX_MRA, EYETEX_END };
	enum SIMBA_EYESTATE { SIMBAEYE_LONG, SIMBAEYE_SMALL, SIMBAEYE_BIG, SIMBAEYE_END };

private:
	CSimba(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSimba(const CSimba& rhs);
	virtual ~CSimba() = default;

public:
	void Set_SimbaEye(SIMBA_EYESTATE eEyeState) { m_eEyeState = eEyeState; }

	_float4 Get_Pos() { return m_vPos; }

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;
	virtual HRESULT Render_LightDepth()				override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()					override;
#endif
	virtual void	Add_AnimEvent()					override;
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

public:
	void			Change_State(SIMBA_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool			IsAnimFinished() { return m_pModelCom->IsFinished(); }
	_bool			IsAnimFinished(_uint iCurrentAnimIndex) { return m_pModelCom->IsFinished(iCurrentAnimIndex); }

private:
	CTexture*		m_pEyeTextureCom[EYETEX_END] = { nullptr, nullptr, nullptr };

	SIMBA_ANIM		m_eCurrentState = { SIMBA_END };
	SIMBA_EYESTATE	m_eEyeState = { SIMBAEYE_END };

	vector<_uint>	m_vecMeshes;
	_uint			m_iEyeMesh = {};
	_uint			m_iEyeLidMesh = {};

	_float			m_fAngle = { 0.f };
	_float4			m_vPos = {};

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

	// FSM
	void			SetUp_FSM();

public:
	static CSimba* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
