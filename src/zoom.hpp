#pragma once

struct Vec2 {
    float x, y;
};

void initialize();

bool onKeyboard(void* user, int keyCode, int action);

bool onMouseScroll(void* user, double x, double y, double dx, double dy);

float getFOV(float original);

Vec2 getTurnDelta(Vec2 original);
