///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  5 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "popup_dlg_generated.h"

///////////////////////////////////////////////////////////////////////////

PopupDialogGenerated::PopupDialogGenerated( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxVERTICAL );
	
	m_panel33 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel33->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	
	wxBoxSizer* bSizer165;
	bSizer165 = new wxBoxSizer( wxHORIZONTAL );
	
	m_bitmapMsgType = new wxStaticBitmap( m_panel33, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer165->Add( m_bitmapMsgType, 0, wxALL, 10 );
	
	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxVERTICAL );
	
	m_staticTextMain = new wxStaticText( m_panel33, wxID_ANY, _("dummy"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextMain->Wrap( -1 );
	bSizer16->Add( m_staticTextMain, 0, wxTOP|wxBOTTOM|wxRIGHT, 15 );
	
	m_textCtrlTextDetail = new wxTextCtrl( m_panel33, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxNO_BORDER );
	bSizer16->Add( m_textCtrlTextDetail, 1, wxEXPAND, 5 );
	
	
	bSizer165->Add( bSizer16, 1, wxEXPAND, 5 );
	
	
	m_panel33->SetSizer( bSizer165 );
	m_panel33->Layout();
	bSizer165->Fit( m_panel33 );
	bSizer24->Add( m_panel33, 1, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	
	m_staticline6 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer24->Add( m_staticline6, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxVERTICAL );
	
	m_checkBoxCustom = new wxCheckBox( this, wxID_ANY, _("dummy"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer25->Add( m_checkBoxCustom, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	bSizerStdButtons = new wxBoxSizer( wxHORIZONTAL );
	
	m_buttonAffirmative = new wxButton( this, wxID_YES, _("dummy"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizerStdButtons->Add( m_buttonAffirmative, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_buttonNegative = new wxButton( this, wxID_NO, _("dummy"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizerStdButtons->Add( m_buttonNegative, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );
	
	m_buttonCancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizerStdButtons->Add( m_buttonCancel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );
	
	
	bSizer25->Add( bSizerStdButtons, 0, wxALIGN_RIGHT, 5 );
	
	
	bSizer24->Add( bSizer25, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer24 );
	this->Layout();
	bSizer24->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( PopupDialogGenerated::OnClose ) );
	m_checkBoxCustom->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( PopupDialogGenerated::OnCheckBoxClick ), NULL, this );
	m_buttonAffirmative->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PopupDialogGenerated::OnButtonAffirmative ), NULL, this );
	m_buttonNegative->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PopupDialogGenerated::OnButtonNegative ), NULL, this );
	m_buttonCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PopupDialogGenerated::OnCancel ), NULL, this );
}

PopupDialogGenerated::~PopupDialogGenerated()
{
}
