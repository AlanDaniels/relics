#pragma once

#include "stdafx.h"
#include "build_settings.h"


class SettingsDialog : public wxDialog
{
public:
    SettingsDialog(wxWindow *parent);
    ~SettingsDialog() {}

    const BuildSettings &getBuildSettings() const { return m_build_settings; }

private:
    // Private methods.
    void setMinCtrlWidth(wxControl *ctrl, int min_width);
    void adjustTextValue(wxTextCtrl *ctrl, double diff, int precision);

    void OnOkayClick(wxCommandEvent &event);

    void OnStonePctSpinUp(wxSpinEvent &event) { adjustTextValue(m_stone_pct_text, 1.0, 1); }
    void OnStonePctSpinDown(wxSpinEvent &event) { adjustTextValue(m_stone_pct_text, -1.0, 1); }
    void OnCoalPctSpinUp(wxSpinEvent &event) { adjustTextValue(m_coal_pct_text, 0.1, 2); }
    void OnCoalPctSpinDown(wxSpinEvent &event) { adjustTextValue(m_coal_pct_text, -0.1, 2); }

    // Private data.
    BuildSettings m_build_settings;
    wxTextCtrl *m_stone_pct_text;
    wxTextCtrl *m_coal_pct_text;
};