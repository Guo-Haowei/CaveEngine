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
        : value_(0) {}
    explicit constexpr Degree(float degree)
        : value_(degree) {}
    explicit constexpr Degree(double degree)
        : value_(static_cast<float>(degree)) {}

    Degree operator*(float scalar) const {
        return Degree{ value_ * scalar };
    }
    Degree operator/(float scalar) const {
        return Degree{ value_ / scalar };
    }
    Degree& operator*=(float scalar) {
        value_ *= scalar;
        return *this;
    }
    Degree& operator/=(float scalar) {
        value_ /= scalar;
        return *this;
    }
    Degree& operator+=(Degree rhs) {
        value_ += rhs.value_;
        return *this;
    }
    Degree& operator-=(Degree rhs) {
        value_ -= rhs.value_;
        return *this;
    }

    constexpr auto operator<=>(const Degree&) const = default;

    constexpr Degree operator-() { return Degree(-value_); }

    void clamp(float low, float high) { value_ = math::clamp(value_, low, high); }
    constexpr float radians() const { return math::radians(value_); }
    constexpr float degrees() const { return value_; }

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
    float value_;
};

class Radian {
public:
    explicit constexpr Radian()
        : value_(0) {}
    explicit constexpr Radian(float rad)
        : value_(rad) {}

    Radian operator*(float scalar) const {
        return Radian{ value_ * scalar };
    }
    Radian operator/(float scalar) const {
        return Radian{ value_ / scalar };
    }
    Radian& operator*=(float scalar) {
        value_ *= scalar;
        return *this;
    }
    Radian& operator/=(float scalar) {
        value_ /= scalar;
        return *this;
    }
    Radian& operator+=(Radian rhs) {
        value_ += rhs.value_;
        return *this;
    }
    Radian& operator-=(Radian rhs) {
        value_ -= rhs.value_;
        return *this;
    }
    Radian& operator+=(Degree rhs) {
        value_ += rhs.radians();
        return *this;
    }
    Radian& operator-=(Degree rhs) {
        value_ -= rhs.radians();
        return *this;
    }

    constexpr auto operator<=>(const Radian&) const = default;

    void clamp(float low, float high) { value_ = math::clamp(value_, low, high); }
    float degrees() const { return math::degrees(value_); }
    float radians() const { return value_; }

    float sin() const { return std::sin(value_); }
    float cos() const { return std::cos(value_); }
    float tan() const { return std::tan(value_); }

private:
    float value_;
};

}  // namespace cave::math
