#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
END

BEGIN(Client)

class CNonRenderWall : public CGameObject
{
private:
	CNonRenderWall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNonRenderWall(const CNonRenderWall& rhs);
	virtual ~CNonRenderWall() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override {};
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };

private:
	HRESULT Add_Components();

public:
	static CNonRenderWall* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END