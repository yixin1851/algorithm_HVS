#include "DialogAPSImportData.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>
#include "AlpMPAlgoInterface.h"
#include "time.h"
#include <thread>
#include<io.h>
#include <qcollator.h>

CDialogAPSImportData::CDialogAPSImportData(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));

	connect(ui.pushButtonBrowser, SIGNAL(clicked()), this, SLOT(SingleBrowser()));
	connect(ui.pushButtonBrowser_2, SIGNAL(clicked()), this, SLOT(MultiBrowser()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(ImportData()), Qt::QueuedConnection);


}

void CDialogAPSImportData::SingleBrowser()
{
	QString strFileName = QFileDialog::getOpenFileName(nullptr, tr("Open APS File"), ui.lineEditDataFile->text(), tr("All Files (*.*) \n Bin File (*.bin) \n Raw File (*.raw)"));
	ui.lineEditDataFile->setText(strFileName);
}

void CDialogAPSImportData::MultiBrowser()
{
	QString strFileName = QFileDialog::getExistingDirectory(nullptr, tr("Open Dir"), ui.lineEditDataFile_2->text());
	ui.lineEditDataFile_2->setText(strFileName);
}

void CDialogAPSImportData::ImportData()
{
	if (ui.tabWidget->currentIndex() == 0)
	{
		SingleImportData();
	}
	else
	{
		MultiImportData();
	}
}

void CDialogAPSImportData::SingleImportData()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	std::string strFileName(ui.lineEditDataFile->text().toLocal8Bit());
	bool bHeaderFooter = ui.checkBoxFrameHeaderFooter->isChecked();
	ui.label_Res->setText(tr(" "));
	clock_t time = 0;
	std::ifstream infile;

	infile.open(strFileName, std::ios::binary | std::ios::in);
	if (!infile.fail())
	{
		infile.seekg(0, std::ios::end);
		uint64_t length = infile.tellg();
		infile.seekg(0, std::ios::beg);
		uint8_t* pRawData = new uint8_t[length];
		infile.read((char*)pRawData, length);
		infile.close();

		auto start = clock();
		bRet = m_pAPSAlgoInterface->ImportRawData(pRawData, length, nIndexStart, nNumber, bHeaderFooter);
		auto end = clock();
		time = end - start;
		delete[] pRawData;
	}
	else
	{
		bRet = false;
	}
	if (bRet)
	{
		ui.label_Res->setStyleSheet("color:green;");
		QString res = QString::number(time);
		ui.label_Res->setText(res);
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText(tr("Fail!"));
	}
}

void CDialogAPSImportData::MultiImportData()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	QDir DataDir(ui.lineEditDataFile_2->text());
	bool bHeaderFooter = ui.checkBoxFrameHeaderFooter->isChecked();
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

		if (DataList.size() < nNumber)
		{
			ui.label_Res->setStyleSheet("color:red;");
			ui.label_Res->setText(tr("Files not enough!"));
		}
		else
		{
			for (uint32_t n = 0; n < nNumber; n++)
			{
				std::ifstream infile;
				std::string strFileName = (DataDir.absolutePath() + "/" + DataList[n]).toLocal8Bit().toStdString();
				infile.open(strFileName, std::ios::binary | std::ios::in);
				if (!infile.fail())
				{
					infile.seekg(0, std::ios::end);
					uint64_t length = infile.tellg();
					infile.seekg(0, std::ios::beg);
					uint8_t* pRawData = new uint8_t[length];
					infile.read((char*)pRawData, length);
					infile.close();

					auto start = clock();
					bRet = m_pAPSAlgoInterface->ImportRawData(pRawData, length, nIndexStart + n, 1, bHeaderFooter);
					auto end = clock();
					time = end - start;
					delete[] pRawData;
				}
				else
				{
					bRet = false;
				}
				if (bRet)
				{
					ui.label_Res->setStyleSheet("color:green;");
					QString res = QString::number(time);
					ui.label_Res->setText(res);
				}
				else
				{
					ui.label_Res->setStyleSheet("color:red;");
					ui.label_Res->setText(QString::number(n) + tr(" Fail!"));
					break;
				}
			}
		}
	}
	else
	{
		ui.label_Res->setStyleSheet("color:red;");
		ui.label_Res->setText("dir not exist");
	}
}
