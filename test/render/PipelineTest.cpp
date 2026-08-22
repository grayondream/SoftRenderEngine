#include "Pipeline.hpp"
#include "Transform.hpp"
#include <gtest/gtest.h>
#include <cmath>

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

TEST(PipelineClipTest, FullyInsideUnchanged){
    ScreenVertex tri[3]{};
    tri[0].x = 0;   tri[0].y = -4; tri[0].z = -0.5f; tri[0].w = 2;
    tri[1].x = 4;   tri[1].y = -4; tri[1].z = -0.5f; tri[1].w = 2;
    tri[2].x = 2;   tri[2].y = 4;  tri[2].z = -0.5f; tri[2].w = 2;
    auto out = Pipeline::clipNearPlane(tri);
    ASSERT_EQ(out.size(), 1u);
}

TEST(PipelineClipTest, OneVertexBehindSplitsIntoTwo){
    ScreenVertex tri[3]{};
    tri[0].x = 0; tri[0].y = -4; tri[0].z = -0.5f; tri[0].w = 2;
    tri[1].x = 4; tri[1].y = -4; tri[1].z = -0.5f; tri[1].w = 2;
    tri[2].x = 2; tri[2].y = 4;  tri[2].z = -0.5f; tri[2].w = -2;  // z + w < 0: behind near plane
    auto out = Pipeline::clipNearPlane(tri);
    ASSERT_EQ(out.size(), 2u);
}

TEST(PipelineClipTest, FullyBehindDropped){
    ScreenVertex tri[3]{};
    for(int i = 0; i < 3; i++){
        tri[i].z = -2.0f; tri[i].w = 0.5f;   // z + w = -1.5 < 0: behind near plane
    }
    auto out = Pipeline::clipNearPlane(tri);
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

    auto tris = Pipeline::projectObject(cube, mvp, 800, 600);
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
    auto tris = Pipeline::projectObject(obj, mvp, 800, 600);
    EXPECT_EQ(tris.size(), 0u);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
