#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)

class CMap_Stage1 final : public CGameObject
{
private:
	CMap_Stage1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMap_Stage1(const CMap_Stage1& rhs);
	virtual ~CMap_Stage1() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CGameObject* m_pBlendMap = { nullptr }; // AlphaBlend로 그려야 하는 맵 메쉬들을 모아놓은 PartObject

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	HRESULT Add_PartObjects();

public:
	static CMap_Stage1* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END
