#pragma once

enum class LineIntersect{
    LineNoIntersect,                        //不相交
    LineIntersectInSegment,                 //相交交点在直线内
    LineIntersectOutSegment,                //不相交但是交点在直线外
};