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

	ui.lineEditUp->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditDown->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditLeft->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditRight->setValidator(new QIntValidator(0, 100000, this));

	connect(ui.comboBoxSensorType, SIGNAL(currentIndexChanged(int)), this, SLOT(RawDataInfoInit(int)), Qt::QueuedConnection);
	emit(ui.comboBoxSensorType->currentIndexChanged(0));
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
	PixelFormatType PixelFormat = PixelFormatType(ui.comboBoxPixelFormat->currentIndex());

	if (m_pDVSAlgoInterface != nullptr)
	{
		delete m_pDVSAlgoInterface;
	}

	m_pDVSAlgoInterface = CreateDVSAlgoInterface(SensorType(nSensorType), strLogDir, PixelFormat);
	m_pDVSAlgoInterface->SetLogEnable(bLogEnable);
	m_pDVSAlgoInterface->SetMultiThreadEnable(bMultiThreadEnable);

	ROIArea ActiveArea = { ui.lineEditUp->text().toUInt(),ui.lineEditDown->text().toUInt(), ui.lineEditLeft->text().toUInt(), ui.lineEditRight->text().toUInt() };
	m_pDVSAlgoInterface->SetActiveArea(ActiveArea);

}

void CWidgetDVSInit::RawDataInfoInit(int nIndex)
{
	if (nIndex == SensorType::ALP_003AA)
	{
		ui.lineEditUp->setText(QString::number(0));
		ui.lineEditDown->setText(QString::number(1223));
		ui.lineEditLeft->setText(QString::number(0));
		ui.lineEditRight->setText(QString::number(1631));
	}
	else if (nIndex == SensorType::ALP_003BA)
	{
		ui.lineEditUp->setText(QString::number(0));
		ui.lineEditDown->setText(QString::number(583));
		ui.lineEditLeft->setText(QString::number(0));
		ui.lineEditRight->setText(QString::number(1631));
	}
	else if (nIndex == SensorType::ALP_003BB)
	{
		ui.lineEditUp->setText(QString::number(0));
		ui.lineEditDown->setText(QString::number(1223));
		ui.lineEditLeft->setText(QString::number(0));
		ui.lineEditRight->setText(QString::number(1631));
	}
	else if (nIndex == SensorType::ALP_003CA)
	{
		ui.lineEditUp->setText(QString::number(0));
		ui.lineEditDown->setText(QString::number(1223));
		ui.lineEditLeft->setText(QString::number(0));
		ui.lineEditRight->setText(QString::number(1631));
	}

}
