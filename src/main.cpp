#include "gamewindow.hpp"
#include "menu.hpp"
#include "zoom.hpp"
#include <cstddef>
#include <dlfcn.h>
#include <libhat.hpp>
#include <link.h>
#include <optional>
#include <span>
#ifdef __x86_64__
#include <safetyhook.hpp>
#endif

extern "C" [[gnu::visibility("default")]] void mod_preinit() {
    auto gwLib = dlopen("libmcpelauncher_gamewindow.so", 0);

    getPrimaryWindow          = reinterpret_cast<decltype(getPrimaryWindow)>(dlsym(gwLib, "game_window_get_primary_window"));
    isMouseLocked             = reinterpret_cast<decltype(isMouseLocked)>(dlsym(gwLib, "game_window_is_mouse_locked"));
    addKeyboardCallback       = reinterpret_cast<decltype(addKeyboardCallback)>(dlsym(gwLib, "game_window_add_keyboard_callback"));
    addMouseScrollCallback    = reinterpret_cast<decltype(addMouseScrollCallback)>(dlsym(gwLib, "game_window_add_mouse_scroll_callback"));
    addWindowCreationCallback = reinterpret_cast<decltype(addWindowCreationCallback)>(dlsym(gwLib, "game_window_add_window_creation_callback"));

    auto menuLib = dlopen("libmcpelauncher_menu.so", 0);

    addMenu     = reinterpret_cast<decltype(addMenu)>(dlsym(menuLib, "mcpelauncher_addmenu"));
    showWindow  = reinterpret_cast<decltype(showWindow)>(dlsym(menuLib, "mcpelauncher_show_window"));
    closeWindow = reinterpret_cast<decltype(closeWindow)>(dlsym(menuLib, "mcpelauncher_close_window"));

    addWindowCreationCallback(nullptr, [](void*) {
        g_window = getPrimaryWindow();
        addKeyboardCallback(g_window, nullptr, onKeyboard);
        addMouseScrollCallback(g_window, nullptr, onMouseScroll);
    });

    initialize();
}

extern "C" [[gnu::visibility("default")]] void mod_init() {
    using namespace hat::literals::signature_literals;

    auto mcLib = dlopen("libminecraftpe.so", 0);

    std::span<std::byte> range1, range2;

    auto callback = [&](const dl_phdr_info& info) {
        if (auto h = dlopen(info.dlpi_name, RTLD_NOLOAD); dlclose(h), h != mcLib)
            return 0;
        range1 = {reinterpret_cast<std::byte*>(info.dlpi_addr + info.dlpi_phdr[1].p_vaddr), info.dlpi_phdr[1].p_memsz};
        range2 = {reinterpret_cast<std::byte*>(info.dlpi_addr + info.dlpi_phdr[2].p_vaddr), info.dlpi_phdr[2].p_memsz};
        return 1;
    };

    dl_iterate_phdr(
        [](dl_phdr_info* info, size_t, void* data) {
            return (*static_cast<decltype(callback)*>(data))(*info);
        },
        &callback);

    auto CameraAPI_typeinfo_name = hat::find_pattern(range1, hat::object_to_signature("9CameraAPI")).get();
    auto CameraAPI_typeinfo      = hat::find_pattern(range2, hat::object_to_signature(CameraAPI_typeinfo_name)).get() - sizeof(void*);
    auto CameraAPI_vtable        = hat::find_pattern(range2, hat::object_to_signature(CameraAPI_typeinfo)).get() + sizeof(void*);
    auto CameraAPI_tryGetFOV     = reinterpret_cast<std::optional<float> (**)(void*)>(CameraAPI_vtable) + 6;

    static auto CameraAPI_tryGetFOV_orig = *CameraAPI_tryGetFOV;

    *CameraAPI_tryGetFOV = [](void* self) { return CameraAPI_tryGetFOV_orig(self).transform(getFOV); };

#ifdef __x86_64__
    static SafetyHookInline localPlayerTurn_hook = safetyhook::create_inline(
        hat::find_pattern(range1, "48 8D 74 24 ? 4C 89 FF E8 ? ? ? ? 49 8B 45 00 4C 89 EF"_sig).rel(9),
        +[](void* self, const Vec2& delta) {
            localPlayerTurn_hook.call<void, void*, const Vec2&>(self, getTurnDelta(delta));
        });
#endif
}
