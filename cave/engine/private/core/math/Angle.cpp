#include "cave/core/math/Angle.h"

namespace cave::math {

Degree& Degree::operator=(const Radian& p_rad) {
    m_value = p_rad.ToDegree();
    return *this;
}

Radian& Radian::operator=(const Degree& p_degree) {
    m_value = p_degree.GetRadians();
    return *this;
}

float Degree::Sin() const {
    return std::sin(GetRadians());
}

float Degree::Cos() const {
    return std::cos(GetRadians());
}

float Degree::Tan() const {
    return std::tan(GetRadians());
}

float Radian::Sin() const {
    return std::sin(m_value);
}

float Radian::Cos() const {
    return std::cos(m_value);
}

float Radian::Tan() const {
    return std::tan(m_value);
}

}  // namespace cave::math
