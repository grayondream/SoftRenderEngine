#pragma once

#include "../SceneUtil.hpp"
#include "Render/ObjLoader.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>
#include <vector>

namespace SGE::Samples {

class SaturnScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        LightingRig rig{};
        rig.ambient = 0.35f;
        rig.specularStrength = 0.3f;
        DirectionalLight key{};
        key.direction = Vector3DBase<double>{-0.5, 0.6, -1.0};
        rig.directional.push_back(key);
        Rasterizer rz{fb};
        ShadingContext ctx{&rig, app.camera().position};
        const auto vp = refViewProj(app.camera());
        // reference planet mesh (planet.obj, mars.png)
        static Texture marsTex = SGE::Render::ImageLoader::loadTexture(
            "assets/models/planet/mars.png");
        static Object4D planet = []{
            Object4D o{};
            loadObjFromFile("assets/models/planet/planet.obj", o);
            return o;
        }();
        const double spin = app.angle() * 0.6;
        auto pm = SGE::Math::translation(0.0, 0.9, 0.0)
            .mul(SGE::Math::rotationY(spin))
            .mul(SGE::Math::scale(0.3, 0.3, 0.3));
        auto pnrm = SGE::Math::normalMatrix(pm);
        SGE::Render::TileRenderer tiled{fb};
        tiled.drawTextured(Pipeline::projectObject(planet, pm,
            vp, pnrm, g_renderW, g_renderH), marsTex, &ctx);
        // reference rock ring: rock.obj instances, radius 20 -> local 4
        static Texture rockTex = SGE::Render::ImageLoader::loadTexture(
            "assets/models/rock/rock.png");
        static Object4D rock = []{
            Object4D o{};
            loadObjFromFile("assets/models/rock/rock.obj", o);
            return o;
        }();
        constexpr int kRocks = 240;
        for(int i = 0; i < kRocks; i++){
            const double angle = i * 360.0 / kRocks;
            const double disp = ((i * 37) % 200) / 10.0 - 10.0;
            const double rad = angle * M_PI / 180.0;
            const double rr = 4.0;
            double x = std::sin(rad) * rr + disp * 0.08;
            double y = disp * 0.032;
            double z = std::cos(rad) * rr + disp * 0.08;
            const double sc = (((i * 13) % 20) / 100.0 + 0.05) * 1.6;
            const double orbit = app.angle() / 10.0;
            const double cx = std::cos(orbit), sy2 = std::sin(orbit);
            const double wx = x * cx - z * sy2;
            const double wz = x * sy2 + z * cx;
            const double ra = (i % 7) * 0.9 + orbit;
            auto rm = SGE::Math::translation(wx, y + 0.9, wz)
                .mul(SGE::Math::rotationY(ra))
                .mul(SGE::Math::rotationZ(ra * 0.4))
                .mul(SGE::Math::scale(sc, sc, sc));
            auto rnrm = SGE::Math::normalMatrix(rm);
            tiled.drawTextured(Pipeline::projectObject(rock, rm,
                vp, rnrm, g_renderW, g_renderH), rockTex, &ctx);
        }
    }
    void drawUi(Application &) override {
        ImGui::Begin("Settings");
        ImGui::End();
    }
    const char *name() const override { return "Saturn Ring System"; }
    const char *group() const override { return "Advanced"; }
};

}
