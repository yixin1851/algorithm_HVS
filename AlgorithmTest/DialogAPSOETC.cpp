#include "DialogAPSOETC.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogAPSOETC::CDialogAPSOETC(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
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
	ui.lineEditRadius->setValidator(new QIntValidator(0, 100000, this));

	ui.lineEditROIUp->setEnabled(false);
	ui.lineEditROIRight->setEnabled(false);
	ui.lineEditROILeft->setEnabled(false);
	ui.lineEditROIDown->setEnabled(false);

	auto temp = m_pAPSAlgoInterface->GetActiveArea();
	ui.lineEditROIUp->setText(QString::number(temp.Up));
	ui.lineEditROILeft->setText(QString::number(temp.Left));
	ui.lineEditROIDown->setText(QString::number(temp.Down));
	ui.lineEditROIRight->setText(QString::number(temp.Right));

	ui.lineEditRadius->setText(QString::number(pAPSAlgoInterface->GetAlgorithmThre().nOETCRadius));

	ui.tabWidgetView->insertTab(0, &m_widgetTableView, "OETCResult");
	ui.tabWidgetView->insertTab(1, &m_widgetChartView, "Chart");
	ui.tabWidgetView->insertTab(2, &m_widgetTableDataView, "OETCData");

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(OETC()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

}


void CDialogAPSOETC::OETC()
{
	bool bRet = true;
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	m_widgetTableView.Clear();
	m_widgetChartView.Clear();
	m_widgetTableDataView.Clear();
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

	auto thre = m_pAPSAlgoInterface->GetAlgorithmThre();
	thre.nLinearityRadius = ui.lineEditRadius->text().toUInt();
	m_pAPSAlgoInterface->SetAlgorithmThre(thre);

	uint32_t nIndex = ui.lineEditIndex->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();

	auto start = clock();
	bRet = m_pAPSAlgoInterface->OETC(nIndex, nNumber, roi, Gr, m_OETCData);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;
		RowName << "";

		ColName << "DR(dB)" << "read noise(DN)" << "read noise(e-)" << "FullWell(DN)" << "FullWell(e-)" << "Conversion Gain";

		std::vector<std::vector<double>> Data(1);

		Data[0].push_back(m_OETCData.DR_dB);
		Data[0].push_back(m_OETCData.ReadNoise);
		Data[0].push_back(m_OETCData.ReadNoise_e);
		Data[0].push_back(m_OETCData.FWC);
		Data[0].push_back(m_OETCData.FWC_e);
		Data[0].push_back(m_OETCData.ConversionGain);

		m_widgetTableView.SetData(RowName, ColName, Data);

		RowName.clear();
		ColName.clear();
		RowName << "DN" << "ReadNoise" << "TNoise";
		for(uint32_t n = 0; n < m_OETCData.DataMean.size(); n++)
		{
			ColName << QString::number(n + 1);
		}

		std::vector<std::vector<double>> OETCData(3);
		OETCData[0] = m_OETCData.DataMean;
		OETCData[1] = m_OETCData.ReadNoiseData;
		OETCData[2] = m_OETCData.TNoiseData;
		m_widgetTableDataView.SetData(RowName, ColName, OETCData);

		QVector<double> XData(m_OETCData.DataMean.begin(), m_OETCData.DataMean.end());
		QVector<double> ReadNoiseData(m_OETCData.ReadNoiseData.begin(), m_OETCData.ReadNoiseData.end());
		QVector<double> TNoiseData(m_OETCData.TNoiseData.begin(), m_OETCData.TNoiseData.end());
		QVector<double> ReadNoise2Data(m_OETCData.ReadNoiseData.begin(), m_OETCData.ReadNoiseData.end());

		m_widgetChartView.SetLine("ReadNoise", XData, ReadNoiseData);
		m_widgetChartView.SetLine("TNoise", XData, TNoiseData);

		for (uint32_t n = 0; n < ReadNoise2Data.size(); n++)
		{
			ReadNoise2Data[n] = ReadNoise2Data[n] * ReadNoise2Data[n];
		}

		m_widgetChartView.SetLine("ReadNoise2", XData, ReadNoise2Data);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogAPSOETC::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toLocal8Bit().toStdString() + "//OETC.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "DR(dB), read noise(DN), read noise(e-), FullWell(DN),  FullWell(e-), Conversion Gain" << std::endl;
			outfile << std::to_string(m_OETCData.DR_dB) << ",";
			outfile << std::to_string(m_OETCData.ReadNoise) << ",";
			outfile << std::to_string(m_OETCData.ReadNoise_e) << ",";
			outfile << std::to_string(m_OETCData.FWC) << ",";
			outfile << std::to_string(m_OETCData.FWC_e) << ",";
			outfile << std::to_string(m_OETCData.ConversionGain) << std::endl;

			outfile << "DataMean, ReadNoise, TNoise" << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_OETCData.DataMean.size(); nIndex++)
			{
				outfile << m_OETCData.DataMean[nIndex] << ",";
				outfile << m_OETCData.ReadNoiseData[nIndex] << ",";
				outfile << m_OETCData.TNoiseData[nIndex] << ",";
				outfile << std::endl;
			}
			outfile.close();
		}
	}
}