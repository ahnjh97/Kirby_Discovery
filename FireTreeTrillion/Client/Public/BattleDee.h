#pragma once
#include "Client_Defines.h"
#include "WaddleDee.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CBattleDee final : public CWaddleDee
{
private:
	CBattleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBattleDee(const CBattleDee& rhs);
	virtual ~CBattleDee() = default;

public:
	static vector<_float3> m_RespawnPoints;


	//목적지의 위치를 가져온다
	virtual _float3					Make_DestPos() override;
	//다음 상태를 뭘로 할 지 정의한다
	virtual pair<DEE_ANIM, _bool>	Make_WhatToDo() override;


	void			Start_Battle(CGameObject* pNotifier) { m_bStartBattle = true; }

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;


	virtual void	Add_AnimEvent()	override;
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif

private:
	DEE_ANIM		m_eMyState = { DEEANIM_END };

	_float			m_fMoveCheckTimePerSec = { 0.f };
	_float			m_fNonMoveTime = { 0.f };
	_float			m_fMovedDistance = { 0.f };

	_bool			m_bStartBattle = { false };
	_bool			m_bTrackKirby = { false };

	HRESULT Add_Components();
	HRESULT Add_PartObjects();
	HRESULT Bind_ShaderResources();

	void	SetUp_FSM();
	void	Make_InitialState(DEE_CHARACTER iDeeCharacter);
	_bool	Custom_Face(_uint iMeshIndex);

public:
	static CBattleDee* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END