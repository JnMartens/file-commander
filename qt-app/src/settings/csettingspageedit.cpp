#ifdef _WIN32
#pragma warning(push, 0) // set W0
#endif

#include "csettingspageedit.h"
#include "ui_csettingspageedit.h"
#include "settings.h"
#include "settings/csettings.h"

#include "../QtAppIncludes"

#ifdef _WIN32
#pragma warning(pop)
#endif

CSettingsPageEdit::CSettingsPageEdit(QWidget *parent) :
	CSettingsPage(parent),
	ui(new Ui::CSettingsPageEdit)
{
	ui->setupUi(this);
	connect(ui->_btnEditorBrowse, SIGNAL(clicked()), SLOT(browseForEditor()));

	ui->_editorNameLine->setText(CSettings::instance()->value(KEY_EDITOR_PATH).toString());
}

CSettingsPageEdit::~CSettingsPageEdit()
{
	delete ui;
}

void CSettingsPageEdit::acceptSettings()
{
	CSettings::instance()->setValue(KEY_EDITOR_PATH, ui->_editorNameLine->text());
}

void CSettingsPageEdit::browseForEditor()
{
#ifdef _WIN32
	const QString mask("Executable files (*.exe *.cmd *.bat)");
#else
	const QString mask;
#endif
	const QString result = QFileDialog::getOpenFileName(this, "Browse for editor program", QFileInfo(ui->_editorNameLine->text()).absolutePath(), mask);
	if (!result.isEmpty())
		ui->_editorNameLine->setText(result);
}