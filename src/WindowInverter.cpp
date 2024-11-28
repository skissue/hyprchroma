#include "WindowInverter.h"
#include <hyprutils/string/String.hpp>

extern HANDLE PHANDLE;

void          WindowInverter::OnRenderWindowPre() {
    bool shouldInvert = (std::find(m_InvertedWindows.begin(), m_InvertedWindows.end(), g_pHyprOpenGL->m_pCurrentWindow.lock()) != m_InvertedWindows.end()) ^
        (std::find(m_ManuallyInvertedWindows.begin(), m_ManuallyInvertedWindows.end(), g_pHyprOpenGL->m_pCurrentWindow.lock()) != m_ManuallyInvertedWindows.end());
    static auto* const* bkgColor = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:chroma:color")->getDataStaticPtr();

    if (shouldInvert) {
        const CColor bkg = CColor(**bkgColor);

        glUseProgram(m_Shaders.RGBA.program);
        glUniform3f(m_Shaders.BKGA, bkg.r, bkg.g, bkg.b);
        glUseProgram(m_Shaders.RGBX.program);
        glUniform3f(m_Shaders.BKGX, bkg.r, bkg.g, bkg.b);
        glUseProgram(m_Shaders.EXT.program);
        glUniform3f(m_Shaders.BKGE, bkg.r, bkg.g, bkg.b);
        std::swap(m_Shaders.EXT, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shEXT);
        std::swap(m_Shaders.RGBA, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shRGBA);
        std::swap(m_Shaders.RGBX, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shRGBX);
        m_ShadersSwapped = true;
    }
}

void WindowInverter::OnRenderWindowPost() {
    if (m_ShadersSwapped) {
        std::swap(m_Shaders.EXT, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shEXT);
        std::swap(m_Shaders.RGBA, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shRGBA);
        std::swap(m_Shaders.RGBX, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shRGBX);
        m_ShadersSwapped = false;
    }
}

void WindowInverter::OnWindowClose(PHLWINDOW window) {
    auto windowIt = std::find(m_InvertedWindows.begin(), m_InvertedWindows.end(), window);
    if (windowIt != m_InvertedWindows.end()) {
        std::swap(*windowIt, *(m_InvertedWindows.end() - 1));
        m_InvertedWindows.pop_back();
    }

    windowIt = std::find(m_ManuallyInvertedWindows.begin(), m_ManuallyInvertedWindows.end(), window);
    if (windowIt != m_ManuallyInvertedWindows.end()) {
        std::swap(*windowIt, *(m_ManuallyInvertedWindows.end() - 1));
        m_ManuallyInvertedWindows.pop_back();
    }
}

void WindowInverter::Init() {
    m_Shaders.Init();
}

void WindowInverter::Unload() {
    if (m_ShadersSwapped) {
        std::swap(m_Shaders.EXT, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shEXT);
        std::swap(m_Shaders.RGBA, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shRGBA);
        std::swap(m_Shaders.RGBX, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shRGBX);
        m_ShadersSwapped = false;
    }

    m_Shaders.Destroy();
}

void WindowInverter::InvertIfMatches(PHLWINDOW window) {
    // for some reason, some events (currently `activeWindow`) sometimes pass a null pointer
    if (!window)
        return;

    std::vector<SWindowRule> rules        = g_pConfigManager->getMatchingRules(window);
    bool                     shouldInvert = std::any_of(rules.begin(), rules.end(), [](const SWindowRule& rule) { return rule.szRule == "plugin:chromakey"; });

    auto                     windowIt = std::find(m_InvertedWindows.begin(), m_InvertedWindows.end(), window);
    if (shouldInvert != (windowIt != m_InvertedWindows.end())) {
        if (shouldInvert)
            m_InvertedWindows.push_back(window);
        else {
            std::swap(*windowIt, *(m_InvertedWindows.end() - 1));
            m_InvertedWindows.pop_back();
        }

        g_pHyprRenderer->damageWindow(window);
    }
}

void WindowInverter::ToggleInvert(PHLWINDOW window) {
    if (!window)
        return;

    auto windowIt = std::find(m_ManuallyInvertedWindows.begin(), m_ManuallyInvertedWindows.end(), window);
    if (windowIt == m_ManuallyInvertedWindows.end())
        m_ManuallyInvertedWindows.push_back(window);
    else {
        std::swap(*windowIt, *(m_ManuallyInvertedWindows.end() - 1));
        m_ManuallyInvertedWindows.pop_back();
    }

    g_pHyprRenderer->damageWindow(window);
}

void WindowInverter::Reload() {
    m_InvertedWindows = {};

    for (const auto& window : g_pCompositor->m_vWindows)
        InvertIfMatches(window);
}
