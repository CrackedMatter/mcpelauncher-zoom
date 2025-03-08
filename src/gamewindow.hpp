#pragma once

inline void* g_window;

inline void* (*getPrimaryWindow)();
inline bool (*isMouseLocked)(void* handle);
inline void (*addKeyboardCallback)(void* handle, void* user, bool (*callback)(void* user, int keyCode, int action));
inline void (*addMouseScrollCallback)(void* handle, void* user, bool (*callback)(void* user, double x, double y, double dx, double dy));
inline void (*addWindowCreationCallback)(void* user, void (*onCreated)(void* user));
