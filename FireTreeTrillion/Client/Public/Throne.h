#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CThrone final : public CGameObject
{
private:
	CThrone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CThrone(const CThrone& rhs);
	virtual ~CThrone() = default;

public:
	void Break(CGameObject* pObj) { m_bPlayAnimation = true; }

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

	_bool m_bPlayAnimation = { false };
	_bool m_bMeshesRemoved = { false };

	_float m_fWhiteColorDiffuse = {};

	unordered_set<_uint> m_setPieces;

public:
	static CThrone* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;

};

END

