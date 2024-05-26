#include "MatrixUtils.hpp"
#include <cstddef>
#include <string>

inline constexpr static const int32_t OUT_STREAM_MAX_ELEMENT   = 7;
inline constexpr static const char* OUT_STREAM_RIGHT_SEP1    = " ]";
inline constexpr static const char* OUT_STREAM_LEFT_SEP1     = "[ ";
inline constexpr static const char* OUT_STREAM_RIGHT_SEP2    = " }";
inline constexpr static const char* OUT_STREAM_LEFT_SEP2     = "{ ";
inline constexpr static const char* OUT_STREAM_SPACE_CHAR    = " ";
inline constexpr static const char* OUT_STREAM_TAB_CHAR      = "\t";
inline constexpr static const char* OUT_STREAM_ENTER_CHAR    = "\n";
inline constexpr static const char* OUT_STREAM_LOOP_CHAR     = "...";
inline constexpr static const char* OUT_STREAM_COMET_CHAR    = ", ";
using std::string;

inline static std::size_t MatrixStringLoopSize(const std::size_t size){
    return size > OUT_STREAM_MAX_ELEMENT ? OUT_STREAM_MAX_ELEMENT - 1 : size - 1;
}

namespace std{
std::string to_string(const Matrix1D &mat){
    std::string ret{ OUT_STREAM_LEFT_SEP1 };
    const auto size = mat.m_d1;
    assert(size >= 0);
    const auto outsz = MatrixStringLoopSize(size);
    for(std::size_t i = 0;i < outsz;i ++){
        ret += std::to_string(mat[i]) + OUT_STREAM_COMET_CHAR;
    }
    
    if(size > OUT_STREAM_MAX_ELEMENT){
        ret += std::string(OUT_STREAM_LOOP_CHAR) + OUT_STREAM_COMET_CHAR;
    }
    return ret + std::to_string(mat[size - 1]) + OUT_STREAM_RIGHT_SEP1;
}

std::string to_string(const Matrix2D &mat){
    std::string ret { OUT_STREAM_LEFT_SEP2 };
    const auto size = mat.m_d2;
    assert(size >= 0);
    const auto outsize = MatrixStringLoopSize(size);
    ret += OUT_STREAM_ENTER_CHAR;
    for(std::size_t i = 0;i < outsize;i ++){
        ret += OUT_STREAM_TAB_CHAR + std::to_string(mat[i]) + OUT_STREAM_COMET_CHAR + OUT_STREAM_ENTER_CHAR;
    }
    
    if(size > OUT_STREAM_MAX_ELEMENT){
        ret += std::string(OUT_STREAM_TAB_CHAR) + OUT_STREAM_LOOP_CHAR + OUT_STREAM_ENTER_CHAR;
    }
    return ret + OUT_STREAM_TAB_CHAR + std::to_string(mat[size - 1]) + OUT_STREAM_ENTER_CHAR + OUT_STREAM_RIGHT_SEP2;
}

std::string to_string(const Matrix3D &mat){
    return {};
}

std::string to_string(const Matrix4D &mat){
    return {};
}

}

