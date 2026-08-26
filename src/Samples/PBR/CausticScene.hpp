#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"
#include "Render/Primitives.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace SGE::Samples {

class CausticScene final : public IScene {
public:
    struct Sphere {
        double x, z;
        double radius;
        PbrMaterial mat;
        double ior;          // index of refraction (for refractive spheres)
        bool refractive;     // glass/crystal with refraction
    };

    bool m_enableCones{true};

    void setup(Application &app) override {
        m_ground = makePlane(15.0, -1.0,
            Vector3DBase<double>{0, 1, 0}, Color32{180, 180, 185, 255}, 15.0);
        m_white = Texture(1, 1, std::vector<uint32_t>{0xFFFFFFFFu}.data());

        // load skybox cubemap for environment reflections
        static const char *files[6] = {
            "assets/textures/Skybox/right.jpg",
            "assets/textures/Skybox/left.jpg",
            "assets/textures/Skybox/top.jpg",
            "assets/textures/Skybox/bottom.jpg",
            "assets/textures/Skybox/front.jpg",
            "assets/textures/Skybox/back.jpg"};
        for(int i = 0; i < 6; i++){
            m_faces[i] = SGE::Render::ImageLoader::loadTexture(files[i]);
        }

        // material pool: 5 types
        struct MatDef {
            Color32 color;
            float metallic, roughness, ao;
            bool refractive;
            double ior;
            double minR, maxR;
        };
        const MatDef defs[5] = {
            {{200, 80, 80, 255},    0.0f, 1.0f,  0.8f, false, 1.0, 1.2, 1.5},  // matte
            {{180, 210, 230, 255},  0.0f, 0.3f,  1.0f, true,  1.5, 0.8, 1.2},  // frosted glass
            {{220, 235, 255, 255},  0.0f, 0.05f, 1.0f, true,  1.5, 0.6, 1.0},  // glass
            {{230, 215, 255, 255},  0.0f, 0.0f,  1.0f, true,  2.0, 0.5, 0.9},  // crystal
            {{200, 200, 210, 255},  1.0f, 0.3f,  1.0f, false, 1.0, 0.6, 1.2},  // metal
        };

        // scatter spheres: random positions, no overlap
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> matPick(0, 4);
        std::uniform_real_distribution<double> posDist(-7.0, 7.0);
        std::uniform_real_distribution<double> tintR(0.6, 1.0);
        std::uniform_real_distribution<double> tintG(0.6, 1.0);
        std::uniform_real_distribution<double> tintB(0.6, 1.0);

        const int N = 40;
        for(int attempt = 0; attempt < 2000 && static_cast<int>(m_spheres.size()) < N; attempt++){
            double x = posDist(rng), z = posDist(rng);
            int mi = matPick(rng);
            const auto &d = defs[mi];
            double r = std::uniform_real_distribution<double>(d.minR, d.maxR)(rng);
            bool ok = true;
            for(const auto &s : m_spheres){
                double dx = x - s.x, dz = z - s.z;
                if(dx*dx + dz*dz < (r + s.radius + 0.3) * (r + s.radius + 0.3)){
                    ok = false; break;
                }
            }
            if(!ok) continue;

            PbrMaterial mat{};
            mat.baseColor = Color32{
                static_cast<int32_t>(d.color.r * tintR(rng)),
                static_cast<int32_t>(d.color.g * tintG(rng)),
                static_cast<int32_t>(d.color.b * tintB(rng)), 255};
            mat.metallic = d.metallic;
            mat.roughness = d.roughness;
            mat.ao = d.ao;

            Sphere sp{x, z, r, mat, d.ior, d.refractive};
            m_spheres.push_back(sp);
        }

        // center cluster: 3 large focal spheres (2 refractive + 1 metal)
        m_spheres.push_back({ 0.0,  0.0, 1.8,
            {Color32{230, 230, 245, 255}, 0.0f, 0.0f, 1.0f},
            1.8, true});
        m_spheres.push_back({-2.5,  1.0, 1.5,
            {Color32{220, 240, 255, 255}, 0.0f, 0.05f, 1.0f},
            1.5, true});
        m_spheres.push_back({ 2.0, -1.0, 1.3,
            {Color32{210, 210, 220, 255}, 1.0f, 0.15f, 1.0f},
            1.0, false});

        resetCamera(app, 0.0, 4.0, 14.0);
    }

private:
    Object4D m_ground{};
    Texture m_white{};
    Texture m_faces[6]{};
    std::vector<Sphere> m_spheres;

    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        Rasterizer rz{fb};
        const auto vp = refViewProj(app.camera());

        // lighting rig
        LightingRig rig{};
        rig.ambient = 0.12f;
        {
            DirectionalLight key{};
            key.direction = Vector3DBase<double>{-0.5, 0.9, -0.7};
            key.color = ColorFlt{1.0f, 0.98f, 0.95f};
            rig.directional.push_back(key);
        }
        {
            DirectionalLight fill{};
            fill.direction = Vector3DBase<double>{0.5, 0.4, 0.6};
            fill.color = ColorFlt{0.4f, 0.42f, 0.48f};
            rig.directional.push_back(fill);
        }
        const double lp[4][3] = {{-8,10,8},{8,10,8},{-8,-5,6},{8,-5,6}};
        for(int i = 0; i < 4; i++){
            PointLight p{};
            p.position = Vector3DBase<double>{lp[i][0], lp[i][1], lp[i][2]};
            p.range = 200.0;
            p.color = ColorFlt{300.0f, 290.0f, 280.0f, 1.0f};
            rig.point.push_back(p);
        }

        // draw ground plane
        ShadingContext gctx{&rig, app.camera().position};
        EnvParams genv{};
        if(m_enableCones){
            genv.enabled = true;
            genv.reflectivity = 0.5;
        }
        gctx.env = &genv;

        auto gm = SGE::Math::translation(0.0, 0.0, 0.0);
        auto gnrm = SGE::Math::normalMatrix(gm);
        auto gt = Pipeline::projectObject(m_ground, gm, vp, gnrm,
                                          g_renderW, g_renderH);
        for(auto &t : gt){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                    m_white, &gctx);
        }

        // draw spheres
        static Object4D protoBall = SGE::Render::MakeSphere(1.0, 20, 14);
        for(const auto &sp : m_spheres){
            Object4D ball = protoBall;
            for(int vi = 0; vi < static_cast<int>(ball.numVertices); vi++){
                ball.vlistLocal[vi].x *= sp.radius;
                ball.vlistLocal[vi].y *= sp.radius;
                ball.vlistLocal[vi].z *= sp.radius;
            }
            auto bm = SGE::Math::translation(sp.x, -1.0 + sp.radius, sp.z);
            auto bnrm = SGE::Math::normalMatrix(bm);
            auto bt = Pipeline::projectObject(ball, bm, vp, bnrm,
                                              g_renderW, g_renderH);

            ShadingContext ctx{&rig, app.camera().position};
            EnvParams ep{};
            if(m_enableCones && sp.refractive){
                ep.enabled = true;
                ep.reflectivity = 1.0;
                ep.refractivity = 0.9;
                ep.ior = static_cast<float>(sp.ior);
                for(int fi = 0; fi < 6; fi++){
                    ep.cubemapFaces[fi] = &m_faces[fi];
                }
                ctx.env = &ep;
            }else if(m_enableCones && sp.mat.metallic > 0.5f){
                ep.enabled = true;
                ep.reflectivity = 0.8;
                ep.refractivity = 0.0;
                for(int fi = 0; fi < 6; fi++){
                    ep.cubemapFaces[fi] = &m_faces[fi];
                }
                ctx.env = &ep;
            }
            ctx.pbr = &sp.mat;

            for(auto &t : bt){
                rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                        m_white, &ctx);
            }
        }

        // draw caustic light cones on ground (refractive spheres only)
        if(m_enableCones){
            for(const auto &sp : m_spheres){
                if(!sp.refractive || sp.ior < 1.01) continue;
                double gY = -1.0;
                double dy = (-1.0 + sp.radius) - gY;
                if(dy < 0.01) continue;
                double spotR = sp.radius * sp.ior * 1.5;
                double intensity = std::min(1.0, 0.6 * (sp.ior - 1.0));
                int br = static_cast<int>(255 * std::min(1.0, intensity * 1.8));
                int bg = static_cast<int>(240 * std::min(1.0, intensity * 1.6));
                int bb = static_cast<int>(200 * std::min(1.0, intensity * 1.4));
                Color32 spot{br, bg, bb, 255};
                double spotY = gY + 0.001;
                drawCausticSpot(fb, vp,
                                sp.x, spotY, sp.z, spotR, spot);
            }
        }
    }

    void drawCausticSpot(FrameBuffer &fb,
                         const Matrix4DBase<double> &vp,
                         double cx, double cy, double cz,
                         double radius,
                         const Color32 &color){
        const int seg = 24;
        ScreenVertex ring[seg];
        bool ok[seg];
        for(int i = 0; i < seg; i++){
            double a = 2.0 * M_PI * i / seg;
            Point4D p{cx + radius * std::cos(a), cy,
                      cz + radius * std::sin(a), 1};
            ok[i] = projectWorldPoint(vp, p, g_renderW, g_renderH, ring[i]);
        }
        int minY = g_renderH, maxY = 0;
        for(int i = 0; i < seg; i++){
            if(!ok[i]) continue;
            int y = static_cast<int>(ring[i].y);
            minY = std::min(minY, y);
            maxY = std::max(maxY, y);
        }
        if(minY >= maxY) return;
        uint32_t c = PackBGRA(color);
        for(int py = minY; py <= maxY; py++){
            int left = g_renderW, right = 0;
            for(int i = 0; i < seg; i++){
                int j = (i + 1) % seg;
                if(!ok[i] || !ok[j]) continue;
                double y0 = ring[i].y, y1 = ring[j].y;
                if((y0 <= py && y1 >= py) || (y1 <= py && y0 >= py)){
                    double t = (y0 == y1) ? 0.0 :
                        static_cast<double>(py - y0) / (y1 - y0);
                    int x = static_cast<int>(ring[i].x + t * (ring[j].x - ring[i].x));
                    left = std::min(left, x);
                    right = std::max(right, x);
                }
            }
            if(left <= right){
                for(int px = left; px <= right; px++){
                    if(px >= 0 && px < g_renderW && py >= 0 && py < g_renderH)
                        fb.setPixelOverlay(px, py, c);
                }
            }
        }
    }

    void drawUi(Application &) override {
        ImGui::Begin("Settings");
        ImGui::Text("Caustic / Light Cone Demo");
        ImGui::Checkbox("Enable Light Cones", &m_enableCones);
        ImGui::Separator();
        ImGui::Text("Spheres: %d", static_cast<int>(m_spheres.size()));
        ImGui::End();
    }

    const char *name() const override { return "PBR Caustic / Light Cone"; }
    const char *group() const override { return "RayTracing"; }
};

}
