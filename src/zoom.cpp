#include "zoom.hpp"
#include "gamewindow.hpp"
#include "menu.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <glaze/json.hpp>
#include <print>
#include <string>

struct Config {
    bool  toggleZoom        = false;
    int   zoomKeyCode       = 67;
    float defaultZoomLevel  = 2.f;
    float minZoomLevel      = 0.f;
    float maxZoomLevel      = 10.f;
    float zoomScrollStep    = 0.1f;
    bool  rememberZoomLevel = false;
    bool  dampenSensitivity = true;
};

static Config config;

static glz::sv configPath = "/data/data/com.mojang.minecraftpe/zoom_config.json";

static bool  zoomHeld    = false;
static bool  zoomToggled = false;
static bool  zooming     = false;
static float targetZoomLevel;
static bool  changingZoomKeybind = false;

static void loadConfig() {
    std::string buffer;
    if (auto ec = glz::read_file_json(config, configPath, buffer))
        std::println(stderr, "Failed to load zoom config: {}", glz::format_error(ec, buffer));
}

static void saveConfig() {
    std::string buffer;
    if (auto ec = glz::write_file_json<glz::opts{.prettify = true}>(config, configPath, buffer))
        std::println(stderr, "Failed to save zoom config: {}", glz::format_error(ec, buffer));
}

static void setTargetZoomLevel(float value) {
    targetZoomLevel = value;
}

static float getTargetZoomLevel() {
    return zooming ? targetZoomLevel : 0.f;
}

static float getCurrentZoomLevel() {
    return getTargetZoomLevel();
}

bool onKeyboard(void*, int keyCode, int action) {
    if (changingZoomKeybind && action == 0) {
        changingZoomKeybind = false;
        closeWindow("Change zoom keybind");
        config.zoomKeyCode = keyCode;
        saveConfig();
        return true;
    }
    if (keyCode == config.zoomKeyCode) {
        if (action == 0) {
            zoomHeld = true;
            if (config.toggleZoom && isMouseLocked(g_window))
                zoomToggled = !zoomToggled;
        } else if (action == 2) {
            zoomHeld = false;
        }
    }
    return false;
}

bool onMouseScroll(void*, double, double, double, double dy) {
    if (dy != 0. && config.zoomScrollStep != 0.f && zooming && isMouseLocked(g_window)) {
        setTargetZoomLevel(std::clamp(
            targetZoomLevel + config.zoomScrollStep * static_cast<float>(dy),
            config.minZoomLevel,
            config.maxZoomLevel));
        return true;
    }
    return false;
}

float getFOV(float original) {
    if (config.toggleZoom ? zoomToggled : zoomHeld && isMouseLocked(g_window)) {
        if (!zooming && !config.rememberZoomLevel)
            setTargetZoomLevel(config.defaultZoomLevel);
        zooming = true;
    } else {
        zooming = false;
    }
    return original * std::exp(-getCurrentZoomLevel());
}

Vec2 getTurnDelta(Vec2 original) {
    float factor = config.dampenSensitivity ? std::exp(-getCurrentZoomLevel()) : 1.f;
    return {original.x * factor, original.y * factor};
}

void initialize() {
    loadConfig();
    saveConfig();
    targetZoomLevel = config.defaultZoomLevel;

    MenuEntryABI menuSubEntries[3];

    menuSubEntries[0] = {
        .name  = "Settings",
        .click = [](void*) {
            control controls[6];
            controls[0].type           = 1;
            controls[0].data.sliderint = {
                .label    = "Keybind behavior (0: hold, 1: toggle)",
                .min      = 0,
                .def      = config.toggleZoom,
                .max      = 1,
                .user     = nullptr,
                .onChange = [](void*, int value) { config.toggleZoom = value; },
            };

            controls[1].type             = 2;
            controls[1].data.sliderfloat = {
                .label    = "Default zoom level",
                .min      = config.minZoomLevel,
                .def      = config.defaultZoomLevel,
                .max      = config.maxZoomLevel,
                .user     = nullptr,
                .onChange = [](void*, float value) { config.defaultZoomLevel = value; },
            };

            controls[2].type             = 2;
            controls[2].data.sliderfloat = {
                .label    = "Scroll modifier (0 to disable)",
                .min      = 0.f,
                .def      = config.zoomScrollStep,
                .max      = 1.f,
                .user     = nullptr,
                .onChange = [](void*, float value) { config.zoomScrollStep = value; },
            };

            controls[3].type           = 1;
            controls[3].data.sliderint = {
                .label    = "Remember zoom level (0: off, 1: on)",
                .min      = 0,
                .def      = config.rememberZoomLevel,
                .max      = 1,
                .user     = nullptr,
                .onChange = [](void*, int value) { config.rememberZoomLevel = value; },
            };

#ifdef __x86_64__
            controls[4].type           = 1;
            controls[4].data.sliderint = {
                .label    = "Dampen turn sensitivity (0: off, 1: on)",
                .min      = 0,
                .def      = config.dampenSensitivity,
                .max      = 1,
                .user     = nullptr,
                .onChange = [](void*, int value) { config.dampenSensitivity = value; },
            };
#else
            controls[4].type      = 3;
            controls[4].data.text = {
                .label = "",
                .size  = 0,
            };
#endif

            controls[5].type        = 0;
            controls[5].data.button = {
                .label   = "Save",
                .user    = nullptr,
                .onClick = [](void*) { saveConfig(); },
            };

            showWindow("Zoom settings", false, nullptr, [](void*) { saveConfig(); }, std::size(controls), controls);
        },
    };

    menuSubEntries[1] = {
        .name  = "Change keybind",
        .click = [](void*) {
            control keybindControl;
            keybindControl.type      = 3;
            keybindControl.data.text = {
                .label = "Press any key to change the zoom keybind",
                .size  = 0,
            };
            changingZoomKeybind = true;
            showWindow("Change zoom keybind", true, nullptr, [](void*) { changingZoomKeybind = false; }, 1, &keybindControl);
        },
    };

    menuSubEntries[2] = {
        .name  = "Reload config",
        .click = [](void*) {
            closeWindow("Zoom settings");
            loadConfig();
        },
    };

    MenuEntryABI menuEntry{
        .name       = "Zoom",
        .length     = std::size(menuSubEntries),
        .subentries = menuSubEntries,
    };

    addMenu(1, &menuEntry);
}
