#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Rect;
class CShader;
class CTexture;
END

BEGIN(Client)

// 미리 15개를 생성하여, 폭탄일때만 활성화한다.
// 모든 연산은 커비가 관리하고, Orbit은 경로 위치에서 
class CBombOrbit final :
	public CGameObject
{
private:
	CBombOrbit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBombOrbit(const CBombOrbit& rhs);
	virtual ~CBombOrbit() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	// 레이를 쏘다가, 도달한 첫 터레인에 포지션과 노말을 쏴줘서 위치시킨다.
	void	Update_OrbitPosition(_fvector vPos, _fvector vNormal);

private:
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CBombOrbit* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
