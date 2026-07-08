// =============================================================================
// File: cave/core/math/Angle.h
// =============================================================================
#pragma once
#include <cmath>
#include "Scalar.h"

namespace cave::math {

class Radian;
class Degree;

class Degree {
public:
    explicit constexpr Degree()
        : m_value(0) {}
    explicit constexpr Degree(float degree)
        : m_value(degree) {}
    explicit constexpr Degree(double degree)
        : m_value(static_cast<float>(degree)) {}

    Degree operator*(float scalar) const {
        return Degree{ m_value * scalar };
    }
    Degree operator/(float scalar) const {
        return Degree{ m_value / scalar };
    }
    Degree& operator*=(float scalar) {
        m_value *= scalar;
        return *this;
    }
    Degree& operator/=(float scalar) {
        m_value /= scalar;
        return *this;
    }
    Degree& operator+=(Degree rhs) {
        m_value += rhs.m_value;
        return *this;
    }
    Degree& operator-=(Degree rhs) {
        m_value -= rhs.m_value;
        return *this;
    }

    constexpr auto operator<=>(const Degree&) const = default;

    constexpr Degree operator-() { return Degree(-m_value); }

    void clamp(float low, float high) { m_value = math::clamp(m_value, low, high); }
    constexpr float radians() const { return math::radians(m_value); }
    constexpr float degrees() const { return m_value; }

    float sin() const {
        return std::sin(radians());
    }

    float cos() const {
        return std::cos(radians());
    }

    float tan() const {
        return std::tan(radians());
    }

private:
    float m_value;
};

class Radian {
public:
    explicit constexpr Radian()
        : m_value(0) {}
    explicit constexpr Radian(float rad)
        : m_value(rad) {}

    Radian operator*(float scalar) const {
        return Radian{ m_value * scalar };
    }
    Radian operator/(float scalar) const {
        return Radian{ m_value / scalar };
    }
    Radian& operator*=(float scalar) {
        m_value *= scalar;
        return *this;
    }
    Radian& operator/=(float scalar) {
        m_value /= scalar;
        return *this;
    }
    Radian& operator+=(Radian rhs) {
        m_value += rhs.m_value;
        return *this;
    }
    Radian& operator-=(Radian rhs) {
        m_value -= rhs.m_value;
        return *this;
    }
    Radian& operator+=(Degree rhs) {
        m_value += rhs.radians();
        return *this;
    }
    Radian& operator-=(Degree rhs) {
        m_value -= rhs.radians();
        return *this;
    }

    constexpr auto operator<=>(const Radian&) const = default;

    void clamp(float low, float high) { m_value = math::clamp(m_value, low, high); }
    float degrees() const { return math::degrees(m_value); }
    float radians() const { return m_value; }

    float sin() const { return std::sin(m_value); }
    float cos() const { return std::cos(m_value); }
    float tan() const { return std::tan(m_value); }

private:
    float m_value;
};

}  // namespace cave::math
