#include "DialogAPSInit.h"
#include "AlpMPAlgoInterface.h"
#include <QFileDialog>

CDialogAPSInit::CDialogAPSInit(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);
	ui.lineEditTotalRowNumber->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditTotalColNumber->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditUp->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditDown->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditLeft->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditRight->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditApsCode->setValidator(new QIntValidator(0, 100000, this));

	connect(this, SIGNAL(accepted()), this, SLOT(Init()));
	connect(ui.pushButtonBrowser, SIGNAL(clicked()), this, SLOT(Browser()));
	connect(ui.comboBoxSensorType, SIGNAL(currentIndexChanged(int)), this, SLOT(RawDataInfoInit(int)), Qt::QueuedConnection);
	connect(ui.lineEditTotalRowNumber, SIGNAL(editingFinished()), this, SLOT(ChangeUpDown()), Qt::QueuedConnection);
	connect(ui.lineEditTotalColNumber, SIGNAL(editingFinished()), this, SLOT(ChangeLeftRight()), Qt::QueuedConnection);
	emit(ui.comboBoxSensorType->currentIndexChanged(0));
}

void CDialogAPSInit::Browser()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Log Directory"), "../", QFileDialog::ShowDirsOnly);
	ui.lineEditLogDir->setText(dir);
}

void CDialogAPSInit::RawDataInfoInit(int nIndex)
{
	if (nIndex == SensorType::ALP_003AA)
	{
		ui.lineEditTotalRowNumber->setText(QString::number(2488));
		ui.lineEditTotalColNumber->setText(QString::number(3312));
		ui.lineEditUp->setText(QString::number(10));
		ui.lineEditDown->setText(QString::number(1241));
		ui.lineEditLeft->setText(QString::number(8));
		ui.lineEditRight->setText(QString::number(1599));
	}
	else if (nIndex == SensorType::ALP_003BA)
	{
		ui.lineEditTotalRowNumber->setText(QString::number(2340));
		ui.lineEditTotalColNumber->setText(QString::number(3264));
		ui.lineEditUp->setText(QString::number(0));
		ui.lineEditDown->setText(QString::number(1169));
		ui.lineEditLeft->setText(QString::number(0));
		ui.lineEditRight->setText(QString::number(1631));
	}
	else if (nIndex == SensorType::ALP_003BB)
	{
		ui.lineEditTotalRowNumber->setText(QString::number(2340));
		ui.lineEditTotalColNumber->setText(QString::number(3264));
		ui.lineEditUp->setText(QString::number(0));
		ui.lineEditDown->setText(QString::number(1169));
		ui.lineEditLeft->setText(QString::number(0));
		ui.lineEditRight->setText(QString::number(1631));
	}
	else if (nIndex == SensorType::ALP_003CA)
	{
		ui.lineEditTotalRowNumber->setText(QString::number(2448));
		ui.lineEditTotalColNumber->setText(QString::number(3264));
		ui.lineEditUp->setText(QString::number(0));
		ui.lineEditDown->setText(QString::number(1223));
		ui.lineEditLeft->setText(QString::number(0));
		ui.lineEditRight->setText(QString::number(1631));
	}
}

void CDialogAPSInit::ChangeUpDown()
{
	uint32_t nRow = ui.lineEditTotalRowNumber->text().toUInt();
	ui.lineEditUp->setText(QString::number(0));
	ui.lineEditDown->setText(QString::number(nRow / 2 - 1));
}

void CDialogAPSInit::ChangeLeftRight()
{
	uint32_t nCol = ui.lineEditTotalColNumber->text().toUInt();
	ui.lineEditLeft->setText(QString::number(0));
	ui.lineEditRight->setText(QString::number(nCol / 2 - 1));
}

void CDialogAPSInit::Init()
{
	int32_t nSensorType = ui.comboBoxSensorType->currentIndex();
	int32_t nRawType = ui.comboBoxRawType->currentIndex();
	std::string strLogDir = ui.lineEditLogDir->text().toLocal8Bit().toStdString();
	bool bMultiThreadEnable = ui.checkBoxMultiThreadEnable->isChecked();
	bool bLogEnable = ui.checkBoxLogEnable->isChecked();
	int32_t nPixelFormat = ui.comboBoxPixelFormat->currentIndex();
	int32_t nCode = ui.lineEditApsCode->text().toInt();

	if (m_pAPSAlgoInterface != nullptr)
	{
		delete m_pAPSAlgoInterface;
	}

	m_pAPSAlgoInterface = CreateAPSAlgoInterface(SensorType(nSensorType), APSRawType(nRawType), strLogDir, PixelFormatType(nPixelFormat), nCode);
	m_pAPSAlgoInterface->SetLogEnable(bLogEnable);
	m_pAPSAlgoInterface->SetMultiThreadEnable(bMultiThreadEnable);

	m_pAPSAlgoInterface->SetRawDataSize(ui.lineEditTotalRowNumber->text().toUInt(), ui.lineEditTotalColNumber->text().toUInt());
	ROIArea ActiveArea = { ui.lineEditUp->text().toUInt(),ui.lineEditDown->text().toUInt(), ui.lineEditLeft->text().toUInt(), ui.lineEditRight->text().toUInt() };
	m_pAPSAlgoInterface->SetActiveArea(ActiveArea);
}