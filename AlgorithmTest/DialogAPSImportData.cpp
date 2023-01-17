#include "DialogAPSImportData.h"
#include <QFiledialog>
#include <qvalidator.h>
#include <fstream>
#include "AlpMPAlgoInterface.h"
#include "time.h"
#include <thread>
#include<io.h>

CDialogAPSImportData::CDialogAPSImportData(QDialog* parent, CAlpAPSMPAlgoInterface* pAPSAlgoInterface, CAlpDVSMPAlgoInterface* pDVSAlgoInterface)
	: QDialog(parent), m_pAPSAlgoInterface(pAPSAlgoInterface), m_pDVSAlgoInterface(pDVSAlgoInterface)
{
	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	ui.setupUi(this);

	ui.lineEditIndexStart->setValidator(new QIntValidator(0, 100000, this));
	ui.lineEditNumber->setValidator(new QIntValidator(1, 100000, this));

	connect(ui.pushButtonBrowser, SIGNAL(clicked()), this, SLOT(Browser()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(ImportData()), Qt::QueuedConnection);


}

void CDialogAPSImportData::FindFiles(std::string strPath, std::vector<std::string> &FileQuene)
{
	_finddata_t file_info;
	std::string current_path = strPath + "/*.raw";
	long long handle = _findfirst(current_path.c_str(), &file_info);
	uint32_t file_num = 0;
	FileQuene.clear();
	if (-1 == handle)
		return;
	do
	{
		std::string attribute;
		if (file_info.attrib != _A_SUBDIR)
		{
			FileQuene.push_back(strPath + "/" + file_info.name);
		}

		file_num++;

	} while (!_findnext(handle, &file_info));
	_findclose(handle);
	return;
}

void CDialogAPSImportData::ImportSingleChannelData(uint32_t nIndexStart, uint32_t nNumber, uint32_t nChannelIndex, std::vector<std::string>& FileQuene, bool & bRet)
{
	clock_t time;
	std::ifstream infile;
	bRet = false;
	for (uint32_t i = 0; i < nNumber; i++)
	{
		infile.open(FileQuene[i], std::ios::binary | std::ios::in);
		if (!infile.fail())
		{
			infile.seekg(0, std::ios::end);
			uint64_t length = infile.tellg();
			infile.seekg(0, std::ios::beg);
			uint8_t* pRawData = new uint8_t[length];
			infile.read((char*)pRawData, length);
			infile.close();

			auto start = clock();
			bRet = m_pAPSAlgoInterface->ImportRawData(pRawData, length, APSSubFrameIndex(nChannelIndex), nIndexStart + i, 1);
			auto end = clock();
			time = end - start;
			delete[] pRawData;
		}
		else
		{
			bRet = false;
		}
		if (!bRet)
			break;
	}

}

void CDialogAPSImportData::Browser()
{
	if (ui.comboBoxImportFormat->currentIndex() == 1)
	{
		QString strFileName = QFileDialog::getOpenFileName(nullptr, tr("Open APS File"), "../", tr("BIN File (*.bin)"));
		ui.lineEditDataFile->setText(strFileName);
	}
	else
	{
		QString strFileName = QFileDialog::getExistingDirectory(nullptr, tr("Open APS File"), "../", QFileDialog::ShowDirsOnly);
		ui.lineEditDataFile->setText(strFileName);
	}
}

void CDialogAPSImportData::ImportData()
{
	bool bRet = true;
	uint32_t nIndexStart = ui.lineEditIndexStart->text().toUInt();
	uint32_t nNumber = ui.lineEditNumber->text().toUInt();
	std::string strFileName = ui.lineEditDataFile->text().toStdString();
	ui.label_Res->setText(tr(" "));
	clock_t time = 0;
	std::ifstream infile;
	if (ui.comboBoxImportFormat->currentIndex() == 1)
	{
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
			bRet = m_pAPSAlgoInterface->ImportRawData(pRawData, length, nIndexStart, nNumber, true);
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
	else
	{
		std::string strChannelName[] = {"/Gb1", "/Gb2", "/B1", "/B2", "/R1", "/R2", "/Gr1", "/Gr2"};
		std::vector<std::string> FileQuene[APSSubFrameIndex::SubFrameNum];
		std::thread* t[APSSubFrameIndex::SubFrameNum];
		bool bSubRet[APSSubFrameIndex::SubFrameNum];
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			FindFiles(strFileName + strChannelName[i], FileQuene[i]);

			if (FileQuene[i].size() < nNumber)
			{
				ui.label_Res->setStyleSheet("color:red;");
				ui.label_Res->setText(tr("Fail!"));
				return;
			}
		}
		bRet = true;
		auto start = clock();
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i] = new std::thread(&CDialogAPSImportData::ImportSingleChannelData, this, nIndexStart, nNumber, i, std::ref(FileQuene[i]), std::ref(bSubRet[i]));
		}
		for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		{
			t[i]->join();
			delete t[i];
			bRet = bRet && bSubRet[i];
		}

		//for (uint32_t i = 0; i < APSSubFrameIndex::SubFrameNum; i++)
		//{
		//	ImportSingleChannelData(nIndexStart, nNumber, i, FileQuene[i], bSubRet[i]);
		//}
		auto end = clock();
		time = end - start;
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
}