#include "DialogDVSBadPixel.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogDVSBadPixel::CDialogDVSBadPixel(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditPeakNum->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditDeadPixelThre->setValidator(new QDoubleValidator(0, 1, 3, this));
	ui.lineEditClusterSizeThre->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditDeadLineThre->setValidator(new QDoubleValidator(0, 1, 3, this));

	ui.lineEditDeadPixelThre->setText(QString::number(m_pDVSAlgoInterface->GetAlgorithmThre().dDeadPixelThre));
	ui.lineEditDeadLineThre->setText(QString::number(m_pDVSAlgoInterface->GetAlgorithmThre().dDeadLineThre));
	ui.lineEditClusterSizeThre->setText(QString::number(m_pDVSAlgoInterface->GetAlgorithmThre().nDeadPixelClusterSizeThre));

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(BadPixel()), Qt::QueuedConnection);
}


void CDialogDVSBadPixel::BadPixel()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	uint32_t nPeakNum = ui.lineEditPeakNum->text().toUInt();
	double dDeadPixelThre = ui.lineEditDeadPixelThre->text().toDouble();
	double dDeadLineThre = ui.lineEditDeadLineThre->text().toDouble();
	uint32_t nClusterSizeThre = ui.lineEditClusterSizeThre->text().toUInt();
	uint32_t nLightType = ui.comboBoxLightType->currentIndex();
	auto temp = m_pDVSAlgoInterface->GetAlgorithmThre();
	temp.dDeadPixelThre = dDeadPixelThre;
	temp.dDeadLineThre = dDeadLineThre;
	temp.nDeadPixelClusterSizeThre = nClusterSizeThre;

	m_pDVSAlgoInterface->SetAlgorithmThre(temp);
	ui.tabBadPixelResult->Clear();
	ui.tabOffEventsBadPixelMask->Clear();
	ui.tabOnEventsBadPixelMask->Clear();

	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	clock_t time = 0;
	auto start = clock();
	bRet = m_pDVSAlgoInterface->BadPixel(nIndexStart, nNumber, nullptr, nPeakNum, DVSLightTrigerType(nLightType + 1), m_Data);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;
		ColName << "DeadPixelNum" << "ClusterNum" << "DeadLine";

		std::vector<std::vector<double>> Data;

		uint32_t nRow = 1224;
		uint32_t nCol = 1632;

		m_pDVSAlgoInterface->GetRawDataSize(nRow, nCol);

		if ((nLightType + 1) & 1)
		{
			RowName << "OffEvents";

			std::vector<double> OffData;
			OffData.push_back(m_Data.nOffEventsDeadPixelNum);
			OffData.push_back(m_Data.nOffEventsClusterNum);
			OffData.push_back(m_Data.nOffEventsDeadLineNum);
			Data.push_back(OffData);

			uint8_t* pImage = new uint8_t[nRow * nCol];
			memset(pImage, 0, nRow * nCol);

			for (uint32_t nIndex = 0; nIndex < m_Data.OffEventsBadPixelMask.BadPixelNum; nIndex++)
			{
				pImage[m_Data.OffEventsBadPixelMask.LocalData[nIndex].x * nCol + m_Data.OffEventsBadPixelMask.LocalData[nIndex].y] = 255;
			}
			ui.tabOffEventsBadPixelMask->SetGrayData(pImage, nCol, nRow);
			delete[] pImage;

		}

		if ((nLightType + 1) & 2)
		{
			RowName << "OnEvents";

			std::vector<double> OnData;
			OnData.push_back(m_Data.nOnEventsDeadPixelNum);
			OnData.push_back(m_Data.nOnEventsClusterNum);
			OnData.push_back(m_Data.nOnEventsDeadLineNum);
			Data.push_back(OnData);

			uint8_t* pImage = new uint8_t[nRow * nCol];
			memset(pImage, 0, nRow * nCol);

			for (uint32_t nIndex = 0; nIndex < m_Data.OnEventsBadPixelMask.BadPixelNum; nIndex++)
			{
				pImage[m_Data.OnEventsBadPixelMask.LocalData[nIndex].x * nCol + m_Data.OnEventsBadPixelMask.LocalData[nIndex].y] = 255;
			}
			ui.tabOnEventsBadPixelMask->SetGrayData(pImage, nCol, nRow);
			delete[] pImage;
		}
		ui.tabBadPixelResult->SetData(RowName, ColName, Data);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogDVSBadPixel::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toLocal8Bit().toStdString() + "//BadPixel.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "DeadPixelNum," << "ClusterNum," << "DeadLine" << std::endl;
			outfile << std::to_string(m_Data.nOffEventsDeadPixelNum) << "," << std::to_string(m_Data.nOffEventsClusterNum) << "," << std::to_string(m_Data.nOffEventsDeadLineNum) << std::endl;
			outfile << std::to_string(m_Data.nOnEventsDeadPixelNum) << "," << std::to_string(m_Data.nOnEventsClusterNum) << "," << std::to_string(m_Data.nOnEventsDeadLineNum) << std::endl;

			outfile << "BadPixelMask" << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_Data.OffEventsBadPixelMask.BadPixelNum; nIndex++)
			{
				outfile << std::to_string(m_Data.OffEventsBadPixelMask.LocalData[nIndex].x);
				outfile << ",";
			}
			outfile << std::endl;
			for (uint32_t nIndex = 0; nIndex < m_Data.OffEventsBadPixelMask.BadPixelNum; nIndex++)
			{
				outfile << std::to_string(m_Data.OffEventsBadPixelMask.LocalData[nIndex].y);
				outfile << ",";
			}
			outfile << std::endl;
			for (uint32_t nIndex = 0; nIndex < m_Data.OffEventsBadPixelMask.BadPixelNum; nIndex++)
			{
				outfile << std::to_string(m_Data.OffEventsBadPixelMask.Flag[nIndex]);
				outfile << ",";
			}
			outfile << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_Data.OnEventsBadPixelMask.BadPixelNum; nIndex++)
			{
				outfile << std::to_string(m_Data.OnEventsBadPixelMask.LocalData[nIndex].x);
				outfile << ",";
			}
			outfile << std::endl;
			for (uint32_t nIndex = 0; nIndex < m_Data.OnEventsBadPixelMask.BadPixelNum; nIndex++)
			{
				outfile << std::to_string(m_Data.OnEventsBadPixelMask.LocalData[nIndex].y);
				outfile << ",";
			}
			outfile << std::endl;
			for (uint32_t nIndex = 0; nIndex < m_Data.OnEventsBadPixelMask.BadPixelNum; nIndex++)
			{
				outfile << std::to_string(m_Data.OnEventsBadPixelMask.Flag[nIndex]);
				outfile << ",";
			}
			outfile << std::endl;

			outfile.close();
		}
	}
}