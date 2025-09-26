#include "DialogAPSOverallSystemGain.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogAPSOverallSystemGain::CDialogAPSOverallSystemGain(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditROIUp->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROIRight->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROILeft->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROIDown->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditBaseIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditBaseNumber->setValidator(new QIntValidator(0, 100000, this));

	ui.lineEditROIUp->setEnabled(false);
	ui.lineEditROIRight->setEnabled(false);
	ui.lineEditROILeft->setEnabled(false);
	ui.lineEditROIDown->setEnabled(false);

	auto temp = m_pAPSAlgoInterface->GetActiveArea();
	ui.lineEditROIUp->setText(QString::number(temp.Up));
	ui.lineEditROILeft->setText(QString::number(temp.Left));
	ui.lineEditROIDown->setText(QString::number(temp.Down));
	ui.lineEditROIRight->setText(QString::number(temp.Right));

	ui.tabWidgetView->insertTab(0, &m_widgetTableView, "OverallSystemGainResult");
	ui.tabWidgetView->insertTab(1, &m_widgetChartView, "Chart");

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(OverallSystemGain()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

}


void CDialogAPSOverallSystemGain::OverallSystemGain()
{
	bool bRet = true;
	QStringList IndexList = ui.lineEditIndex->text().split(",");
	uint32_t nBaseIndexStart = ui.lineEditBaseIndexStart->text().toUInt();
	uint32_t nBaseNumber = ui.lineEditBaseNumber->text().toUInt();
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

	if (IndexList.size() % 2 != 0 || 0 == IndexList.size())
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Index Input Format Error!"));
	}
	else
	{
		std::vector<APSTNoiseType> LightTNoise;
		std::vector<APSDataMeanType> LightMean;
		APSTNoiseType BaseTNoise;
		APSTNoiseType OneRes;
		APSDataMeanType OneDataRes;

		for (uint32_t i = 0; i < IndexList.size() / 2; i++)
		{
			uint32_t nIndexStart = IndexList[2 * i].toUInt();
			uint32_t nNumber = IndexList[2 * i + 1].toUInt();

			if (m_pAPSAlgoInterface->TNoise(nIndexStart, nNumber, roi, OneRes))
			{
				LightTNoise.push_back(OneRes);
			}
			else
			{
				bRet = false;
			}
			if (m_pAPSAlgoInterface->DataMean(nIndexStart, nNumber, roi, OneDataRes))
			{
				LightMean.push_back(OneDataRes);
			}
			else
			{
				bRet = false;
			}
		}
		if (bRet)
		{
			bRet = m_pAPSAlgoInterface->TNoise(nBaseIndexStart, nBaseNumber, roi, BaseTNoise);
			if(bRet)
			{
				auto start = clock();
				bRet = m_pAPSAlgoInterface->OverallSystemGain(LightTNoise, LightMean, BaseTNoise, m_GainK);
				auto end = clock();
				time = end - start;
			}
		}

		if (bRet)
		{
			ui.label_Res->setStyleSheet("color:green;");
			QString res = QString::number(time);
			ui.label_Res->setText(res);

			QStringList RowName, ColName;
			RowName << " ";
			if (m_GainK.SubFrameGainK.size() == 1)
			{
				ColName << "Total";
			}
			else if (m_GainK.SubFrameGainK.size() == 4)
			{
				ColName << "Gb" << "B" << "R" << "Gr";
			}
			else
			{
				ColName << "Gb1" << "Gb2" << "Gb3" << "Gb4" << "B1" << "B2" << "B3" << "B4" << "R1" << "R2" << "R3" << "R4" << "Gr1" << "Gr2" << "Gr3" << "Gr4";
			}
			std::vector<std::vector<double>> Data(1);

			for (uint32_t nIndex = 0; nIndex < m_GainK.SubFrameGainK.size(); nIndex++)
			{
				Data[0].push_back(m_GainK.SubFrameGainK[nIndex]);
			}

			m_widgetTableView.SetData(RowName, ColName, Data);

			std::vector<QVector<double>> XData(m_GainK.SubFrameGainK.size());
			std::vector <QVector<double>> YData(m_GainK.SubFrameGainK.size());

			for (uint32_t nIndex = 0; nIndex < LightTNoise.size(); nIndex++)
			{
				for (uint32_t nChannel = 0; nChannel < m_GainK.SubFrameGainK.size(); nChannel++)
				{
					XData[nChannel].push_back(LightMean[nIndex].SubFrameDataMean[nChannel]);

					YData[nChannel].push_back(LightTNoise[nIndex].SubFrameTNoiseData[nChannel].TempNoise * LightTNoise[nIndex].SubFrameTNoiseData[nChannel].TempNoise - BaseTNoise.SubFrameTNoiseData[nChannel].TempNoise * BaseTNoise.SubFrameTNoiseData[nChannel].TempNoise);
				}
			}

			if (m_GainK.SubFrameGainK.size() == 1)
			{
				m_widgetChartView.SetLine("Total", XData[0], YData[0]);
			}
			else if (m_GainK.SubFrameGainK.size() == 4)
			{
				m_widgetChartView.SetLine("Gb", XData[Gb], YData[Gb]);
				m_widgetChartView.SetLine("B",  XData[B],  YData[B]);
				m_widgetChartView.SetLine("R",  XData[R],  YData[R]);
				m_widgetChartView.SetLine("Gr", XData[Gr], YData[Gr]);
			}
			else
			{
				m_widgetChartView.SetLine("Gb1", XData[0], YData[0]);
				m_widgetChartView.SetLine("Gb2", XData[1], YData[1]);
				m_widgetChartView.SetLine("Gb3", XData[2], YData[2]);
				m_widgetChartView.SetLine("Gb4", XData[3], YData[3]);
				m_widgetChartView.SetLine("B1", XData[4], YData[4]);
				m_widgetChartView.SetLine("B2", XData[5], YData[5]);
				m_widgetChartView.SetLine("B3", XData[6], YData[6]);
				m_widgetChartView.SetLine("B4", XData[7], YData[7]);
				m_widgetChartView.SetLine("R1", XData[8], YData[8]);
				m_widgetChartView.SetLine("R2", XData[9], YData[9]);
				m_widgetChartView.SetLine("R3", XData[10], YData[10]);
				m_widgetChartView.SetLine("R4", XData[11], YData[11]);
				m_widgetChartView.SetLine("Gr1", XData[12], YData[12]);
				m_widgetChartView.SetLine("Gr2", XData[13], YData[13]);
				m_widgetChartView.SetLine("Gr3", XData[14], YData[14]);
				m_widgetChartView.SetLine("Gr4", XData[15], YData[15]);
			}
		}
		else
		{
			ui.label_Res->setStyleSheet("color:red;");
			ui.label_Res->setText(tr("Fail!"));
		}
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogAPSOverallSystemGain::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toLocal8Bit().toStdString() + "//OverallSystemGain.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			if (m_GainK.SubFrameGainK.size() == 1)
			{
				outfile << "Total" << std::endl;
			}
			if (m_GainK.SubFrameGainK.size() == 4)
			{
				outfile << "Gb,B,R,Gr" << std::endl;
			}
			else
			{
				outfile << "Gb1,Gb2,Gb3,Gb4,B1,B2,B3,B4,R1,R2,R3,R4,Gr1,Gr2,Gr3,Gr4" << std::endl;
			}
			for (uint32_t nIndex = 0; nIndex < m_GainK.SubFrameGainK.size(); nIndex++)
			{
				outfile << std::to_string(m_GainK.SubFrameGainK[nIndex]) << ",";
			}
			outfile << std::endl;

			outfile.close();
		}
	}
}
