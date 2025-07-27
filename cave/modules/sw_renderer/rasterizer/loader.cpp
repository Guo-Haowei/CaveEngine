#include "loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "tinygltf/stb_image.h"

namespace cave::rs {

void loadTexture(Texture& texture, const char* path) {
    int width, height, channel;
    unsigned char* data = stbi_load(path, &width, &height, &channel, 0);
    if (data) {
        std::vector<Color> buffer(width * height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int bufferIndex = y * width + x;
                Color& c = buffer[bufferIndex];
                int channelIndex = channel * bufferIndex;
                // NOTE: bitmap is in bgra format
                c.r = data[channelIndex + 2];
                c.g = data[channelIndex + 1];
                c.b = data[channelIndex + 0];
                c.a = channel == 4 ? data[channelIndex + 3] : 255;
            }
        }

        texture.create({ width, height, buffer.data() });
        stbi_image_free(data);
    } else {
        printf("Failed to load texture '%s'\n", path);
    }
}

}  // namespace cave::rs
