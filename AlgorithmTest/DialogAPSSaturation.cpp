#include "DialogAPSSaturation.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogAPSSaturation::CDialogAPSSaturation(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));
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

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(Saturation()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

}


void CDialogAPSSaturation::Saturation()
{
	ui.widgetTableView->Clear();
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	uint32_t nChannel = ui.comboBoxSubChannel->currentIndex();
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);

	ROIArea* roi = nullptr;
	ROIArea temp;

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
	bRet = m_pAPSAlgoInterface->Saturation(nIndexStart, nNumber, roi, SubFrameIndex(nChannel), m_Data);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;
		RowName << "";
		ColName << "SaturationMean"<<"SaturationTNoise"<<"SaturationSNR";

		std::vector<std::vector<double>> Data(1);
		Data[0].push_back(m_Data.SaturationMean);
		Data[0].push_back(m_Data.SaturationTNoise);
		Data[0].push_back(m_Data.SaturationSNR);
		ui.widgetTableView->SetData(RowName, ColName, Data);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogAPSSaturation::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toStdString() + "//Saturation.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "SaturationMean," << "SaturationTNoise," << "SaturationSNR" << std::endl;

			outfile << std::to_string(m_Data.SaturationMean) << ",";
			outfile << std::to_string(m_Data.SaturationTNoise) << ",";
			outfile << std::to_string(m_Data.SaturationSNR) << std::endl;

			outfile.close();
		}
	}
}