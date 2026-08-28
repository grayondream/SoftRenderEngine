#pragma once

#include "../SceneUtil.hpp"
#include "Render/RayTrace.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace SGE::Samples {

class CausticScene final : public IScene {
public:
    bool m_enableCones{true};

    void setup(Application &app) override {
        resetCamera(app, 0.0, 3.0, -4.5, 0.0, -0.85);

        const double S = 5.0;
        const double FY = -S;

        auto addQuad = [&](Vector3DBase<double> a, Vector3DBase<double> b,
                           Vector3DBase<double> c, Vector3DBase<double> d,
                           Color32 alb, float refl){
            SGE::Render::RayTriangle t1, t2;
            t1.a = a; t1.b = b; t1.c = c;
            t2.a = a; t2.b = c; t2.c = d;
            t1.albedo = alb; t1.reflectivity = refl;
            t2.albedo = alb; t2.reflectivity = refl;
            m_scene.triangles.push_back(t1);
            m_scene.triangles.push_back(t2);
        };

        Color32 red{220, 40, 40, 255};
        Color32 grn{40, 200, 40, 255};
        Color32 blu{50, 80, 230, 255};
        float fg = 0.25f;

        // floor  y=-S  (normal up) — frosted glass
        addQuad({-S,FY,-S},{ S,FY,-S},{ S,FY, S},{-S,FY, S}, red, fg);
        // ceiling y=+S  (normal down = inward) — frosted glass
        addQuad({-S, S,-S},{ S, S, S},{-S, S, S},{ S, S,-S}, red, fg);
        // left   x=-S  (normal +x) — frosted glass
        addQuad({-S,FY,-S},{-S,FY, S},{-S, S, S},{-S, S,-S}, grn, fg);
        // right  x=+S  (normal -x) — frosted glass
        addQuad({ S,FY, S},{ S,FY,-S},{ S, S,-S},{ S, S, S}, grn, fg);
        // back   z=-S  (normal +z) — frosted glass
        addQuad({ S,FY,-S},{-S,FY,-S},{-S, S,-S},{ S, S,-S}, blu, fg);
        // front  z=+S  (normal -z = inward) — frosted glass
        addQuad({-S,FY, S},{ S,FY, S},{ S, S, S},{-S, S, S}, blu, fg);

        // --- objects on the floor ---
        // 1. large metal sphere (center)
        {
            SGE::Render::RaySphere sp{};
            sp.center = {0.0, FY + 1.4, 0.0};
            sp.radius = 1.4;
            sp.albedo = {210, 210, 220, 255};
            sp.reflectivity = 0.95f;
            m_scene.spheres.push_back(sp);
        }
        // 2. frosted glass sphere
        {
            SGE::Render::RaySphere sp{};
            sp.center = {-2.8, FY + 1.0, 1.0};
            sp.radius = 1.0;
            sp.albedo = {200, 220, 240, 255};
            sp.reflectivity = 0.08f;
            sp.refractivity = 0.75f;
            sp.ior = 1.45;
            m_scene.spheres.push_back(sp);
        }
        // 3. clear glass sphere
        {
            SGE::Render::RaySphere sp{};
            sp.center = {2.5, FY + 0.8, -1.5};
            sp.radius = 0.8;
            sp.albedo = {230, 240, 255, 255};
            sp.reflectivity = 0.05f;
            sp.refractivity = 0.92f;
            sp.ior = 1.5;
            m_scene.spheres.push_back(sp);
        }
        // 4. small gold metal sphere
        {
            SGE::Render::RaySphere sp{};
            sp.center = {1.0, FY + 0.5, 2.5};
            sp.radius = 0.5;
            sp.albedo = {240, 225, 180, 255};
            sp.reflectivity = 0.95f;
            m_scene.spheres.push_back(sp);
        }
        // 5. crystal sphere
        {
            SGE::Render::RaySphere sp{};
            sp.center = {-1.5, FY + 0.65, -2.5};
            sp.radius = 0.65;
            sp.albedo = {240, 225, 255, 255};
            sp.reflectivity = 0.05f;
            sp.refractivity = 0.95f;
            sp.ior = 2.0;
            m_scene.spheres.push_back(sp);
        }

        // --- cone (24-sided, metal) ---
        {
            const double cx = -2.5, cz = -2.0;
            const double cr = 0.7, ch = 2.0;
            const double cy = FY + ch;
            const int N = 24;
            for(int i = 0; i < N; i++){
                double a0 = 2.0 * M_PI * i / N;
                double a1 = 2.0 * M_PI * (i + 1) / N;
                SGE::Render::RayTriangle side;
                side.a = {cx, cy, cz};
                side.b = {cx + cr * std::cos(a0), FY, cz + cr * std::sin(a0)};
                side.c = {cx + cr * std::cos(a1), FY, cz + cr * std::sin(a1)};
                side.albedo = {210, 210, 220, 255};
                side.reflectivity = 0.92f;
                m_scene.triangles.push_back(side);
            }
            for(int i = 0; i < N; i++){
                double a0 = 2.0 * M_PI * i / N;
                double a1 = 2.0 * M_PI * (i + 1) / N;
                SGE::Render::RayTriangle cap;
                cap.a = {cx, FY, cz};
                cap.b = {cx + cr * std::cos(a1), FY, cz + cr * std::sin(a1)};
                cap.c = {cx + cr * std::cos(a0), FY, cz + cr * std::sin(a0)};
                cap.albedo = {210, 210, 220, 255};
                cap.reflectivity = 0.92f;
                m_scene.triangles.push_back(cap);
            }
        }

        // --- cube (matte gray, 12 triangles) ---
        {
            const double cx = 2.0, cy = FY + 0.75, cz = 2.0;
            const double hs = 0.75;
            Vector3DBase<double> v[8] = {
                {cx-hs, cy-hs, cz-hs}, {cx+hs, cy-hs, cz-hs},
                {cx+hs, cy+hs, cz-hs}, {cx-hs, cy+hs, cz-hs},
                {cx-hs, cy-hs, cz+hs}, {cx+hs, cy-hs, cz+hs},
                {cx+hs, cy+hs, cz+hs}, {cx-hs, cy+hs, cz+hs}
            };
            int faces[6][3][3] = {
                {{0,1,2},{0,2,3}},
                {{5,4,7},{5,7,6}},
                {{1,5,6},{1,6,2}},
                {{4,0,3},{4,3,7}},
                {{3,2,6},{3,6,7}},
                {{4,5,1},{4,1,0}}
            };
            Color32 galb{160, 160, 170, 255};
            for(int f = 0; f < 6; f++){
                for(int t = 0; t < 2; t++){
                    SGE::Render::RayTriangle tri;
                    tri.a = v[faces[f][t][0]];
                    tri.b = v[faces[f][t][1]];
                    tri.c = v[faces[f][t][2]];
                    tri.albedo = galb;
                    tri.reflectivity = 0.0f;
                    m_scene.triangles.push_back(tri);
                }
            }
        }

        // --- tall metal cylinder ---
        {
            const double cx = -1.0, cz = 2.0;
            const double cr = 0.45, ch = 1.8;
            const int N = 20;
            for(int i = 0; i < N; i++){
                double a0 = 2.0 * M_PI * i / N;
                double a1 = 2.0 * M_PI * (i + 1) / N;
                double x0 = cx + cr * std::cos(a0), z0 = cz + cr * std::sin(a0);
                double x1 = cx + cr * std::cos(a1), z1 = cz + cr * std::sin(a1);
                SGE::Render::RayTriangle s1, s2;
                s1.a = {x0, FY, z0}; s1.b = {x1, FY, z1}; s1.c = {x1, FY+ch, z1};
                s2.a = {x0, FY, z0}; s2.b = {x1, FY+ch, z1}; s2.c = {x0, FY+ch, z0};
                s1.albedo = {180, 160, 120, 255}; s1.reflectivity = 0.95f;
                s2.albedo = {180, 160, 120, 255}; s2.reflectivity = 0.95f;
                m_scene.triangles.push_back(s1);
                m_scene.triangles.push_back(s2);
            }
        }

        // --- spot cone ---
        m_scene.cone.position = {0.0, S - 0.5, 0.0};
        m_scene.cone.direction = {0.0, -1.0, 0.0};
        m_scene.cone.cutoffCos = 0.85;
        m_scene.cone.range = 20.0;
        m_scene.cone.intensity = 1.0f;
        m_scene.cone.enabled = true;

        m_scene.bboxMin = {-50.0, -50.0, -50.0};
        m_scene.bboxMax = {50.0, 50.0, 50.0};
        m_scene.hasBBox = false;
    }

private:
    SGE::Render::RayScene m_scene;

    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);

        LightingRig rig{};
        rig.ambient = 0.15f;
        rig.specularStrength = 0.2f;
        rig.shininess = 24.0f;

        {
            DirectionalLight key{};
            key.direction = Vector3DBase<double>{-0.3, 0.9, -0.5};
            key.color = ColorFlt{1.0f, 0.95f, 0.9f};
            rig.directional.push_back(key);
        }
        {
            DirectionalLight fill{};
            fill.direction = Vector3DBase<double>{0.7, 0.3, 0.4};
            fill.color = ColorFlt{0.4f, 0.45f, 0.55f};
            rig.directional.push_back(fill);
        }

        // 4 strong point lights inside the room
        auto addPL = [&](double x, double y, double z,
                         float r, float g, float b, float intensity){
            PointLight pl{};
            pl.position = Vector3DBase<double>{x, y, z};
            pl.color = ColorFlt{r * intensity, g * intensity, b * intensity};
            pl.range = 15.0;
            pl.linear = 0.04f;
            pl.quadratic = 0.02f;
            rig.point.push_back(pl);
        };
        addPL(-2.0, 3.5, -2.0, 1.0, 0.85, 0.7, 2.0f);
        addPL( 2.5, 3.0,  1.5, 0.7, 0.9, 1.0, 2.0f);
        addPL( 0.0, 4.0,  0.0, 1.0, 1.0, 1.0, 2.5f);
        addPL( 1.5, 1.5, -3.0, 1.0, 0.6, 0.5, 1.5f);

        SGE::Render::RayTraceOptions opt{};
        opt.maxDepth = 3;
        opt.background = Color32{20, 20, 30, 255};
        opt.renderScale = 2;

        m_scene.cone.enabled = m_enableCones;

        SGE::Render::RayTracer tracer{fb};
        tracer.render(m_scene, app.camera(), rig, opt);
    }

    void drawUi(Application &) override {
        ImGui::Begin("Settings");
        ImGui::Text("Cube Room - Ray Traced Infinity Mirrors");
        ImGui::Checkbox("Enable Light Cones", &m_enableCones);
        ImGui::Separator();
        ImGui::Text("Spheres: %d", static_cast<int>(m_scene.spheres.size()));
        ImGui::Text("Triangles: %d", static_cast<int>(m_scene.triangles.size()));
        ImGui::End();
    }

    const char *name() const override {
        return "Cube Room - Infinity Mirrors";
    }
    const char *group() const override { return "RayTracing"; }
};

}
