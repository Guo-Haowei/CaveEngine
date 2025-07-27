#pragma once

namespace app {

void initialize();
void finalize();

void pollEvents();
bool shouldQuit();

void setWindowTitle(const char* title);
void drawPixels(int width, int height, const void* data);

}  // namespace app
