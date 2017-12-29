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
    void addGridLabel(const std::string &text, int row);
    void addGridControl(wxControl *ctrl, int row, int preferred_width);
    void readFromBuildSettings();

    void onOkayClick(wxCommandEvent &event);

    // Private data.
    BuildSettings m_build_settings;

    wxPanel *m_panel;
    wxGridBagSizer *m_grid;

    wxFilePickerCtrl *m_height_map_picker;
    wxSpinCtrlDouble *m_stone_percent_spinner;
    wxSpinCtrlDouble *m_stone_subtracted_spinner;
    wxSpinCtrlDouble *m_stone_displacement_spinner;
    wxSpinCtrlDouble *m_stone_noise_scale_spinner;
    wxSpinCtrlDouble *m_coal_density_spinner;
};