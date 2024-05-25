#include "MatrixUtils.hpp"
#include <string>

inline constexpr static const int32_t OUT_STREAM_MAX_ELEMENT   = 10;
inline constexpr static const char* OUT_STREAM_RIGHT_SEP1    = " ]";
inline constexpr static const char* OUT_STREAM_LEFT_SEP1     = "[ ";
inline constexpr static const char* OUT_STREAM_RIGHT_SEP2    = " }";
inline constexpr static const char* OUT_STREAM_LEFT_SEP2     = "{ ";
inline constexpr static const char* OUT_STREAM_SPACE_CHAR    = " ";

using std::string;

namespace std{
std::string to_string(const Matrix1D &mat){
    std::string ret{ OUT_STREAM_LEFT_SEP1 };
    const auto size = mat.m_d1;
    assert(size >= 0);
    const auto outsz = size > OUT_STREAM_MAX_ELEMENT ? OUT_STREAM_MAX_ELEMENT - 1 : size - 1;
    for(std::size_t i = 0;i < outsz;i ++){
        ret += std::to_string(mat[i]) + ", ";
    }
    
    if(size > OUT_STREAM_MAX_ELEMENT){
        ret += "..., ";
    }
    return ret + std::to_string(mat[size - 1]) + OUT_STREAM_RIGHT_SEP1;
}

std::string to_string(const Matrix2D &mat){
    return {};
}

std::string to_string(const Matrix3D &mat){
    return {};
}

std::string to_string(const Matrix4D &mat){
    return {};
}

}

