#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_AlgorithmTest.h"
#include "AlpMPAlgoInterface.h"

#pragma comment (lib, "AlgorithmLibrary.lib")

class AlgorithmTest : public QMainWindow
{
    Q_OBJECT

public:
    AlgorithmTest(QWidget *parent = nullptr);
    ~AlgorithmTest();

private:
    Ui::AlgorithmTestClass ui;
};
