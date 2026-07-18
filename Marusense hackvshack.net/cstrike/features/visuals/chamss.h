
#pragma once
#include "../../sdk/datatypes/stronghandle.h"



static constexpr char szVMatBufferLatexVisible[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
	shader = "csgo_complex.vfx"
	F_PAINT_VERTEX_COLORS = 1
	F_TRANSLUCENT = 1
	F_SPECULAR = 1
	g_vColorTint = [ 1.000000, 1.000000, 1.000000, 1.000000 ]
	g_flMetalness = 1.000000
	g_flSpecularScale = [ 5.000000, 5.000000, 5.000000, 5.000000 ]
	g_flReflectivity = [ 1.000000, 1.000000, 1.000000, 1.000000 ]
	g_flRoughnessScaleFactor = 0.050000
	g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tTintMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
})";

static constexpr char szVMatBufferLatexInvisible[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
	shader = "csgo_complex.vfx"
	F_PAINT_VERTEX_COLORS = 1
	F_TRANSLUCENT = 1
	F_SPECULAR = 1
	F_DISABLE_Z_BUFFERING = 1
	g_vColorTint = [ 1.000000, 1.000000, 1.000000, 1.000000 ]
	g_flMetalness = 1.000000
	g_flSpecularScale = [ 5.000000, 5.000000, 5.000000, 5.000000 ]
	g_flReflectivity = [ 1.000000, 1.000000, 1.000000, 1.000000 ]
	g_flRoughnessScaleFactor = 0.050000
	g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tTintMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
})";
static constexpr char szVMatBufferGlowVisible[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
	shader = "csgo_complex.vfx"
	F_SELF_ILLUM = 1
	F_PAINT_VERTEX_COLORS = 1
	F_TRANSLUCENT = 1
	F_BLEND_MODE = 6
	F_ADDITIVE_BLEND = 1
	g_vColorTint = [ 1.000000, 1.000000, 1.000000, 0.080000 ]
	g_flSelfIllumScale = [ 12.000000, 12.000000, 12.000000, 12.000000 ]
	g_flSelfIllumBrightness = [ 12.000000, 12.000000, 12.000000, 12.000000 ]
	g_vSelfIllumTint = [ 30.000000, 30.000000, 30.000000, 1.000000 ]
	g_flOpacityScale = 0.050000
	g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tSelfIllumMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
})";

static constexpr char szVMatBufferGlowInisible[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
	shader = "csgo_complex.vfx"
	F_SELF_ILLUM = 1
	F_PAINT_VERTEX_COLORS = 1
	F_TRANSLUCENT = 1
	F_BLEND_MODE = 6
	F_ADDITIVE_BLEND = 1
	F_DISABLE_Z_BUFFERING = 1
	g_vColorTint = [ 1.000000, 1.000000, 1.000000, 0.040000 ]
	g_flSelfIllumScale = [ 12.000000, 12.000000, 12.000000, 12.000000 ]
	g_flSelfIllumBrightness = [ 12.000000, 12.000000, 12.000000, 12.000000 ]
	g_vSelfIllumTint = [ 30.000000, 30.000000, 30.000000, 1.000000 ]
	g_flOpacityScale = 0.025000
	g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tSelfIllumMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
})";


static constexpr char szVMatBufferWhiteVisible[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
	shader = "csgo_unlitgeneric.vfx"

	F_PAINT_VERTEX_COLORS = 1
	F_TRANSLUCENT = 1
	F_BLEND_MODE = 1

	g_vColorTint = [1, 1, 1, 1]

	TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tTintMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
})";

static constexpr char szVMatBufferWhiteInvisible[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
	shader = "csgo_unlitgeneric.vfx"

	F_PAINT_VERTEX_COLORS = 1
	F_TRANSLUCENT = 1
	F_BLEND_MODE = 1
	F_DISABLE_Z_BUFFERING = 1

	g_vColorTint = [1, 1, 1, 1]

	TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tTintMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
})";

static constexpr char szVMatBufferIlluminateVisible[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
	shader = "csgo_complex.vfx"

	F_SELF_ILLUM = 1
	F_PAINT_VERTEX_COLORS = 1
	F_TRANSLUCENT = 1

	g_vColorTint = [ 1.000000, 1.000000, 1.000000, 1.000000 ]
	g_flSelfIllumScale = [ 3.000000, 3.000000, 3.000000, 3.000000 ]
	g_flSelfIllumBrightness = [ 3.000000, 3.000000, 3.000000, 3.000000 ]
    g_vSelfIllumTint = [ 10.000000, 10.000000, 10.000000, 10.000000 ]

	g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tSelfIllumMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	TextureAmbientOcclusion = resource:"materials/debug/particleerror.vtex"
	g_tAmbientOcclusion = resource:"materials/debug/particleerror.vtex"
})";

static constexpr char szVMatBufferIlluminateInvisible[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
	shader = "csgo_complex.vfx"

	F_SELF_ILLUM = 1
	F_PAINT_VERTEX_COLORS = 1
	F_TRANSLUCENT = 1
    F_DISABLE_Z_BUFFERING = 1

	g_vColorTint = [ 1.000000, 1.000000, 1.000000, 1.000000 ]
	g_flSelfIllumScale = [ 3.000000, 3.000000, 3.000000, 3.000000 ]
	g_flSelfIllumBrightness = [ 3.000000, 3.000000, 3.000000, 3.000000 ]
    g_vSelfIllumTint = [ 10.000000, 10.000000, 10.000000, 10.000000 ]

	g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tSelfIllumMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	TextureAmbientOcclusion = resource:"materials/debug/particleerror.vtex"
	g_tAmbientOcclusion = resource:"materials/debug/particleerror.vtex"
})";


class CMaterial2;
class CMeshData;

namespace F::VISUALS::CHAMS
{
    bool Initialize();
    void Destroy();

    bool OnDrawObject(
        void* pAnimatableSceneObjectDesc,
        void* pDx11,
        CMeshData* arrMeshDraw,
        int nDataCount,
        void* pSceneView,
        void* pSceneLayer,
        void* pUnk,
        void* pUnk2
    );

    bool OnDrawObject2(
        void* pAnimatableSceneObjectDesc,
        void* pDx11,
        CMeshData* arrMeshDraw,
        int nDataCount,
        void* pSceneView,
        void* pSceneLayer,
        void* pUnk,
        void* pUnk2
    );
    CStrongHandle<CMaterial2> CreateMaterial(
        const char* szMaterialName,
        const char szVmatBuffer[]
    );

    bool OverrideMaterial(
        void* pAnimatableSceneObjectDesc,
        void* pDx11,
        CMeshData* arrMeshDraw,
        int nDataCount,
        void* pSceneView,
        void* pSceneLayer,
        void* pUnk,
        void* pUnk2
    );

    inline bool bInitialized = false;

}
