
#include "stdafx.h"
#include "settings_dlg.h"

// Default ctor.
SettingsDialog::SettingsDialog(wxWindow *parent) :
    wxDialog(parent, wxID_ANY, "Project Settings")
{
    wxPanel *panel = new wxPanel(this, -1);

    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

    wxStaticBox   *st  = new wxStaticBox(panel, -1, wxT("Colors"), wxPoint(5, 5), wxSize(240, 150));
    wxRadioButton *rb0 = new wxRadioButton(panel, -1, wxT("256 Colors"), wxPoint(15, 30), wxDefaultSize, wxRB_GROUP);
    wxRadioButton *rb1 = new wxRadioButton(panel, -1, wxT("16 Colors"), wxPoint(15, 55));
    wxRadioButton *rb2 = new wxRadioButton(panel, -1, wxT("2 Colors"), wxPoint(15, 80));
    wxRadioButton *rb3 = new wxRadioButton(panel, -1, wxT("Custom"), wxPoint(15, 105));
    wxTextCtrl    *tc  = new wxTextCtrl(panel, -1, wxT(""), wxPoint(95, 105));

    wxButton *okBtn     = new wxButton(this, wxID_OK,     wxT("OK"),     wxDefaultPosition, wxSize(70, 30));
    wxButton *cancelBtn = new wxButton(this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxSize(70, 30));
    okBtn->SetDefault();

    hbox->Add(okBtn, 1);
    hbox->Add(cancelBtn, 1, wxLEFT, 5);

    vbox->Add(panel, 1);
    vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);
    SetSizer(vbox);

    // Bind our events.
    Bind(wxEVT_BUTTON, &SettingsDialog::OnOkayClick, this, wxID_OK);

    // And away we go.
    Centre();
    ShowModal();
    Destroy();
}


void SettingsDialog::OnOkayClick(wxCommandEvent& event)
{
    wxLogMessage("Greetings from Okay Click!");
}
