#pragma once
#include <qdialog.h>
#include "ui_DialogAPSImportData.h"

class CAlpAPSMPAlgoInterface;
class CAlpDVSMPAlgoInterface;

class CDialogAPSImportData :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSImportData(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
protected:
    virtual void FindFiles(std::string strPath, std::vector<std::string> &FileQuene);
    virtual void ImportSingleChannelData(uint32_t nIndexStart, uint32_t nNumber, uint32_t nChannelIndex, std::vector<std::string>& FileQuene, bool& bRet);
private slots:
    virtual void Browser();
    virtual void ImportData();
private:
    Ui::DialogAPSImportData ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;

};

