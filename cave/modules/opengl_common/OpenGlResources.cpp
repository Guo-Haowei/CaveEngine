#include "OpenGlResources.h"

#include "OpenGlHelpers.h"

namespace cave {

void OpenGlBuffer::Clear() {
    if (handle) {
        glDeleteBuffers(1, &handle);
        handle = 0;
    }
}

void OpenGlGpuTexture::Clear() {
    if (handle) {
        glDeleteTextures(1, &handle);
        handle = 0;
        residentHandle = 0;
    }
}

void OpenGlConstantBuffer::clear() {
    if (handle) {
        glDeleteBuffers(1, &handle);
        handle = 0;
    }
}

void OpenGlStructuredBuffer::clear() {
    if (handle) {
        glDeleteBuffers(1, &handle);
        handle = 0;
    }
}

}  // namespace cave
