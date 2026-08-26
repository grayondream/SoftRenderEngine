#include "IScene.hpp"

#include "Base/AngleLine.hpp"
#include "Base/Triangle.hpp"
#include "Base/Rect.hpp"
#include "Base/SimpleTexture.hpp"
#include "Base/Cube.hpp"
#include "Base/Camera.hpp"
#include "Light/Ambient.hpp"
#include "Light/Diffuse.hpp"
#include "Light/Specular.hpp"
#include "Light/Material.hpp"
#include "Light/LightMap.hpp"
#include "Light/DirLight.hpp"
#include "Light/PointLight.hpp"
#include "Light/SpotLightScene.hpp"
#include "Light/MultiLights.hpp"
#include "Model/LoadModel.hpp"
#include "Advanced/DepthTest.hpp"
#include "Advanced/Blend.hpp"
#include "Advanced/CullFace.hpp"
#include "Advanced/FrameBuffer.hpp"
#include "Advanced/Skybox.hpp"
#include "Advanced/SimpleGeometry.hpp"
#include "Advanced/Explode.hpp"
#include "Advanced/NormalLine.hpp"
#include "Advanced/MultiInstance.hpp"
#include "Advanced/Saturn.hpp"
#include "Advanced/MSAA.hpp"
#include "LightAdv/BlinnPhong.hpp"
#include "LightAdv/Gamma.hpp"
#include "LightAdv/ShadowMap.hpp"
#include "LightAdv/PointLightShadow.hpp"
#include "LightAdv/NormalMap.hpp"
#include "LightAdv/ParallaxMap.hpp"
#include "LightAdv/HDR.hpp"
#include "LightAdv/Bloom.hpp"
#include "LightAdv/Defer.hpp"
#include "LightAdv/SSAO.hpp"
#include "PBR/PbrBase.hpp"
#include "PBR/PbrTexture.hpp"
#include "PBR/Irradiance.hpp"
#include "PBR/CausticScene.hpp"

namespace SGE::Samples {

void registerBuiltinScenes(){
    static bool done = false;
    if(done){ return; }
    done = true;
    auto &reg = SceneRegistry::instance();
    // Order mirrors GraphicsAPILearn AppType enum (GL samples sequence)
        reg.add("Base", "30-degree Line",
             [] { return std::make_unique<AngleLineScene>(); });
reg.add("Base", "Triangle (vertex color)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<TriangleScene>(); });
    reg.add("Base", "Textured Rect",
        []() -> std::unique_ptr<IScene> { return std::make_unique<RectScene>(); });
    reg.add("Base", "Simple Texture",
        []() -> std::unique_ptr<IScene> { return std::make_unique<SimpleTextureScene>(); });
    reg.add("Base", "Textured Cube (container2)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<CubeTextureScene>(); });
    reg.add("Base", "Camera Walkthrough",
        []() -> std::unique_ptr<IScene> { return std::make_unique<CameraWalkScene>(); });
    reg.add("Light", "Ambient Light Only",
        []() -> std::unique_ptr<IScene> { return std::make_unique<AmbientLightScene>(); });
    reg.add("Light", "Diffuse (Lambert)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<DiffuseLightScene>(); });
    reg.add("Light", "Specular Highlights",
        []() -> std::unique_ptr<IScene> { return std::make_unique<SpecularLightScene>(); });
    reg.add("Light", "Material System (4 materials)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<MaterialScene>(); });
    reg.add("Light", "Light Maps (diffuse tex)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<LightMapScene>(); });
    reg.add("Light", "Directional Light Source",
        []() -> std::unique_ptr<IScene> { return std::make_unique<DirLightWallScene>(); });
    reg.add("Light", "Point Light Source (attenuation)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<PointLightScene>(); });
    reg.add("Light", "Spot Light Source (soft cone)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<SpotLightScene>(); });
    reg.add("Light", "Multiple Light Sources",
        []() -> std::unique_ptr<IScene> { return std::make_unique<MultiLightsScene>(); });
    reg.add("Model", "Load Model (nanosuit)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<LoadModelScene>(); });
    reg.add("Advanced", "Depth Test Overlap",
        []() -> std::unique_ptr<IScene> { return std::make_unique<DepthTestScene>(); });
    reg.add("Advanced", "Alpha Blend Windows",
        []() -> std::unique_ptr<IScene> { return std::make_unique<BlendScene>(); });
    reg.add("Advanced", "Backface Culling",
        []() -> std::unique_ptr<IScene> { return std::make_unique<CullFaceScene>(); });
    reg.add("Advanced", "Render-to-Texture Cube",
        []() -> std::unique_ptr<IScene> { return std::make_unique<FrameBufferScene>(); });
    reg.add("Advanced", "HDR Equirect Skybox (newport loft)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<SkyboxScene>(); });
    reg.add("Advanced", "Simple Geometry",
        []() -> std::unique_ptr<IScene> { return std::make_unique<SimpleGeometryScene>(); });
    reg.add("Advanced", "Explode (CPU vertex offset)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<ExplodeScene>(); });
    reg.add("Advanced", "Normal Visualization Lines",
        []() -> std::unique_ptr<IScene> { return std::make_unique<NormalLineScene>(); });
    reg.add("Advanced", "100 Cubes Instancing",
        []() -> std::unique_ptr<IScene> { return std::make_unique<MultiInstanceScene>(); });
    reg.add("Advanced", "Saturn Ring System",
        []() -> std::unique_ptr<IScene> { return std::make_unique<SaturnScene>(); });
    reg.add("Advanced", "Anti-aliasing (2x SSAA)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<MSAAScene>(); });
    reg.add("LightAdv", "Phong vs Blinn-Phong highlight",
        []() -> std::unique_ptr<IScene> { return std::make_unique<BlinnPhongScene>(); });
    reg.add("LightAdv", "Gamma Correction",
        []() -> std::unique_ptr<IScene> { return std::make_unique<GammaScene>(); });
    reg.add("LightAdv", "Spot Shadow Map + PCF",
        []() -> std::unique_ptr<IScene> { return std::make_unique<ShadowMapScene>(); });
    reg.add("LightAdv", "Point Light Cube Shadow",
        []() -> std::unique_ptr<IScene> { return std::make_unique<PointLightShadowScene>(); });
    reg.add("LightAdv", "Normal Mapping (brickwall)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<NormalMapScene>(); });
    reg.add("LightAdv", "Parallax Mapping (steep)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<ParallaxMapScene>(); });
    reg.add("LightAdv", "HDR Tonemapping",
        []() -> std::unique_ptr<IScene> { return std::make_unique<HDRTonemapScene>(); });
    reg.add("LightAdv", "Glowing spheres with bloom",
        []() -> std::unique_ptr<IScene> { return std::make_unique<BloomScene>(); });
    reg.add("LightAdv", "Deferred Shading (G-buffer)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<DeferScene>(); });
    reg.add("LightAdv", "SSAO (depth contrast)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<SSAOScene>(); });
    reg.add("PBR", "PBR Base (metal/rough array)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<PbrBaseScene>(); });
    reg.add("PBR", "PBR Textured Sphere (rusted iron)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<PbrTextureScene>(); });
    reg.add("PBR", "IBL Diffuse Irradiance (loft)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<IrradianceScene>(); });
    reg.add("RayTracing", "PBR Caustic / Light Cone",
        []() -> std::unique_ptr<IScene> { return std::make_unique<CausticScene>(); });
}

}
