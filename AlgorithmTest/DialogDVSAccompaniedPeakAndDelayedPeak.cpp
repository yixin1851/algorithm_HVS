#include "DialogDVSAccompaniedPeakAndDelayedPeak.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogDVSAccompaniedPeakAndDelayedPeak::CDialogDVSAccompaniedPeakAndDelayedPeak(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditPeakNum->setValidator(new QIntValidator(1, 100000, this));

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(AccompaniedPeakAndDelayedPeak()), Qt::QueuedConnection);
}


void CDialogDVSAccompaniedPeakAndDelayedPeak::AccompaniedPeakAndDelayedPeak()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	uint32_t nPeakNum = ui.lineEditPeakNum->text().toUInt();
	uint32_t nLigtType = ui.comboBoxLightType->currentIndex();
	ui.widgetTableView->Clear();
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	clock_t time = 0;
	auto start = clock();
	bRet = m_pDVSAlgoInterface->AccompaniedPeakAndDelayedPeak(nIndexStart, nNumber, nullptr, nPeakNum, DVSLightTrigerType(nLigtType + 1), m_Data);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);
		QStringList RowName, ColName;
		std::vector<std::vector<double>> Data;
		ColName << "All" << "Gb" << "B" << "R" << "Gr" ;

		if ((nLigtType + 1) & 1)
		{
			std::vector<double> AccompaniedPeakEventsRatioOff;
			RowName << "AccompaniedPeakEventsRatio(Off)";
			AccompaniedPeakEventsRatioOff.push_back(m_Data.dAccompaniedPeakOffEventsRatio[SubFrameIndex::All]);
			AccompaniedPeakEventsRatioOff.push_back(m_Data.dAccompaniedPeakOffEventsRatio[SubFrameIndex::Gb]);
			AccompaniedPeakEventsRatioOff.push_back(m_Data.dAccompaniedPeakOffEventsRatio[SubFrameIndex::B]);
			AccompaniedPeakEventsRatioOff.push_back(m_Data.dAccompaniedPeakOffEventsRatio[SubFrameIndex::R]);
			AccompaniedPeakEventsRatioOff.push_back(m_Data.dAccompaniedPeakOffEventsRatio[SubFrameIndex::Gr]);
			Data.push_back(AccompaniedPeakEventsRatioOff);

			std::vector<double> DelayedPeakEventsRatioOff;
			RowName << "DelayedPeakEventsRatio(Off)";
			DelayedPeakEventsRatioOff.push_back(m_Data.dDelayedPeakOffEventsRatio[SubFrameIndex::All]);
			DelayedPeakEventsRatioOff.push_back(m_Data.dDelayedPeakOffEventsRatio[SubFrameIndex::Gb]);
			DelayedPeakEventsRatioOff.push_back(m_Data.dDelayedPeakOffEventsRatio[SubFrameIndex::B]);
			DelayedPeakEventsRatioOff.push_back(m_Data.dDelayedPeakOffEventsRatio[SubFrameIndex::R]);
			DelayedPeakEventsRatioOff.push_back(m_Data.dDelayedPeakOffEventsRatio[SubFrameIndex::Gr]);
			Data.push_back(DelayedPeakEventsRatioOff);

		}

		if ((nLigtType + 1) & 2)
		{
			std::vector<double> AccompaniedPeakEventsRatioOn;
			RowName << "AccompaniedPeakEventsRatio(On)";
			AccompaniedPeakEventsRatioOn.push_back(m_Data.dAccompaniedPeakOnEventsRatio[SubFrameIndex::All]);
			AccompaniedPeakEventsRatioOn.push_back(m_Data.dAccompaniedPeakOnEventsRatio[SubFrameIndex::Gb]);
			AccompaniedPeakEventsRatioOn.push_back(m_Data.dAccompaniedPeakOnEventsRatio[SubFrameIndex::B]);
			AccompaniedPeakEventsRatioOn.push_back(m_Data.dAccompaniedPeakOnEventsRatio[SubFrameIndex::R]);
			AccompaniedPeakEventsRatioOn.push_back(m_Data.dAccompaniedPeakOnEventsRatio[SubFrameIndex::Gr]);
			Data.push_back(AccompaniedPeakEventsRatioOn);

			std::vector<double> DelayedPeakEventsRatioOn;
			RowName << "DelayedPeakEventsRatio(On)";
			DelayedPeakEventsRatioOn.push_back(m_Data.dDelayedPeakOnEventsRatio[SubFrameIndex::All]);
			DelayedPeakEventsRatioOn.push_back(m_Data.dDelayedPeakOnEventsRatio[SubFrameIndex::Gb]);
			DelayedPeakEventsRatioOn.push_back(m_Data.dDelayedPeakOnEventsRatio[SubFrameIndex::B]);
			DelayedPeakEventsRatioOn.push_back(m_Data.dDelayedPeakOnEventsRatio[SubFrameIndex::R]);
			DelayedPeakEventsRatioOn.push_back(m_Data.dDelayedPeakOnEventsRatio[SubFrameIndex::Gr]);
			Data.push_back(DelayedPeakEventsRatioOn);
		}
		ui.widgetTableView->SetData(RowName, ColName, Data);

	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogDVSAccompaniedPeakAndDelayedPeak::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toLocal8Bit().toStdString() + "//AccompaniedPeakAndDelayedPeak.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "All" << "Gb" << "B" << "R" << "Gr"<<std::endl;

			outfile << std::to_string(m_Data.dAccompaniedPeakOffEventsRatio[SubFrameIndex::All]) << ",";
			outfile << std::to_string(m_Data.dAccompaniedPeakOffEventsRatio[SubFrameIndex::Gb]) << ",";
			outfile << std::to_string(m_Data.dAccompaniedPeakOffEventsRatio[SubFrameIndex::B]) << ",";
			outfile << std::to_string(m_Data.dAccompaniedPeakOffEventsRatio[SubFrameIndex::R]) << ",";
			outfile << std::to_string(m_Data.dAccompaniedPeakOffEventsRatio[SubFrameIndex::Gr]) << std::endl;

			outfile << std::to_string(m_Data.dDelayedPeakOffEventsRatio[SubFrameIndex::All]) << ",";
			outfile << std::to_string(m_Data.dDelayedPeakOffEventsRatio[SubFrameIndex::Gb]) << ",";
			outfile << std::to_string(m_Data.dDelayedPeakOffEventsRatio[SubFrameIndex::B]) << ",";
			outfile << std::to_string(m_Data.dDelayedPeakOffEventsRatio[SubFrameIndex::R]) << ",";
			outfile << std::to_string(m_Data.dDelayedPeakOffEventsRatio[SubFrameIndex::Gr]) << std::endl;

			outfile<<std::to_string(m_Data.dAccompaniedPeakOnEventsRatio[SubFrameIndex::All])<<",";
			outfile<<std::to_string(m_Data.dAccompaniedPeakOnEventsRatio[SubFrameIndex::Gb]) << ",";
			outfile<<std::to_string(m_Data.dAccompaniedPeakOnEventsRatio[SubFrameIndex::B]) << ",";
			outfile<<std::to_string(m_Data.dAccompaniedPeakOnEventsRatio[SubFrameIndex::R]) << ",";
			outfile<<std::to_string(m_Data.dAccompaniedPeakOnEventsRatio[SubFrameIndex::Gr]) << std::endl;

			outfile << std::to_string(m_Data.dDelayedPeakOnEventsRatio[SubFrameIndex::All]) << ",";
			outfile << std::to_string(m_Data.dDelayedPeakOnEventsRatio[SubFrameIndex::Gb]) << ",";
			outfile << std::to_string(m_Data.dDelayedPeakOnEventsRatio[SubFrameIndex::B]) << ",";
			outfile << std::to_string(m_Data.dDelayedPeakOnEventsRatio[SubFrameIndex::R]) << ",";
			outfile << std::to_string(m_Data.dDelayedPeakOnEventsRatio[SubFrameIndex::Gr]) << std::endl;

			outfile.close();
		}
	}
}