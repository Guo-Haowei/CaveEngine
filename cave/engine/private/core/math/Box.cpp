#include "cave/core/math/Box.h"
#include "cave/core/math/Vector.h"

namespace cave::math {

// @TODO: this should be moved
template<>
float Box<3>::SurfaceArea() const {
    if (!IsValid()) {
        return 0.0f;
    }
    Vector<float, 3> span(abs(Size()));
    const float result = 2.0f * (span.x * span.y +
                                 span.x * span.z +
                                 span.y * span.z);
    return result;
}

}  // namespace cave::math
