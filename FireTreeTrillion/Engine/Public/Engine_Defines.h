#pragma once
#pragma warning (disable : 4251)
#pragma warning (disable : 5208)
#pragma warning (disable : 4267)
#pragma warning (disable : 4099)
#pragma warning (disable : 26495)
//#pragma warning (disable : 4819)
//#pragma warning (disable : 4101)

#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <DirectXCollision.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace Engine
{
	enum KEYSTATE { KEY_FREE = 0, KEY_DOWN, KEY_PRESS, KEY_UP, KEY_END };
	enum MOUSEKEYSTATE { DIMKS_LBUTTON, DIMKS_RBUTTON, DIMKS_WHEEL, DIMKS_X, DIMKS_END };
	enum MOUSEMOVESTATE { DIMMS_X, DIMMS_Y, DIMMS_WHEEL, DIMMS_END };
	enum TYPE { TYPE_NONANIM, TYPE_ANIM, TYPE_END };
	enum EASING { EASE_LINEAR, EASE_IN, EASE_IN_FAST, EASE_OUT, EASE_OUT_FAST, EASE_INOUT, EASE_INOUT_FAST, EASE_END };
    enum KF_PROPERTY {KF_POS, KF_ROT, KF_SCALE, KF_RCOLOR, KF_GCOLOR, KF_BCOLOR, KF_ALPHA, KF_MASK, KF_UVOFFSET, KF_MASKUVOFFSET, KF_MASKUVANGLE, KF_END};
    enum RIGID_SHAPE { RIGID_BOX, RIGID_SPHERE, RIGID_CAPSULE, RIGID_END };
    enum INSTANCE_PROPERTY {INSTANCE_DROP, INSTANCE_SPREAD, INSTANCE_DECELERATE, INSTANCE_END};

    enum COLLISION_TYPE
    { 
        PLAYER = 0, PLAYER_EFFECT,
        MONSTER,
        INTERACT,
        ITEM,
        TRIGGER,
        COLLISION_END
    };

    enum COLLISION_CONTENT
    {
        CONTENT_ATTACK,		// 공격 - 피격 처리
        CONTENT_INTERACT,	// 상호작용하는 객체끼리의 충돌
        CONTENT_ACQUIRE,	// 만나면 ObjDest가 삭제되는 충돌액션
        CONTENT_NONEVENT,	// 공통적인 충돌처리가 없는 경우
        CONTENT_END
    };
}

// Set_Dead() 매크로
#define OBJ_NOEVENT 0
#define OBJ_DEAD 1

#ifdef _DEBUG
// IMGUI
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "ImGuiFileDialog.h"
#include "ImGuiFileDialogConfig.h"
#endif


// SHADER - EFFECT
#include "Effects11/d3dx11effect.h"

// TEXTURE
#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"

// COLLIDER : BOUNDING friends
#include "DirectXTK/ScreenGrab.h"
#include "DirectXTK/PrimitiveBatch.h"
#include "DirectXTK/VertexTypes.h"
#include "DirectXTK/Effects.h"

// FONT
#include "DirectXTK/SpriteBatch.h"
#include "DirectXTK/SpriteFont.h"

// SimpleMath
#include <DirectXTK/SimpleMath.h>

// RapidJSON
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

// PhysX
#include "PxPhysics.h"
#include "PxPhysicsAPI.h"
#include "cooking/PxCooking.h"

// Assimp 
#define AI_TEXTURE_TYPE_MAX 21
using namespace DirectX;
using namespace SimpleMath;
using namespace physx;

#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <array>
#include <set>
#include <queue>
#include <unordered_map>
#include <cassert>
#include <random>
#include <chrono>
#include <functional>
#include <fstream>
#include <cmath>
#include <tuple>
#include <numeric>
#include <limits>
#include <atlconv.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
//#include <tchar.h>
//#include <memory.h>
#include <utility>

//// for Fmod
#include "fmod.h"
#include "fmod.hpp"
#include "fmod_errors.h"
#include <io.h>
#pragma comment (lib, "fmod_vc.lib")

using namespace std;
using namespace rapidjson;
using namespace filesystem;

namespace Engine
{
	const wstring g_strTransformTag = TEXT("Com_Transform");
	const wstring g_strColliderTag = TEXT("Com_Collider");

	enum CHANNELID {
		CHANNEL_PLAYERVOICE = 100,
		CHANNEL_BOSSVOICE,
		CHANNEL_SOUND1, CHANNEL_SOUND2, CHANNEL_SOUND3, CHANNEL_SOUND4, CHANNEL_SOUND5, 
		CHANNEL_SOUND6, CHANNEL_SOUND7, CHANNEL_SOUND8, CHANNEL_SOUND9, CHANNEL_SOUND10,
		CHANNEL_SOUND11, CHANNEL_SOUND12, CHANNEL_SOUND13, CHANNEL_SOUND14, CHANNEL_SOUND15,
		CHANNEL_BGM, CHANNEL_END
	};
}

enum TEXTURETYPE
{
    /** Dummy value.
     *
     *  No texture, but the value to be used as 'texture semantic'
     *  (#aiMaterialProperty::mSemantic) for all material properties
     *  *not* related to textures.
     */
    TextureType_NONE = 0,

    /** LEGACY API MATERIALS
     * Legacy refers to materials which
     * Were originally implemented in the specifications around 2000.
     * These must never be removed, as most engines support them.
     */

     /** The texture is combined with the result of the diffuse
      *  lighting equation.
      */
    TextureType_DIFFUSE = 1,

    /** The texture is combined with the result of the specular
     *  lighting equation.
     */
    TextureType_SPECULAR = 2,

    /** The texture is combined with the result of the ambient
     *  lighting equation.
     */
    TextureType_AMBIENT = 3,

    /** The texture is added to the result of the lighting
     *  calculation. It isn't influenced by incoming light.
     */
    TextureType_EMISSIVE = 4,

    /** The texture is a height map.
     *
     *  By convention, higher gray-scale values stand for
     *  higher elevations from the base height.
     */
    TextureType_HEIGHT = 5,

    /** The texture is a (tangent space) normal-map.
     *
     *  Again, there are several conventions for tangent-space
     *  normal maps. Assimp does (intentionally) not
     *  distinguish here.
     */
    TextureType_NORMALS = 6,

    /** The texture defines the glossiness of the material.
     *
     *  The glossiness is in fact the exponent of the specular
     *  (phong) lighting equation. Usually there is a conversion
     *  function defined to map the linear color values in the
     *  texture to a suitable exponent. Have fun.
    */
    TextureType_SHININESS = 7,

    /** The texture defines per-pixel opacity.
     *
     *  Usually 'white' means opaque and 'black' means
     *  'transparency'. Or quite the opposite. Have fun.
    */
    TextureType_OPACITY = 8,

    /** Displacement texture
     *
     *  The exact purpose and format is application-dependent.
     *  Higher color values stand for higher vertex displacements.
    */
    TextureType_DISPLACEMENT = 9,

    /** Lightmap texture (aka Ambient Occlusion)
     *
     *  Both 'Lightmaps' and dedicated 'ambient occlusion maps' are
     *  covered by this material property. The texture contains a
     *  scaling value for the final color value of a pixel. Its
     *  intensity is not affected by incoming light.
    */
    TextureType_LIGHTMAP = 10,

    /** Reflection texture
     *
     * Contains the color of a perfect mirror reflection.
     * Rarely used, almost never for real-time applications.
    */
    TextureType_REFLECTION = 11,

    /** PBR Materials
     * PBR definitions from maya and other modelling packages now use this standard.
     * This was originally introduced around 2012.
     * Support for this is in game engines like Godot, Unreal or Unity3D.
     * Modelling packages which use this are very common now.
     */

    TextureType_BASE_COLOR = 12,
    TextureType_NORMAL_CAMERA = 13,
    TextureType_EMISSION_COLOR = 14,
    TextureType_METALNESS = 15,
    TextureType_DIFFUSE_ROUGHNESS = 16,
    TextureType_AMBIENT_OCCLUSION = 17,

    /** Unknown texture
     *
     *  A texture reference that does not match any of the definitions
     *  above is considered to be 'unknown'. It is still imported,
     *  but is excluded from any further post-processing.
    */
    TextureType_UNKNOWN = 18,
    TextureType_SHEEN = 19,
    TextureType_CLEARCOAT = 20,
    TextureType_TRANSMISSION = 21,

#ifndef SWIG
    TextureType_Force32Bit = INT_MAX
#endif
};

enum PASS_DEFERRED {
    DEFERRED_DEBUG, DEFERRED_DIRECTLIGHT, DEFERRED_POINTLIGHT, DEFERRED_FINAL, 
    DEFERRED_BLUR_X, DEFERRED_BLUR_Y, DEFERRED_BLUR_R, DEFERRED_COLORCORRECT, DEFERRED_DOF_X,
    DEFERRED_MOTIONBLUR,
    DEFERRED_UI, DEFERRED_DOF_Y,
    DEFERRED_DIRECTLIGHT_TOOL, DEFERRED_FINAL_TOOL,
    DEFERRED_GODRAY, 
    DEFERRED_END
};

#define TEXTURE_TYPE_MAX  TextureType_UNKNOWN


#include "Engine_Macro.h"
#include "Engine_Function.h"
#include "Engine_Typedef.h"
#include "Engine_Struct.h"


using namespace Engine;

//#ifdef _DEBUG
//
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//
//#ifndef DBG_NEW 
//
//#define new DBG_NEW 
//#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
//
//#endif
//
//#endif // _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//#ifdef _DEBUG
//#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
//#endif // _DEBUG

#ifdef _DEBUG
#define new new(_CLIENT_BLOCK, __FILE__, __LINE__)
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#endif