#pragma once

#include "../SceneUtil.hpp"
#include "Render/ImageLoader.hpp"

#include <cmath>
#include <array>

namespace SGE::Samples {

class SkyboxScene final : public IScene {
public:
    void setup(Application &app) override {
        resetCamera(app, 0.0, 0.0, 3.0);
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
    }
    void render(Application &app) override {
        auto &fb = app.framebuffer();
        fb.clear(kRefClear);
        auto &cam = app.camera();
        // sample 6-face cubemap per pixel from view ray
        const Vector3DBase<double> fwd = cam.forward().normalize();
        const Vector3DBase<double> right = cam.right().normalize();
        const Vector3DBase<double> up{right.y * fwd.z - right.z * fwd.y,
            right.z * fwd.x - right.x * fwd.z,
            right.x * fwd.y - right.y * fwd.x};
        up.normalize();
        const double aspect = 800.0 / 600.0;
        const double tanH = std::tan(M_PI / 6);
        const double tanV = tanH / aspect;
        for(std::size_t y2 = 0; y2 < 600; y2++){
            const double ny = -(2.0 * (static_cast<double>(y2) + 0.5)
                / 600.0 - 1.0);
            for(std::size_t x2 = 0; x2 < 800; x2++){
                const double nx = 2.0 * (static_cast<double>(x2) + 0.5)
                    / 800.0 - 1.0;
                Vector3DBase<double> dir{
                    fwd.x + right.x * nx * tanH + up.x * ny * tanV,
                    fwd.y + right.y * nx * tanH + up.y * ny * tanV,
                    fwd.z + right.z * nx * tanH + up.z * ny * tanV};
                dir = dir.normalize();
                fb.setPixel(x2, y2, sampleCube(dir), -2.0f);
            }
        }
        // center dog cube (reference: RotY(45))
        Rasterizer rz{fb};
        static Texture dog = SGE::Render::ImageLoader::loadTexture(
            "assets/textures/dog.jpg");
        Object4D cube = app.cube();
        auto cm = SGE::Math::translation(0.0, 0.0, -4.0 + 4.0)
            .mul(SGE::Math::rotationY(M_PI / 4));
        auto cnrm = SGE::Math::normalMatrix(cm);
        auto ct = Pipeline::projectObject(cube, cm,
            refViewProj(cam), cnrm, 800, 600);
        for(auto &t : ct){
            rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2],
                                    dog, nullptr, TextureFilter::Bilinear,
                                    TextureWrap::Clamp);
        }
    }
    void drawUi(Application &) override {
        ImGui::Text("6-face skybox + dog cube");
    }
    const char *name() const override { return "Skybox (6-face cubemap)"; }
    const char *group() const override { return "Advanced"; }
private:
    Texture m_faces[6]{};
    uint32_t sampleCube(const Vector3DBase<double> &d){
        const double ax = std::abs(d.x), ay = std::abs(d.y),
                     az = std::abs(d.z);
        int face; double u, v;
        if(ax >= ay && ax >= az){
            face = d.x > 0 ? 0 : 1;
            u = -d.z / ax; v = -d.y / ax;
            if(d.x < 0){ u = -u; }
        }else if(ay >= az){
            face = d.y > 0 ? 2 : 3;
            u = d.x / ay; v = d.z / ay;
        }else{
            face = d.z > 0 ? 4 : 5;
            u = d.x / az; v = -d.y / az;
            if(d.z < 0){ u = -u; }
        }
        u = std::clamp(u * 0.5 + 0.5, 0.0, 0.99999);
        v = std::clamp(v * 0.5 + 0.5, 0.0, 0.99999);
        return m_faces[face].sample(u, v, TextureFilter::Bilinear,
                                    TextureWrap::Clamp);
    }
};

inline uint32_t SkyboxScene_dummy(){ return 0; }

}
