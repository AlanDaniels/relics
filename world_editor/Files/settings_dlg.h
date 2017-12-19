#pragma once

#include "stdafx.h"

class SettingsDialog : public wxDialog
{
public:
    SettingsDialog(wxWindow *parent);
    ~SettingsDialog() {}

    void OnOkayClick(wxCommandEvent& event);
};