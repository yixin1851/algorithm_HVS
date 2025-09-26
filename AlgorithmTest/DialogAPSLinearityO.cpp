#include "DialogAPSLinearityO.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogAPSLinearityO::CDialogAPSLinearityO(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
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

	ui.lineEditRadius->setText(QString::number(pAPSAlgoInterface->GetAlgorithmThre().nLinearityRadius));

	ui.tabWidgetView->insertTab(0, &m_widgetTableView, "LinearityResult");
	ui.tabWidgetView->insertTab(1, &m_widgetChartView, "Chart");

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(Linearity()), Qt::QueuedConnection);
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIUp, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIDown, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROILeft, SLOT(setEnabled(bool)));
	connect(ui.checkBoxROI, SIGNAL(clicked(bool)), ui.lineEditROIRight, SLOT(setEnabled(bool)));

}


void CDialogAPSLinearityO::Linearity()
{
	bool bRet = true;
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

	auto thre = m_pAPSAlgoInterface->GetAlgorithmThre();
	thre.nLinearityRadius = ui.lineEditRadius->text().toUInt();
	m_pAPSAlgoInterface->SetAlgorithmThre(thre);

	uint32_t nIndex = ui.lineEditIndex->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();

	auto start = clock();
	bRet = m_pAPSAlgoInterface->Linearity(nIndex, nNumber, roi, Gb, m_LinearityData);
	auto end = clock();
	time = end - start;

	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;
		RowName << "";

		ColName << "MaxSNR";

		std::vector<std::vector<double>> Data(1);

		Data[0].push_back(m_LinearityData.MaxSSNR);

		m_widgetTableView.SetData(RowName, ColName, Data);

		QVector<double> XData(m_LinearityData.DataMean.begin(), m_LinearityData.DataMean.end());
		QVector<double> SNoiseData(m_LinearityData.SNoiseData.begin(), m_LinearityData.SNoiseData.end());
		QVector<double> SSNRData(m_LinearityData.SSNR.begin(), m_LinearityData.SSNR.end());

		m_widgetChartView.SetLine("SNoise", XData, SNoiseData);
		m_widgetChartView.SetLine("SSNR", XData, SSNRData);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogAPSLinearityO::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toLocal8Bit().toStdString() + "//LinearityO.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "MaxSSNR" << std::endl;
			outfile << std::to_string(m_LinearityData.MaxSSNR) << std::endl;
			outfile << "DataMean, SNoise, SSNR" << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_LinearityData.DataMean.size(); nIndex++)
			{
				outfile << m_LinearityData.DataMean[nIndex] << ",";
				outfile << m_LinearityData.SNoiseData[nIndex] << ",";
				outfile << m_LinearityData.SSNR[nIndex] << ",";
				outfile << std::endl;
			}
			outfile.close();
		}
	}
}