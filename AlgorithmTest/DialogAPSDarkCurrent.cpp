#include "DialogAPSDarkCurrent.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogAPSDarkCurrent::CDialogAPSDarkCurrent(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
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

	ui.tabWidgetView->insertTab(0, &m_widgetTableView, "DarkCurrentResult");
	ui.tabWidgetView->insertTab(1, &m_widgetChartView, "Chart");

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(DarkCurrent()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

}


void CDialogAPSDarkCurrent::DarkCurrent()
{
	bool bRet = true;
	QStringList IndexList = ui.lineEditIndex->text().split(",");
	QStringList ExpList = ui.lineEditExpTime->text().split(",");
	bool bUseMeanFunc = ui.comboBoxMethod->currentIndex() == 0;
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
		std::vector<APSDataMeanType> DarkData;
		std::vector<APSTNoiseType> DarkTNoiseData;
		std::vector<double> ExpTime;
		APSDataMeanType OneDataMeanRes;
		APSTNoiseType OneTNoiseRes;

		for (uint32_t i = 0; i < ExpList.size(); i++)
		{
			uint32_t nIndexStart = IndexList[2 * i].toUInt();
			uint32_t nNumber = IndexList[2 * i + 1].toUInt();
			double dExpTime = ExpList[i].toDouble();
			ExpTime.push_back(dExpTime);

			if (bUseMeanFunc)
			{
				if (m_pAPSAlgoInterface->DataMean(nIndexStart, nNumber, roi, OneDataMeanRes))
				{
					DarkData.push_back(OneDataMeanRes);
				}
				else
				{
					bRet = false;
				}
			}
			else
			{
				if (m_pAPSAlgoInterface->TNoise(nIndexStart, nNumber, roi, OneTNoiseRes))
				{
					DarkTNoiseData.push_back(OneTNoiseRes);
				}
				else
				{
					bRet = false;
				}
			}
		}
		if (bRet)
		{
			auto start = clock();
			if (bUseMeanFunc)
			{
				bRet = m_pAPSAlgoInterface->DarkCurrent(DarkData, ExpTime, m_DarkCurrent);
			}
			else
			{
				bRet = m_pAPSAlgoInterface->DarkCurrent(DarkTNoiseData, ExpTime, m_DarkCurrent);
			}
			auto end = clock();
			time = end - start;
		}

		if (bRet)
		{
			ui.label_Res->setStyleSheet("color:green;");
			QString res = QString::number(time);
			ui.label_Res->setText(res);

			QStringList RowName, ColName;
			RowName << " ";
			if (m_DarkCurrent.SubFrameKValue.size() == 1)
			{
				ColName << "Total";
			}
			else if (m_DarkCurrent.SubFrameKValue.size() == 4)
			{
				ColName << "Gb" << "B" << "R" << "Gr";
			}
			else
			{
				ColName << "Gb1" << "Gb2" << "Gb3" << "Gb4" << "B1" << "B2" << "B3" << "B4" << "R1" << "R2" << "R3" << "R4" << "Gr1" << "Gr2" << "Gr3" << "Gr4";
			}

			std::vector<std::vector<double>> Data(1);

			for (uint32_t nIndex = 0; nIndex < m_DarkCurrent.SubFrameKValue.size(); nIndex++)
			{
				Data[0].push_back(m_DarkCurrent.SubFrameKValue[nIndex]);
			}

			m_widgetTableView.SetData(RowName, ColName, Data);

			QVector<double> XData;
			std::vector<QVector<double>> YData(m_DarkCurrent.SubFrameKValue.size());

			for (uint32_t nIndex = 0; nIndex < ExpTime.size(); nIndex++)
			{
				XData.push_back(ExpTime[nIndex]);
				for (uint32_t nChannel = 0; nChannel < m_DarkCurrent.SubFrameKValue.size(); nChannel++)
				{
					if (bUseMeanFunc)
					{
						YData[nChannel].push_back(DarkData[nIndex].SubFrameDataMean[nChannel]);
					}
					else
					{
						YData[nChannel].push_back(DarkTNoiseData[nIndex].SubFrameTNoiseData[nChannel].TempNoise * DarkTNoiseData[nIndex].SubFrameTNoiseData[nChannel].TempNoise);
					}
				}
			}

			if (m_DarkCurrent.SubFrameKValue.size() == 1)
			{
				m_widgetChartView.SetLine("Total", XData, YData[0]);
			}
			else if (m_DarkCurrent.SubFrameKValue.size() == 4)
			{
				m_widgetChartView.SetLine("Gb", XData, YData[Gb]);
				m_widgetChartView.SetLine("B", XData, YData[B]);
				m_widgetChartView.SetLine("R", XData, YData[R]);
				m_widgetChartView.SetLine("Gr", XData, YData[Gr]);
			}
			else
			{
				m_widgetChartView.SetLine("Gb1", XData, YData[0]);
				m_widgetChartView.SetLine("Gb2", XData, YData[1]);
				m_widgetChartView.SetLine("Gb3", XData, YData[2]);
				m_widgetChartView.SetLine("Gb4", XData, YData[3]);
				m_widgetChartView.SetLine("B1", XData, YData[4]);
				m_widgetChartView.SetLine("B2", XData, YData[5]);
				m_widgetChartView.SetLine("B3", XData, YData[6]);
				m_widgetChartView.SetLine("B4", XData, YData[7]);
				m_widgetChartView.SetLine("R1", XData, YData[8]);
				m_widgetChartView.SetLine("R2", XData, YData[9]);
				m_widgetChartView.SetLine("R3", XData, YData[10]);
				m_widgetChartView.SetLine("R4", XData, YData[11]);
				m_widgetChartView.SetLine("Gr1", XData, YData[12]);
				m_widgetChartView.SetLine("Gr2", XData, YData[13]);
				m_widgetChartView.SetLine("Gr3", XData, YData[14]);
				m_widgetChartView.SetLine("Gr4", XData, YData[15]);

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

void CDialogAPSDarkCurrent::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toLocal8Bit().toStdString() + "//DarkCurrent.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			if (m_DarkCurrent.SubFrameKValue.size() == 1)
			{
				outfile << "Total" << std::endl;
			}
			else if (m_DarkCurrent.SubFrameKValue.size() == 4)
			{
				outfile << "Gb,B,R,Gr" << std::endl;
			}
			else
			{
				outfile << "Gb1,Gb2,Gb3,Gb4,B1,B2,B3,B4,R1,R2,R3,R4,Gr1,Gr2,Gr3,Gr4" << std::endl;
			}
			for (uint32_t nIndex = 0; nIndex < m_DarkCurrent.SubFrameKValue.size(); nIndex++)
			{
				outfile << std::to_string(m_DarkCurrent.SubFrameKValue[nIndex]) << ",";
			}
			outfile << std::endl;

			outfile.close();
		}
	}
}