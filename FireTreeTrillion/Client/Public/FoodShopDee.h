#pragma once
#include "Client_Defines.h"
#include "WaddleDee.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END


BEGIN(Client)

class CFoodShopDee final : public CWaddleDee
{
private:
	CFoodShopDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFoodShopDee(const CFoodShopDee& rhs);
	virtual ~CFoodShopDee() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;


	virtual void	Add_AnimEvent()	override;
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;




#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif
	void			Change_State(DEE_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);

private:
	CTexture* m_pEyeTextureCom = { nullptr };



	HRESULT Add_Components();
	HRESULT Add_PartObjects();
	HRESULT Bind_ShaderResources();

	void	SetUp_FSM();


public:
	static CFoodShopDee* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
