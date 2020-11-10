#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <wiringPi.h>
#include <QThread>
#include <QDebug>
#include <mcp3004.h>
#include <wiringPiSPI.h>
#include <softPwm.h>
#include <Iir.h>
#include <QtSql>
#include <QSqlDatabase>
#include <math.h>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    int readadc(int);

    void makePlot();
    void on_pushButton_19_clicked();

    void on_toolButton_clicked();

    void on_toolButton_2_clicked();

    void on_pushButton_21_clicked();

    void on_pushButton_22_clicked();

    void on_pushButton_23_clicked();

    void on_pushButton_24_clicked();

    void on_pushButton_25_clicked();

    void on_pushButton_26_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_18_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_11_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_14_clicked();

    void on_pushButton_15_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_16_clicked();

    void on_pushButton_17_clicked();

    void on_toolButton_3_clicked();

    void on_toolButton_4_clicked();

    void on_pushButton_45_clicked();

    void on_pushButton_46_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
