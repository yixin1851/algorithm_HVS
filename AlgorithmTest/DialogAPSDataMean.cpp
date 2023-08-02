#include "DialogAPSDataMean.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogAPSDataMean::CDialogAPSDataMean(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
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
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(DataMean()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

}


void CDialogAPSDataMean::DataMean()
{
	ui.widgetTableView->Clear();
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
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
	bRet = m_pAPSAlgoInterface->DataMean(nIndexStart, nNumber, roi, m_DataMean);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;
		RowName << "";
		if (m_DataMean.SubFrameDataMean.size() == 4)
		{
			ColName << "Gb" << "B" << "R" << "Gr";
		}
		else
		{
			ColName << "Gb1" << "Gb2" << "Gb3" << "Gb4" << "B1" << "B2" << "B3" << "B4" << "R1" << "R2" << "R3" << "R4" << "Gr1" << "Gr2" << "Gr3" << "Gr4";
		}

		std::vector<std::vector<double>> Data;
		Data.push_back(m_DataMean.SubFrameDataMean);
		ui.widgetTableView->SetData(RowName, ColName, Data);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogAPSDataMean::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toLocal8Bit().toStdString() + "//DataMean.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			if (m_DataMean.SubFrameDataMean.size() == 4)
			{
				outfile << "Gb,B,R,Gr" << std::endl;
			}
			else
			{
				outfile << "Gb1,Gb2,Gb3,Gb4,B1,B2,B3,B4,R1,R2,R3,R4,Gr1,Gr2,Gr3,Gr4" << std::endl;
			}
			for (uint32_t nIndex = 0; nIndex < m_DataMean.SubFrameDataMean.size(); nIndex++)
			{
				outfile << std::to_string(m_DataMean.SubFrameDataMean[nIndex]) << ",";
			}
			outfile << std::endl;

			outfile.close();
		}
	}
}