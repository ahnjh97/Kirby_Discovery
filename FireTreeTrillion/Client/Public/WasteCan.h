#pragma once
#include "Client_Defines.h"
#include "RigidObject.h"

BEGIN(Engine)
class CModel;
END

BEGIN(Client)

class CWasteCan final : public CRigidObject
{
private:
	CWasteCan(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWasteCan(const CWasteCan& rhs);
	virtual ~CWasteCan() = default;

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;
	virtual HRESULT Render_LightDepth()				override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()					override;
#endif

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

private:
	CModel*			m_pModelCom = nullptr;

public:
	static CWasteCan*	 Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;

};

END