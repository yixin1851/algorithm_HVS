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

	connect(this, SIGNAL(accepted()), this, SLOT(Init()));
	connect(ui.pushButtonBrowser, SIGNAL(clicked()), this, SLOT(Browser()));
	connect(ui.comboBoxSensorType, SIGNAL(currentIndexChanged(int)), this, SLOT(RawDataInfoInit(int)), Qt::QueuedConnection);
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
		ui.lineEditUp->setText(QString::number(5));
		ui.lineEditDown->setText(QString::number(620));
		ui.lineEditLeft->setText(QString::number(4));
		ui.lineEditRight->setText(QString::number(799));
	}
	else if (nIndex == SensorType::ALP_003BA)
	{
		ui.lineEditTotalRowNumber->setText(QString::number(2340));
		ui.lineEditTotalColNumber->setText(QString::number(3264));
		ui.lineEditUp->setText(QString::number(0));
		ui.lineEditDown->setText(QString::number(584));
		ui.lineEditLeft->setText(QString::number(0));
		ui.lineEditRight->setText(QString::number(815));
	}
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

	m_pAPSAlgoInterface->SetRawDataSize(ui.lineEditTotalRowNumber->text().toUInt(), ui.lineEditTotalColNumber->text().toUInt() / 2);
	ROIArea ActiveArea = { ui.lineEditUp->text().toUInt(),ui.lineEditDown->text().toUInt(), ui.lineEditLeft->text().toUInt(), ui.lineEditRight->text().toUInt() };
	m_pAPSAlgoInterface->SetActiveArea(ActiveArea);
}