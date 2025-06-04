
#include "stdafx.h"
#include "settings_dlg.h"

// Side note: The "wxALL" flag should have been called "wxBORDER_ALL"
// or something. It means "the border size applies to all four sides".

enum {
    ID_HEIGHT_MAP_PICKER,
    ID_STONE_PERCENT_SPINNER,
    ID_STONE_SUBTRACTED_SPINNER,
    ID_STONE_DISPLACEMENT_SPINNER,
    ID_STONE_NOISE_SCALE_SPINNER,
    ID_COAL_DENSITY_SPINNER
};


// Default ctor.
// For any "magic values" in here such as borders, or spinner ranges, I'm just
// making stuff up. Feel free to tweak this later if you don't like how it works.
SettingsDialog::SettingsDialog(wxWindow *parent) :
    wxDialog(parent, wxID_ANY, "World Settings")
{
    const int BORDER = 5;
    const int WIDE_WIDTH = 400;
    const int THIN_WIDTH = 75;

    // Create our panel and grid.
    m_panel = new wxPanel(this, -1);
    m_grid  = new wxGridBagSizer(2, 5);

    // First row, height map.
    m_height_map_picker = new wxFilePickerCtrl(m_panel, ID_HEIGHT_MAP_PICKER);

    addGridLabel("Height Map:", 0);
    addGridControl(m_height_map_picker, 0, WIDE_WIDTH);

    // Second row, stone percent.
    m_stone_percent_spinner = new wxSpinCtrlDouble(m_panel, ID_STONE_PERCENT_SPINNER);
    m_stone_percent_spinner->SetRange(0, 100);
    m_stone_percent_spinner->SetDigits(1);

    addGridLabel("Stone Percent:", 1);
    addGridControl(m_stone_percent_spinner, 1, THIN_WIDTH);

    // Third row, stone subtracted.
    m_stone_subtracted_spinner = new wxSpinCtrlDouble(m_panel, ID_STONE_SUBTRACTED_SPINNER);
    m_stone_subtracted_spinner->SetRange(-50, 50);
    m_stone_subtracted_spinner->SetDigits(1);

    addGridLabel("Stone Subtracted:", 2);
    addGridControl(m_stone_subtracted_spinner, 2, THIN_WIDTH);

    // Fourth row, stone displacement due to noise.
    m_stone_displacement_spinner = new wxSpinCtrlDouble(m_panel, ID_STONE_DISPLACEMENT_SPINNER);
    m_stone_displacement_spinner->SetRange(-50, 50);
    m_stone_displacement_spinner->SetDigits(1);

    addGridLabel("Stone Displacement:", 3);
    addGridControl(m_stone_displacement_spinner, 3, THIN_WIDTH);

    // Fifth row, stone noise scale.
    m_stone_noise_scale_spinner = new wxSpinCtrlDouble(m_panel, ID_STONE_NOISE_SCALE_SPINNER);
    m_stone_noise_scale_spinner->SetRange(1, 1000);
    m_stone_noise_scale_spinner->SetValue(10);
    m_stone_noise_scale_spinner->SetDigits(0);

    addGridLabel("Stone Noise Scale:", 4);
    addGridControl(m_stone_noise_scale_spinner, 4, THIN_WIDTH);

    // Sixth row, coal density.
    m_coal_density_spinner = new wxSpinCtrlDouble(m_panel, ID_COAL_DENSITY_SPINNER);
    m_coal_density_spinner->SetRange(0.0, 100.0);
    m_coal_density_spinner->SetValue(1.0);
    m_coal_density_spinner->SetDigits(1);

    addGridLabel("Coal Density:", 5);
    addGridControl(m_coal_density_spinner, 5, THIN_WIDTH);

    // All done. Put the panel in a static box.
    m_panel->SetSizer(m_grid);
    m_grid->AddGrowableCol(1);

    wxStaticBoxSizer *static_box = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT(" Build Settings "));
    static_box->Add(m_panel);

    // Our "show stats" check box.
    m_show_stats_checkbox = new wxCheckBox(this, -1, "Show build stats once we're done");
    m_show_stats_checkbox->SetValue(true);

    // A horizontal box for our buttons.
    wxButton *okayButton   = new wxButton(this, wxID_OK, wxT("OK"));
    wxButton *cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"));
    okayButton->SetDefault();

    wxBoxSizer *button_box = new wxBoxSizer(wxHORIZONTAL);
    button_box->Add(okayButton,   0, wxALL, BORDER);
    button_box->Add(cancelButton, 0, wxALL, BORDER);

    // A vertical box for the dialog content.
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(static_box, 1, wxALL, BORDER);
    vbox->Add(m_show_stats_checkbox, 0, wxALL, BORDER);
    vbox->Add(button_box, 0, wxALL, BORDER);
    SetSizer(vbox);

    // Bind our events.
    Bind(wxEVT_BUTTON, &SettingsDialog::onOkayClick, this, wxID_OK);

    // And away we go. The caller needs to call "ShowModal".
    readFromBuildSettings();
    Fit();
    Centre();
}


// Add a right-aligned label to our grid, in the first column.
void SettingsDialog::addGridLabel(const std::string &label_text, int row) 
{
    wxStaticText *label = new wxStaticText(m_panel, -1, label_text.c_str());
    m_grid->Add(label, wxGBPosition(row, 0), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
}


// Add a control to our grid, in the second column.
void SettingsDialog::addGridControl(wxControl *ctrl, int row, int preferred_width)
{
    int height = ctrl->GetMinHeight();
    ctrl->SetMinSize(wxSize(preferred_width, height));

    m_grid->Add(ctrl, wxGBPosition(row, 1), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
}


// Read in our dialog contents from the build settings.
void SettingsDialog::readFromBuildSettings()
{
    const std::string &our_fname = m_build_settings.getHeightMapFilename();
    wxFileName dlg_fname;
    dlg_fname.SetFullName(our_fname.c_str());
    m_height_map_picker->SetFileName(dlg_fname);

    double stone_pct = m_build_settings.getStonePercent();
    m_stone_percent_spinner->SetValue(stone_pct);

    double stone_subtracted = m_build_settings.getStoneSubtracted();
    m_stone_subtracted_spinner->SetValue(stone_subtracted);

    double stone_displacement = m_build_settings.getStoneDisplacement();
    m_stone_displacement_spinner->SetValue(stone_displacement);

    double stone_noise_scale = m_build_settings.getStoneNoiseScale();
    m_stone_noise_scale_spinner->SetValue(stone_noise_scale);

    double coal_density = m_build_settings.getCoalDensity();
    m_coal_density_spinner->SetValue(coal_density);
}


// TODO: Ideally we'd be calling "on close" rather than dealing
// with the button press directly. Figure this out later.
void SettingsDialog::onOkayClick(wxCommandEvent& event)
{
    wxString dlg_fname = m_height_map_picker->GetFileName().GetFullPath();
    std::string fname = dlg_fname.ToStdString();

    double stone_pct          = m_stone_percent_spinner->GetValue();
    double stone_subtracted   = m_stone_subtracted_spinner->GetValue();
    double stone_displacement = m_stone_displacement_spinner->GetValue();
    double stone_noise_scale  = m_stone_noise_scale_spinner->GetValue();
    double coal_density       = m_coal_density_spinner->GetValue();

    BuildSettings new_settings;

    bool valid = (
        new_settings.setHeightMapFilename(fname) &&
        new_settings.setStonePercent(stone_pct) &&
        new_settings.setStoneSubtracted(stone_subtracted) &&
        new_settings.setStoneDisplacement(stone_displacement) &&
        new_settings.setStoneNoiseScale(stone_noise_scale) &&
        new_settings.setCoalDensity(coal_density));

    if (valid) {
        m_build_settings = new_settings;
        EndModal(wxID_OK);
    }
}
