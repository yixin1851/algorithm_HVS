#include "DialogDVSStationaryNoise.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogStationaryNoise::CDialogStationaryNoise(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(StationaryNoise()), Qt::QueuedConnection);
}

CDialogStationaryNoise::~CDialogStationaryNoise()
{

}

void CDialogStationaryNoise::StationaryNoise()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	ui.widgetTableView->Clear();
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	clock_t time = 0;
	auto start = clock();
	bRet = m_pDVSAlgoInterface->StationaryNoise(nIndexStart, nNumber, m_Data);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;
		RowName << "";
		ColName << "Mean(All)" << " Mean(On)"<< "Mean(Off)" << "Std(All)" << "Std(On)" << "Std(Off)" << "RowSNoise" << "ColSNoise";

		std::vector<std::vector<double>> Data(1);
		Data[0].resize(8);
		Data[0][0] = m_Data.dStationaryNoiseMeanAll;
		Data[0][1] = m_Data.dStationaryNoiseMeanOn;
		Data[0][2] = m_Data.dStationaryNoiseMeanOff;
		Data[0][3] = m_Data.dStationaryNoiseStdAll;
		Data[0][4] = m_Data.dStationaryNoiseStdOn;
		Data[0][5] = m_Data.dStationaryNoiseStdOff;
		Data[0][6] = m_Data.dStationaryRowSNoise;
		Data[0][7] = m_Data.dStationaryColSNoise;

		ui.widgetTableView->SetData(RowName, ColName, Data);

	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogStationaryNoise::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toStdString() + "//StationaryNoise.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "StationaryNoiseMeanAll, StationaryNoiseMeanOn, StationaryNoiseMeanOff, StationaryNoiseStdAll, StationaryNoiseStdOn, StationaryNoiseStdOff, StationaryRowSNoise, StationaryColSNoise" << std::endl;
			outfile << std::to_string(m_Data.dStationaryNoiseMeanAll) << ",";
			outfile << std::to_string(m_Data.dStationaryNoiseMeanOn) << ",";
			outfile << std::to_string(m_Data.dStationaryNoiseMeanOff) << ",";
			outfile << std::to_string(m_Data.dStationaryNoiseStdAll) << ",";
			outfile << std::to_string(m_Data.dStationaryNoiseStdOn) << ",";
			outfile << std::to_string(m_Data.dStationaryNoiseStdOff) << ",";
			outfile << std::to_string(m_Data.dStationaryRowSNoise) << ",";
			outfile << std::to_string(m_Data.dStationaryColSNoise) << std::endl;

			outfile.close();
		}
	}
}