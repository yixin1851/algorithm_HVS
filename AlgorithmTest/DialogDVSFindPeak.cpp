#include "DialogDVSFindPeak.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogDVSFindPeak::CDialogDVSFindPeak(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditPeakCycle->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditPeakNum->setValidator(new QIntValidator(0, 100000, this));

	ui.lineEditPeakCycle->setText(QString::number(m_pDVSAlgoInterface->GetAlgorithmThre().nPeakCycle));

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(FindPeak()), Qt::QueuedConnection);
}


void CDialogDVSFindPeak::FindPeak()
{
	ui.widgetChartView->Clear();
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	uint32_t nPeakNumber = ui.lineEditPeakNum->text().toUInt();

	uint32_t nLightType = ui.comboBoxLightType->currentIndex();

	auto temp = m_pDVSAlgoInterface->GetAlgorithmThre();
	temp.nPeakCycle = ui.lineEditPeakCycle->text().toUInt();
	m_pDVSAlgoInterface->SetAlgorithmThre(temp);

	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	clock_t time = 0;
	auto start = clock();
	bRet = m_pDVSAlgoInterface->FindPeak(nIndexStart, nNumber, nPeakNumber, m_Data, DVSLightTrigerType(nLightType+1));
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		DVSEventsNumberCountType CountData;
		m_pDVSAlgoInterface->EventsNumberCount(nIndexStart, nNumber, CountData);

		QVector<double> XData(CountData.nDataNumber);
		QVector<double> YDataOnAll(CountData.nDataNumber);
		QVector<double> YDataOffAll(CountData.nDataNumber);


		for (uint32_t nIndex = 0; nIndex < CountData.nDataNumber; nIndex++)
		{
			XData[nIndex] = nIndex;
			YDataOnAll[nIndex] = CountData.OnEventsNum[SubFrameIndex::All][nIndex];
			YDataOffAll[nIndex] = CountData.OffEventsNum[SubFrameIndex::All][nIndex];
		}

		if ((nLightType + 1) & 1)
		{
			ui.widgetChartView->SetLine("OffEvents", XData, YDataOffAll);
			QVector<double> XOffPeakData(m_Data.nOffEventsPeakNumber);
			QVector<double> YOffPeakData(m_Data.nOffEventsPeakNumber);

			for (uint32_t nIndex = 0; nIndex < m_Data.nOffEventsPeakNumber; nIndex++)
			{
				XOffPeakData[nIndex] = m_Data.OffEventsPeakPos[nIndex] - nIndexStart;
				YOffPeakData[nIndex] = CountData.OffEventsNum[SubFrameIndex::All][XOffPeakData[nIndex]];
			}
			ui.widgetChartView->SetScatter("OffEventsPeak", XOffPeakData, YOffPeakData);
		}
		
		if ((nLightType + 1) & 2)
		{
			ui.widgetChartView->SetLine("OnEvents", XData, YDataOnAll);
			QVector<double> XOnPeakData(m_Data.nOnEventsPeakNumber);
			QVector<double> YOnPeakData(m_Data.nOnEventsPeakNumber);

			for (uint32_t nIndex = 0; nIndex < m_Data.nOnEventsPeakNumber; nIndex++)
			{
				XOnPeakData[nIndex] = m_Data.OnEventsPeakPos[nIndex] - nIndexStart;
				YOnPeakData[nIndex] = CountData.OnEventsNum[SubFrameIndex::All][XOnPeakData[nIndex]];
			}
			ui.widgetChartView->SetScatter("OnEventsPeak", XOnPeakData, YOnPeakData);
		}
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogDVSFindPeak::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toLocal8Bit().toStdString() + "//FindPeak.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			if (m_Data.nOffEventsPeakNumber != 0)
			{
				outfile << "OffEventsPeak" << std::endl;
				for (uint32_t nIndex = 0; nIndex < m_Data.nOffEventsPeakNumber; nIndex++)
				{
					outfile << std::to_string(m_Data.OffEventsPeakPos[nIndex]) << ",";
				}
				outfile << std::endl;
			}
			if (m_Data.nOnEventsPeakNumber != 0)
			{
				outfile << "OnEventsPeak" << std::endl;
				for (uint32_t nIndex = 0; nIndex < m_Data.nOnEventsPeakNumber; nIndex++)
				{
					outfile << std::to_string(m_Data.OnEventsPeakPos[nIndex]) << ",";
				}
				outfile << std::endl;
			}
			outfile.close();
		}
	}
}