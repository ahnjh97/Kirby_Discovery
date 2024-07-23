#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CDimensionGate final : public CGameObject
{
public:
	struct DIMENSIONGATE_DESC : public CGameObject::GAMEOBJECT_DESC {
		_bool	bSwitch = { false };
		_bool	bCamera = { false };
		_float	fScale = {};
		_vector	vPosition = {};
	};

private:
	CDimensionGate(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDimensionGate(const CDimensionGate& rhs);
	virtual ~CDimensionGate() = default;

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;
	virtual HRESULT Render_LightDepth()				override;
#ifdef _DEBUG
	virtual	void Render_IMGUI()			override;
#endif

private:
	CModel*		m_pModelCom = { nullptr };
	CShader*	m_pShaderCom = { nullptr };

	_float		m_fTimeDelta = { 0.f };
	_float		m_fLifeTime = { 0.f };

	_float		m_fScale = { 0.f };
	_float			m_fWhiteColorDiffuse = { 0.f };

	_bool		m_bSetAnim = { false };
	_bool		m_bSwitch = { false };
	_bool		m_bCamera = { false };

	_vector		m_vPosition = {};

	enum TEXTURETYPE { TYPE_DIFFUSE, TYPE_MASK, TYPE_MASK2, TYPE_END };
	CTexture*	m_pTextureCom[TYPE_END] = { nullptr, nullptr, nullptr };

	_float		m_fDimensionTime = { 0.f };
	_float		m_fDimensionMin = { 0.f };
	_float		m_fDimensionMax = { 0.f };
	_float2		m_vUVOffset = { 1.f, 0.f };

	_float		m_gtest = { 0.f };


private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	
	void	Compute_DimensionGateMaskRatio(_float fTimeDelta);
	void	Compute_MotionBlur();

public:
	static CDimensionGate* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END