#include "Pipeline.hpp"
#include "Transform.hpp"
#include <gtest/gtest.h>
#include <cmath>

namespace{
Matrix4DBase<double> IdentityModel(){
    return SGE::Math::translation(0.0, 0.0, 0.0);
}
Matrix3DBase<double> IdentityNormal(){
    Matrix3DBase<double> m(1,3,3);
    for(int r = 0; r < 3; r++)
        for(int c = 0; c < 3; c++)
            m[0][r][c] = (r == c) ? 1.0 : 0.0;
    return m;
}
}

TEST(PipelineBackfaceTest, SignConvention){
    // Derived numerically end-to-end (lookAt + perspective + y-flip viewport):
    // an OUTWARD-facing (visible) triangle yields NEGATIVE screen-space area,
    // because lookAt facing +Z makes camera-right = world -X.
    // => area > 0 means back-facing; area <= 0 means front/degenerate.
    ScreenVertex a{}, b{}, c{};
    a.x = 0; a.y = 0;
    b.x = 4; b.y = 0;
    c.x = 0; c.y = 4;
    // area(a,b,c) = +16 -> back-facing
    EXPECT_TRUE(Pipeline::isBackFacing(a, b, c));
    // reversed winding: area = -16 -> front-facing
    EXPECT_FALSE(Pipeline::isBackFacing(a, c, b));
}

TEST(PipelineClipTest, FrustumFullyInsideKeepsOne){
    ScreenVertex tri[3]{};
    tri[0].x = -1;  tri[0].y = -1;   tri[0].z = -0.5f; tri[0].w = 2;
    tri[1].x = 1;   tri[1].y = -1;   tri[1].z = -0.5f; tri[1].w = 2;
    tri[2].x = 0;   tri[2].y = 1.5;  tri[2].z = -0.5f; tri[2].w = 2;
    auto out = Pipeline::clipTriangle(tri);
    ASSERT_EQ(out.size(), 1u);
}

TEST(PipelineClipTest, FrustumNearBehindDropped){
    ScreenVertex tri[3]{};
    for(int i = 0; i < 3; i++){
        tri[i].z = -2.0f; tri[i].w = 0.5f;
    }
    auto out = Pipeline::clipTriangle(tri);
    EXPECT_EQ(out.size(), 0u);
}

TEST(PipelineClipTest, FrustumOneVertexNearSplits){
    ScreenVertex tri[3]{};
    tri[0].x = -0.7; tri[0].y = -1;  tri[0].z = -0.5f; tri[0].w = 2;
    tri[1].x = 0.7;  tri[1].y = -1;  tri[1].z = -0.5f; tri[1].w = 2;
    tri[2].x = 0;    tri[2].y = 2;   tri[2].z = -0.5f; tri[2].w = -2;
    auto out = Pipeline::clipTriangle(tri);
    ASSERT_EQ(out.size(), 2u);
}

TEST(PipelineClipTest, FrustumLeftPlaneClips){
    ScreenVertex tri[3]{};
    tri[0].x = -3;  tri[0].y = 0;    tri[0].z = -0.5f; tri[0].w = 1;
    tri[1].x = 1;   tri[1].y = -1.5; tri[1].z = -0.5f; tri[1].w = 2;
    tri[2].x = 1;   tri[2].y = 1.5;  tri[2].z = -0.5f; tri[2].w = 2;
    auto out = Pipeline::clipTriangle(tri);
    ASSERT_EQ(out.size(), 2u);
}

TEST(PipelineClipTest, FrustumBottomPlaneClips){
    ScreenVertex tri[3]{};
    tri[0].x = 0;    tri[0].y = -3;  tri[0].z = -0.5f; tri[0].w = 1;
    tri[1].x = 1.5;  tri[1].y = 1;   tri[1].z = -0.5f; tri[1].w = 2;
    tri[2].x = 1.5;  tri[2].y = 1.5; tri[2].z = -0.5f; tri[2].w = 2;
    auto out = Pipeline::clipTriangle(tri);
    ASSERT_EQ(out.size(), 2u);
}

TEST(PipelineClipTest, FrustumTopPlaneClips){
    ScreenVertex tri[3]{};
    tri[0].x = 0;    tri[0].y = 3;    tri[0].z = -0.5f; tri[0].w = 1;
    tri[1].x = 1.5;  tri[1].y = -1;   tri[1].z = -0.5f; tri[1].w = 2;
    tri[2].x = 1.5;  tri[2].y = -1.5; tri[2].z = -0.5f; tri[2].w = 2;
    auto out = Pipeline::clipTriangle(tri);
    ASSERT_EQ(out.size(), 2u);
}

TEST(PipelineClipTest, FrustumCrossCornerSplits){
    ScreenVertex tri[3]{};
    tri[0].x = -2;  tri[0].y = -0.5; tri[0].z = -2; tri[0].w = 1;
    tri[1].x = 1;   tri[1].y = -1.5; tri[1].z = 1;  tri[1].w = 2;
    tri[2].x = 1;   tri[2].y = 1.5;  tri[2].z = 1;  tri[2].w = 2;

    auto out = Pipeline::clipTriangle(tri);
    ASSERT_FALSE(out.empty());

    auto inAll = [](const ScreenVertex &v){
        const double d[6] = {v.z+v.w, v.w-v.z, v.x+v.w, v.w-v.x, v.y+v.w, v.w-v.y};
        for(int i = 0; i < 6; i++){
            if(d[i] < -1e-9) return false;
        }
        return true;
    };
    std::size_t count = 0;
    for(auto &t : out){
        for(int i = 0; i < 3; i++){
            EXPECT_TRUE(inAll(t.v[i]));
            count++;
        }
    }
    EXPECT_LE(count, 9u);
}

TEST(PipelineClipTest, FrustumFarPlaneCulls){
    ScreenVertex tri[3]{};
    for(int i = 0; i < 3; i++){
        tri[i].z = 3.0f; tri[i].w = 1.0f;
    }
    auto out = Pipeline::clipTriangle(tri);
    EXPECT_EQ(out.size(), 0u);
}

TEST(PipelineProjectTest, CubeFrontFaceProjectsCentered){
    Object4D cube{};
    cube.numVertices = 8;
    double s = 1.0;
    Point4D verts[8] = {{-s,-s,-s,1},{s,-s,-s,1},{s,s,-s,1},{-s,s,-s,1},
                        {-s,-s, s,1},{s,-s, s,1},{s,s, s,1},{-s,s, s,1}};
    for(int i = 0; i < 8; i++) cube.vlistLocal[i] = verts[i];
    cube.numPolys = 1;
    cube.plist[0].vlist[0] = verts[0];
    cube.plist[0].vlist[1] = verts[3];
    cube.plist[0].vlist[2] = verts[2];
    cube.plist[0].color = Color32{255, 0, 255, 255};

    Matrix4DBase<double> view = SGE::Math::lookAt(
        Vector3DBase<double>{0, 2, -6}, Vector3DBase<double>{0, 0, 0}, Vector3DBase<double>{0, 1, 0});
    Matrix4DBase<double> proj = SGE::Math::perspective(M_PI/3, 800.0/600.0, 0.1, 100.0);
    Matrix4DBase<double> model = SGE::Math::translation(0.0, 0.0, 0.0);
    auto mvp = proj.mul(view).mul(model);

    auto tris = Pipeline::projectObject(cube, IdentityModel(), mvp, IdentityNormal(), 800, 600);
    ASSERT_GE(tris.size(), 0u);
    if(!tris.empty()){
        auto cx = (tris[0].v[0].x + tris[0].v[1].x + tris[0].v[2].x)/3.0;
        auto cy = (tris[0].v[0].y + tris[0].v[1].y + tris[0].v[2].y)/3.0;
        EXPECT_NEAR(cx, 400, 60);
        EXPECT_NEAR(cy, 300, 60);
        for(auto &t : tris){
            for(int i = 0; i < 3; i++){
                EXPECT_GT(t.v[i].w, 0.0f);
            }
        }
    }
}

TEST(PipelineProjectTest, BehindCameraProducesNothing){
    Object4D obj{};
    obj.numVertices = 3;
    Point4D v[3] = {{-1,-1,5,1},{1,-1,5,1},{0,1,5,1}};   // behind camera (positive Z)
    for(int i = 0; i < 3; i++) obj.vlistLocal[i] = v[i];
    obj.numPolys = 1;
    obj.plist[0].vlist[0] = v[0];
    obj.plist[0].vlist[1] = v[1];
    obj.plist[0].vlist[2] = v[2];

    Matrix4DBase<double> view = SGE::Math::lookAt(
        Vector3DBase<double>{0, 0, 0}, Vector3DBase<double>{0, 0, -1}, Vector3DBase<double>{0, 1, 0});
    Matrix4DBase<double> proj = SGE::Math::perspective(M_PI/3, 1.0, 0.1, 100.0);
    auto mvp = proj.mul(view);
    auto tris = Pipeline::projectObject(obj, IdentityModel(), mvp, IdentityNormal(), 800, 600);
    EXPECT_EQ(tris.size(), 0u);
}

TEST(PipelineProjectTest, UvPassthrough){
    Object4D obj{};
    obj.numVertices = 4;
    Point4D v[4] = {{-1,-1,-1,1},{1,-1,-1,1},{1,1,-1,1},{-1,1,-1,1}};
    for(int i = 0; i < 4; i++) obj.vlistLocal[i] = v[i];
    obj.numPolys = 1;
    obj.plist[0].vlist[0] = v[0];
    obj.plist[0].vlist[1] = v[3];
    obj.plist[0].vlist[2] = v[2];
    obj.plist[0].uvlist[0] = {0.0, 0.0};
    obj.plist[0].uvlist[1] = {0.25, 0.5};
    obj.plist[0].uvlist[2] = {1.0, 1.0};

    Matrix4DBase<double> view = SGE::Math::lookAt(
        Vector3DBase<double>{0, 0, -5}, Vector3DBase<double>{0, 0, 0},
        Vector3DBase<double>{0, 1, 0});
    Matrix4DBase<double> proj = SGE::Math::perspective(M_PI/3, 800.0/600.0, 0.1, 100.0);
    auto mvp = proj.mul(view);

    auto tris = Pipeline::projectObject(obj, IdentityModel(), mvp, IdentityNormal(), 800, 600);
    ASSERT_EQ(tris.size(), 1u);
    EXPECT_FLOAT_EQ(tris[0].v[0].u, 0.0f);
    EXPECT_FLOAT_EQ(tris[0].v[0].v, 0.0f);
    EXPECT_FLOAT_EQ(tris[0].v[1].u, 0.25f);
    EXPECT_FLOAT_EQ(tris[0].v[1].v, 0.5f);
    EXPECT_FLOAT_EQ(tris[0].v[2].u, 1.0f);
    EXPECT_FLOAT_EQ(tris[0].v[2].v, 1.0f);
}

TEST(PipelineProjectTest, NormalWorldPassthrough){
    Object4D cube{};
    cube.numVertices = 4;
    Point4D v[4] = {{-1,-1,-1,1},{1,-1,-1,1},{1,1,-1,1},{-1,1,-1,1}};
    for(int i = 0; i < 4; i++) cube.vlistLocal[i] = v[i];
    cube.numPolys = 1;
    cube.plist[0].vlist[0] = v[0];
    cube.plist[0].vlist[1] = v[3];
    cube.plist[0].vlist[2] = v[2];
    cube.plist[0].nlist[0] = Vector3DBase<double>{0, 0, -1};
    cube.plist[0].nlist[1] = Vector3DBase<double>{0, 0, -1};
    cube.plist[0].nlist[2] = Vector3DBase<double>{0, 0, -1};

    Matrix4DBase<double> view = SGE::Math::lookAt(
        Vector3DBase<double>{0, 0, -5}, Vector3DBase<double>{0, 0, 0},
        Vector3DBase<double>{0, 1, 0});
    Matrix4DBase<double> proj = SGE::Math::perspective(M_PI/3, 800.0/600.0, 0.1, 100.0);
    auto mvp = proj.mul(view);

    auto tris = Pipeline::projectObject(cube, IdentityModel(), mvp, IdentityNormal(), 800, 600);
    ASSERT_EQ(tris.size(), 1u);
    for(int i = 0; i < 3; i++){
        EXPECT_DOUBLE_EQ(tris[0].v[i].nx, 0.0);
        EXPECT_DOUBLE_EQ(tris[0].v[i].ny, 0.0);
        EXPECT_DOUBLE_EQ(tris[0].v[i].nz, -1.0);
    }
    EXPECT_DOUBLE_EQ(tris[0].v[0].wx, -1.0);
    EXPECT_DOUBLE_EQ(tris[0].v[0].wy, -1.0);
    EXPECT_DOUBLE_EQ(tris[0].v[0].wz, -1.0);
    EXPECT_DOUBLE_EQ(tris[0].v[2].wx, 1.0);
    EXPECT_DOUBLE_EQ(tris[0].v[2].wy, 1.0);
}

TEST(PipelineClipTest, FrustumClipInterpolatesAttributes){
    ScreenVertex tri[3]{};
    tri[0].x = -0.7; tri[0].y = -1; tri[0].z = -0.5f; tri[0].w = 2;
    tri[0].u = 0; tri[0].v = 0;
    tri[0].nx = 0; tri[0].ny = 0; tri[0].nz = -1;
    tri[0].wx = 1; tri[0].wy = 2; tri[0].wz = 3;
    tri[1].x = 0.7; tri[1].y = -1; tri[1].z = -0.5f; tri[1].w = 2;
    tri[1].u = 1; tri[1].v = 0;
    tri[1].nx = 1; tri[1].ny = 1; tri[1].nz = 1;
    tri[1].wx = 5; tri[1].wy = 6; tri[1].wz = 7;
    tri[2].x = 0; tri[2].y = 2; tri[2].z = -0.5f; tri[2].w = -2;
    tri[2].u = 0; tri[2].v = 1;
    tri[2].nx = 0; tri[2].ny = 1; tri[2].nz = 0;
    tri[2].wx = 9; tri[2].wy = 10; tri[2].wz = 11;

    auto out = Pipeline::clipTriangle(tri);
    ASSERT_EQ(out.size(), 2u);

    int foundA = 0, foundB = 0;
    for(auto &t : out){
        for(int i = 0; i < 3; i++){
            const ScreenVertex &v = t.v[i];
            if(std::fabs(v.u - 0.625) < 1e-9 && std::fabs(v.v - 0.375) < 1e-9 &&
               std::fabs(v.nx - 0.625) < 1e-9 && std::fabs(v.ny - 1.0) < 1e-9 &&
               std::fabs(v.nz - 0.625) < 1e-9 &&
               std::fabs(v.wx - 6.5) < 1e-9 && std::fabs(v.wy - 7.5) < 1e-9 &&
               std::fabs(v.wz - 8.5) < 1e-9) foundA++;
            if(std::fabs(v.u) < 1e-9 && std::fabs(v.v - 0.375) < 1e-9 &&
               std::fabs(v.nx) < 1e-9 && std::fabs(v.ny - 0.375) < 1e-9 &&
               std::fabs(v.nz + 0.625) < 1e-9 &&
               std::fabs(v.wx - 4.0) < 1e-9 && std::fabs(v.wy - 5.0) < 1e-9 &&
               std::fabs(v.wz - 6.0) < 1e-9) foundB++;
        }
    }
    EXPECT_EQ(foundA, 2);
    EXPECT_EQ(foundB, 1);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
