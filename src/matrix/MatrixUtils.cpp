#include "MatrixUtils.hpp"
#include "Matrix.hpp"
#include <cstddef>
#include <locale>
#include <string>
#include <type_traits>

inline constexpr static const int32_t OUT_STREAM_MAX_ELEMENT   = 7;
inline constexpr static const char* OUT_STREAM_RIGHT_SEP1    = " ]";
inline constexpr static const char* OUT_STREAM_LEFT_SEP1     = "[ ";
inline constexpr static const char* OUT_STREAM_RIGHT_SEP2    = "}";
inline constexpr static const char* OUT_STREAM_LEFT_SEP2     = "{";
inline constexpr static const char* OUT_STREAM_SPACE_CHAR    = " ";
inline constexpr static const char* OUT_STREAM_TAB_CHAR      = "  ";
inline constexpr static const char* OUT_STREAM_ENTER_CHAR    = "\n";
inline constexpr static const char* OUT_STREAM_LOOP_CHAR     = "...";
inline constexpr static const char* OUT_STREAM_COMET_CHAR    = ", ";
using std::string;

inline static std::size_t MatrixStringLoopSize(const std::size_t size){
    return size > OUT_STREAM_MAX_ELEMENT ? OUT_STREAM_MAX_ELEMENT - 1 : size - 1;
}

template<class T>
std::size_t MatrixGetSize(const T &mat){
    if constexpr(std::is_same_v<T, Matrix1D>){
        return mat.d1;
    }else if constexpr(std::is_same_v<T, Matrix2D>){
        return mat.d2;
    }else if constexpr(std::is_same_v<T, Matrix3D>){
        return mat.d3;
    }else if constexpr(std::is_same_v<T, Matrix4D>){
        return mat.d4;
    }

    return 0;
}

template<class T>
std::string MatrixGetTab(const T &mat){
    if constexpr(std::is_same_v<T, Matrix1D>){
        return {};
    }else if constexpr(std::is_same_v<T, Matrix2D>){
        return std::string(OUT_STREAM_TAB_CHAR);
    }else if constexpr(std::is_same_v<T, Matrix3D>){
        return std::string(OUT_STREAM_TAB_CHAR);
    }else if constexpr(std::is_same_v<T, Matrix4D>){
        return std::string(OUT_STREAM_TAB_CHAR);
    }

    return {};
}

template<class T>
inline static std::string MatrixSplice2String(const T &mat, const std::string valueSep){
    std::string ret { OUT_STREAM_LEFT_SEP2 };
    const auto size = MatrixGetSize(mat);
    const auto outsize = MatrixStringLoopSize(size);
    auto enter = "";
    if(valueSep.find(OUT_STREAM_ENTER_CHAR) != std::string::npos){
        enter = OUT_STREAM_ENTER_CHAR;
    }
    
    ret += enter;
    
    for(std::size_t i = 0;i < outsize;i ++){
        ret += std::to_string(mat[i]) + valueSep;
    }
    
    if(size > OUT_STREAM_MAX_ELEMENT){
        ret += std::string(OUT_STREAM_LOOP_CHAR) + OUT_STREAM_COMET_CHAR + enter;
    }

    return ret + std::to_string(mat[size - 1]) + enter + OUT_STREAM_RIGHT_SEP2;
}

namespace std{
std::string to_string(const Matrix1D &mat){
    return MatrixSplice2String(mat, OUT_STREAM_COMET_CHAR);
}

std::string to_string(const Matrix2D &mat){
    return MatrixSplice2String(mat, std::string(OUT_STREAM_COMET_CHAR) + OUT_STREAM_ENTER_CHAR);
}

std::string to_string(const Matrix3D &mat){
    return MatrixSplice2String(mat, std::string(OUT_STREAM_COMET_CHAR) + OUT_STREAM_ENTER_CHAR);
}

std::string to_string(const Matrix4D &mat){
    return MatrixSplice2String(mat, std::string(OUT_STREAM_COMET_CHAR) + OUT_STREAM_ENTER_CHAR);
}

}

