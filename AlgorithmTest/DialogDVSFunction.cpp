#include "DialogDVSFunction.h"
#include "WidgetDVSInit.h"
#include "DialogDVSImportData.h"
#include "DialogDVSCountEvents.h"
#include "DialogDVSStationaryNoise.h"
#include "DialogDVSStationaryUniformity.h"
#include "DialogDVSHotPixel.h"
#include "DialogDVSFindPeak.h"
#include "DialogDVSImageContrastSensitivity.h"
#include "DialogDVSAccompaniedPeakAndDelayedPeak.h"
#include "DialogDVSSpatialResponseUniformity.h"
#include "DialogDVSBadPixel.h"
#include "DialogDVSShow.h"

CDialogDVSFunction::CDialogDVSFunction(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);
	connect(ui.pushButtonInit, SIGNAL(clicked()), this, SLOT(Init()));
	connect(ui.pushButtonImportData, SIGNAL(clicked()), this, SLOT(ImportData()));
	connect(ui.pushButtonCountEvents, SIGNAL(clicked()), this, SLOT(CountEvents()));
	connect(ui.pushButtonStationaryNoise, SIGNAL(clicked()), this, SLOT(StationaryNoise()));
	connect(ui.pushButtonStationaryUniformity, SIGNAL(clicked()), this, SLOT(StationaryUniformity()));
	connect(ui.pushButtonHotPixel, SIGNAL(clicked()), this, SLOT(HotPixel()));
	connect(ui.pushButtonFindPeak, SIGNAL(clicked()), this, SLOT(FindPeak()));
	connect(ui.pushButtonImageContrastSensitivity, SIGNAL(clicked()), this, SLOT(ImageContrastSensitivity()));
	connect(ui.pushButtonImageAccompaniedPeakAndDelayedPeak, SIGNAL(clicked()), this, SLOT(AccompaniedPeakAndDelayedPeak()));
	connect(ui.pushButtonSpatialResponseUniformity, SIGNAL(clicked()), this, SLOT(SpatialResponseUniformity()));
	connect(ui.pushButtonBadPixel, SIGNAL(clicked()), this, SLOT(BadPixel()));
	connect(ui.pushButtonShow, SIGNAL(clicked()), this, SLOT(Show()));

	if (m_pDVSAlgoInterface)
	{
		ui.labelVersion->setText(QString::fromStdString("Algo Version: " + m_pDVSAlgoInterface->GetVersion()));
	}
}

void CDialogDVSFunction::SetDVSAlgoInterface(CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
{
	m_pDVSAlgoInterface = pDVSAlgoInterface;
}

CDialogDVSFunction::~CDialogDVSFunction()
{
	if (m_pDVSAlgoInterface)
	{
		delete m_pDVSAlgoInterface;
		m_pDVSAlgoInterface = nullptr;
	}
}

void CDialogDVSFunction::ImportData()
{
	CDialogDVSImportData ImportData(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	ImportData.exec();
}

void CDialogDVSFunction::Init()
{
	CWidgetDVSInit init(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	if (QDialog::Accepted == init.exec())
	{
		m_pDVSAlgoInterface = init.GetDVSAlgoInterface();
	}
}

void CDialogDVSFunction::CountEvents()
{
	CDialogCountEvents CountEvents(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	CountEvents.exec();
}

void CDialogDVSFunction::StationaryNoise()
{
	CDialogStationaryNoise StationaryNoise(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	StationaryNoise.exec();
}

void CDialogDVSFunction::StationaryUniformity()
{
	CDialogStationaryUniformity StationaryUniformity(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	StationaryUniformity.exec();
}

void CDialogDVSFunction::HotPixel()
{
	CDialogDVSHotPixel HotPixel(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	HotPixel.exec();
}

void CDialogDVSFunction::FindPeak()
{
	CDialogDVSFindPeak FindPeak(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	FindPeak.exec();
}

void CDialogDVSFunction::ImageContrastSensitivity()
{
	CDialogDVSImageContrastSensitivity ImageContrastSensitivity(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	ImageContrastSensitivity.exec();
}

void CDialogDVSFunction::AccompaniedPeakAndDelayedPeak()
{
	CDialogDVSAccompaniedPeakAndDelayedPeak AccompaniedPeakAndDelayedPeak(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	AccompaniedPeakAndDelayedPeak.exec();

}

void CDialogDVSFunction::SpatialResponseUniformity()
{
	CDialogDVSSpatialResponseUniformity SpatialResponseUniformity(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	SpatialResponseUniformity.exec();
}

void CDialogDVSFunction::BadPixel()
{
	CDialogDVSBadPixel BadPixel(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	BadPixel.exec();
}

void CDialogDVSFunction::Show()
{
	CDialogDVSShow Show(nullptr, m_pAPSAlgoInterface, m_pDVSAlgoInterface);
	Show.exec();
}
