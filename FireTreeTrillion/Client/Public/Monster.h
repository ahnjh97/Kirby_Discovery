#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CFSM;
class CCharacterController;
END

BEGIN(Client)

class CMonster abstract : public CGameObject
{
public:
	typedef struct tagMonster_Desc : public CGameObject 
	{

	}MONSTER_DESC;

protected:
	CMonster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonster(const CMonster& rhs);
	virtual ~CMonster() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;
	virtual void	Render_IMGUI() override;

protected:
	CModel*					m_pModelCom = { nullptr };
	CShader*				m_pShaderCom = { nullptr };
	//class CLight*			m_pLight = { nullptr };
	CCharacterController*	m_pControllerCom = { nullptr };

protected:
	// For_PhysX
	_float			m_fOffsetTurn = { 7.f };
	_float4			m_vOriginUp = { 0.f, 1.f, 0.f, 0.f };

protected:
	void			SetOn_Slope(_float fTimeDelta);
	void			Lerp_UpVector(_fvector _vTargetUp, _float _maxAngle, _float fTimeDelta);

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END