#include "ObjLoader.hpp"
#include "Rasterizer.hpp"
#include "Pipeline.hpp"
#include "Transform.hpp"
#include "Texture.hpp"
#include <gtest/gtest.h>
#include <cstdio>
#include <cmath>

namespace{
std::string TmpPath(const char *name){
    return std::string("/tmp/opencode/") + name;
}

bool WriteFile(const std::string &path, const std::string &content){
    FILE *f = std::fopen(path.c_str(), "wb");
    if(!f) return false;
    std::fwrite(content.data(), 1, content.size(), f);
    std::fclose(f);
    return true;
}
}

TEST(ObjTest, VerticesAndTriangleFaces){
    ASSERT_TRUE(WriteFile(TmpPath("tri.obj"),
        "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n"));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("tri.obj"), obj));
    EXPECT_EQ(obj.numVertices, 3);
    EXPECT_EQ(obj.numPolys, 1);
    EXPECT_DOUBLE_EQ(obj.plist[0].vlist[0].x, 0.0);
    EXPECT_DOUBLE_EQ(obj.plist[0].vlist[1].x, 1.0);
    EXPECT_DOUBLE_EQ(obj.plist[0].vlist[2].y, 1.0);
    EXPECT_STREQ(obj.name, "obj");
}

TEST(ObjTest, QuadFaceFanTriangulated){
    ASSERT_TRUE(WriteFile(TmpPath("quad.obj"),
        "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nf 1 2 3 4\n"));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("quad.obj"), obj));
    EXPECT_EQ(obj.numVertices, 4);
    EXPECT_EQ(obj.numPolys, 2);
    EXPECT_EQ(obj.plist[0].vlist[0].x, 0.0);
    EXPECT_EQ(obj.plist[0].vlist[1].x, 1.0);
    EXPECT_EQ(obj.plist[1].vlist[1].y, 1.0);
    EXPECT_EQ(obj.plist[1].vlist[2].x, 0.0);
}

TEST(ObjTest, SlashFormats){
    ASSERT_TRUE(WriteFile(TmpPath("slash.obj"),
        "v 0 0 0\nv 1 0 0\nv 0 1 0\n"
        "vt 0.25 0.5\n"
        "vn 0 0 1\n"
        "f 1/1 2/1 3/1\n"
        "f 1//1 2//1 3//1\n"
        "f 1/1/1 2/1/1 3/1/1\n"));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("slash.obj"), obj));
    EXPECT_EQ(obj.numPolys, 3);
    EXPECT_NEAR(obj.plist[0].uvlist[0].u, 0.25, 1e-12);
    EXPECT_NEAR(obj.plist[0].uvlist[1].v, 0.5, 1e-12);
    EXPECT_DOUBLE_EQ(obj.plist[1].nlist[0].z, 1.0);
    EXPECT_NEAR(obj.plist[2].uvlist[2].u, 0.25, 1e-12);
    EXPECT_DOUBLE_EQ(obj.plist[2].nlist[2].z, 1.0);
}

TEST(ObjTest, NegativeIndices){
    ASSERT_TRUE(WriteFile(TmpPath("neg.obj"),
        "v 0 0 0\nv 1 0 0\nv 0 1 0\nf -3 -2 -1\n"));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("neg.obj"), obj));
    EXPECT_EQ(obj.numPolys, 1);
    EXPECT_DOUBLE_EQ(obj.plist[0].vlist[2].y, 1.0);
}

TEST(ObjTest, MissingNormalsFlatComputed){
    ASSERT_TRUE(WriteFile(TmpPath("nonorm.obj"),
        "v 0 0 0\nv 1 0 0\nv 0 0 -1\nf 1 2 3\n"));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("nonorm.obj"), obj));
    const auto &n = obj.plist[0].nlist[0];
    EXPECT_NEAR(n.x, 0.0, 1e-12);
    EXPECT_NEAR(n.y, 1.0, 1e-12);
    EXPECT_NEAR(n.z, 0.0, 1e-12);
    EXPECT_TRUE(n == obj.plist[0].nlist[1]);
    EXPECT_TRUE(n == obj.plist[0].nlist[2]);
}

TEST(ObjTest, MissingUvsZeroed){
    ASSERT_TRUE(WriteFile(TmpPath("nouvs.obj"), "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n"));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("nouvs.obj"), obj));
    EXPECT_DOUBLE_EQ(obj.plist[0].uvlist[0].u, 0.0);
    EXPECT_DOUBLE_EQ(obj.plist[0].uvlist[2].v, 0.0);
}

TEST(ObjTest, CapacityExceededFails){
    std::string content;
    for(int i = 0; i < kObject4vListLen + 1; i++){
        content += "v 0 0 0\n";
    }
    ASSERT_TRUE(WriteFile(TmpPath("cap.obj"), content));
    Object4D obj{};
    EXPECT_FALSE(loadObjFromFile(TmpPath("cap.obj"), obj));
    EXPECT_EQ(obj.numVertices, 0);
    EXPECT_EQ(obj.numPolys, 0);
}

TEST(ObjTest, BadTokenFails){
    ASSERT_TRUE(WriteFile(TmpPath("bad.obj"), "v x 0 0\n"));
    Object4D obj{};
    EXPECT_FALSE(loadObjFromFile(TmpPath("bad.obj"), obj));
}

TEST(ObjTest, MissingFileFails){
    Object4D obj{};
    EXPECT_FALSE(loadObjFromFile("/tmp/opencode/no_such_12345.obj", obj));
}

TEST(ObjTest, BomFileParsesCorrectly){
    std::string content = "v 10 0 0\nv 0 0 0\nv 0 1 0\nf 1 2 3\n";
    content.insert(content.begin(), {static_cast<char>(0xEF), static_cast<char>(0xBB), static_cast<char>(0xBF)});
    ASSERT_TRUE(WriteFile(TmpPath("bom.obj"), content));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("bom.obj"), obj));
    EXPECT_EQ(obj.numVertices, 3);
    EXPECT_DOUBLE_EQ(obj.plist[0].vlist[0].x, 10.0);
}

TEST(ObjTest, HalfFilledClearedOnFailure){
    ASSERT_TRUE(WriteFile(TmpPath("half.obj"),
        "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\nv garbage\n"));
    Object4D obj{};
    EXPECT_FALSE(loadObjFromFile(TmpPath("half.obj"), obj));
    EXPECT_EQ(obj.numPolys, 0);
}

TEST(ObjTest, RealCubeAsset){
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile("assets/cube.obj", obj));
    EXPECT_EQ(obj.numVertices, 8);
    EXPECT_EQ(obj.numPolys, 12);
    EXPECT_DOUBLE_EQ(obj.plist[0].nlist[0].z, -1.0);
    EXPECT_DOUBLE_EQ(obj.plist[1].nlist[0].z, -1.0);
}

TEST(ObjTest, EndToEndRendersPixels){
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile("assets/cube.obj", obj));

    auto model = SGE::Math::rotationY(0.6).mul(SGE::Math::rotationX(0.4));
    auto view = SGE::Math::lookAt(Vector3DBase<double>{0, 2, -6},
                                  Vector3DBase<double>{0, 0, 0},
                                  Vector3DBase<double>{0, 1, 0});
    auto proj = SGE::Math::perspective(M_PI/3, 800.0/600.0, 0.1, 100.0);
    auto vp = proj.mul(view);
    auto nrm = SGE::Math::normalMatrix(model);

    FrameBuffer fb(800, 600);
    Rasterizer rz{fb};
    uint32_t px[1] = {0xFFFFFFFFu};
    Texture tex(1, 1, px);
    for(auto &t : Pipeline::projectObject(obj, model, vp, nrm, 800, 600)){
        rz.drawTriangleTextured(t.v[0], t.v[1], t.v[2], tex);
    }

    std::size_t lit = 0;
    for(std::size_t i = 0; i < fb.width() * fb.height(); i++){
        if(fb.colorData()[i] != 0xFF000000u) lit++;
    }
    EXPECT_GT(lit, 1000u);
}

TEST(ObjTest, UvCapacityLimitFailsClosed){
    std::string content;
    for(int i = 0; i < 4097; i++){
        content += "vt 0.5 0.5\n";
    }
    content += "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1/1 2/2 3/3\n";
    ASSERT_TRUE(WriteFile(TmpPath("uv_overflow.obj"), content));
    Object4D obj{};
    EXPECT_FALSE(loadObjFromFile(TmpPath("uv_overflow.obj"), obj));
}

TEST(ObjTest, NormalCapacityLimitFailsClosed){
    std::string content;
    for(int i = 0; i < 4097; i++){
        content += "vn 0 0 1\n";
    }
    content += "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1//1 2//2 3//3\n";
    ASSERT_TRUE(WriteFile(TmpPath("n_overflow.obj"), content));
    Object4D obj{};
    EXPECT_FALSE(loadObjFromFile(TmpPath("n_overflow.obj"), obj));
}

TEST(ObjTest, DegenerateFaceYieldsZeroNormalButLoads){
    ASSERT_TRUE(WriteFile(TmpPath("degenerate.obj"),
        "v 0 0 0\nv 1 0 0\nv 2 0 0\nf 1 2 3\n"));
    Object4D obj{};
    ASSERT_TRUE(loadObjFromFile(TmpPath("degenerate.obj"), obj));
    EXPECT_EQ(obj.numPolys, 1);
    const auto &n = obj.plist[0].nlist[0];
    EXPECT_DOUBLE_EQ(n.x, 0.0);
    EXPECT_DOUBLE_EQ(n.y, 0.0);
    EXPECT_DOUBLE_EQ(n.z, 0.0);
}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
