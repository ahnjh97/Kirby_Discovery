#pragma once

#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CTextureArray final : public CComponent
{
private:
	CTextureArray(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTextureArray(const CTextureArray& rhs);
	virtual ~CTextureArray() = default;

#pragma region Getter & Setter

public:
	_uint Get_NumTexture() { return m_iNumTextures; }

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
	static CTextureArray* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wstring& strTextureFilePath, _uint iNumTexture = 1);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END
