#include "DialogAPSInit.h"
#include "AlpMPAlgoInterface.h"
#include <QFileDialog>

CDialogAPSInit::CDialogAPSInit(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);
	connect(this, SIGNAL(accepted()), this, SLOT(Init()));
	connect(ui.pushButtonBrowser, SIGNAL(clicked()), this, SLOT(Browser()));
}

void CDialogAPSInit::Browser()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Log Directory"), "../", QFileDialog::ShowDirsOnly);
	ui.lineEditLogDir->setText(dir);
}

void CDialogAPSInit::Init()
{
	int32_t nSensorType = ui.comboBoxSensorType->currentIndex();
	int32_t nRawType = ui.comboBoxRawType->currentIndex();
	std::string strLogDir = ui.lineEditLogDir->text().toStdString();
	bool bMultiThreadEnable = ui.checkBoxMultiThreadEnable->isChecked();
	bool bLogEnable = ui.checkBoxLogEnable->isChecked();

	if (m_pAPSAlgoInterface != nullptr)
	{
		delete m_pAPSAlgoInterface;
	}

	m_pAPSAlgoInterface = CreateAPSAlgoInterface(SensorType(nSensorType), RawType(nRawType), strLogDir);
	m_pAPSAlgoInterface->SetLogEnable(bLogEnable);
	m_pAPSAlgoInterface->SetMultiThreadEnable(bMultiThreadEnable);
}