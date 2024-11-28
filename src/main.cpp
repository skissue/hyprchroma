#include "WindowInverter.h"

#include <hyprland/src/SharedDefs.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprlang.hpp>

inline HANDLE                            PHANDLE = nullptr;

inline WindowInverter                    g_WindowInverter;
inline std::mutex                        g_InverterMutex;

inline std::vector<SP<HOOK_CALLBACK_FN>> g_Callbacks;

APICALL EXPORT PLUGIN_DESCRIPTION_INFO   PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH = __hyprland_api_get_hash();

    // ALWAYS add this to your plugins. It will prevent random crashes coming from
    // mismatched header versions.
    if (HASH != GIT_COMMIT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[hyprchroma] Mismatched headers! Can't proceed.", CColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[hyprchroma] Version mismatch");
    }

    {
        std::lock_guard<std::mutex> lock(g_InverterMutex);
        g_WindowInverter.Init();
    }

    g_Callbacks = {};

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:chroma:color", Hyprlang::INT(0x000000));

    g_Callbacks.push_back(HyprlandAPI::registerCallbackDynamic(PHANDLE, "render", [&](void* self, SCallbackInfo&, std::any data) {
        std::lock_guard<std::mutex> lock(g_InverterMutex);
        eRenderStage                renderStage = std::any_cast<eRenderStage>(data);

        if (renderStage == eRenderStage::RENDER_PRE_WINDOW)
            g_WindowInverter.OnRenderWindowPre();
        if (renderStage == eRenderStage::RENDER_POST_WINDOW)
            g_WindowInverter.OnRenderWindowPost();
    }));
    g_Callbacks.push_back(HyprlandAPI::registerCallbackDynamic(PHANDLE, "configReloaded", [&](void* self, SCallbackInfo&, std::any data) {
        std::lock_guard<std::mutex> lock(g_InverterMutex);

        g_WindowInverter.Reload();
    }));
    g_Callbacks.push_back(HyprlandAPI::registerCallbackDynamic(PHANDLE, "closeWindow", [&](void* self, SCallbackInfo&, std::any data) {
        std::lock_guard<std::mutex> lock(g_InverterMutex);
        g_WindowInverter.OnWindowClose(std::any_cast<PHLWINDOW>(data));
    }));

    g_Callbacks.push_back(HyprlandAPI::registerCallbackDynamic(PHANDLE, "windowUpdateRules", [&](void* self, SCallbackInfo&, std::any data) {
        std::lock_guard<std::mutex> lock(g_InverterMutex);
        g_WindowInverter.InvertIfMatches(std::any_cast<PHLWINDOW>(data));
    }));

    HyprlandAPI::addDispatcher(PHANDLE, "togglewindowchromakey", [&](std::string args) {
        std::lock_guard<std::mutex> lock(g_InverterMutex);
        g_WindowInverter.ToggleInvert(g_pCompositor->getWindowByRegex(args));
    });
    HyprlandAPI::addDispatcher(PHANDLE, "togglechromakey", [&](std::string args) {
        std::lock_guard<std::mutex> lock(g_InverterMutex);
        g_WindowInverter.ToggleInvert(g_pCompositor->m_pLastWindow.lock());
    });

    return {"hyprchroma", "Applies ChromaKey algorithm to windows for transparency effect", "alexhulbert", "1.0.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    std::lock_guard<std::mutex> lock(g_InverterMutex);
    g_Callbacks = {};
    g_WindowInverter.Unload();
}

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}
