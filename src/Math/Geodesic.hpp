#pragma once
#include <array>

namespace ns
{
    const std::array<float, 3> projection_curvature(float R, float x, float y);
    const std::array<float, 6> curvature_gradient(float R, float x, float y);
    const std::array<float, 6> christoffel_symbols(float R, float x, float y);
    void geodesic_delta(float R, float dt, std::array<float, 4> &u0);
}
