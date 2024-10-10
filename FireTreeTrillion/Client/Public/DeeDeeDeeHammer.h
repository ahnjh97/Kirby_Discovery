#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CDeeDeeDeeHammer :
    public CPartObject
{
public:
	typedef struct : public CPartObject::PARTOBJECT_DESC
	{
		_float4x4* pBoneMatrix = { nullptr };
		_float* pWhite = { nullptr };
	}DEEDEEDEEHAMMER_DESC;

private:
	CDeeDeeDeeHammer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDeeDeeDeeHammer(const CDeeDeeDeeHammer& rhs);
	virtual ~CDeeDeeDeeHammer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;

private:
	void			Ready_FadeIn();
	void			Ready_FadeOut();

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

private:
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };
	_float			m_fShowDialog = { 0.f };


	_float4x4* m_pBoneMatrix = { nullptr };
	_float* m_pWhiteColorDiffuse = { nullptr };

	_bool	m_bItemTrigger = { true };
	_bool	m_bShowDialog = { FALSE };
	_bool	m_bFadeOutEnd = false;
	_bool	m_bDeeOnce = { true };
private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	void	Compute_MotionBlur();

public:
	static CDeeDeeDeeHammer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
