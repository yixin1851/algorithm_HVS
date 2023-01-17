#include "DialogAPSDSNU.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogAPSDSNU::CDialogAPSDSNU(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditDSNUBlockSize->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditROIUp->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROIRight->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROILeft->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROIDown->setValidator(new QIntValidator(0, 100000, this));

	ui.lineEditROIUp->setEnabled(false);
	ui.lineEditROIRight->setEnabled(false);
	ui.lineEditROILeft->setEnabled(false);
	ui.lineEditROIDown->setEnabled(false);

	auto temp = m_pAPSAlgoInterface->GetActiveArea();
	ui.lineEditROIUp->setText(QString::number(temp.Up));
	ui.lineEditROILeft->setText(QString::number(temp.Left));
	ui.lineEditROIDown->setText(QString::number(temp.Down));
	ui.lineEditROIRight->setText(QString::number(temp.Right));

	ui.lineEditDSNUBlockSize->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nDSNUBlockSize));

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(DSNU()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

}


void CDialogAPSDSNU::DSNU()
{
	ui.widgetTableView->Clear();
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);

	ROIArea* roi = nullptr;
	ROIArea temp;

	auto tempThre = m_pAPSAlgoInterface->GetAlgorithmThre();
	tempThre.nDSNUBlockSize = ui.lineEditDSNUBlockSize->text().toUInt();
	m_pAPSAlgoInterface->SetAlgorithmThre(tempThre);

	if (ui.checkBoxROI->isChecked())
	{
		temp.Up = ui.lineEditROIUp->text().toUInt();
		temp.Down = ui.lineEditROIDown->text().toUInt();
		temp.Left = ui.lineEditROILeft->text().toUInt();
		temp.Right = ui.lineEditROIRight->text().toUInt();
		roi = &temp;
	}

	clock_t time = 0;
	auto start = clock();
	bRet = m_pAPSAlgoInterface->DSNU(nIndexStart, nNumber, roi, m_DSNUData);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;
		RowName << "";
		ColName << "RangeR" << "RangeG" << "RangeB" << "SignalMax" << "DeltaSignalMax" << "DeltaSignalCentreMax" << "DeltaSignalEdgeMax" << "DeltaSignalCornerMax";

		std::vector<std::vector<double>> Data(1);
		Data[0].push_back(m_DSNUData.RangeR);
		Data[0].push_back(m_DSNUData.RangeG);
		Data[0].push_back(m_DSNUData.RangeB);
		Data[0].push_back(m_DSNUData.SignalMax);
		Data[0].push_back(m_DSNUData.DeltaSignalMax);
		Data[0].push_back(m_DSNUData.DeltaSignalCentreMax);
		Data[0].push_back(m_DSNUData.DeltaSignalEdgeMax);
		Data[0].push_back(m_DSNUData.DeltaSignalCornerMax);

		ui.widgetTableView->SetData(RowName, ColName, Data);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogAPSDSNU::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toStdString() + "//DSNU.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "RangeR," << "RangeG," << "RangeB," << "SignalMax," << "DeltaSignalMax," << "DeltaSignalCentreMax," << "DeltaSignalEdgeMax," << "DeltaSignalCornerMax"<<std::endl;
			outfile << std::to_string(m_DSNUData.RangeR) << ",";
			outfile << std::to_string(m_DSNUData.RangeG) << ",";
			outfile << std::to_string(m_DSNUData.RangeB) << ",";
			outfile << std::to_string(m_DSNUData.SignalMax) << ",";
			outfile << std::to_string(m_DSNUData.DeltaSignalMax) << ",";
			outfile << std::to_string(m_DSNUData.DeltaSignalCentreMax) << ",";
			outfile << std::to_string(m_DSNUData.DeltaSignalEdgeMax) << ",";
			outfile << std::to_string(m_DSNUData.DeltaSignalCornerMax) <<std::endl;

			outfile.close();
		}
	}
}