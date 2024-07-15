#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class COriginCage final : public CGameObject
{
public:
	enum CAGE_STATE { CAGE_STATE_BEFORE, CAGE_STATE_CRACK, CAGE_STATE_AFTER, CAGE_STATE_AFTER_END, CAGE_STATE_END };
	enum CAGE_ANIM { CAGE_BREAK, CAGE_BREAKAFTER, CAGE_BROKENWAIT, CAGE_WAIT, CAGE_END };

private:
	COriginCage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	COriginCage(const COriginCage& rhs);
	virtual ~COriginCage() = default;

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

	void				SetUpMeshIndices();

	void				RenderMesh(_uint iMeshIdex);
	void				RenderMeshes(const vector<_uint>& _vecMeshIndices);

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	class CBone* m_pBone = { nullptr };

	_float m_fWhiteColorDiffuse = {};

	CAGE_STATE m_eState = { CAGE_STATE_END };

	_bool m_bActivacted = { false };
	_float m_fTime = {};

	_uint m_iGlassCrackMesh = {};
	_uint m_iGlassBreakMesh = {};
	vector<_uint> m_vecDefaultMeshes;
	vector<_uint> m_vecBeforeMeshes;
	vector<_uint> m_vecAfterMeshes;

public:
	static COriginCage* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;

};

END