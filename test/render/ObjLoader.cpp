#include "ObjLoader.hpp"
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

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
