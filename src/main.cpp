#include "WindowChroma.h"

#include <hyprland/src/SharedDefs.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprlang.hpp>

inline HANDLE                            PHANDLE = nullptr;

inline WindowChroma                      g_WindowChroma;
inline std::mutex                        g_ChromaMutex;

inline std::vector<SP<HOOK_CALLBACK_FN>> g_Callbacks;

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH = __hyprland_api_get_hash();

    // ALWAYS add this to your plugins. It will prevent random crashes coming from
    // mismatched header versions.
    if (HASH != GIT_COMMIT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[hyprchroma] Mismatched headers! Can't proceed.", CColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[hyprchroma] Version mismatch");
    }

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:chroma:color", Hyprlang::INT(0x000000));
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:chroma:alpha", Hyprlang::FLOAT(0.85f));
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:chroma:similarity", Hyprlang::FLOAT(0.1f));
    HyprlandAPI::addDispatcher(PHANDLE, "togglewindowchromakey", [&](std::string args) {
        std::lock_guard<std::mutex> lock(g_ChromaMutex);
        g_WindowChroma.ToggleChroma(g_pCompositor->getWindowByRegex(args));
    });
    HyprlandAPI::addDispatcher(PHANDLE, "togglechromakey", [&](std::string args) {
        std::lock_guard<std::mutex> lock(g_ChromaMutex);
        g_WindowChroma.ToggleChroma(g_pCompositor->m_pLastWindow.lock());
    });

    {
        std::lock_guard<std::mutex> lock(g_ChromaMutex);
        g_WindowChroma.Init();
    }

    g_Callbacks = {};

    g_Callbacks.push_back(HyprlandAPI::registerCallbackDynamic(PHANDLE, "render", [&](void* self, SCallbackInfo&, std::any data) {
        std::lock_guard<std::mutex> lock(g_ChromaMutex);
        eRenderStage                renderStage = std::any_cast<eRenderStage>(data);

        if (renderStage == eRenderStage::RENDER_PRE_WINDOW)
            g_WindowChroma.OnRenderWindowPre();
        if (renderStage == eRenderStage::RENDER_POST_WINDOW)
            g_WindowChroma.OnRenderWindowPost();
    }));
    g_Callbacks.push_back(HyprlandAPI::registerCallbackDynamic(PHANDLE, "configReloaded", [&](void* self, SCallbackInfo&, std::any data) {
        std::lock_guard<std::mutex> lock(g_ChromaMutex);

        g_WindowChroma.Reload();
    }));
    g_Callbacks.push_back(HyprlandAPI::registerCallbackDynamic(PHANDLE, "closeWindow", [&](void* self, SCallbackInfo&, std::any data) {
        std::lock_guard<std::mutex> lock(g_ChromaMutex);
        g_WindowChroma.OnWindowClose(std::any_cast<PHLWINDOW>(data));
    }));

    g_Callbacks.push_back(HyprlandAPI::registerCallbackDynamic(PHANDLE, "windowUpdateRules", [&](void* self, SCallbackInfo&, std::any data) {
        std::lock_guard<std::mutex> lock(g_ChromaMutex);
        g_WindowChroma.ChromaIfMatches(std::any_cast<PHLWINDOW>(data));
    }));

    return {"hyprchroma", "Applies ChromaKey algorithm to windows for transparency effect", "alexhulbert", "1.0.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    std::lock_guard<std::mutex> lock(g_ChromaMutex);
    g_Callbacks = {};
    g_WindowChroma.Unload();
}
