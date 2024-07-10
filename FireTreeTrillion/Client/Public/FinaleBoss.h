#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CFinaleBoss final : public CMonster
{
public:
	enum FINALEBOSS_ANIM {
		FINALEBOSS_CUT1, FINALEBOSS_CUT10, FINALEBOSS_CUT11, FINALEBOSS_CUT12, FINALEBOSS_CUT13, FINALEBOSS_CUT14, FINALEBOSS_CUT15, FINALEBOSS_CUT16, FINALEBOSS_CUT17, 
		FINALEBOSS_CUT18, FINALEBOSS_CUT19, FINALEBOSS_CUT2, FINALEBOSS_CUT20, FINALEBOSS_CUT21, FINALEBOSS_CUT22, FINALEBOSS_CUT23, FINALEBOSS_CUT24, FINALEBOSS_CUT25, 
		FINALEBOSS_CUT3, FINALEBOSS_CUT4, FINALEBOSS_CUT5, FINALEBOSS_CUT6, FINALEBOSS_CUT7, FINALEBOSS_CUT8, FINALEBOSS_CUT9, FINALEBOSS_DEMOWAITAIR, 
		FINALEBOSS_END
	};

private:
	CFinaleBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinaleBoss(const CFinaleBoss& rhs);
	virtual ~CFinaleBoss() = default;

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
	virtual void	Add_AnimEvent() override;
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

public:
	void Change_State(FINALEBOSS_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();
	_float4 Compute_RootPos();

private:
	FINALEBOSS_ANIM		m_eCurrentState = { FINALEBOSS_END };


	//컷신 위치 확인을 위해 조금 넣어 사용하것습니다
	_float4				m_vBonePos = { 0.f, 0.f, 0.f, 0.f };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	// FSM
	void SetUp_FSM();

public:
	static CFinaleBoss* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END