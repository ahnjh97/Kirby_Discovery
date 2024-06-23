#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Client)
class CSkySphere final : public CGameObject
{
public:
	enum TEX_TYPE { TEX_DIFFUSE, TEX_NORMAL, TEX_EMISSIVE, TEX_HEIGHT, TEX_MRA, TEX_NONE };
	enum SKY_TYPE { 
		SKY_LAB_2PASE, SKY_LAB_1PASE, SKY_LAB_FIELD, //LEVEL_FINALBOSS (LAB_DISCOVERA_2PASE)
		SKY_NONE};
	
private:
	CSkySphere(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSkySphere(const CSkySphere& rhs);
	virtual ~CSkySphere() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CShader*				m_pShaderCom = { nullptr };
	CModel*					m_pModelCom = { nullptr };
	CTexture*				m_pLabSkyTex[TEX_NONE] = { nullptr };
	//class CCamera*		m_pHifiCam = { nullptr };
	HRESULT					Add_Components();
	HRESULT					Bind_ShaderResources();

public:
	static CSkySphere*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

private:

};

END