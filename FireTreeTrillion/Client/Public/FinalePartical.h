#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CFinalePartical final :  public CGameObject
{
private:
	CFinalePartical(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinalePartical(const CFinalePartical& rhs);
	virtual ~CFinalePartical() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual HRESULT Render_LightDepth()							override;


	void Set_Partical(_float4 vPos, _float4 vDir, _float fSpeed);

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

private:
	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };
	enum TEXTURETYPE { 
		TEXTURE_ROAD_DIFFUSE, TEXTURE_ROAD_NORMAL, TEXTURE_ROAD_MRA,
		TEXTURE_ROADSIDE_DIFFUSE, TEXTURE_ROADSIDE_NORMAL, TEXTURE_ROADSIDE_MRA,
		TEXTURE_BREAKROAD_DIFFUSE, TEXTURE_BREAKROAD_NORMAL, TEXTURE_BREAKROAD, TEXTURE_END
	};
	CTexture*		m_pTextureCom = { nullptr };

	_float4			m_vDir = { 0.f, 0.f, 0.f, 0.f };
	_float			m_fSpeed = { 0.f };
	_bool			m_bActive = { false };

	_float			m_fGravity = { 0.f };
	_int			m_iRandomTexture = { 0 };
	

	_float			m_fTimeDelta = { 0.f };

public:
	static CFinalePartical* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;


};

END