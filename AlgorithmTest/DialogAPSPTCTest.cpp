#include "DialogAPSPTCTest.h"
#include <fstream>

CDialogAPSPTC::CDialogAPSPTC(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditROIUp->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROIRight->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROILeft->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditROIDown->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditIndex->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditStep->setValidator(new QIntValidator(0, 100000, this));

	ui.lineEditROIUp->setEnabled(false);
	ui.lineEditROIRight->setEnabled(false);
	ui.lineEditROILeft->setEnabled(false);
	ui.lineEditROIDown->setEnabled(false);

	auto temp = m_pAPSAlgoInterface->GetActiveArea();
	ui.lineEditROIUp->setText(QString::number(temp.Up));
	ui.lineEditROILeft->setText(QString::number(temp.Left));
	ui.lineEditROIDown->setText(QString::number(temp.Down));
	ui.lineEditROIRight->setText(QString::number(temp.Right));

	ui.lineEditStep->setText(QString::number(1));

	if (1 == (m_pAPSAlgoInterface->GetCode() & 1))
	{
		ui.tabWidgetView->insertTab(0, &m_widgetTableView[0], "Gb1");
		ui.tabWidgetView->insertTab(1, &m_widgetTableView[1], "Gb2");
		ui.tabWidgetView->insertTab(2, &m_widgetTableView[2], "Gb3");
		ui.tabWidgetView->insertTab(3, &m_widgetTableView[3], "Gb4");
		ui.tabWidgetView->insertTab(4, &m_widgetTableView[4], "B1");
		ui.tabWidgetView->insertTab(5, &m_widgetTableView[5], "B2");
		ui.tabWidgetView->insertTab(6, &m_widgetTableView[6], "B3");
		ui.tabWidgetView->insertTab(7, &m_widgetTableView[7], "B4");
		ui.tabWidgetView->insertTab(8, &m_widgetTableView[8], "R1");
		ui.tabWidgetView->insertTab(9, &m_widgetTableView[9], "R2");
		ui.tabWidgetView->insertTab(10, &m_widgetTableView[10], "R3");
		ui.tabWidgetView->insertTab(11, &m_widgetTableView[11], "R4");
		ui.tabWidgetView->insertTab(12, &m_widgetTableView[12], "Gr1");
		ui.tabWidgetView->insertTab(13, &m_widgetTableView[13], "Gr2");
		ui.tabWidgetView->insertTab(14, &m_widgetTableView[14], "Gr3");
		ui.tabWidgetView->insertTab(15, &m_widgetTableView[15], "Gr4");
		ui.tabWidgetView->insertTab(16, &m_widgetTableView[16], "All");

	}
	else
	{
		ui.tabWidgetView->insertTab(0, &m_widgetTableView[0], "Gb");
		ui.tabWidgetView->insertTab(1, &m_widgetTableView[1], "B");
		ui.tabWidgetView->insertTab(2, &m_widgetTableView[2], "R");
		ui.tabWidgetView->insertTab(3, &m_widgetTableView[3], "Gr");
		ui.tabWidgetView->insertTab(4, &m_widgetTableView[4], "All");
	}


	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(PTC()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

	connect(ui.pushButtonBrowser_2, SIGNAL(clicked()), this, SLOT(MultiBrowser()));
}

bool CDialogAPSPTC::ReadFile(QDir DataDir, QStringList DataList, int nSaveIndex, int nFileIndex)
{
	bool bRet = true;
	std::ifstream infile;
	std::string strFileName = (DataDir.absolutePath() + "/" + DataList[nFileIndex]).toLocal8Bit().toStdString();
	infile.open(strFileName, std::ios::binary | std::ios::in);
	if (!infile.fail())
	{
		infile.seekg(0, std::ios::end);
		uint64_t length = infile.tellg();
		infile.seekg(0, std::ios::beg);
		uint8_t* pRawData = new uint8_t[length];
		infile.read((char*)pRawData, length);
		infile.close();

		bRet = m_pAPSAlgoInterface->ImportRawData(pRawData, length, nSaveIndex, 1, false);
		delete[] pRawData;
	}
	else
	{
		bRet = false;
	}
	return bRet;
}

void CDialogAPSPTC::PTC()
{
	bool bRet = true;
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	m_widgetChartView.Clear();

	for (int i = 0; i < 17; i++)
	{
		m_widgetTableView[i].Clear();
	}
	m_PTCData.clear();

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

	uint32_t nIndex = ui.lineEditIndex->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	uint32_t nStep = ui.lineEditStep->text().toUInt();

	auto start = clock();

	if (ui.checkBoxImportData->isChecked())
	{
		QDir DataDir(ui.lineEditDataFile_2->text());
		ui.label_Res->setText(tr(" "));
		clock_t time = 0;

		if (DataDir.exists())
		{
			QString filtername = "*.raw";
			QStringList filter;
			filter << filtername;
			DataDir.setNameFilters(filter);
			QStringList DataList = DataDir.entryList(filter, QDir::Files, QDir::Time | QDir::Reversed);

			QCollator collator;
			collator.setNumericMode(true); // 数字模式
			collator.setCaseSensitivity(Qt::CaseInsensitive); // 大小写敏感
			std::sort(DataList.begin(), DataList.end(), collator);///< 使用std::sort函数

			if (DataList.size() < nNumber + nIndex)
			{
				ui.label_Res->setStyleSheet("color:red;");
				ui.label_Res->setText(tr("Files not enough!"));
				ui.pushButtonStart->setEnabled(true);
				return;
			}

			for (uint32_t n = 0; n < nNumber; n += nStep)
			{
				for (uint32_t j = 0; j < nStep; j++)
				{
					if (!ReadFile(DataDir, DataList, j, n + nIndex + j))
					{
						ui.label_Res->setStyleSheet("color:red;");
						ui.label_Res->setText(tr("Read Data Fail!"));
						ui.pushButtonStart->setEnabled(true);
					}
				}
				APSDataMeanType dataMean;
				APSTNoiseType tNoise;

				bRet = m_pAPSAlgoInterface->DataMean(0, nStep, roi, dataMean)
					&& m_pAPSAlgoInterface->TNoise(0, nStep, roi, tNoise);
				if (!bRet)
				{
					break;
				}
				else
				{
					m_PTCData.push_back({ tNoise, dataMean });
				}

			}

		}
		else
		{
			ui.label_Res->setStyleSheet("color:red;");
			ui.label_Res->setText(tr("Dir not exist!"));
			ui.pushButtonStart->setEnabled(true);
			return;
		}
	}
	else
	{
		for (uint32_t n = 0; n < nNumber; n += nStep)
		{
			APSDataMeanType dataMean;
			APSTNoiseType tNoise;

			bRet = m_pAPSAlgoInterface->DataMean(nIndex + n, nStep, roi, dataMean)
				&& m_pAPSAlgoInterface->TNoise(nIndex + n, nStep, roi, tNoise);
			if (!bRet)
			{
				break;
			}
			else
			{
				m_PTCData.push_back({ tNoise, dataMean });
			}

		}
	}
	auto end = clock();
	time = end - start;

	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;

		std::vector<double> DataMean[17], TempNoise[17], RowTemp[16], ColTemp[16], PixelTemp[16], TempRNRatio[16], TempCNRatio[16];

		uint32_t nSubFrame = m_PTCData[0].datamean.SubFrameDataMean.size();

		for (uint32_t n = 0; n < m_PTCData.size(); n++)
		{
			ColName << QString::number(n + 1);

			for (uint32_t j = 0; j < nSubFrame; j++)
			{
				DataMean[j].push_back(m_PTCData[n].datamean.SubFrameDataMean[j]);
				TempNoise[j].push_back(m_PTCData[n].tnoise.SubFrameTNoiseData[j].TempNoise * m_PTCData[n].tnoise.SubFrameTNoiseData[j].TempNoise);
				RowTemp[j].push_back(m_PTCData[n].tnoise.SubFrameTNoiseData[j].RowTemp * m_PTCData[n].tnoise.SubFrameTNoiseData[j].RowTemp);
				ColTemp[j].push_back(m_PTCData[n].tnoise.SubFrameTNoiseData[j].ColTemp * m_PTCData[n].tnoise.SubFrameTNoiseData[j].ColTemp);
				PixelTemp[j].push_back(m_PTCData[n].tnoise.SubFrameTNoiseData[j].PixelTemp * m_PTCData[n].tnoise.SubFrameTNoiseData[j].PixelTemp);
				TempRNRatio[j].push_back(m_PTCData[n].tnoise.SubFrameTNoiseData[j].TempRNRatio);
				TempCNRatio[j].push_back(m_PTCData[n].tnoise.SubFrameTNoiseData[j].TempCNRatio);
			}

			DataMean[nSubFrame].push_back(m_PTCData[n].datamean.DataMeanFrame);
			TempNoise[nSubFrame].push_back(m_PTCData[n].tnoise.TNoiseFrame * m_PTCData[n].tnoise.TNoiseFrame);
		}

		RowName << "DataMean" << "TempNoise2" << "RowTemp2" << "ColTemp2" << "PixelTemp2" << "TempRNRatio" << "TempCNRatio";

		for (uint32_t j = 0; j < nSubFrame; j++)
		{
			std::vector<std::vector<double>> Data(7);
			Data[0] = DataMean[j];
			Data[1] = TempNoise[j];
			Data[2] = RowTemp[j];
			Data[3] = ColTemp[j];
			Data[4] = PixelTemp[j];
			Data[5] = TempRNRatio[j];
			Data[6] = TempCNRatio[j];

			m_widgetTableView[j].SetData(RowName, ColName, Data);
		}

		RowName.clear();
		RowName << "DataMean" << "TempNoise2";
		std::vector<std::vector<double>> DataAll(2);
		DataAll[0] = DataMean[nSubFrame];
		DataAll[1] = TempNoise[nSubFrame];
		m_widgetTableView[nSubFrame].SetData(RowName, ColName, DataAll);


	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogAPSPTC::Export()
{

}

void CDialogAPSPTC::MultiBrowser()
{
	QString strFileName = QFileDialog::getExistingDirectory(nullptr, tr("Open Dir"), ui.lineEditDataFile_2->text());
	ui.lineEditDataFile_2->setText(strFileName);
}
