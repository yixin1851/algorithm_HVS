#include "WidgetDVSInit.h"
#include "AlpMPAlgoInterface.h"
#include <QFileDialog>

CWidgetDVSInit::CWidgetDVSInit(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);
	connect(this, SIGNAL(accepted()), this, SLOT(Init()));
	connect(ui.pushButtonBrowser, SIGNAL(clicked()), this, SLOT(Browser()));
}

void CWidgetDVSInit::Browser()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Log Directory"), "../",QFileDialog::ShowDirsOnly);
	ui.lineEditLogDir->setText(dir);
}

void CWidgetDVSInit::Init()
{
	int32_t nSensorType = ui.comboBoxSensorType->currentIndex();
	std::string strLogDir = ui.lineEditLogDir->text().toStdString();
	bool bMultiThreadEnable = ui.checkBoxMultiThreadEnable->isChecked();
	bool bLogEnable = ui.checkBoxLogEnable->isChecked();

	if (m_pDVSAlgoInterface != nullptr)
	{
		delete m_pDVSAlgoInterface;
	}

	m_pDVSAlgoInterface = CreateDVSAlgoInterface(SensorType(nSensorType), strLogDir);
	m_pDVSAlgoInterface->SetLogEnable(bLogEnable);
	m_pDVSAlgoInterface->SetMultiThreadEnable(bMultiThreadEnable);
}