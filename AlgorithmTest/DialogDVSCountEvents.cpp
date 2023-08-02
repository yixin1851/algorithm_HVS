#include "DialogDVSCountEvents.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogCountEvents::CDialogCountEvents(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(CountEvents()), Qt::QueuedConnection);
}

CDialogCountEvents::~CDialogCountEvents()
{
}

void CDialogCountEvents::CountEvents()
{
	ui.widgetChartView->Clear();
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	clock_t time = 0;
	auto start = clock();
	bRet = m_pDVSAlgoInterface->EventsNumberCount(nIndexStart, nNumber, m_Data);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QVector<double> XData(m_Data.nDataNumber);
		QVector<double> YDataOnAll(m_Data.nDataNumber);
		QVector<double> YDataOffAll(m_Data.nDataNumber);
		QVector<double> YDataAllAll(m_Data.nDataNumber);
		QVector<double> YDataOnGb(m_Data.nDataNumber);
		QVector<double> YDataOffGb(m_Data.nDataNumber);
		QVector<double> YDataAllGb(m_Data.nDataNumber);
		QVector<double> YDataOnB(m_Data.nDataNumber);
		QVector<double> YDataOffB(m_Data.nDataNumber);
		QVector<double> YDataAllB(m_Data.nDataNumber);
		QVector<double> YDataOnR(m_Data.nDataNumber);
		QVector<double> YDataOffR(m_Data.nDataNumber);
		QVector<double> YDataAllR(m_Data.nDataNumber);
		QVector<double> YDataOnGr(m_Data.nDataNumber);
		QVector<double> YDataOffGr(m_Data.nDataNumber);
		QVector<double> YDataAllGr(m_Data.nDataNumber);


		for (uint32_t nIndex = 0; nIndex < m_Data.nDataNumber; nIndex++)
		{
			XData[nIndex] = nIndex;
			YDataOnAll[nIndex] = m_Data.OnEventsNum[SubFrameIndex::All][nIndex];
			YDataOffAll[nIndex] = m_Data.OffEventsNum[SubFrameIndex::All][nIndex];
			YDataAllAll[nIndex] = m_Data.AllEventsNum[SubFrameIndex::All][nIndex];
			YDataOnGb[nIndex] = m_Data.OnEventsNum[SubFrameIndex::Gb][nIndex];
			YDataOffGb[nIndex] = m_Data.OffEventsNum[SubFrameIndex::Gb][nIndex];
			YDataAllGb[nIndex] = m_Data.AllEventsNum[SubFrameIndex::Gb][nIndex];
			YDataOnB[nIndex] = m_Data.OnEventsNum[SubFrameIndex::B][nIndex];
			YDataOffB[nIndex] = m_Data.OffEventsNum[SubFrameIndex::B][nIndex];
			YDataAllB[nIndex] = m_Data.AllEventsNum[SubFrameIndex::B][nIndex];
			YDataOnR[nIndex] = m_Data.OnEventsNum[SubFrameIndex::R][nIndex];
			YDataOffR[nIndex] = m_Data.OffEventsNum[SubFrameIndex::R][nIndex];
			YDataAllR[nIndex] = m_Data.AllEventsNum[SubFrameIndex::R][nIndex];
			YDataOnGr[nIndex] = m_Data.OnEventsNum[SubFrameIndex::Gr][nIndex];
			YDataOffGr[nIndex] = m_Data.OffEventsNum[SubFrameIndex::Gr][nIndex];
			YDataAllGr[nIndex] = m_Data.AllEventsNum[SubFrameIndex::Gr][nIndex];

		}
		ui.widgetChartView->SetLine("OnEvents(All)", XData, YDataOnAll);
		ui.widgetChartView->SetLine("OffEvents(All)", XData, YDataOffAll);
		ui.widgetChartView->SetLine("AllEvents(All)", XData, YDataAllAll);

		ui.widgetChartView->SetLine("OnEvents(Gb)", XData, YDataOnGb);
		ui.widgetChartView->SetLine("OffEvents(Gb)", XData, YDataOffGb);
		ui.widgetChartView->SetLine("AllEvents(Gb)", XData, YDataAllGb);

		ui.widgetChartView->SetLine("OnEvents(B)", XData, YDataOnB);
		ui.widgetChartView->SetLine("OffEvents(B)", XData, YDataOffB);
		ui.widgetChartView->SetLine("AllEvents(B)", XData, YDataAllB);

		ui.widgetChartView->SetLine("OnEvents(R)", XData, YDataOnR);
		ui.widgetChartView->SetLine("OffEvents(R)", XData, YDataOffR);
		ui.widgetChartView->SetLine("AllEvents(R)", XData, YDataAllR);

		ui.widgetChartView->SetLine("OnEvents(Gr)", XData, YDataOnGr);
		ui.widgetChartView->SetLine("OffEvents(Gr)", XData, YDataOffGr);
		ui.widgetChartView->SetLine("AllEvents(Gr)", XData, YDataAllGr);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogCountEvents::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "" && m_Data.nDataNumber != 0)
	{
		std::string strFile = dir.toLocal8Bit().toStdString() + "//CountEvents.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "On Events(All), Off Events(All), All Events(All), On Events(Gb), Off Events(Gb), All Events(Gb), On Events(B), Off Events(B), All Events(B), On Events(R), Off Events(R), All Events(R), On Events(Gr), Off Events(Gr), All Events(Gr)" << std::endl;
			for (uint32_t nIndex = 0; nIndex < m_Data.nDataNumber; nIndex++)
			{
				outfile << std::to_string(m_Data.OnEventsNum[SubFrameIndex::All][nIndex]) << ",";
				outfile << std::to_string(m_Data.OffEventsNum[SubFrameIndex::All][nIndex]) << ",";
				outfile << std::to_string(m_Data.AllEventsNum[SubFrameIndex::All][nIndex]) << ",";
				outfile << std::to_string(m_Data.OnEventsNum[SubFrameIndex::Gb][nIndex]) << ",";
				outfile << std::to_string(m_Data.OffEventsNum[SubFrameIndex::Gb][nIndex]) << ",";
				outfile << std::to_string(m_Data.AllEventsNum[SubFrameIndex::Gb][nIndex]) << ",";
				outfile << std::to_string(m_Data.OnEventsNum[SubFrameIndex::B][nIndex]) << ",";
				outfile << std::to_string(m_Data.OffEventsNum[SubFrameIndex::B][nIndex]) << ",";
				outfile << std::to_string(m_Data.AllEventsNum[SubFrameIndex::B][nIndex]) << ",";
				outfile << std::to_string(m_Data.OnEventsNum[SubFrameIndex::R][nIndex]) << ",";
				outfile << std::to_string(m_Data.OffEventsNum[SubFrameIndex::R][nIndex]) << ",";
				outfile << std::to_string(m_Data.AllEventsNum[SubFrameIndex::R][nIndex]) << ",";
				outfile << std::to_string(m_Data.OnEventsNum[SubFrameIndex::Gr][nIndex]) << ",";
				outfile << std::to_string(m_Data.OffEventsNum[SubFrameIndex::Gr][nIndex]) << ",";
				outfile << std::to_string(m_Data.AllEventsNum[SubFrameIndex::Gr][nIndex]);
				outfile << std::endl;
			}
			outfile.close();
		}
	}
}