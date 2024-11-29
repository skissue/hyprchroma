#include "WindowChroma.h"
#include <hyprlang.hpp>
#include <hyprutils/string/String.hpp>

extern HANDLE PHANDLE;

void          WindowChroma::Init() {
    m_Shaders.Init();
}

void WindowChroma::Reload() {
    // Reload shader settings
    static auto* const* bkgColor   = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:chroma:color")->getDataStaticPtr();
    static auto* const* similarity = (Hyprlang::FLOAT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:chroma:similarity")->getDataStaticPtr();
    const CColor        bkg        = CColor(**bkgColor);

    glUseProgram(m_Shaders.RGBA.cshader.program);
    glUniform3f(m_Shaders.RGBA.bkg, bkg.r, bkg.g, bkg.b);
    glUniform1f(m_Shaders.RGBA.similarity, **similarity);
    glUseProgram(m_Shaders.EXT.cshader.program);
    glUniform3f(m_Shaders.EXT.bkg, bkg.r, bkg.g, bkg.b);
    glUniform1f(m_Shaders.EXT.similarity, **similarity);

    m_ChromaWindows = {};
    for (const auto& window : g_pCompositor->m_vWindows)
        ChromaIfMatches(window);
}

void WindowChroma::Unload() {
    if (m_ShadersSwapped) {
        std::swap(m_Shaders.EXT.cshader, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shEXT);
        std::swap(m_Shaders.RGBA.cshader, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shRGBA);
        m_ShadersSwapped = false;
    }

    m_Shaders.Destroy();
}
void WindowChroma::ChromaIfMatches(PHLWINDOW window) {
    // for some reason, some events (currently `activeWindow`) sometimes pass a null pointer
    if (!window)
        return;

    std::vector<SWindowRule> rules        = g_pConfigManager->getMatchingRules(window);
    bool                     shouldInvert = std::any_of(rules.begin(), rules.end(), [](const SWindowRule& rule) { return rule.szRule == "plugin:chromakey"; });

    auto                     windowIt = std::find(m_ChromaWindows.begin(), m_ChromaWindows.end(), window);
    if (shouldInvert != (windowIt != m_ChromaWindows.end())) {
        if (shouldInvert)
            m_ChromaWindows.push_back(window);
        else {
            std::swap(*windowIt, *(m_ChromaWindows.end() - 1));
            m_ChromaWindows.pop_back();
        }

        g_pHyprRenderer->damageWindow(window);
    }
}

void WindowChroma::ToggleChroma(PHLWINDOW window) {
    if (!window)
        return;

    auto windowIt = std::find(m_ManuallyChromaWindows.begin(), m_ManuallyChromaWindows.end(), window);
    if (windowIt == m_ManuallyChromaWindows.end())
        m_ManuallyChromaWindows.push_back(window);
    else {
        std::swap(*windowIt, *(m_ManuallyChromaWindows.end() - 1));
        m_ManuallyChromaWindows.pop_back();
    }

    g_pHyprRenderer->damageWindow(window);
}

void WindowChroma::OnRenderWindowPre() {
    bool shouldInvert = (std::find(m_ChromaWindows.begin(), m_ChromaWindows.end(), g_pHyprOpenGL->m_pCurrentWindow.lock()) != m_ChromaWindows.end()) ^
        (std::find(m_ManuallyChromaWindows.begin(), m_ManuallyChromaWindows.end(), g_pHyprOpenGL->m_pCurrentWindow.lock()) != m_ManuallyChromaWindows.end());

    if (shouldInvert) {
        std::swap(m_Shaders.RGBA.cshader, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shRGBA);
        std::swap(m_Shaders.EXT.cshader, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shEXT);
        m_ShadersSwapped = true;
    }
}

void WindowChroma::OnRenderWindowPost() {
    if (m_ShadersSwapped) {
        std::swap(m_Shaders.RGBA.cshader, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shRGBA);
        std::swap(m_Shaders.EXT.cshader, g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shEXT);
        m_ShadersSwapped = false;
    }
}

void WindowChroma::OnWindowClose(PHLWINDOW window) {
    auto windowIt = std::find(m_ChromaWindows.begin(), m_ChromaWindows.end(), window);
    if (windowIt != m_ChromaWindows.end()) {
        std::swap(*windowIt, *(m_ChromaWindows.end() - 1));
        m_ChromaWindows.pop_back();
    }

    windowIt = std::find(m_ManuallyChromaWindows.begin(), m_ManuallyChromaWindows.end(), window);
    if (windowIt != m_ManuallyChromaWindows.end()) {
        std::swap(*windowIt, *(m_ManuallyChromaWindows.end() - 1));
        m_ManuallyChromaWindows.pop_back();
    }
}
