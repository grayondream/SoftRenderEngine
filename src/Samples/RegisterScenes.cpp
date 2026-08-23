#include "IScene.hpp"
#include "Showcase/ShowcaseLit.hpp"
#include "Showcase/ShowcaseFilter.hpp"
#include "Showcase/ShowcaseAlpha.hpp"
#include "Showcase/ShowcaseShadow.hpp"
#include "Showcase/ShowcaseEnv.hpp"
#include "Showcase/ShowcasePbr.hpp"
#include "Showcase/ShowcaseRayTrace.hpp"

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
}

}
