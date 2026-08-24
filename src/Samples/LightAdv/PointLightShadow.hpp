#pragma once

#include "../SceneUtil.hpp"
#include "../Light/LightUtil.hpp"
#include "Render/ImageLoader.hpp"
#include "Render/Shadow.hpp"

#include <cmath>

namespace SGE::Samples {

class PointLightShadowScene final : public IScene {
public:
    bool m_shadows{true};
private:
    static Object4D MakeCubeRef(){
        // large inverted cube used as room interior
        Object4D cube{};
        std::snprintf(cube.name, sizeof(cube.name), "%s", "room");
        double s = 0.5;
        Point4D v[8] = {{-s,-s,-s,1},{s,-s,-s,1},{s,s,-s,1},{-s,s,-s,1},
                        {-s,-s,s,1},{s,-s,s,1},{s,s,s,1},{-s,s,s,1}};
        for(int i = 0; i < 8; i++){ cube.vlistLocal[i] = v[i]; }
        struct F{ int a,b,c; };
        const F fs[12] = {{0,3,2},{0,2,1},{4,5,6},{4,6,7},{0,1,5},{0,5,4},
                          {3,7,6},{3,6,2},{1,2,6},{1,6,5},{0,4,7},{0,7,3}};
        cube.numVertices = 8;
        cube.numPolys = 12;
        for(int i = 0; i < 12; i++){
            // flipped winding -> inward faces
            cube.plist[static_cast<std::size_t>(i)].vlist[0] =
                v[fs[i].a];
            cube.plist[static_cast<std::size_t>(i)].vlist[1] =
                v[fs[i].c];
            cube.plist[static_cast<std::size_t>(i)].vlist[2] =
                v[fs[i].b];
            Vector3DBase<double> fn{-fs[i].a * 0.0 - fs[i].b * 0.0,
                0, 0};
            (void)fn;
        }
        return cube;
    }

    void render(Application &app) override {
        auto &fb = app.framebuffer();
        auto rig = makeDefaultRig();
        rig.ambient = 0.3f;
        rig.specularStrength = 0.4f;
        rig.shininess = 64.0f;
        const double ang = app.angle();

        fb.clear(kRefClear);
        Rasterizer rz{fb};
        const auto viewProj = refViewProj(app.camera());
        const auto nrm = SGE::Math::normalMatrix(
            SGE::Math::translation(0.0, 0.0, 0.0));

        static Texture wood = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/wood.png");

        // room box: inverted cube (flipped winding) scale 10
        static Object4D room = []{
            Object4D r = MakeCubeRef();
            return r;
        }();
        auto rm = SGE::Math::translation(0.0, 3.5, 2.5)
            .mul(SGE::Math::scale(10.0, 10.0, 10.0));
        auto rnrm = SGE::Math::normalMatrix(rm);

        // point light oscillates along z (reference z = sin(t) * 10)
        const Vector3DBase<double> lightPos{0.0, 1.2,
            2.5 + 4.0 * std::sin(ang)};
        PointLight pl{};
        pl.position = lightPos;
        pl.color = ColorFlt{1.0f, 0.96f, 0.88f};
        pl.range = 60.0;
        rig.point.push_back(pl);

        struct Obstacle{ double x, y, z, s; };
        // reference cube array
        static const Obstacle obs[5] = {
            {4.0 * 0.35, -3.5 * 0.25 + 1.6, 0.0 + 2.5, 0.5},
            {2.0 * 0.35, 3.0 * 0.25 + 1.6, 1.0 + 2.5, 0.75},
            {-3.0 * 0.35, -1.0 * 0.25 + 1.6, 0.0 + 2.5, 0.5},
            {-1.5 * 0.35, 1.0 * 0.25 + 1.6, 1.5 + 2.5, 0.5},
            {-1.5 * 0.35, 2.0 * 0.25 + 1.6, -3.0 + 2.5, 0.75}};

        static FrameBuffer faces[6] = {
            FrameBuffer{256, 256}, FrameBuffer{256, 256},
            FrameBuffer{256, 256}, FrameBuffer{256, 256},
            FrameBuffer{256, 256}, FrameBuffer{256, 256}};
        for(int f = 0; f < 6; f++){
            faces[f].clear();
            Rasterizer srz{faces[f]};
            const auto vp = SGE::Render::cubeFaceVP(lightPos, f);
            for(const auto &ob : obs){
                Object4D c = unitCube(app);
                auto om = SGE::Math::translation(ob.x, ob.y, ob.z)
                    .mul(SGE::Math::scale(ob.s, ob.s, ob.s));
                auto onrm = SGE::Math::normalMatrix(om);
                auto ot = Pipeline::projectObject(c, om, vp, onrm, 256, 256);
                for(auto &t : ot){
                    if(m_shadows){
                        srz.drawTriangleDepth(t.v[0], t.v[1], t.v[2]);
                    }
                }
            }
        }

        SGE::Render::CubeShadowData cs{};
        cs.lightPos = lightPos;
        cs.farPlane = 25.0;
        cs.bias = 0.008;
        for(int f = 0; f < 6; f++){ cs.faces[f] = &faces[f]; }
        ShadingContext ctx = m_shadows
            ? ShadingContext{&rig, app.camera().position,
                nullptr, nullptr, &cs}
            : ShadingContext{&rig, app.camera().position};

        drawLamp(app, rz, lightPos, 0.1);
        SGE::Render::TileRenderer tiled{fb};
        tiled.drawTextured(Pipeline::projectObject(room, rm, viewProj, rnrm,
            800, 600), wood, &ctx);
        for(const auto &ob : obs){
            Object4D c = unitCube(app);
            auto om = SGE::Math::translation(ob.x, ob.y, ob.z)
                .mul(SGE::Math::scale(ob.s, ob.s, ob.s));
            auto onrm = SGE::Math::normalMatrix(om);
            tiled.drawTextured(Pipeline::projectObject(c, om, viewProj, onrm,
                800, 600), wood, &ctx);
        }
    }
    void drawUi(Application &) override {
        ImGui::Checkbox("Shadows", &m_shadows);
        ImGui::Text("light oscillates along z");
    }
    const char *name() const override { return "Point Light Cube Shadow"; }
    const char *group() const override { return "LightAdv"; }
};

}
