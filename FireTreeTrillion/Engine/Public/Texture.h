#pragma once

#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CTexture final : public CComponent
{
private:
	CTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTexture(const CTexture& rhs);
	virtual ~CTexture() = default;

#pragma region Getter & Setter
public:
	_uint Get_NumTexture() { return m_iNumTextures; }
	ID3D11ShaderResourceView* Get_pSRV_ByIndex(_int iIndex) { return m_Textures[iIndex]; }
#pragma endregion

public:
	virtual HRESULT Initialize_Prototype(const wstring& strTextureFilePath, _uint iNumTexture);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName, _uint iTextureIndex = 0);
	HRESULT Bind_ShaderResources(class CShader* pShader, const _char* pConstantName);

private:
	vector<ID3D11ShaderResourceView*>			m_Textures;
	_uint										m_iNumTextures = { 0 };

public:
	static CTexture* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wstring& strTextureFilePath, _uint iNumTexture = 1);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END