#pragma once
#include "Client_Defines.h"
#include "GameObject.h"
#include "FSM.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

class CPopStar final : public CGameObject
{
private:
	CPopStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPopStar(const CPopStar& rhs);
	virtual ~CPopStar() = default;


public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;


#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif

private:
	CModel* m_pModelCom = { nullptr };
	CTexture* m_pEyeTextureCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CFSM* m_pFSM = { nullptr };


	HRESULT Add_Components();
	HRESULT Add_PartObjects();
	HRESULT Bind_ShaderResources();

	void	SetUp_FSM();
public:
	static CPopStar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

