#include "DialogDVSImageContrastSensitivity.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogDVSImageContrastSensitivity::CDialogDVSImageContrastSensitivity(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditPeakNum->setValidator(new QIntValidator(1, 100000, this));

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(ImageContrastSensitivity()), Qt::QueuedConnection);
}


void CDialogDVSImageContrastSensitivity::ImageContrastSensitivity()
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
	bRet = m_pDVSAlgoInterface->ImageContrastSensitivity(nIndexStart, nNumber, nullptr, nPeakNum, LightTrigerType(nLigtType + 1), m_Data);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);
		QStringList RowName, ColName;
		std::vector<std::vector<double>> Data;

		ColName << "EventsRatio(All)" << "EventsRatio(Gb)" << "EventsRatio(B)" << "EventsRatio(R)" << "EventsRatio(Gr)" << "R/Gb" << "B/Gb" << "Gr/Gb";

		if ((nLigtType + 1) & 1)
		{
			std::vector<double> OffData;
			RowName << "OffEvents";
			OffData.push_back(m_Data.OffEventsRatio[DVSSubFrameIndex::All]);
			OffData.push_back(m_Data.OffEventsRatio[DVSSubFrameIndex::Gb]);
			OffData.push_back(m_Data.OffEventsRatio[DVSSubFrameIndex::B]);
			OffData.push_back(m_Data.OffEventsRatio[DVSSubFrameIndex::R]);
			OffData.push_back(m_Data.OffEventsRatio[DVSSubFrameIndex::Gr]);
			OffData.push_back(m_Data.R_Gb_OffEventsRatio);
			OffData.push_back(m_Data.B_Gb_OffEventsRatio);
			OffData.push_back(m_Data.Gr_Gb_OffEventsRatio);
			Data.push_back(OffData);
		}

		if ((nLigtType + 1) & 2)
		{
			std::vector<double> OnData;
			RowName << "OnEvents";
			OnData.push_back(m_Data.OnEventsRatio[DVSSubFrameIndex::All]);
			OnData.push_back(m_Data.OnEventsRatio[DVSSubFrameIndex::Gb]);
			OnData.push_back(m_Data.OnEventsRatio[DVSSubFrameIndex::B]);
			OnData.push_back(m_Data.OnEventsRatio[DVSSubFrameIndex::R]);
			OnData.push_back(m_Data.OnEventsRatio[DVSSubFrameIndex::Gr]);
			OnData.push_back(m_Data.R_Gb_OnEventsRatio);
			OnData.push_back(m_Data.B_Gb_OnEventsRatio);
			OnData.push_back(m_Data.Gr_Gb_OnEventsRatio);
			Data.push_back(OnData);
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

void CDialogDVSImageContrastSensitivity::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toStdString() + "//ImageContrastSensitivity.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "EventsRatio(All)," << "EventsRatio(Gb)," << "EventsRatio(B)," << "EventsRatio(R)," << "EventsRatio(Gr)," << "R/Gb," << "B/Gb," << "Gr/Gb" << std::endl;
			outfile << std::to_string(m_Data.OffEventsRatio[DVSSubFrameIndex::All]) << ",";
			outfile << std::to_string(m_Data.OffEventsRatio[DVSSubFrameIndex::Gb]) << ",";
			outfile << std::to_string(m_Data.OffEventsRatio[DVSSubFrameIndex::B]) << ",";
			outfile << std::to_string(m_Data.OffEventsRatio[DVSSubFrameIndex::R]) << ",";
			outfile << std::to_string(m_Data.OffEventsRatio[DVSSubFrameIndex::Gr]) << ",";
			outfile << std::to_string(m_Data.R_Gb_OffEventsRatio) << ",";
			outfile << std::to_string(m_Data.B_Gb_OffEventsRatio) << ",";
			outfile << std::to_string(m_Data.Gr_Gb_OffEventsRatio) << std::endl;

			outfile << std::to_string(m_Data.OnEventsRatio[DVSSubFrameIndex::All]) << ",";
			outfile << std::to_string(m_Data.OnEventsRatio[DVSSubFrameIndex::Gb]) << ",";
			outfile << std::to_string(m_Data.OnEventsRatio[DVSSubFrameIndex::B]) << ",";
			outfile << std::to_string(m_Data.OnEventsRatio[DVSSubFrameIndex::R]) << ",";
			outfile << std::to_string(m_Data.OnEventsRatio[DVSSubFrameIndex::Gr]) << ",";
			outfile << std::to_string(m_Data.R_Gb_OnEventsRatio) << ",";
			outfile << std::to_string(m_Data.B_Gb_OnEventsRatio) << ",";
			outfile << std::to_string(m_Data.Gr_Gb_OnEventsRatio) << std::endl;

			outfile.close();
		}
	}
}