#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CBrontoBurt final : public CMonster
{
public:
	enum BRONTOBURT_ANIM {
		BRONTOBURT_3DFLY, BRONTOBURT_3DFLYSTART, BRONTOBURT_DAMAGE, BRONTOBURT_FLY, BRONTOBURT_FLYSTART, BRONTOBURT_WAIT, BRONTOBURT_WALK,
		BRONTOBURT_END
	};

	enum BRONTOBURTEYE_STATE { BRONTOBURTEYE_IDLE, BRONTOBURTEYE_HALF, BRONTOBURTEYE_END };

	struct BRONTOBURT_DESC : public CMonster::MONSTER_DESC {
		vector<_float4> vecRallyPoints;
	};

private:
	CBrontoBurt(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBrontoBurt(const CBrontoBurt& rhs);
	virtual ~CBrontoBurt() = default;

public:
	_float4 Get_LastPos() {
		return m_vLastPos;
	}

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

public:
	void Change_State(BRONTOBURT_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();

private:
	CTexture* m_pEyeTextureCom = { nullptr };

	BRONTOBURT_ANIM			m_eCurrentState = { BRONTOBURT_END };
	BRONTOBURTEYE_STATE		m_eEyeState = { BRONTOBURTEYE_END };

	// 충돌 후 복귀
	_float4				m_vLastPos = {};
	_bool				m_bReturn = { false };
	_bool				m_bLerp = { false };

	// 회전 상태의 파리
	_float				m_fDistance = { 0.f };
	_float				m_fAngle = { 0.f };

	_float4				m_vOriginPos = {};
	_float4				m_vRotatePos = {};
	_float4				m_vBeforePos = {};

	// 와리가리 상태의 파리
	_float4				m_vRally = {};
	_uint				m_iCnt = { 0 };
	_bool				m_bConvert = { false };

	vector<_float4>		m_vecRallyPoint;

	_float				m_fMoveTime = { 0.f };
	_float				m_fSpeed = { 0.f };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();
	_bool Custom_Face(_uint iMeshIndex);

public:
	static CBrontoBurt* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END