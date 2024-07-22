#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CBossOrigin final : public CGameObject
{
	enum BOSSORIGIN_ANIM { BO_ATTACK, BO_DAMAGE, BO_GETOUT, BO_GIANT, BO_LIPSYNCSUB, BO_STAGGERWAIT, BO_WAIT, BO_WAIT_EYEOPEN };
private:
	CBossOrigin(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBossOrigin(const CBossOrigin& rhs);
	virtual ~CBossOrigin() = default;

public:
	void Activate(CGameObject* pObj);

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

	vector<_uint> m_vecMeshes;
	vector<_uint> m_vecEyeMeshes;

	_float m_fWhiteColorDiffuse = {};

	_bool m_bActivated = { false };
	_float m_fTime = {};

public:
	static CBossOrigin* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;

};

END

