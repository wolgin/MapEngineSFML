#include "Geodesic.hpp"

namespace ns {

    const std::array<float, 3> projection_curvature(float R, float x, float y) {
        const float divider = R * R - x * x - y * y;
        return {
            (R * R - y * y) / divider,
            x * y / divider,
            (R * R - x * x) / divider
        };
    }

    const std::array<float, 6> curvature_gradient(float R, float x, float y) {
        const float divider = (R * R - x * x - y * y) * (R * R - x * x - y * y);
        return {
            2.0f * x * (R * R - y * y) / divider,
            2.0f * x * x * y / divider,
            y * (R * R - y * y + x * x) / divider,
            x * (R * R - x * x + y * y) / divider,
            2.0f * y * y * x / divider,
            2.0f * x * (R * R - y * y) / divider
        };
    }

    const std::array<float, 6> christoffel_symbols(float R, float x, float y) {
        const std::array<float, 3> curv = projection_curvature(R, x, y);
        const std::array<float, 6> grad = curvature_gradient(R, x, y);
        const float det = 2.0f * (curv[0] * curv[2] - curv[1] * curv[1]);
        const std::array<float, 3> inv = { curv[2] / det, -curv[1] / det, curv[0] / det };
        return {
            inv[0] * grad[0] +                      inv[1] * (2.0f * grad[2] - grad[1]),
            inv[0] * grad[1] +                      inv[1] * grad[4],
            inv[0] * (2.0f * grad[3] - grad[4]) +   inv[1] * grad[5],
            inv[1] * grad[0] +                      inv[2] * (2.0f * grad[2] - grad[1]),
            inv[1] * grad[1] +                      inv[2] * grad[4],
            inv[1] * (2.0f * grad[3] - grad[4]) +   inv[2] * grad[5]
        };
    }

    void geodesic_delta(float R, float dt, std::array<float, 4> &u0) {
        auto rhs = [R](const std::array<float, 4> u) -> const std::array<float, 4> {
            const std::array<float, 6> cs = christoffel_symbols(R, u[0], u[1]);
            return {
                u[2],
                u[3],
                -(cs[0] * u[2] * u[2] + 2.0f * cs[1] * u[2] * u[3] + cs[2] * u[3] * u[3]),
                -(cs[3] * u[2] * u[2] + 2.0f * cs[4] * u[2] * u[3] + cs[5] * u[3] * u[3])
            };
        };
        const std::array<float, 4> f0 = rhs(u0);
        const std::array<float, 4> u1 = {
            u0[0] + dt * f0[0] / 2.0f,
            u0[1] + dt * f0[1] / 2.0f,
            u0[2] + dt * f0[2] / 2.0f,
            u0[3] + dt * f0[3] / 2.0f
        };
        const std::array<float, 4> f1 = rhs(u1);
        const std::array<float, 4> u2 = {
            u0[0] + dt * f1[0] / 2.0f,
            u0[1] + dt * f1[1] / 2.0f,
            u0[2] + dt * f1[2] / 2.0f,
            u0[3] + dt * f1[3] / 2.0f
        };
        const std::array<float, 4> f2 = rhs(u2);
        const std::array<float, 4> u3 = {
            u0[0] + dt * f2[0],
            u0[1] + dt * f2[1],
            u0[2] + dt * f2[2],
            u0[3] + dt * f2[3]
        };
        const std::array<float, 4> f3 = rhs(u3);
        u0[0] = u0[0] + dt * ( f0[0] + 2.0f * f1[0] + 2.0f * f2[0] + f3[0] ) / 6.0f;
        u0[1] = u0[1] + dt * ( f0[1] + 2.0f * f1[1] + 2.0f * f2[1] + f3[1] ) / 6.0f;
        u0[2] = u0[2] + dt * ( f0[2] + 2.0f * f1[2] + 2.0f * f2[2] + f3[2] ) / 6.0f;
        u0[3] = u0[3] + dt * ( f0[3] + 2.0f * f1[3] + 2.0f * f2[3] + f3[3] ) / 6.0f;
    }
}
