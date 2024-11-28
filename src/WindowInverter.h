#pragma once

#include <tuple>
#include <vector>
#include <hyprland/src/Compositor.hpp>

#include "Helpers.h"

class WindowInverter {
  public:
    void                                  Init();
    void                                  Unload();


    void                                  InvertIfMatches(PHLWINDOW window);
    void                                  ToggleInvert(PHLWINDOW window);
    // TODO remove deprecated
    void SetRules(std::vector<SWindowRule>&& rules);
    void Reload();

    void OnRenderWindowPre();
    void OnRenderWindowPost();
    void OnWindowClose(PHLWINDOW window);

  private:
    std::vector<SWindowRule> m_InvertWindowRules;
    std::vector<PHLWINDOW>   m_InvertedWindows;
    std::vector<PHLWINDOW>   m_ManuallyInvertedWindows;

    ShaderHolder             m_Shaders;
    bool                     m_ShadersSwapped = false;

    // TODO remove deprecated
    bool MatchesDeprecatedRule(PHLWINDOW window);
};
