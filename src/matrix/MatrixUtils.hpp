#pragma once
#include "DynamicMatrix.hpp"
#include "StaticMatrix.hpp"
#include <string>
#include <cassert>
#include <cstddef>
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
inline constexpr static const char* OUT_STREAM_EMPTY_CHAR    = "";

namespace std{
    template<class T>
    std::string to_string(const Matrix1DBase<T> &mat);

    template<class T>
    std::string to_string(const Matrix2DBase<T> &mat);

    template<class T>
    std::string to_string(const Matrix3DBase<T> &mat);

    template<class T>
    std::string to_string(const Matrix4DBase<T> &mat);

    template<class T>
    std::string to_string(const MatrixIndex1<T> &mat){
        return std::to_string(Matrix1DBase<T>(mat));
    }

    template<class T>
    std::string to_string(const MatrixIndex2<T> &mat){
        return std::to_string(Matrix2DBase<T>(mat));
    }

    template<class T>
    std::string to_string(const MatrixIndex3<T> &mat){
        return std::to_string(Matrix3DBase<T>(mat));
    }

    template<class T>
    std::string to_string(const MatrixIndex4<T> &mat){
        return std::to_string(Matrix4DBase<T>(mat));
    }
}

inline static std::size_t MatrixStringLoopSize(const std::size_t size){
    return size > OUT_STREAM_MAX_ELEMENT ? OUT_STREAM_MAX_ELEMENT - 1 : size - 1;
}

template<class T, template<typename> class MatrixType>
std::size_t MatrixGetSize(const MatrixType<T> &mat){
    using type = MatrixType<T>;
    if constexpr(std::is_same_v<type, Matrix1DBase<T>>){
        return mat.d1;
    }else if constexpr(std::is_same_v<type, Matrix2DBase<T>>){
        return mat.d2;
    }else if constexpr(std::is_same_v<type, Matrix3DBase<T>>){
        return mat.d3;
    }else if constexpr(std::is_same_v<type, Matrix4DBase<T>>){
        return mat.d4;
    }

    return 0;
}

template<class T, template<typename> class MatrixType>
inline static std::string MatrixSplice2String(const MatrixType<T> &mat, const std::string valueSep){
    std::string ret { OUT_STREAM_LEFT_SEP2 };
    const auto size = MatrixGetSize(mat);
    const auto outsize = MatrixStringLoopSize(size);
    auto enter = OUT_STREAM_EMPTY_CHAR;
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
template<class T>
std::string to_string(const Matrix1DBase<T> &mat){
    return MatrixSplice2String(mat, OUT_STREAM_COMET_CHAR);
}

template<class T>
std::string to_string(const Matrix2DBase<T> &mat){
    return MatrixSplice2String(mat, std::string(OUT_STREAM_COMET_CHAR) + OUT_STREAM_ENTER_CHAR);
}

template<class T>
std::string to_string(const Matrix3DBase<T> &mat){
    return MatrixSplice2String(mat, std::string(OUT_STREAM_COMET_CHAR) + OUT_STREAM_ENTER_CHAR);
}

template<class T>
std::string to_string(const Matrix4DBase<T> &mat){
    return MatrixSplice2String(mat, std::string(OUT_STREAM_COMET_CHAR) + OUT_STREAM_ENTER_CHAR);
}

}

