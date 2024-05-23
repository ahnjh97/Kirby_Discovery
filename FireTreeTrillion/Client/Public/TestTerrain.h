#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)

class CTestTerrain final : public CGameObject
{
private:
	CTestTerrain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestTerrain(const CTestTerrain& rhs);
	virtual ~CTestTerrain() = default;

public:
	virtual HRESULT Initialize_Prototype()		 override;
	virtual HRESULT Initialize(void* pArg)		 override;
	virtual _int	Tick(_float fTimeDelta)		 override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render()					 override;
	virtual void	Render_IMGUI()				 override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CTestTerrain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END