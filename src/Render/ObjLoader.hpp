#pragma once
#include <string>
#include <vector>
#include "GeoObject/Object4D.hpp"

bool loadObjFromFile(const std::string &path, Object4D &out);

struct ObjMaterialInfo {
    std::vector<std::string> names;      // material order of appearance
    std::vector<std::string> diffuseMap; // map_Kd filename per material ("" if none)
    std::string mtlDir;                  // directory containing mtl & maps
};

// Loads an OBJ into multiple Object4D chunks (each within static capacity).
// faceMaterial gets one material index per output polygon (global order:
// chunk by chunk). Materials come from usemtl / mtllib parsing.
bool loadObjMultiMaterial(const std::string &path,
                          std::vector<Object4D> &chunks,
                          std::vector<int> &faceMaterial,
                          ObjMaterialInfo &materials);
