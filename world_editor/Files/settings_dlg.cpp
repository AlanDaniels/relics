
#include "stdafx.h"
#include "settings_dlg.h"

// Side note: The "wxALL" flag should have been called "wxBORDER_ALL"
// or something. It means "the border size applies to all four sides".

enum {
    ID_HEIGHT_MAP_BUTTON,
    ID_HEIGHT_MAP_OUTPUT,
    ID_STONE_PCT_TEXTBOX,
    ID_STONE_PCT_SPINNER,
    ID_COAL_PCT_TEXTBOX,
    ID_COAL_PCT_SPINNER
};


// Default ctor.
SettingsDialog::SettingsDialog(wxWindow *parent) :
    wxDialog(parent, wxID_ANY, "World Settings")
{
    const int BORDER = 10;
    const int PREFERRED_WIDTH = 100;

    wxPanel *panel = new wxPanel(this, -1);
    panel->SetMinSize(wxSize(1000, 500));

    // Vertical gap of two pixels, horizontal of 5.
    wxGridBagSizer *grid = new wxGridBagSizer(2, 5);

    // First row, height map.
    wxStaticText *height_map_label  = new wxStaticText(panel, -1, wxT("Height Map (Image):"));
    wxButton     *height_map_button = new wxButton(panel, ID_HEIGHT_MAP_BUTTON, wxT("File..."));
    wxStaticText *height_map_output = new wxStaticText(panel, ID_HEIGHT_MAP_OUTPUT, wxT("[nothing yet]"));

    setMinCtrlWidth(height_map_button, PREFERRED_WIDTH);

    grid->Add(height_map_label,  wxGBPosition(0, 0), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    grid->Add(height_map_button, wxGBPosition(0, 1), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER);
    grid->Add(height_map_output, wxGBPosition(0, 2), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);

    // Second row, how much stone vs. dirt.
    wxStaticText *stone_pct_label   = new wxStaticText(panel, -1, wxT("Stone vs. Dirt (pct):"));
    m_stone_pct_text = new wxTextCtrl(panel, ID_STONE_PCT_TEXTBOX, wxT("50.0"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT);
    wxSpinButton *stone_pct_spinner = new wxSpinButton(panel, ID_STONE_PCT_SPINNER);

    setMinCtrlWidth(m_stone_pct_text, PREFERRED_WIDTH);
    
    grid->Add(stone_pct_label,   wxGBPosition(1, 0), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    grid->Add(m_stone_pct_text,  wxGBPosition(1, 1), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER);
    grid->Add(stone_pct_spinner, wxGBPosition(1, 2), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);

    // Third row, how much stone becomes coal?
    wxStaticText *coal_pct_label = new wxStaticText(panel, -1, wxT("Coal Found In Stone (Pct):"));
    m_coal_pct_text = new wxTextCtrl(panel, ID_COAL_PCT_TEXTBOX, wxT("5.00"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT);
    wxSpinButton *coal_pct_spinner = new wxSpinButton(panel, ID_COAL_PCT_SPINNER);

    setMinCtrlWidth(m_coal_pct_text, PREFERRED_WIDTH);

    grid->Add(coal_pct_label,   wxGBPosition(2, 0), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    grid->Add(m_coal_pct_text,  wxGBPosition(2, 1), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER);
    grid->Add(coal_pct_spinner, wxGBPosition(2, 2), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);

    // All done. Put this in a static box.
    grid->AddGrowableCol(2);
    panel->SetSizer(grid);

    wxStaticBoxSizer *static_box = new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Options"));
    static_box->Add(panel);

    wxButton *okayButton   = new wxButton(this, wxID_OK,     wxT("OK"));
    wxButton *cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"));
    okayButton->SetDefault();

    // A horizontal box for our buttons.
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(okayButton,   0, wxALL, BORDER);
    hbox->Add(cancelButton, 0, wxALL, BORDER);

    // A vertical box for the dialog content.
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(static_box, 1, wxALL, BORDER);
    vbox->Add(hbox, 0, wxALL, BORDER);
    SetSizer(vbox);

    // Bind our events.
    Bind(wxEVT_BUTTON, &SettingsDialog::OnOkayClick, this, wxID_OK);

    Bind(wxEVT_SPIN_UP, &SettingsDialog::OnStonePctSpinUp, this, ID_STONE_PCT_SPINNER);
    Bind(wxEVT_SPIN_DOWN, &SettingsDialog::OnStonePctSpinDown, this, ID_STONE_PCT_SPINNER);

    Bind(wxEVT_SPIN_UP, &SettingsDialog::OnCoalPctSpinUp, this, ID_COAL_PCT_SPINNER);
    Bind(wxEVT_SPIN_DOWN, &SettingsDialog::OnCoalPctSpinDown, this, ID_COAL_PCT_SPINNER);

    // And away we go.
    Centre();
    ShowModal();
    Destroy();
}


// Set the min width of a control.
void SettingsDialog::setMinCtrlWidth(wxControl *ctrl, int min_width)
{
    int height = ctrl->GetMinHeight();
    ctrl->SetMinSize(wxSize(min_width, height));
}


// Change the value of a text control with a floating point value.
void SettingsDialog::adjustTextValue(wxTextCtrl *ctrl, double diff, int precision)
{
    double val = 0.0;
    bool result = ctrl->GetValue().ToCDouble(&val);
    if (result) {
        val += diff;
        if (val > 100.0) {
            val = 100.0;
        }
        else if (val < 0.0) {
            val = 0.0;
        }
    }
    else {
        val = 0.0;
    }

    char buffer[32];
    sprintf(buffer, "%.*f", precision, val);
    ctrl->SetValue(wxString(buffer));
}


void SettingsDialog::OnOkayClick(wxCommandEvent& event)
{
    wxLogMessage("Greetings from Okay Click!");
}
