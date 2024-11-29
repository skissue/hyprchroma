#pragma once

#include <tuple>
#include <vector>
#include <hyprland/src/Compositor.hpp>

#include "Helpers.h"

class WindowChroma {
  public:
    void Init();
    void Reload();
    void Unload();

    void ChromaIfMatches(PHLWINDOW window);
    void ToggleChroma(PHLWINDOW window);

    void OnRenderWindowPre();
    void OnRenderWindowPost();
    void OnWindowClose(PHLWINDOW window);

  private:
    std::vector<SWindowRule> m_ChromaWindowRules;
    std::vector<PHLWINDOW>   m_ChromaWindows;
    std::vector<PHLWINDOW>   m_ManuallyChromaWindows;

    ShaderHolder             m_Shaders;
    bool                     m_ShadersSwapped = false;
};
