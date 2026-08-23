#pragma once
#include <algorithm>
#include <vector>

namespace SGE::Render{

// Painter's-algorithm helper: returns indices sorted far-to-near relative to eye.
template<class PosFn>
std::vector<int> SortFarToNear(int count, PosFn positionOf, const Vector3DBase<double> &eye){
    std::vector<int> order(static_cast<std::size_t>(count));
    for(int i = 0; i < count; i++) order[static_cast<std::size_t>(i)] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b){
        const auto pa = positionOf(a);
        const auto pb = positionOf(b);
        const double da = (pa.x-eye.x)*(pa.x-eye.x) + (pa.y-eye.y)*(pa.y-eye.y)
                        + (pa.z-eye.z)*(pa.z-eye.z);
        const double db = (pb.x-eye.x)*(pb.x-eye.x) + (pb.y-eye.y)*(pb.y-eye.y)
                        + (pb.z-eye.z)*(pb.z-eye.z);
        return da > db;
    });
    return order;
}

}
