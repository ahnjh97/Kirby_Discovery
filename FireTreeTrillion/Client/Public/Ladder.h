#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END


BEGIN(Client)

class CLadder final : public CGameObject
{
private:
	CLadder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLadder(const CLadder& rhs);
	virtual ~CLadder() = default;

public:
	virtual HRESULT Initialize_Prototype()	override;
	virtual HRESULT Initialize(void* pArg)	override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render()				override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif
	_bool			Is_Collide(_fvector vPos);
	_float4			Get_LadderPoint() { return m_vLadderPoint; }
	_float4			Get_LadderOriginalPos() { return m_vLadderOriginalPos; }

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	PxRigidStatic* m_pStaticActor = { nullptr };

	_float	m_fHeight = { 8.5f };
	_float	m_fRadius = { 1.3f };
	_float4 m_vLadderPoint = { 0.f, 0.f, 0.f, 0.f };
	_float4 m_vLadderOriginalPos = { 0.f, 0.f, 0.f, 0.f };




public:
	static CLadder* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END