#pragma once
#include <qwidget>
#include "ui_WidgetWelcome.h"

class CWidgetWelcome :
    public QWidget
{
    Q_OBJECT

public:
    CWidgetWelcome(QWidget* parent = nullptr);
    ~CWidgetWelcome();
private slots:
    void OpenDVSFunctionWidget();
    void OpenAPSFunctionWidget();
private:
    Ui::WidgetWelcome ui;
};

