#include "IScene.hpp"
#include "Showcase/ShowcaseLit.hpp"
#include "Showcase/ShowcaseFilter.hpp"
#include "Showcase/ShowcaseAlpha.hpp"
#include "Showcase/ShowcaseShadow.hpp"
#include "Showcase/ShowcaseEnv.hpp"
#include "Showcase/ShowcasePbr.hpp"
#include "Showcase/ShowcaseRayTrace.hpp"
#include "Base/Triangle.hpp"
#include "Base/Rect.hpp"
#include "Base/Cube.hpp"
#include "Base/Camera.hpp"
#include "Base/SimpleTexture.hpp"
#include "Light/Ambient.hpp"
#include "Light/Diffuse.hpp"
#include "Light/Specular.hpp"
#include "Light/Material.hpp"
#include "Light/LightMap.hpp"
#include "Light/DirLight.hpp"
#include "Light/PointLight.hpp"
#include "Light/SpotLightScene.hpp"
#include "Light/MultiLights.hpp"
#include "LightAdv/NormalMap.hpp"
#include "LightAdv/ParallaxMap.hpp"
#include "Advanced/DepthTest.hpp"
#include "Advanced/CullFace.hpp"
#include "Advanced/Blend.hpp"
#include "Advanced/Explode.hpp"
#include "Advanced/NormalLine.hpp"
#include "Advanced/MultiInstance.hpp"
#include "Advanced/Saturn.hpp"

namespace SGE::Samples {

void registerBuiltinScenes(){
    static bool done = false;
    if(done){ return; }
    done = true;
    auto &reg = SceneRegistry::instance();
    reg.add("Showcase", "Geometry Showcase + Lighting",
        []() -> std::unique_ptr<IScene> { return std::make_unique<ShowcaseLitScene>(); });
    reg.add("Showcase", "Mipmap + Trilinear vs Nearest",
        []() -> std::unique_ptr<IScene> { return std::make_unique<ShowcaseFilterScene>(); });
    reg.add("Showcase", "Depth + Alpha Blending",
        []() -> std::unique_ptr<IScene> { return std::make_unique<ShowcaseAlphaScene>(); });
    reg.add("Showcase", "Spot Shadow Mapping + PCF",
        []() -> std::unique_ptr<IScene> { return std::make_unique<ShowcaseShadowScene>(); });
    reg.add("Showcase", "Env Reflection / Refraction",
        []() -> std::unique_ptr<IScene> { return std::make_unique<ShowcaseEnvScene>(); });
    reg.add("Showcase", "Cook-Torrance PBR Sphere Array",
        []() -> std::unique_ptr<IScene> { return std::make_unique<ShowcasePbrScene>(); });
    reg.add("Showcase", "Light Cone Ray Tracing",
        []() -> std::unique_ptr<IScene> { return std::make_unique<ShowcaseRayTraceScene>(); });
    reg.add("Base", "Triangle (vertex color)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<TriangleScene>(); });
    reg.add("Base", "Textured Rect",
        []() -> std::unique_ptr<IScene> { return std::make_unique<RectScene>(); });
    reg.add("Base", "Textured Cube (container2)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<CubeTextureScene>(); });
    reg.add("Base", "Camera Walkthrough",
        []() -> std::unique_ptr<IScene> { return std::make_unique<CameraWalkScene>(); });
    reg.add("Base", "Simple Texture",
        []() -> std::unique_ptr<IScene> { return std::make_unique<SimpleTextureScene>(); });
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
    reg.add("Light", "Directional Light",
        []() -> std::unique_ptr<IScene> { return std::make_unique<DirLightWallScene>(); });
    reg.add("Light", "Point Light (attenuation)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<PointLightScene>(); });
    reg.add("Light", "Spot Light (soft cone)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<SpotLightScene>(); });
    reg.add("Light", "Multiple Lights",
        []() -> std::unique_ptr<IScene> { return std::make_unique<MultiLightsScene>(); });
    reg.add("LightAdv", "Normal Mapping (brickwall)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<NormalMapScene>(); });
    reg.add("LightAdv", "Parallax Mapping (steep)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<ParallaxMapScene>(); });
    reg.add("Advanced", "Depth Test Overlap",
        []() -> std::unique_ptr<IScene> { return std::make_unique<DepthTestScene>(); });
    reg.add("Advanced", "Backface Culling",
        []() -> std::unique_ptr<IScene> { return std::make_unique<CullFaceScene>(); });
    reg.add("Advanced", "Alpha Blend Windows",
        []() -> std::unique_ptr<IScene> { return std::make_unique<BlendScene>(); });
    reg.add("Advanced", "Explode (CPU vertex offset)",
        []() -> std::unique_ptr<IScene> { return std::make_unique<ExplodeScene>(); });
    reg.add("Advanced", "Normal Visualization Lines",
        []() -> std::unique_ptr<IScene> { return std::make_unique<NormalLineScene>(); });
    reg.add("Advanced", "100 Cubes Instancing",
        []() -> std::unique_ptr<IScene> { return std::make_unique<MultiInstanceScene>(); });
    reg.add("Advanced", "Saturn Ring System",
        []() -> std::unique_ptr<IScene> { return std::make_unique<SaturnScene>(); });
}

}
