#pragma once
#include <qwidget.h>
#include "ui_WidgetChartView.h"

class TextLabel : public QObject
{
    Q_OBJECT

public:
    TextLabel(QCustomPlot* parent, double dXValue, double dYValue);
    ~TextLabel();
    void GetCoords(double &X, double &Y);
private slots:
    void Replot();
private:
    QCustomPlot* m_parent;
    QCPItemText* textLabel;
    QCPItemLine* arrow;
    double m_dXValue;
    double m_dYValue;
};

class CWidgetChartView :
    public QWidget
{
    Q_OBJECT
public:
    CWidgetChartView(QWidget* parent = nullptr);
    void SetLine(std::string strLineName, QVector<double> &XData, QVector<double> &YData);
    void SetScatter(std::string strLineName, QVector<double>& XData, QVector<double>& YData);
    void Clear();
private slots:
    void LineVisibleCheck();
    void AddToolTips(QCPAbstractPlottable* plottable, int dataIndex, QMouseEvent* event);
private:
    Ui::ChartViewWidget ui;
    uint32_t m_nLineNum;
    QVector<QCheckBox*> QCheckBoxControl;
    TextLabel * ToolTips;
};

