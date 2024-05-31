#pragma once

namespace Engine
{
	typedef struct
	{
		HWND			hWnd;
		bool			isWindowed;
		unsigned int	iWinSizeX, iWinSizeY;
	}ENGINE_DESC;

	typedef struct
	{
		enum TYPE { TYPE_DIRECTIONAL, TYPE_POINT, TYPE_FLASH, TYPE_SUPERFLASH, TYPE_END };
		TYPE		eType;

		XMFLOAT4	vDirection;
		XMFLOAT4	vPosition;
		float		fRange;	

		XMFLOAT4	vDiffuse;
		XMFLOAT4	vAmbient;
		XMFLOAT4	vSpecular;

	}LIGHT_DESC;
	
	typedef struct
	{

		//수명
		_float		fLifetime = { 1.f };
		_float		fLifetimeRandomOffset;

		_float		fStartDelay;
		_float		fStarDelayRandomOffset;

		//시작 범위(랜덤X)
		_float3		vCenter;
		_float3		vRange;

		//회전
		_float3		vRotation;
		_float3		vRotationRandomOffset;

		//크기
		_float3		vScale = {1.f, 1.f, 1.f};
		_float3		vScaleRandomOffset;
		
		_float3		vDir = {1.f, 1.f, 1.f};
		_float3		vDirRandomOffset;

		_float		fSpeed = {1.f};
		_float		fSpeedRandomOffset;


		_float3		vColor = {1.f, 1.f, 1.f};
		_float3		vColorRandomOffset;

		_float		fAlpha = {1.f};
		_float		fAlphaRandomOffset;


		//기준점(랜덤X)
		_float3		vPivot;

		_bool		bIsLoop;
		_bool		bIsBillboard;
		_bool		bIsColorRender;
		_bool		bIsBloom;

		vector<_bool> vecMoveCommands;

	}INSTANCE_DESC;

	typedef struct 
	{
		class CTexture*	MaterialTextures[AI_TEXTURE_TYPE_MAX];
	}MESH_MATERIAL;

	typedef struct
	{
		XMFLOAT3	vScale;
		XMFLOAT4	vRotation;
		XMFLOAT3	vTranslation;
		float		fTime;
	}KEYFRAME;

	typedef struct
	{
		_float fTimeRatio;
		_float3 vValue = {};
		EASING eEasing = { EASE_LINEAR };

	}FX_KEYFRAME;

	typedef struct
	{
		_uint	iNameStrLen;
		string	strName;

		_uint	iBufferStrLen;
		string	strBufferName;

		_uint	iTexStrLen;
		string	strTexName;

		_uint	iMaskTexStrLen;
		string	strMaskTexName;

		_float					fDuration = { 0.f };
		pair<_float, _float>	fLifetime = { 0.f, 1.f };

		_int	iPassIdx = { 0 };
		_int	iTexIdx = { 0 };
		_int	iMaskTexIdx = { 0 };

		_bool	bIsLoop = { false };
		_bool	bIsBillboard = { false };
		_bool	bIsOrthographic = { false };
		_bool	bIsColorRender = { false };
		_bool	bIsBloom = { false };

		_float	fRimLightThreshold = { 0.f };

		_uint iPropertyMapNum;
		vector<pair<KF_PROPERTY, _uint>> vecKeyframeInfo;
		vector<vector<FX_KEYFRAME>> vecKeyframes;

		_uint eRenderGroup = { 0 };

	}SINGLE_FX_DATA;

	typedef struct
	{
		_uint		iNameStrLen;
		string		strName;

		_uint		iBufferStrLen;
		string		strBufferName;

		_uint		iTexStrLen;
		string		strTexName;

		_uint		iMaskTexStrLen;
		string		strMaskTexName;

		_uint		iNumInstance = { 0 };

		_float		fDuration = { 1.f };

		_float		fLifetime = { 1.f };
		_float		fLifetimeRandomOffset;

		_float		fStartDelay;
		_float		fStarDelayRandomOffset;

		//시작 범위(랜덤X)
		_float3		vCenter;
		_float3		vRange;

		//회전
		_float3		vRotation;
		_float3		vRotationRandomOffset;

		//크기
		_float3		vScale = { 1.f, 1.f, 1.f };
		_float3		vScaleRandomOffset;

		_float3		vDir = { 1.f, 1.f, 1.f };
		_float3		vDirRandomOffset;

		_float		fSpeed = { 1.f };
		_float		fSpeedRandomOffset;


		_float3		vColor = { 1.f, 1.f, 1.f };
		_float3		vColorRandomOffset;

		_float		fAlpha = { 1.f };
		_float		fAlphaRandomOffset;


		//기준점(랜덤X)
		_float3		vPivot;

		_bool		bIsLoop;
		_bool		bIsBillboard;
		_bool		bIsColorRender;
		_bool		bIsBloom;

		_uint			iMoveCommandsNum;
		vector<_bool>	vecMoveCommands;

		_uint eRenderGroup = { 0 };


	}PARTICLE_DATA;

	typedef struct
	{
		_uint	iNameStrLen;
		string	strName;

		_uint iFXsNum;
		vector<pair<_uint, string>> FXs;

	}MULTI_FX_DATA;
	
	typedef struct
	{
		_float fExposure = { -1.f };
		_float fHue = { -1.f };
		_float fSaturation = { -1.f };
		_float fBrightness = { -1.f };
		_float fGamma = { -1.f };
		_float fVibrance = { -1.f };
		_float fContrast = { -1.f };

		_float vWhiteBalance[3] = { -1.f,  -1.f,  -1.f };
		_float vColorBalance[3] = { -1.f,  -1.f,  -1.f };

		_float vShadowColor[3] = { -1.f,  -1.f,  -1.f };
		_float fShadowIntensity = { -1.f };
		_float vMidtoneColor[3] = { -1.f,  -1.f,  -1.f };
		_float fMidtoneIntensity = { -1.f };
		_float vHighlightColor[3] = { -1.f,  -1.f,  -1.f };
		_float fHighlightIntensity = { -1.f };
		_float fShadowThreshold = { -1.f };
		_float fHighlightThreshold = { -1.f };

	}COLOR_DATA;

	typedef struct ENGINE_DLL
	{
		XMFLOAT3		vPosition;		

		static const unsigned int	iNumElements = { 1 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[1];
	}VTXPOS;


	typedef struct ENGINE_DLL
	{
		XMFLOAT3		vPosition;
		XMFLOAT2		vTexcoord;

		static const unsigned int	iNumElements = { 2 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[2];
	}VTXPOSTEX;



	typedef struct ENGINE_DLL
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vTexcoord;

		static const unsigned int	iNumElements = { 2 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[2];
	}VTXCUBE;

	typedef struct ENGINE_DLL
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;

		static const unsigned int	iNumElements = { 3 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[3];
	}VTXNORTEX;

	typedef struct ENGINE_DLL
	{
		_float4		vRight;
		_float4		vUp;
		_float4		vLook;
		_float4		vPosition;		
		bool			bAlive;

	}VTXMATRIX;

	typedef struct ENGINE_DLL
	{
		static const unsigned int	iNumElements = { 7 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[7];		
	}VTXINSTANCE_RECT;

	typedef struct ENGINE_DLL
	{
		static const unsigned int	iNumElements = { 6 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[6];
	}VTXINSTANCE_POINT;


	typedef struct ENGINE_DLL
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;
		XMFLOAT3		vTangent;	

		static const unsigned int	iNumElements = { 4 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[4];
	}VTXMESH;

	typedef struct ENGINE_DLL
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;
		XMFLOAT3		vTangent;

		/* 이 정점에게 영향을 주는 뼈들의 인덱스 */
		/* 뼈들의 인덱스 : 이 메시에게 영향을 주는 뼈들의 인덱스를 의미한다. */
		XMUINT4			vBlendIndices;
		XMFLOAT4		vBlendWeights;

		static const unsigned int	iNumElements = { 6 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[6];
	}VTXANIMMESH;

	struct ENGINE_DLL MODEL
	{
		string strModelName = "";
		TYPE eType = { TYPE_END };

		_float fScale = { 1.f };
		_float fDegree = { 0.f };
		_uint iRootNode = { 4 };

		MODEL(string _strModelName = "",
			TYPE _eType = { TYPE_END },
			_float _fScale = { 1.f },
			_float _fDegree = { 0.f },
			_uint _iRootNode = { 4 }
		) : strModelName(_strModelName), eType(_eType), fScale(_fScale), fDegree(_fDegree), iRootNode(_iRootNode) {}
	};
}