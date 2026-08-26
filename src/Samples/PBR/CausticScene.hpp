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
        resetCamera(app, 0.0, 4.0, 14.0);

        // --- ground plane as 2 triangles ---
        const double gY = -1.0, gS = 20.0;
        SGE::Render::RayTriangle t1, t2;
        t1.a = {-gS, gY, -gS}; t1.b = {gS, gY, -gS}; t1.c = {gS, gY, gS};
        t2.a = {-gS, gY, -gS}; t2.b = {gS, gY, gS};  t2.c = {-gS, gY, gS};
        t1.albedo = {180, 180, 185, 255}; t1.reflectivity = 0.15f;
        t2.albedo = {180, 180, 185, 255}; t2.reflectivity = 0.15f;
        m_scene.triangles.push_back(t1);
        m_scene.triangles.push_back(t2);

        // --- material pool: 5 types ---
        struct MatDef {
            Color32 color;
            float refl, refr;
            double ior;
            double minR, maxR;
        };
        const MatDef defs[5] = {
            {{210, 80, 70, 255},    0.0f, 0.0f, 1.0,  0.8, 1.3},  // matte
            {{200, 220, 240, 255},  0.1f, 0.7f, 1.45, 0.7, 1.1},  // frosted glass
            {{230, 240, 255, 255},  0.05f,0.9f, 1.5,  0.6, 1.0},  // clear glass
            {{240, 225, 255, 255},  0.05f,0.95f,2.0,  0.5, 0.9},  // crystal
            {{220, 220, 230, 255},  0.95f,0.0f, 1.0,  0.7, 1.2},  // metal
        };

        std::mt19937 rng(42);
        std::uniform_int_distribution<int> matPick(0, 4);
        std::uniform_real_distribution<double> posDist(-8.0, 8.0);
        std::uniform_real_distribution<double> tintR(0.7, 1.0);
        std::uniform_real_distribution<double> tintG(0.7, 1.0);
        std::uniform_real_distribution<double> tintB(0.7, 1.0);

        const int N = 30;
        for(int attempt = 0; attempt < 3000
            && static_cast<int>(m_scene.spheres.size()) < N; attempt++){
            double x = posDist(rng), z = posDist(rng);
            int mi = matPick(rng);
            const auto &d = defs[mi];
            double r = std::uniform_real_distribution<double>(
                d.minR, d.maxR)(rng);
            bool ok = true;
            for(const auto &s : m_scene.spheres){
                double dx = x - s.center.x, dz = z - s.center.z;
                double minD = r + s.radius + 0.3;
                if(dx*dx + dz*dz < minD * minD){ ok = false; break; }
            }
            if(!ok) continue;
            SGE::Render::RaySphere sp{};
            sp.center = Vector3DBase<double>{x, -1.0 + r, z};
            sp.radius = r;
            sp.albedo = Color32{
                static_cast<int32_t>(d.color.r * tintR(rng)),
                static_cast<int32_t>(d.color.g * tintG(rng)),
                static_cast<int32_t>(d.color.b * tintB(rng)), 255};
            sp.reflectivity = d.refl;
            sp.refractivity = d.refr;
            sp.ior = d.ior;
            m_scene.spheres.push_back(sp);
        }

        // --- 3 focal spheres (center cluster) ---
        {
            SGE::Render::RaySphere g{};
            g.center = {0.0, 1.0, 0.0};
            g.radius = 2.0;
            g.albedo = {230, 240, 255, 255};
            g.reflectivity = 0.05f;
            g.refractivity = 0.9f;
            g.ior = 1.5;
            m_scene.spheres.push_back(g);
        }
        {
            SGE::Render::RaySphere c{};
            c.center = {-3.2, 0.6, 1.5};
            c.radius = 1.6;
            c.albedo = {240, 225, 255, 255};
            c.reflectivity = 0.05f;
            c.refractivity = 0.95f;
            c.ior = 2.0;
            m_scene.spheres.push_back(c);
        }
        {
            SGE::Render::RaySphere m{};
            m.center = {2.8, 0.5, -1.5};
            m.radius = 1.5;
            m.albedo = {220, 220, 230, 255};
            m.reflectivity = 0.95f;
            m.refractivity = 0.0f;
            m.ior = 1.0;
            m_scene.spheres.push_back(m);
        }

        // --- spot cone (for caustic) ---
        m_scene.cone.position = {0.0, 12.0, 0.0};
        m_scene.cone.direction = {0.0, -1.0, 0.0};
        m_scene.cone.cutoffCos = 0.85;
        m_scene.cone.range = 30.0;
        m_scene.cone.intensity = 1.0f;
        m_scene.cone.enabled = true;
    }

private:
    SGE::Render::RayScene m_scene;

    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);

        // --- directional lighting ---
        LightingRig rig{};
        rig.ambient = 0.18f;
        {
            DirectionalLight key{};
            key.direction = Vector3DBase<double>{-0.4, 0.9, -0.6};
            key.color = ColorFlt{1.0f, 0.95f, 0.9f};
            rig.directional.push_back(key);
        }
        {
            DirectionalLight fill{};
            fill.direction = Vector3DBase<double>{0.6, 0.4, 0.5};
            fill.color = ColorFlt{0.35f, 0.4f, 0.5f};
            rig.directional.push_back(fill);
        }

        SGE::Render::RayTraceOptions opt{};
        opt.maxDepth = 4;
        opt.background = Color32{40, 50, 70, 255};

        m_scene.cone.enabled = m_enableCones;

        SGE::Render::RayTracer tracer{fb};
        tracer.render(m_scene, app.camera(), rig, opt);
    }

    void drawUi(Application &) override {
        ImGui::Begin("Settings");
        ImGui::Text("Ray-Traced Caustic / Light Cone");
        ImGui::Checkbox("Enable Light Cones", &m_enableCones);
        ImGui::Separator();
        ImGui::Text("Spheres: %d", static_cast<int>(m_scene.spheres.size()));
        ImGui::Text("Triangles: %d", static_cast<int>(m_scene.triangles.size()));
        ImGui::End();
    }

    const char *name() const override {
        return "RayTraced Caustic / Light Cone";
    }
    const char *group() const override { return "RayTracing"; }
};

}
