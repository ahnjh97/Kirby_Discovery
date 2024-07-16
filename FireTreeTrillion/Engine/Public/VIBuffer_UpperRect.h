#pragma once

#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_UpperRect final : public CVIBuffer
{
private:
	CVIBuffer_UpperRect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_UpperRect(const CVIBuffer_UpperRect& rhs);
	virtual ~CVIBuffer_UpperRect() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);


public:
	static CVIBuffer_UpperRect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};

END
