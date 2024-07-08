#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CFSM;
END

BEGIN(Client)
 
class CSurprisedBoard final : public CPhysXObject
{
public:
	enum ANIM_STATE	  { ARM_MOVE_L, ARM_MOVE_R, 
						BREAK_L, BREAK_R, 
						POP_OUT_L, POP_OUT_R,
						PREPOP_OUT_L, PREPOP_OUT_R,
						RETURN_L, RETURN_R,
						WAIT_L, WAIT_R, ANIM_STATE_END };
	enum STATE { WAIT, POPOUT, STOP, STATE_END };

private:
	CSurprisedBoard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSurprisedBoard(const CSurprisedBoard& rhs);
	virtual ~CSurprisedBoard() = default;

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
	void			Change_State(ANIM_STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool			IsAnimFinished();
	_bool			IsAnimFinished(_uint iCurrentAnimIndex);

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

	// FOR FSM
	void			SetUp_FSM();

private:
	CModel*			m_pModelCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };

	// FOR FSM
	CFSM*			m_pFSM = { nullptr };

public:
	static CSurprisedBoard* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END