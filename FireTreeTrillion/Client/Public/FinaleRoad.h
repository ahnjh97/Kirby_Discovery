#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CFinaleRoad final : public CPhysXObject
{
public:
	struct ROAD_DESC : public GAMEOBJECT_DESC
	{
		wstring strModelTag = { L"NONE" };
		_bool	bIsAnimModel = { false };
	};

private:
	CFinaleRoad(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinaleRoad(const CFinaleRoad& rhs);
	virtual ~CFinaleRoad() = default;

public:
	void OnCollision();

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
	HRESULT			Add_Components(wstring _strModelTag, _bool _bIsAnimModel);
	HRESULT			Bind_ShaderResources();
	void			Make_Particles();

	_bool			m_bIsAnimModel = { false };


	_float			m_fWhiteColorDiffuse = {};
	CModel*			m_pModelCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };

public:
	static CFinaleRoad* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END
