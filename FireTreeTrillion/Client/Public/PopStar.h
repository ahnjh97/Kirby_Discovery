#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
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

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	enum CUT { 
		IDLE, IDLE2, CUT1, CUT10, CUT11, CUT12, CUT13, CUT14, CUT15, CUT17, CUT18, CUT19, 
		CUT2, CUT20,
		CUT3, CUT4, CUT5, CUT6, CUT7, CUT8, CUT9, WAIT,
		CUT_END
	};
	CUT				m_eCurCut = { CUT_END };
	CUT				m_ePreCut = { CUT_END };
	void			Set_Animation();


	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	_int	m_RenderCount = { 0 };


public:
	static CPopStar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

