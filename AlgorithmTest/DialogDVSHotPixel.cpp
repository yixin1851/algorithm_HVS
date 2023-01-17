#include "DialogDVSHotPixel.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>

CDialogDVSHotPixel::CDialogDVSHotPixel(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));
	ui.lineEditHotPixelThre->setValidator(new QDoubleValidator(0, 1, 3, this));
	ui.lineEditHotLineThre->setValidator(new QDoubleValidator(0, 1, 3, this));
	ui.lineEditHotPixelThre->setText(QString::number(m_pDVSAlgoInterface->GetAlgorithmThre().dHotPixelThre));
	ui.lineEditHotLineThre->setText(QString::number(m_pDVSAlgoInterface->GetAlgorithmThre().dHotLineThre));

	connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(Export()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(HotPixel()), Qt::QueuedConnection);
}


void CDialogDVSHotPixel::HotPixel()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	double dHotPixelThre = ui.lineEditHotPixelThre->text().toDouble();
	double dHotLineThre = ui.lineEditHotLineThre->text().toDouble();

	auto temp = m_pDVSAlgoInterface->GetAlgorithmThre();
	temp.dHotPixelThre = dHotPixelThre;
	temp.dHotLineThre = dHotLineThre;

	m_pDVSAlgoInterface->SetAlgorithmThre(temp);
	ui.tabHotPixelResult->Clear();
	ui.tabHotPixelMask->Clear();

	ui.label_Res->setText(tr(" "));
	ui.pushButtonStart->setEnabled(false);
	clock_t time = 0;
	auto start = clock();
	bRet = m_pDVSAlgoInterface->HotPixel(nIndexStart, nNumber, m_Data);
	auto end = clock();
	time = end - start;
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);

		QStringList RowName, ColName;
		RowName << "";
		ColName << "HotPixelNum" << "HotLineNum";

		std::vector<std::vector<double>> Data(1);
		Data[0].resize(2);
		Data[0][0] = m_Data.HotPixelNum;
		Data[0][1] = m_Data.HotLineNum;

		ui.tabHotPixelResult->SetData(RowName, ColName, Data);

		uint32_t nRow = 1224;
		uint32_t nCol = 1632;

		uint8_t* pImage = new uint8_t[nRow * nCol];
		memset(pImage, 0, nRow * nCol);

		for (uint32_t nIndex = 0; nIndex < m_Data.HotPixelMask.BadPixelNum; nIndex++)
		{
			pImage[m_Data.HotPixelMask.LocalData[nIndex].x * nCol + m_Data.HotPixelMask.LocalData[nIndex].y] = 255;
		}
		ui.tabHotPixelMask->SetGrayData(pImage, nCol, nRow);
		delete[] pImage;
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
	ui.pushButtonStart->setEnabled(true);
}

void CDialogDVSHotPixel::Export()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Save Directory"), "../", QFileDialog::ShowDirsOnly);

	if (dir != "")
	{
		std::string strFile = dir.toStdString() + "//HotPixel.csv";
		std::ofstream outfile;
		outfile.open(strFile, std::ios::trunc);
		if (!outfile.fail())
		{
			outfile << "HotPixelNum, HotLineNum" << std::endl;
			outfile << std::to_string(m_Data.HotPixelNum) << "," << std::to_string(m_Data.HotLineNum) << std::endl;
			outfile << "HotPixelMask" << std::endl;

			for (uint32_t nIndex = 0; nIndex < m_Data.HotPixelMask.BadPixelNum; nIndex++)
			{
				outfile << std::to_string(m_Data.HotPixelMask.LocalData[nIndex].x);
				outfile << ",";
			}
			outfile << std::endl;
			for (uint32_t nIndex = 0; nIndex < m_Data.HotPixelMask.BadPixelNum; nIndex++)
			{
				outfile << std::to_string(m_Data.HotPixelMask.LocalData[nIndex].y);
				outfile << ",";
			}
			outfile << std::endl;
			for (uint32_t nIndex = 0; nIndex < m_Data.HotPixelMask.BadPixelNum; nIndex++)
			{
				outfile << std::to_string(m_Data.HotPixelMask.Flag[nIndex]);
				outfile << ",";
			}
			outfile << std::endl;

			outfile.close();
		}
	}
}