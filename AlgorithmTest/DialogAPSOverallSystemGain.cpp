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
		std::vector<std::vector<double>> LightTNoise;
		std::vector<std::vector<double>> LightMean;
		std::vector<double> BaseTNoise;
		std::vector<double>OneRes;

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
			if (m_pAPSAlgoInterface->DataMean(nIndexStart, nNumber, roi, OneRes))
			{
				LightMean.push_back(OneRes);
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
			ColName << "Gb1" << "Gb2" << "B1" << "B2" << "R1" << "R2" << "Gr1" << "Gr2";

			std::vector<std::vector<double>> Data(1);

			for (uint32_t nIndex = 0; nIndex < APSSubFrameIndex::SubFrameNum; nIndex++)
			{
				Data[0].push_back(m_GainK[nIndex]);
			}

			m_widgetTableView.SetData(RowName, ColName, Data);

			QVector<double> XData[APSSubFrameIndex::SubFrameNum];
			QVector<double> YData[APSSubFrameIndex::SubFrameNum];

			for (uint32_t nIndex = 0; nIndex < LightTNoise.size(); nIndex++)
			{
				for (uint32_t nChannel = 0; nChannel < APSSubFrameIndex::SubFrameNum; nChannel++)
				{
					XData[nChannel].push_back(LightMean[nIndex][nChannel]);

					YData[nChannel].push_back(LightTNoise[nIndex][nChannel] * LightTNoise[nIndex][nChannel] - BaseTNoise[nChannel]* BaseTNoise[nChannel]);
				}
			}

			m_widgetChartView.SetLine("Gb1", XData[Gb1], YData[Gb1]);
			m_widgetChartView.SetLine("Gb2", XData[Gb2], YData[Gb2]);
			m_widgetChartView.SetLine("B1",  XData[B1], YData[B1]);
			m_widgetChartView.SetLine("B2",  XData[B2], YData[B2]);
			m_widgetChartView.SetLine("R1",  XData[R1], YData[R1]);
			m_widgetChartView.SetLine("R2",  XData[R2], YData[R2]);
			m_widgetChartView.SetLine("Gr1", XData[Gr1], YData[Gr1]);
			m_widgetChartView.SetLine("Gr2", XData[Gr2], YData[Gr2]);
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
		std::string strFile = dir.toStdString() + "//OverallSystemGain.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "Gb1,Gb2,B1,B2,R1,R2,Gr1,Gr2" << std::endl;
			for (uint32_t nIndex = 0; nIndex < m_GainK.size(); nIndex++)
			{
				outfile << std::to_string(m_GainK[nIndex]) << ",";
			}
			outfile << std::endl;

			outfile.close();
		}
	}
}
