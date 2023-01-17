#include "DialogAPSLinearity.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogAPSLinearity::CDialogAPSLinearity(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

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

	ui.tabWidgetView->insertTab(0, &m_widgetTableView, "LinearityResult");
	ui.tabWidgetView->insertTab(1, &m_widgetChartView, "Chart");

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(Linearity()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

}


void CDialogAPSLinearity::Linearity()
{
	bool bRet = true;
	QStringList IndexList = ui.lineEditIndex->text().split(",");
	QStringList ExpList = ui.lineEditExpTime->text().split(",");
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	m_widgetTableView.Clear();
	m_widgetChartView.Clear();
	clock_t time = 0;

	ROIArea* roi = nullptr;
	ROIArea tempROI;

	if (ui.checkBoxROI->isChecked())
	{
		tempROI.Up = ui.lineEditROIUp->text().toUInt();
		tempROI.Down = ui.lineEditROIDown->text().toUInt();
		tempROI.Left = ui.lineEditROILeft->text().toUInt();
		tempROI.Right = ui.lineEditROIRight->text().toUInt();
		roi = &tempROI;
	}

	if (IndexList.size() != 2 * ExpList.size() || 0 == ExpList.size())
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Index Or ExpTime Input Format Error!"));
	}
	else
	{
		std::vector<std::vector<double>> LightData;
		std::vector<double> ExpTime;
		std::vector<double>OneRes;

		for (uint32_t i = 0; i < ExpList.size(); i++)
		{
			uint32_t nIndexStart = IndexList[2 * i].toUInt();
			uint32_t nNumber = IndexList[2 * i + 1].toUInt();
			double dExpTime = ExpList[i].toDouble();
			ExpTime.push_back(dExpTime);

			if (m_pAPSAlgoInterface->DataMean(nIndexStart, nNumber, roi, OneRes))
			{
				LightData.push_back(OneRes);
			}
			else
			{
				bRet = false;
			}

		}
		if (bRet)
		{
			auto start = clock();
			bRet = m_pAPSAlgoInterface->Linearity(LightData, ExpTime, m_LinearityData);
			auto end = clock();
			time = end - start;
		}

		if (bRet)
		{
			ui.label_Res->setStyleSheet("color:green;");
			QString res = QString::number(time);
			ui.label_Res->setText(res);

			QStringList RowName, ColName;
			RowName << "K" << "B" << "LeMin" << "LeMax";
			ColName << "Gb1" << "Gb2" << "B1" << "B2" << "R1" << "R2" << "Gr1" << "Gr2";

			std::vector<std::vector<double>> Data(4);

			for (uint32_t nIndex = 0; nIndex < APSSubFrameIndex::SubFrameNum; nIndex++)
			{
				Data[0].push_back(m_LinearityData[nIndex].k);
				Data[1].push_back(m_LinearityData[nIndex].b);
				Data[2].push_back(m_LinearityData[nIndex].LeMin);
				Data[3].push_back(m_LinearityData[nIndex].LeMax);
			}

			m_widgetTableView.SetData(RowName, ColName, Data);

			QVector<double> XData;
			QVector<double> YData[APSSubFrameIndex::SubFrameNum];

			for (uint32_t nIndex = 0; nIndex < ExpTime.size(); nIndex++)
			{
				XData.push_back(ExpTime[nIndex]);
				for (uint32_t nChannel = 0; nChannel < APSSubFrameIndex::SubFrameNum; nChannel++)
				{
					YData[nChannel].push_back(LightData[nIndex][nChannel]);
				}
			}

			m_widgetChartView.SetLine("Gb1", XData, YData[Gb1]);
			m_widgetChartView.SetLine("Gb2", XData, YData[Gb2]);
			m_widgetChartView.SetLine("B1", XData, YData[B1]);
			m_widgetChartView.SetLine("B2", XData, YData[B2]);
			m_widgetChartView.SetLine("R1", XData, YData[R1]);
			m_widgetChartView.SetLine("R2", XData, YData[R2]);
			m_widgetChartView.SetLine("Gr1", XData, YData[Gr1]);
			m_widgetChartView.SetLine("Gr2", XData, YData[Gr2]);
		}
		else
		{
			ui.label_Res->setStyleSheet("color:red;");
			ui.label_Res->setText(tr("Fail!"));
		}
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogAPSLinearity::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toStdString() + "//Linearity.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "Gb1,Gb2,B1,B2,R1,R2,Gr1,Gr2" << std::endl;
			for (uint32_t nIndex = 0; nIndex < m_LinearityData.size(); nIndex++)
			{
				outfile << std::to_string(m_LinearityData[nIndex].k) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_LinearityData.size(); nIndex++)
			{
				outfile << std::to_string(m_LinearityData[nIndex].b) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_LinearityData.size(); nIndex++)
			{
				outfile << std::to_string(m_LinearityData[nIndex].LeMin) << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_LinearityData.size(); nIndex++)
			{
				outfile << std::to_string(m_LinearityData[nIndex].LeMax) << ",";
			}
			outfile << std::endl;

			outfile.close();
		}
	}
}