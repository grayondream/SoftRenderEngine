#pragma once
#include "FrameBuffer.hpp"
#include "Light.hpp"
#include "Camera.hpp"
#include <vector>

namespace SGE::Render{

struct RaySphere{
    Vector3DBase<double> center{};
    double radius{1.0};
    Color32 albedo{255, 255, 255, 255};
    float reflectivity{0.0f};
    float refractivity{0.0f};
    double ior{1.5};
};

struct RayTriangle{
    Vector3DBase<double> a{}, b{}, c{};
    Color32 albedo{255, 255, 255, 255};
    float reflectivity{0.0f};
};

struct SpotCone{
    Vector3DBase<double> position{};
    Vector3DBase<double> direction{};
    double cutoffCos{0.85};
    double range{30.0};
    float intensity{1.0f};
    bool enabled{false};
};

struct RayScene{
    std::vector<RaySphere> spheres{};
    std::vector<RayTriangle> triangles{};
    SpotCone cone{};
    Vector3DBase<double> bboxMin{}, bboxMax{};
    bool hasBBox{false};
};

struct RayTraceOptions{
    int maxDepth{3};
    Color32 background{};
    int renderScale{1};
};

namespace RayDetail{
struct Hit{
    bool hit{false};
    double dist{0};
    Vector3DBase<double> point{};
    Vector3DBase<double> normal{};
    Color32 albedo{255, 255, 255, 255};
    float reflectivity{0.0f};
    float refractivity{0.0f};
    double ior{1.0};
};

bool raySphereIntersect(const RaySphere &s,
                        const Vector3DBase<double> &origin,
                        const Vector3DBase<double> &dir, double &outDist);

bool rayTriangleIntersect(const RayTriangle &tri,
                          const Vector3DBase<double> &origin,
                          const Vector3DBase<double> &dir, double &outDist);
}

class RayTracer{
public:
    explicit RayTracer(FrameBuffer &fb) : m_fb(fb){ }

    void render(const RayScene &scene, const Camera &camera,
                const LightingRig &rig, const RayTraceOptions &opt = {});

private:
    RayDetail::Hit traceRay(const RayScene &scene,
                            const Vector3DBase<double> &origin,
                            const Vector3DBase<double> &dir) const;
    bool occluded(const RayScene &scene,
                  const Vector3DBase<double> &origin,
                  const Vector3DBase<double> &dir, double maxDist) const;
    uint32_t shadeHit(const RayScene &scene, const LightingRig &rig,
                      const RayDetail::Hit &hit,
                      const Vector3DBase<double> &viewPos, int depth,
                      const RayTraceOptions &opt);
    static Vector3DBase<double> Add(const Vector3DBase<double> &a, const Vector3DBase<double> &b){
        return Vector3DBase<double>{a.x + b.x, a.y + b.y, a.z + b.z};
    }

    FrameBuffer &m_fb;
};

}
