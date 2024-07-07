#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CTurbine final : public CGameObject
{
private:
	CTurbine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTurbine(const CTurbine& rhs);
	virtual ~CTurbine() = default;

public:
	virtual HRESULT		Initialize_Prototype()			override;
	virtual HRESULT		Initialize(void* pArg)			override;
	virtual _int		Tick(_float fTimeDelta)			override;
	virtual void		Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT		Render()						override;

private:
	HRESULT				Add_Components(wstring& wstrModelName);
	HRESULT				Bind_ShaderResources();

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

	_float m_fWhiteColorDiffuse = {};

public:
	static CTurbine* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;

};

END



