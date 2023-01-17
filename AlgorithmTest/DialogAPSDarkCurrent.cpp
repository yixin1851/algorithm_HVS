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
		std::vector<std::vector<double>> DarkData;
		std::vector<double> ExpTime;
		std::vector<double>OneRes;

		for (uint32_t i = 0; i < ExpList.size(); i++)
		{
			uint32_t nIndexStart = IndexList[2 * i].toUInt();
			uint32_t nNumber = IndexList[2 * i + 1].toUInt();
			double dExpTime = ExpList[i].toDouble();
			ExpTime.push_back(dExpTime);

			if (bUseMeanFunc)
			{
				if (m_pAPSAlgoInterface->DataMean(nIndexStart, nNumber, roi, OneRes))
				{
					DarkData.push_back(OneRes);
				}
				else
				{
					bRet = false;
				}
			}
			else
			{
				if (m_pAPSAlgoInterface->TNoise(nIndexStart, nNumber, roi, OneRes))
				{
					DarkData.push_back(OneRes);
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
			bRet = m_pAPSAlgoInterface->DarkCurrent(DarkData, ExpTime, bUseMeanFunc, m_DarkCurrent);
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
			ColName << "Gb1" << "Gb2" << "B1" << "B2" << "R1" << "R2" << "Gr1" << "Gr2";

			std::vector<std::vector<double>> Data(1);

			for (uint32_t nIndex = 0; nIndex < APSSubFrameIndex::SubFrameNum; nIndex++)
			{
				Data[0].push_back(m_DarkCurrent[nIndex]);
			}

			m_widgetTableView.SetData(RowName, ColName, Data);

			QVector<double> XData;
			QVector<double> YData[APSSubFrameIndex::SubFrameNum];

			for (uint32_t nIndex = 0; nIndex < ExpTime.size(); nIndex++)
			{
				XData.push_back(ExpTime[nIndex]);
				for (uint32_t nChannel = 0; nChannel < APSSubFrameIndex::SubFrameNum; nChannel++)
				{
					if (bUseMeanFunc)
					{
						YData[nChannel].push_back(DarkData[nIndex][nChannel]);
					}
					else
					{
						YData[nChannel].push_back(DarkData[nIndex][nChannel] * DarkData[nIndex][nChannel]);
					}
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

void CDialogAPSDarkCurrent::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toStdString() + "//DarkCurrent.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "Gb1,Gb2,B1,B2,R1,R2,Gr1,Gr2" << std::endl;
			for (uint32_t nIndex = 0; nIndex < m_DarkCurrent.size(); nIndex++)
			{
				outfile << std::to_string(m_DarkCurrent[nIndex]) << ",";
			}
			outfile << std::endl;

			outfile.close();
		}
	}
}