#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CAwoofy final : public CMonster
{
private:
	CAwoofy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAwoofy(const CAwoofy& rhs);
	virtual ~CAwoofy() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;
	virtual void	Render_IMGUI() override;

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CAwoofy* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END