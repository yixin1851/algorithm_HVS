#include "DialogAPSShading.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogAPSShading::CDialogAPSShading(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
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
	ui.lineEditOCFindRadius->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditShadingTestRadius->setValidator(new QIntValidator(1, 100000, this));

	ui.lineEditROIUp->setEnabled(false);
	ui.lineEditROIRight->setEnabled(false);
	ui.lineEditROILeft->setEnabled(false);
	ui.lineEditROIDown->setEnabled(false);

	auto temp = m_pAPSAlgoInterface->GetActiveArea();
	ui.lineEditROIUp->setText(QString::number(temp.Up));
	ui.lineEditROILeft->setText(QString::number(temp.Left));
	ui.lineEditROIDown->setText(QString::number(temp.Down));
	ui.lineEditROIRight->setText(QString::number(temp.Right));

	ui.lineEditOCFindRadius->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nOpticalFindRadius));
	ui.lineEditShadingTestRadius->setText(QString::number(m_pAPSAlgoInterface->GetAlgorithmThre().nShadingTestRadius));

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(Shading()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

}


void CDialogAPSShading::Shading()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	ui.widgetTableView->Clear();
	ui.widgetTableView_2->Clear();

	ROIArea* roi = nullptr;
	ROIArea tempROI;

	auto tempThre = m_pAPSAlgoInterface->GetAlgorithmThre();
	tempThre.nOpticalFindRadius = ui.lineEditOCFindRadius->text().toUInt();
	tempThre.nShadingTestRadius = ui.lineEditShadingTestRadius->text().toUInt();
	m_pAPSAlgoInterface->SetAlgorithmThre(tempThre);

	if (ui.checkBoxROI->isChecked())
	{
		tempROI.Up = ui.lineEditROIUp->text().toUInt();
		tempROI.Down = ui.lineEditROIDown->text().toUInt();
		tempROI.Left = ui.lineEditROILeft->text().toUInt();
		tempROI.Right = ui.lineEditROIRight->text().toUInt();
		roi = &tempROI;
	}

	clock_t time = 0;
	auto start = clock();
	bRet = m_pAPSAlgoInterface->Shading(nIndexStart, nNumber, roi, m_Data);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;
		RowName << "LumaShading(Left-Top)" << "LumaShading(Left-Bottom)" << "LumaShading(Right-Top)" << "LumaShading(Right-Bottom)";
		ColName << "Gb1" << "Gb2" << "B1" << "B2" << "R1" << "R2" << "Gr1" << "Gr2";

		std::vector<std::vector<double>> Data(4);

		for (uint32_t nIndex = 0; nIndex < APSSubFrameIndex::SubFrameNum; nIndex++)
		{
			Data[0].push_back(m_Data.LumaShadingLT[nIndex]);
			Data[1].push_back(m_Data.LumaShadingLB[nIndex]);
			Data[2].push_back(m_Data.LumaShadingRT[nIndex]);
			Data[3].push_back(m_Data.LumaShadingRB[nIndex]);
		}
		ui.widgetTableView->SetData(RowName, ColName, Data);

		QStringList RowName2, ColName2;
		ColName2 << "OpticalCenterRow" << "OpticalCenterCol" << "R/Gb" << "B/Gb" << "Gr/Gb";
		RowName2 << "";
		std::vector<std::vector<double>> Data2(1);
		Data2[0].push_back(m_Data.CenterRow);
		Data2[0].push_back(m_Data.CenterCol);
		Data2[0].push_back(m_Data.R_Gb_Ratio);
		Data2[0].push_back(m_Data.B_Gb_Ratio);
		Data2[0].push_back(m_Data.Gr_Gb_Ratio);
		ui.widgetTableView_2->SetData(RowName2, ColName2, Data2);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogAPSShading::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toStdString() + "//Shading.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "Gb1,Gb2,B1,B2,R1,R2,Gr1,Gr2" << std::endl;
			for (uint32_t nIndex = 0; nIndex < APSSubFrameIndex::SubFrameNum; nIndex++)
			{
				outfile << std::to_string(m_Data.LumaShadingLT[nIndex]) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < APSSubFrameIndex::SubFrameNum; nIndex++)
			{
				outfile << std::to_string(m_Data.LumaShadingLB[nIndex]) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < APSSubFrameIndex::SubFrameNum; nIndex++)
			{
				outfile << std::to_string(m_Data.LumaShadingRT[nIndex]) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < APSSubFrameIndex::SubFrameNum; nIndex++)
			{
				outfile << std::to_string(m_Data.LumaShadingRB[nIndex]) << ",";
			}
			outfile << std::endl;

			outfile << "OpticalCenterRow," << "OpticalCenterCol," << "R/Gb," << "B/Gb," << "Gr/Gb" <<std::endl;
			outfile << std::to_string(m_Data.CenterRow) << "," << std::to_string(m_Data.CenterCol) << ",";
			outfile << std::to_string(m_Data.R_Gb_Ratio) << "," << std::to_string(m_Data.B_Gb_Ratio) << "," << std::to_string(m_Data.Gr_Gb_Ratio) << std::endl;

			outfile.close();
		}
	}
}