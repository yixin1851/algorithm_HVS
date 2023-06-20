#include "DialogAPSFunction.h"
#include "DialogAPSInit.h"
#include "DialogAPSImportData.h"
#include "DialogAPSShow.h"
#include "DialogAPSTNoise.h"
#include "DialogAPSSNoise.h"
#include "DialogAPSHotPixel.h"
#include "DialogAPSBLC.h"
#include "DialogAPSShading.h"
#include "DialogAPSBadPixel.h"
#include "DialogAPSDarkCurrent.h"
#include "DialogAPSDSNU.h"
#include "DialogAPSLinearity.h"
#include "DialogAPSOverallSystemGain.h"
#include "DialogAPSDataMean.h"
#include "DialogAPSSaturation.h"
#include "DialogAPSPedestal.h"
#include "DialogAPSReadNoise.h"

CDialogAPSFunction::CDialogAPSFunction(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);
	connect(ui.pushButtonInit, SIGNAL(clicked()), this, SLOT(Init()));
	connect(ui.pushButtonImportData, SIGNAL(clicked()), this, SLOT(ImportData()));
	connect(ui.pushButtonTNoise, SIGNAL(clicked()), this, SLOT(TNoise()));
	connect(ui.pushButtonSNoise, SIGNAL(clicked()), this, SLOT(SNoise()));
	connect(ui.pushButtonBLC, SIGNAL(clicked()), this, SLOT(BLC()));
	connect(ui.pushButtonHotPixel, SIGNAL(clicked()), this, SLOT(HotPixel()));
	connect(ui.pushButtonShading, SIGNAL(clicked()), this, SLOT(Shading()));
	connect(ui.pushButtonDarkCurrent, SIGNAL(clicked()), this, SLOT(DarkCurrent()));
	connect(ui.pushButtonDSNU, SIGNAL(clicked()), this, SLOT(DSNU()));
	connect(ui.pushButtonLinearity, SIGNAL(clicked()), this, SLOT(Linearity()));
	connect(ui.pushButtonBadPixel, SIGNAL(clicked()), this, SLOT(BadPixel()));
	connect(ui.pushButtonOverallSystemGain, SIGNAL(clicked()), this, SLOT(OverallSystemGain()));
	connect(ui.pushButtonDataMean, SIGNAL(clicked()), this, SLOT(DataMean()));
	connect(ui.pushButtonSaturation, SIGNAL(clicked()), this, SLOT(Saturation()));
	connect(ui.pushButtonShow, SIGNAL(clicked()), this, SLOT(Show()));
	connect(ui.pushButtonPedestal, SIGNAL(clicked()), this, SLOT(Pedestal()));
	connect(ui.pushButtonReadNoise, SIGNAL(clicked()), this, SLOT(ReadNoise()));

	if (m_pAPSAlgoInterface)
	{
		ui.labelVersion->setText(QString::fromStdString("Algo Version: " + m_pAPSAlgoInterface->GetVersion()));
	}
}

void CDialogAPSFunction::SetAPSAlgoInterface(CAlpAPSMPAlgoInterface* pAPSAlgoInterface)
{
	m_pAPSAlgoInterface = pAPSAlgoInterface;
}

CDialogAPSFunction::~CDialogAPSFunction()
{
	if (m_pAPSAlgoInterface)
	{
		delete m_pAPSAlgoInterface;
		m_pAPSAlgoInterface = nullptr;
	}
}

void CDialogAPSFunction::Init()
{
	CDialogAPSInit init(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	if (QDialog::Accepted == init.exec())
	{
		m_pAPSAlgoInterface = init.GetAPSAlgoInterface();
	}
}

void CDialogAPSFunction::ImportData()
{
	CDialogAPSImportData ImportData(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	ImportData.exec();
}

void CDialogAPSFunction::Show()
{
	CDialogAPSShow Show(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	Show.exec();
}

void CDialogAPSFunction::TNoise()
{
	CDialogAPSTNoise TNoise(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	TNoise.exec();
}

void CDialogAPSFunction::SNoise()
{
	CDialogAPSSNoise SNoise(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	SNoise.exec();
}

void CDialogAPSFunction::HotPixel()
{
	CDialogAPSHotPixel HotPixel(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	HotPixel.exec();
}

void CDialogAPSFunction::BLC()
{
	CDialogAPSBLC BLC(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	BLC.exec();
}

void CDialogAPSFunction::Shading()
{
	CDialogAPSShading Shading(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	Shading.exec();
}

void CDialogAPSFunction::BadPixel()
{
	CDialogAPSBadPixel BadPixel(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	BadPixel.exec();
}

void CDialogAPSFunction::DarkCurrent()
{
	CDialogAPSDarkCurrent DarkCurrent(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	DarkCurrent.exec();
}

void CDialogAPSFunction::DSNU()
{
	CDialogAPSDSNU DSNU(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	DSNU.exec();
}

void CDialogAPSFunction::Linearity()
{
	CDialogAPSLinearity Linearity(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	Linearity.exec();
}

void CDialogAPSFunction::OverallSystemGain()
{
	CDialogAPSOverallSystemGain OverallSystemGain(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	OverallSystemGain.exec();
}

void CDialogAPSFunction::DataMean()
{
	CDialogAPSDataMean DataMean(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	DataMean.exec();
}

void CDialogAPSFunction::Saturation()
{
	CDialogAPSSaturation Saturation(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	Saturation.exec();
}

void CDialogAPSFunction::Pedestal()
{
	CDialogAPSPedestal Pedestal(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	Pedestal.exec();
}

void CDialogAPSFunction::ReadNoise()
{
	CDialogAPSReadNoise ReadNoise(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	ReadNoise.exec();
}

