#include "MathConst.hpp"

#include "Geometry.hpp"
#include "MathConst.hpp"
#include <type_traits>
#include "Math.hpp"

namespace Math{
template<class FPType>
inline constexpr static FPType FixedPointGetShift(){
    FPType shift = 0;
    if constexpr(std::is_same_v<FPType, FPNumber32>){
        shift = FP32Shift;
    } else if constexpr(std::is_same_v<FPType, FPNumber64>){
        shift = FP64Shift;
    }

    return shift;
}

template<class FPType>
inline constexpr static FPType FixedPointGetDpMask(){
    FPType mask = 0;
    if constexpr(std::is_same_v<FPType, FPNumber32>){
        mask = FP32DpMask;
    } else if constexpr(std::is_same_v<FPType, FPNumber64>){
        mask = FP64DpMask;
    }

    return mask;
}

template<class FPType>
inline constexpr static FPType FixedPointGetWpMask(){
    FPType mask = 0;
    if constexpr(std::is_same_v<FPType, FPNumber32>){
        mask = FP32WpMask;
    } else if constexpr(std::is_same_v<FPType, FPNumber64>){
        mask = FP64WpMask;
    }

    return mask;
}

template<class FPType>
inline constexpr static FPType FixedPointGetMag(){
    FPType mag = 0;
    if constexpr(std::is_same_v<FPType, FPNumber32>){
        mag = FP32Mag;
    } else if constexpr(std::is_same_v<FPType, FPNumber64>){
        mag = FP64Mag;
    }

    return mag;
}

template<class FPType>
inline constexpr static FPType FixedPointDp(const FPType fp){
    return fp >> FPGetShift<FPType>();
}

template<class FPType>
inline constexpr static FPType FixedPointWp(const FPType fp){
    return fp && FixedPointGetWpMask<FPType>();
}

template<class FPType, class T>
inline constexpr static FPType NValue2FixedPoint(const T v){
    if constexpr(std::is_integral_v<T>){
        return v << FixedPointGetShift<FPType>();
    } else if constexpr(std::is_floating_point_v<T>){
        return v * FixedPointGetMag<FPType>() + 0.5;
    }
}

template<class FPType>
inline constexpr static MathConstType FixedPoint2NValue(const FPType value){
    return value * 1.0 / FixedPointGetMag<FPType>();
}

template<class FPType>
class FixedPoint{
    using FPValueType = FPType;
public:
    template<class T>
    FixedPoint(const T fp){
        if constexpr(std::is_integral_v<T>){
            value =  fp << FixedPointGetShift<FPValueType>();
        }else if constexpr(std::is_same_v<T, float> || std::is_same_v<T, double>){
            value = fp * static_cast<T>(FixedPointGetMag<FPValueType>()) + 0.5;
        }else{
            static_assert(false, "The value is not supported!");
        }
    }

    auto toDouble() const{
        return static_cast<double>(value) / FixedPointGetMag<FPValueType>();
    }

    auto wp() const{
        return value >> FixedPointGetShift<FPValueType>();
    }

    auto dp() const{
        return value & FixedPointGetWpMask<FPValueType>();
    }
    
public:
    FPValueType value{};
};

}
