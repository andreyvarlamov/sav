#ifndef VA_RECT_H
#define VA_RECT_H

#include <varand/varand_types.h>

struct rect
{
    f32 X;
    f32 Y;
    f32 Width;
    f32 Height;
};

inline rect
Rect(f32 X, f32 Y, f32 Width, f32 Height)
{
    rect Rect;
    Rect.X = X;
    Rect.Y = Y;
    Rect.Width = Width;
    Rect.Height = Height;
    return Rect;
}

inline rect
Rect(f32 Width, f32 Height)
{
    return Rect(0, 0, Width, Height);
}

inline rect
Rect(f32 Dim)
{
    return Rect(Dim, Dim);
}

inline rect
Rect(i32 X, i32 Y, i32 Width, i32 Height)
{
    return Rect((f32) X, (f32) Y, (f32) Width, (f32) Height);
}

inline rect
Rect(i32 Width, i32 Height)
{
    return Rect((f32) Width, (f32) Height);
}

inline rect
Rect(i32 Dim)
{
    return Rect((f32) Dim);
}

#endif
