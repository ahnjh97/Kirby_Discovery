#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CFinalDump final : public CGameObject
{
public:
	struct DUMPDESC : GAMEOBJECT_DESC
	{
		_float4x4 Matrix;
		_float fScale;
	};

private:
	CFinalDump(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinalDump(const CFinalDump& rhs);
	virtual ~CFinalDump() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual HRESULT Render_LightDepth()							override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()								override;
#endif

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	_float	m_fGravity = { 0.f };
	_bool	m_bBound = { false };
	_bool	m_bOnTerrain = { false };

public:
	static CFinalDump* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END