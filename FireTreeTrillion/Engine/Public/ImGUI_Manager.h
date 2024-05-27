#pragma once
#include "Base.h"

BEGIN(Client)
class CUI_Editor;
END

BEGIN(Engine)
class CImGUI_Manager final : public CBase
{
private:
	CImGUI_Manager() = default;
	virtual ~CImGUI_Manager() = default;

public:
	HRESULT Initialize(HWND hWnd, ID3D11Device* pGraphic_Device, ID3D11DeviceContext* pContext);
	void	Tick(_float fDeltaTime);
	void	Late_Tick(_float fDeltaTime);
	HRESULT Render();
	HRESULT	RenderUpdate();
	

public:
	void SetDockSpace();
	void RenderGrid();
	void EditTransform(_float4x4& matrix);

	// FileDialog
	void Set_FileDialog();

private:
	ID3D11Device*		 m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CUI_Editor*	 m_pUIEditor = { nullptr };

public:
	static CImGUI_Manager* Create(HWND hWnd, ID3D11Device* pGraphic_Device, ID3D11DeviceContext* pContext);
	virtual void Free() override;

};

END

