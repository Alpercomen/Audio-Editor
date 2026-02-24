#pragma once

#include <Application/Core/Definitions/Definitions.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace Math
{
    constexpr Float32 Pi = 3.14159265358979323846f;

    using namespace glm;

    template<Arithmetic T>
    using Quat = glm::tquat<T>;

    template<Arithmetic T>
    using Vec2 = glm::tvec2<T>;

    template<Arithmetic T>
    using Vec3 = glm::tvec3<T>;

    template<Arithmetic T>
    using Vec4 = glm::tvec4<T>;

    template<Arithmetic T>
    using Mat2 = glm::tmat2x2<T>;

    template<Arithmetic T>
    using Mat3 = glm::tmat3x3<T>;

    template<Arithmetic T>
    using Mat4 = glm::tmat4x4<T>;

    using Quatf = Quat<Float32>;
    using Quatd = Quat<Float64>;

    using Vec2uc = Vec2<Uint8>;
    using Vec3uc = Vec3<Uint8>;
    using Vec4uc = Vec4<Uint8>;

    using Vec2f = Vec2<Float32>;
    using Vec3f = Vec3<Float32>;
    using Vec4f = Vec4<Float32>;

    using Vec2d = Vec2<Float64>;
    using Vec3d = Vec3<Float64>;
    using Vec4d = Vec4<Float64>;

    using Vec2i = Vec2<Int32>;
    using Vec3i = Vec3<Int32>;
    using Vec4i = Vec4<Int32>;

    using Vec2u = Vec2<Uint32>;
    using Vec3u = Vec3<Uint32>;
    using Vec4u = Vec4<Uint32>;

    using Mat2f = Mat2<Float32>;
    using Mat3f = Mat3<Float32>;
    using Mat4f = Mat4<Float32>;

    using Mat2d = Mat2<Float64>;
    using Mat3d = Mat3<Float64>;
    using Mat4d = Mat4<Float64>;
} // namespace Math