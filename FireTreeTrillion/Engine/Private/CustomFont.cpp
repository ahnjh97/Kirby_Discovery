#include "..\Public\CustomFont.h"

CCustomFont::CCustomFont(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CCustomFont::Initialize(const wstring & strFontFilePath)
{
	m_pFont = new SpriteFont(m_pDevice, strFontFilePath.c_str());
	m_pBatch = new SpriteBatch(m_pContext);

	return S_OK;
}

HRESULT CCustomFont::Render(const wstring& strText, const _float2& vPosition, _fvector vColor, _float fRadian)
{
	m_pBatch->Begin();

	m_pFont->DrawString(m_pBatch, strText.c_str(), vPosition, vColor, fRadian);

	m_pBatch->End();

	return S_OK;
}

//스케일 옵션, 줄 간격 옵션 설정
HRESULT CCustomFont::Render(const wstring& strText, const _float2& vPosition, _fvector vColor, _float fRadian, 
	_fvector vOrigin, _gvector vScale, _float fLineSpacing)
{
	m_pBatch->Begin();
	
	//_float fCurLineSpacing = m_pFont->GetLineSpacing(); //현재 줄 간격
	//m_pFont->SetLineSpacing(fCurLineSpacing + fLineSpacing);

	m_pFont->DrawString(m_pBatch, strText.c_str(), vPosition, vColor, fRadian, vOrigin, vScale);

	m_pBatch->End();

	return S_OK;
}

//원근투영 폰트 렌더
HRESULT CCustomFont::Render_Proj(_matrix _matrix, const wstring& strText, const _float2& vPosition, _fvector vColor, _float fRadian, _fvector vOrigin, _gvector vScale)
{
	//뷰, 투영행렬 변환컴포넌트 정보로 원근세팅
	m_pBatch->Begin(SpriteSortMode_Deferred, nullptr, nullptr, nullptr, nullptr, nullptr, _matrix);

	m_pFont->DrawString(m_pBatch, strText.c_str(), vPosition, vColor, fRadian, vOrigin, vScale);

	m_pBatch->End();
	 
	return S_OK;
}

//텍스트 길이 측정
XMVECTOR CCustomFont::Measure_String(const wstring& strFontTag, const wstring& strText)
{
	m_pBatch->Begin();

	XMVECTOR XMMeasureString = m_pFont->MeasureString(strText.c_str());

	return XMMeasureString;
}

CCustomFont * CCustomFont::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const wstring & strFontFilePath)
{
	CCustomFont*		pInstance = new CCustomFont(pDevice, pContext);

	if (FAILED(pInstance->Initialize(strFontFilePath)))
	{
		MSG_BOX(TEXT("Failed to Created : CCustomFont"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCustomFont::Free()
{
	Safe_Delete_Array(m_pBatch);
	Safe_Delete_Array(m_pFont);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
