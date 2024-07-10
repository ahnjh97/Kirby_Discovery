#include "..\Public\Font_Manager.h"

#include "CustomFont.h"

CFont_Manager::CFont_Manager()
{

}

HRESULT CFont_Manager::Add_Font(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const wstring & strFontTag, const wstring & strFontFilePath)
{
	if (nullptr != Find_Font(strFontTag))
		return E_FAIL;

	CCustomFont*		pFont = CCustomFont::Create(pDevice, pContext, strFontFilePath);
	if (nullptr == pFont)
		return E_FAIL;

	m_Fonts.emplace(strFontTag, pFont);

	return S_OK;
}

HRESULT CFont_Manager::Render(const wstring & strFontTag, const wstring & strText, const _float2 & vPosition, _fvector vColor, _float fRadian)
{
	CCustomFont*		pFont = Find_Font(strFontTag);
	if (nullptr == pFont)
		return E_FAIL;

	return pFont->Render(strText, vPosition, vColor, fRadian);	
}

HRESULT CFont_Manager::Render(const wstring& strFontTag, const wstring& strText, const _float2& vPosition, _fvector vColor, 
	_float fRadian, _fvector vOrigin, _gvector vScale, _float fLineSpacing)
{
	CCustomFont* pFont = Find_Font(strFontTag);
	if (nullptr == pFont)
		return E_FAIL;

	return pFont->Render(strText, vPosition, vColor, fRadian, vOrigin, vScale, fLineSpacing);
}

HRESULT CFont_Manager::Render_Proj(_matrix _matrix, const wstring& strFontTag, const wstring& strText, const _float2& vPosition, _fvector vColor, _float fRadian, _fvector vOrigin, _gvector vScale)
{
	CCustomFont* pFont = Find_Font(strFontTag);
	if (nullptr == pFont)
		return E_FAIL;

	return pFont->Render_Proj(_matrix, strText, vPosition, vColor, fRadian, vOrigin, vScale);
}

XMVECTOR CFont_Manager::Measure_String(const wstring& strFontTag, const wstring& strText)
{
	CCustomFont* pFont = Find_Font(strFontTag);
	if (nullptr == pFont)
		return _float2();

	return pFont->Measure_String(strFontTag, strText);
}

CCustomFont * CFont_Manager::Find_Font(const wstring & strFontTag)
{
	auto	iter = m_Fonts.find(strFontTag);

	if (iter == m_Fonts.end())
		return nullptr;

	return iter->second;
}

CFont_Manager * CFont_Manager::Create()
{
	return new CFont_Manager();
}

void CFont_Manager::Free()
{
	for (auto& Pair : m_Fonts)
		Safe_Release(Pair.second);

	m_Fonts.clear();
}

