#include "mainwindow.h"
#include "ui_mainwindow.h"

#define steps 7
#define dir 0
#define en 3
#define hm_sen 4

#define LED 26
#define init 0
#define range 100

#define BARCODE_GND 21

#define ADC_BASE 100
#define SPI_CHAN 0

#define LED_BASE 200

#define dataPin 21
#define clockPin 22
#define latchPin 23

static int read[20000];
static int filtdata[20000];

static int filt360nm[200];
static int filt405nm[200];
static int filt505nm[200];
static int filt525nm[200];
static int filt570nm[200];
static int filt625nm[200];
static int filt660nm[200];
static int filtwave[200];

static int readbc[200];
//static int filtwave[200];

static int opt=0;
static double bc_y_val=0;






MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    wiringPiSetup () ;
    mcp3004Setup (ADC_BASE, SPI_CHAN) ;
    sr595Setup (LED_BASE, 8, dataPin, clockPin, latchPin) ;
    //softPwmCreate(LED, init,range);
    pinMode (en, OUTPUT) ;
    pinMode (dir, OUTPUT) ;
    pinMode (steps, OUTPUT) ;
    pinMode (hm_sen, INPUT) ;
    pinMode (BARCODE_GND, OUTPUT) ;
    //softPwmWrite(LED,0);
    digitalWrite(en,HIGH);

    digitalWrite(BARCODE_GND,LOW);
    pinMode (LED, PWM_OUTPUT);
    pwmWrite (LED, 0);
    ui->stackedWidget->setCurrentIndex(0);
    QSqlDatabase sqdb = QSqlDatabase::addDatabase("QSQLITE");
    sqdb.setDatabaseName("/home/pi/git/stepper_test/FIA.db");
    if(!sqdb.open())
    {
        qDebug() << "Can't Connect to DB !";
    }
    else
    {
        qDebug() << "Connected Successfully to DB !";
    }
    setWindowFlags(Qt::FramelessWindowHint);
    ui->customPlot->setBackground(QColor(0,0,26));
    ui->customPlot->xAxis->setBasePen(QPen(Qt::white, 1));
    ui->customPlot->yAxis->setBasePen(QPen(Qt::white, 1));
    ui->customPlot->xAxis->setTickPen(QPen(Qt::white, 1));
    ui->customPlot->yAxis->setTickPen(QPen(Qt::white, 1));
    //     ui->customPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
    //     ui->customPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
    ui->customPlot->xAxis->setTickLabelColor(Qt::white);
    ui->customPlot->yAxis->setTickLabelColor(Qt::white);
    //     ui->customPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    //     ui->customPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    //     ui->customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    //     ui->customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    //     ui->customPlot->xAxis->grid()->setSubGridVisible(true);
    //     ui->customPlot->yAxis->grid()->setSubGridVisible(true);
    //     ui->customPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
    //     ui->customPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);
    //     ui->customPlot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    //     ui->customPlot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);


    //ui->customPlot_2->setBackground(QColor(0,0,26));
    ui->customPlot_2->addGraph();
    ui->customPlot_2->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->customPlot_2->graph(0)->setLineStyle(QCPGraph::lsLine);

    ui->customPlot_7->addGraph();
    ui->customPlot_7->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->customPlot_7->graph(0)->setLineStyle(QCPGraph::lsLine);

    ui->customPlot_8->addGraph();
    ui->customPlot_8->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->customPlot_8->graph(0)->setLineStyle(QCPGraph::lsLine);

    ui->customPlot_9->addGraph();
    ui->customPlot_9->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->customPlot_9->graph(0)->setLineStyle(QCPGraph::lsLine);

    ;

}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_pushButton_clicked()
{
    //Homing
    unsigned long homing_speed=0;
    QSqlQuery query;
    query.prepare("select homespeed from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        homing_speed=query.value(0).toUInt();
    }
    digitalWrite(en,LOW);
    digitalWrite(dir,HIGH);
    for (int i=0;i<15000;i++)
    {
        if(digitalRead(hm_sen)==1)
        {
            break;
        }
        else
        {
            digitalWrite(steps, HIGH);
            QThread::usleep(homing_speed);
            digitalWrite(steps, LOW);
            QThread::usleep(homing_speed);
        }

    }
    digitalWrite(en,HIGH);
}

void MainWindow::on_pushButton_2_clicked()
{
    //Initialization
    unsigned long homing_speed=0;
    QSqlQuery query;
    query.prepare("select homespeed from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        homing_speed=query.value(0).toUInt();
    }
    on_pushButton_clicked();

    digitalWrite(en,LOW);
    digitalWrite(dir,LOW);
    for (int i=0;i<12000;i++)
    {
        digitalWrite(steps, HIGH);
        QThread::usleep(homing_speed);
        digitalWrite(steps, LOW);
        QThread::usleep(homing_speed);
    }
    digitalWrite(en,HIGH);

}

void MainWindow::on_pushButton_3_clicked()
{


    //reading
    int intensity=0, win_start=0,win_end=0;
    unsigned long reading_speed=0;
    double samplingrate=0,cutoff_frequency=0;
    QSqlQuery query;
    query.prepare("select intensity, samprate, cutoff,readspeed,startregion,endregion from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        intensity=query.value(0).toInt();
        samplingrate=query.value(1).toDouble();
        cutoff_frequency=query.value(2).toDouble();
        reading_speed=query.value(3).toUInt();
        win_start=query.value(4).toInt();
        win_end=query.value(5).toInt();
    }
    on_pushButton_clicked();
    QThread::sleep(1);
    const int order = 2; // 4th order (=2 biquads)
    Iir::Butterworth::LowPass<order> f;
    //const float samplingrate = 1500; // Hz
    //const float cutoff_frequency = 4; // Hz
    f.setup (samplingrate, cutoff_frequency);

    digitalWrite(en,LOW);
    digitalWrite(dir,LOW);
    for (int i=0;i<12000;i++)
    {
        if(i>win_start && i<win_end)
        {
            pwmWrite (LED, intensity);
        }
        else
        {
            pwmWrite (LED, 0);
        }
        digitalWrite(steps, HIGH);
        QThread::usleep(reading_speed);
        digitalWrite(steps, LOW);
        QThread::usleep(reading_speed);

        read[i]=readadc(7);
        filtdata[i]=f.filter(read[i]);
        qDebug()<<read[i];
    }
    digitalWrite(en,HIGH);
    pwmWrite (LED, 0);
    makePlot();

}

int MainWindow::readadc( int pin)
{
    //    for reading channel 0
    //    unsigned char data[3]={0x06,0x00,0x00};
    //    wiringPiSPIDataRW(0,data,3);
    //    int adcValue=(data[1]&15)<<8|data[2];
    //    return adcValue;

    //for reading channel by number in Binary
    //     unsigned char buff[] = {static_cast<char>(0b110 | ((pin & 0b111) >> 2)),
    //                              static_cast<char>((pin & 0b111) << 6),
    //                              static_cast<char>(0)};

    //      wiringPiSPIDataRW(SPI_CHAN, buff, 3);
    //      return ((buff[1] & 0b1111) << 8) | buff[2];

    //for reading channel by number in hexadecimal
    unsigned char buff[] = {static_cast<char>(0x6 | ((pin & 0x7) >> 2)),
                            static_cast<char>((pin & 0x7) << 6),
                            static_cast<char>(0)};

    wiringPiSPIDataRW(SPI_CHAN, buff, 3);
    return ((buff[1] & 0xf) << 8) | buff[2];
}


void MainWindow::makePlot()
{
    // generate some data:
    int win_start=0,win_end=0;
    QSqlQuery query;
    query.prepare("select startregion, endregion from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        win_start=query.value(0).toInt();
        win_end=query.value(1).toInt();
    }
    QVector<double> x(20000), y(20000), y1(20000);// initialize with entries 0..100
    for (int i=0; i<12000; i++)
    {
        //      x[i] = i/50.0 - 1; // x goes from -1 to 1
        //      y[i] = x[i]*x[i]; // let's plot a quadratic function
        x[i]=i;
        y[i]=read[i];
        y1[i]=filtdata[i];
    }
    double temp1=0;
    int pos1=0;
    for (int i=win_start;i<(win_start+(win_end-win_start)/2)-50;i++)
    {

        if(temp1<y1[i])
        {
            temp1=y1[i];
            pos1=i;
        }

    }

    double temp2=0;
    int pos2=0;
    for (int i=(win_start+(win_end-win_start)/2)+50;i<win_end;i++)
    {

        if(temp2<y1[i])
        {
            temp2=y1[i];
            pos2=i;
        }

    }
    qDebug()<<temp1<<temp2;
    qDebug()<<pos1<<pos2;
    QVector<double> xv1(2);
    QVector<double> yv1(2);
    QVector<double> xv2(2);
    QVector<double> yv2(2);
    xv1[0]=xv1[1]=pos1;
    xv2[0]=xv2[1]=pos2;
    yv1[0]=yv2[0]=0;
    yv1[1]=temp1;
    yv2[1]=temp2;

    int test=0;
    for(int i=pos1-10;i<pos1+10;i++)
    {
        test+=y1[i];
    }
    test=test/20;
    int control=0;
    for(int i=pos2-10;i<pos2+10;i++)
    {
        control+=y1[i];
    }
    control=control/20;

    int max=0;
    if(control>test)
        max=control;
    else max=test;

    ui->label->setNum(test);
    ui->label_2->setNum(control);
    // create graph and assign data to it:
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(x,y);
    ui->customPlot->graph(0)->setVisible(false);
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setPen(QPen(Qt::red,3));
    ui->customPlot->graph(1)->setData(x,y1);
    //vertical  line 2
    ui->customPlot->addGraph();
    ui->customPlot->graph(2)->setPen(QPen(Qt::white,3));
    ui->customPlot->graph(2)->setData(xv1,yv1);
    //vertical line 2
    ui->customPlot->addGraph();
    ui->customPlot->graph(3)->setPen(QPen(Qt::white,3));
    ui->customPlot->graph(3)->setData(xv2,yv2);

    // give the axes some labels:
    ui->customPlot->xAxis->setLabel("x");
    ui->customPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->customPlot->xAxis->setRange(win_start, win_end);
    ui->customPlot->yAxis->setRange(0, max+100);
    ui->customPlot->replot();

}

void MainWindow::on_pushButton_19_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Intensity[0-1024]");
    opt=1;
    int intensity=0;
    QSqlQuery query;
    query.prepare("select intensity from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        intensity=query.value(0).toInt();
    }
    QString ity=QString::number(intensity);
    ui->lineEdit_9->setText(ity);
}

void MainWindow::on_toolButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->toolButton_2->setChecked(false);
    ui->toolButton_4->setChecked(false);
}

void MainWindow::on_toolButton_2_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->toolButton->setChecked(false);
    ui->toolButton_4->setChecked(false);
    int intensity=0,samprate=0,cutoff=0,homing_speed=0,reading_speed=0,win_start=0,win_end=0;

    QSqlQuery query;
    query.prepare("select intensity, samprate, cutoff, homespeed,readspeed, startregion,endregion from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        intensity=query.value(0).toInt();
        samprate=query.value(1).toInt();
        cutoff=query.value(2).toInt();
        homing_speed=query.value(3).toInt();
        reading_speed=query.value(4).toInt();
        win_start=query.value(5).toInt();
        win_end=query.value(6).toInt();
    }
    QString ity=QString::number(intensity);
    QString srt=QString::number(samprate);
    QString cut=QString::number(cutoff);
    QString hms=QString::number(homing_speed);
    QString rds=QString::number(reading_speed);
    QString wis=QString::number(win_start);
    QString wie=QString::number(win_end);

    ui->lineEdit->setText(ity);
    ui->lineEdit_3->setText(srt);
    ui->lineEdit_4->setText(cut);
    ui->lineEdit_5->setText(hms);
    ui->lineEdit_6->setText(rds);
    ui->lineEdit_7->setText(wis);
    ui->lineEdit_8->setText(wie);

}


void MainWindow::on_pushButton_21_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Sampling Rate");
    opt=2;
    int samprate=0;
    QSqlQuery query;
    query.prepare("select samprate from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        samprate=query.value(0).toInt();
    }
    QString srt=QString::number(samprate);
    ui->lineEdit_9->setText(srt);
}

void MainWindow::on_pushButton_22_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Cut Off Freq.");
    opt=3;
    int cutoff=0;
    QSqlQuery query;
    query.prepare("select cutoff from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        cutoff=query.value(0).toInt();
    }
    QString cut=QString::number(cutoff);
    ui->lineEdit_9->setText(cut);
}

void MainWindow::on_pushButton_23_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Homing Speed");
    opt=4;
    int homespeed=0;
    QSqlQuery query;
    query.prepare("select homespeed from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        homespeed=query.value(0).toInt();
    }
    QString hms=QString::number(homespeed);
    ui->lineEdit_9->setText(hms);
}

void MainWindow::on_pushButton_24_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Reading Speed");
    opt=5;
    int readspeed=0;
    QSqlQuery query;
    query.prepare("select readspeed from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        readspeed=query.value(0).toInt();
    }
    QString rds=QString::number(readspeed);
    ui->lineEdit_9->setText(rds);
}

void MainWindow::on_pushButton_25_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Starting Region");
    opt=6;
    int startregion=0;
    QSqlQuery query;
    query.prepare("select startregion from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        startregion=query.value(0).toInt();
    }
    QString wis=QString::number(startregion);
    ui->lineEdit_9->setText(wis);
}

void MainWindow::on_pushButton_26_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Ending Region");
    opt=7;
    int endregion=0;
    QSqlQuery query;
    query.prepare("select endregion from FIA where sno=1");
    query.exec();
    while(query.next())
    {
        endregion=query.value(0).toInt();
    }
    QString wie=QString::number(endregion);
    ui->lineEdit_9->setText(wie);
}

void MainWindow::on_pushButton_4_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"1");
}

void MainWindow::on_pushButton_18_clicked()
{
    ui->lineEdit_9->backspace();
}

void MainWindow::on_pushButton_5_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"2");
}

void MainWindow::on_pushButton_6_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"3");
}

void MainWindow::on_pushButton_11_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"4");
}

void MainWindow::on_pushButton_12_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"5");
}

void MainWindow::on_pushButton_10_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"6");
}

void MainWindow::on_pushButton_14_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"7");
}

void MainWindow::on_pushButton_15_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"8");
}

void MainWindow::on_pushButton_13_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"9");
}

void MainWindow::on_pushButton_16_clicked()
{
    ui->lineEdit_9->setText(ui->lineEdit_9->text()+"0");
}

void MainWindow::on_pushButton_17_clicked()
{
    QString val=ui->lineEdit_9->text();
    //qDebug()<<opt;
    if(opt==8)
    {
        ui->lineEdit_10->setText(val);
        ui->stackedWidget->setCurrentIndex(4);
    }
    else if(opt==9)
    {
        ui->lineEdit_19->setText(val);
        ui->stackedWidget->setCurrentIndex(4);
    }
    else if(opt==10)
    {
        ui->lineEdit_31->setText(val);
        ui->stackedWidget->setCurrentIndex(6);
    }
    else if(opt==11)
    {
        ui->lineEdit_32->setText(val);
        ui->stackedWidget->setCurrentIndex(7);
    }
    else if(opt==12)
    {
        ui->lineEdit_33->setText(val);
        ui->stackedWidget->setCurrentIndex(7);
    }
    else if(opt==13)
    {
        ui->lineEdit_35->setText(val);
        ui->stackedWidget->setCurrentIndex(8);
    }
    else if(opt==14)
    {
        ui->lineEdit_34->setText(val);
        ui->stackedWidget->setCurrentIndex(8);
    }
    else if(opt==15)
    {
        ui->lineEdit_36->setText(val);
        ui->stackedWidget->setCurrentIndex(8);
    }
    else
    {
        QSqlQuery query;

        if(opt==1)
            query.prepare("update FIA set intensity=:val where sno=1");
        else if(opt==2)
            query.prepare("update FIA set samprate=:val where sno=1");
        else if(opt==3)
            query.prepare("update FIA set cutoff=:val where sno=1");
        else if(opt==4)
            query.prepare("update FIA set homespeed=:val where sno=1");
        else if(opt==5)
            query.prepare("update FIA set readspeed=:val where sno=1");
        else if(opt==6)
            query.prepare("update FIA set startregion=:val where sno=1");
        else if(opt==7)
            query.prepare("update FIA set endregion=:val where sno=1");

        query.bindValue(":val",val);
        query.exec();
        on_toolButton_2_clicked();
    }



}

void MainWindow::on_toolButton_3_clicked()
{
    qApp->exit();
}

void MainWindow::on_toolButton_4_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
    ui->toolButton->setChecked(false);
    ui->toolButton_2->setChecked(false);
}

void MainWindow::on_pushButton_45_clicked()
{

    double samplingrate=0,cutoff_frequency=0;
    QSqlQuery query;
    query.prepare("select samprate, cutoff from FIA where sno=1");
    query.exec();
    while(query.next())
    {

        samplingrate=query.value(0).toDouble();
        cutoff_frequency=query.value(1).toDouble();

    }
    digitalWrite (LED_BASE + 0,HIGH) ;
    digitalWrite (LED_BASE + 1,HIGH) ;
    digitalWrite (LED_BASE + 2,HIGH) ;
    digitalWrite (LED_BASE + 3,HIGH) ;
    digitalWrite (LED_BASE + 4,HIGH) ;
    digitalWrite (LED_BASE + 5,HIGH) ;
    digitalWrite (LED_BASE + 6,HIGH) ;
    QThread::sleep(1);
    const int order = 2; // 4th order (=2 biquads)
    Iir::Butterworth::LowPass<order> f360,f405,f505,f525,f570,f625,f660;
    //const float samplingrate = 1500; // Hz
    //const float cutoff_frequency = 4; // Hz
    f360.setup (samplingrate, cutoff_frequency);
    f405.setup (samplingrate, cutoff_frequency);
    f505.setup (samplingrate, cutoff_frequency);
    f525.setup (samplingrate, cutoff_frequency);
    f570.setup (samplingrate, cutoff_frequency);
    f625.setup (samplingrate, cutoff_frequency);
    f660.setup (samplingrate, cutoff_frequency);
    //qDebug()<<samplingrate<<cutoff_frequency;
    int b360nm[200],b405nm[200],b505nm[200],b525nm[200],b570nm[200],b625nm[200],b660nm[200];
    for(int t=0;t<2;t++)
    {
        for(int i=0;i<200;i++)
        {
            b360nm[i]=readadc(6);
            b405nm[i]=readadc(5);
            b505nm[i]=readadc(4);
            b525nm[i]=readadc(3);
            b570nm[i]=readadc(2);
            b625nm[i]=readadc(1);
            b660nm[i]=readadc(0);
            QThread::msleep(1);

            filt360nm[i]=f360.filter(b360nm[i]);
            filt405nm[i]=f405.filter(b405nm[i]);
            filt505nm[i]=f505.filter(b505nm[i]);
            filt525nm[i]=f525.filter(b525nm[i]);
            filt570nm[i]=f570.filter(b570nm[i]);
            filt625nm[i]=f625.filter(b625nm[i]);
            filt660nm[i]=f660.filter(b660nm[i]);

            //qDebug()<<b525nm[i]<<filt525nm[i];
        }
    }

    digitalWrite (LED_BASE + 0,LOW) ;
    digitalWrite (LED_BASE + 1,LOW) ;
    digitalWrite (LED_BASE + 2,LOW) ;
    digitalWrite (LED_BASE + 3,LOW) ;
    digitalWrite (LED_BASE + 4,LOW) ;
    digitalWrite (LED_BASE + 5,LOW) ;
    digitalWrite (LED_BASE + 6,LOW) ;
    for(int i=100;i<200;i++)
    {
        filt360nm[0]+=filt360nm[i];
        filt405nm[0]+=filt405nm[i];
        filt505nm[0]+=filt505nm[i];
        filt525nm[0]+=filt525nm[i];
        filt570nm[0]+=filt570nm[i];
        filt625nm[0]+=filt625nm[i];
        filt660nm[0]+=filt660nm[i];

    }

    filt360nm[0]=filt360nm[0]/100;
    filt405nm[0]=filt405nm[0]/100;
    filt505nm[0]=filt505nm[0]/100;
    filt525nm[0]=filt525nm[0]/100;
    filt570nm[0]=filt570nm[0]/100;
    filt625nm[0]=filt625nm[0]/100;
    filt660nm[0]=filt660nm[0]/100;


    ui->label_27->setNum(filt360nm[0]);
    ui->label_60->setNum(filt405nm[0]);
    ui->label_63->setNum(filt505nm[0]);
    ui->label_66->setNum(filt525nm[0]);
    ui->label_69->setNum(filt570nm[0]);
    ui->label_72->setNum(filt625nm[0]);
    ui->label_75->setNum(filt660nm[0]);

}

void MainWindow::on_pushButton_46_clicked()
{
    double samplingrate=0,cutoff_frequency=0;
    QSqlQuery query;
    query.prepare("select samprate, cutoff from FIA where sno=1");
    query.exec();
    while(query.next())
    {

        samplingrate=query.value(0).toDouble();
        cutoff_frequency=query.value(1).toDouble();

    }
    digitalWrite (LED_BASE + 0,HIGH) ;
    digitalWrite (LED_BASE + 1,HIGH) ;
    digitalWrite (LED_BASE + 2,HIGH) ;
    digitalWrite (LED_BASE + 3,HIGH) ;
    digitalWrite (LED_BASE + 4,HIGH) ;
    digitalWrite (LED_BASE + 5,HIGH) ;
    digitalWrite (LED_BASE + 6,HIGH) ;
    QThread::sleep(1);
    const int order = 2; // 4th order (=2 biquads)
    Iir::Butterworth::LowPass<order> f360,f405,f505,f525,f570,f625,f660;
    //const float samplingrate = 1500; // Hz
    //const float cutoff_frequency = 4; // Hz
    f360.setup (samplingrate, cutoff_frequency);
    f405.setup (samplingrate, cutoff_frequency);
    f505.setup (samplingrate, cutoff_frequency);
    f525.setup (samplingrate, cutoff_frequency);
    f570.setup (samplingrate, cutoff_frequency);
    f625.setup (samplingrate, cutoff_frequency);
    f660.setup (samplingrate, cutoff_frequency);
    //qDebug()<<samplingrate<<cutoff_frequency;
    int b360nm[200],b405nm[200],b505nm[200],b525nm[200],b570nm[200],b625nm[200],b660nm[200];

    for(int t=0;t<2;t++)
    {
        for(int i=0;i<200;i++)
        {
            b360nm[i]=readadc(6);
            b405nm[i]=readadc(5);
            b505nm[i]=readadc(4);
            b525nm[i]=readadc(3);
            b570nm[i]=readadc(2);
            b625nm[i]=readadc(1);
            b660nm[i]=readadc(0);
            QThread::msleep(1);

            filt360nm[i]=f360.filter(b360nm[i]);
            filt405nm[i]=f405.filter(b405nm[i]);
            filt505nm[i]=f505.filter(b505nm[i]);
            filt525nm[i]=f525.filter(b525nm[i]);
            filt570nm[i]=f570.filter(b570nm[i]);
            filt625nm[i]=f625.filter(b625nm[i]);
            filt660nm[i]=f660.filter(b660nm[i]);

            //qDebug()<<b525nm[i]<<filt525nm[i];
        }
    }
    digitalWrite (LED_BASE + 0,LOW) ;
    digitalWrite (LED_BASE + 1,LOW) ;
    digitalWrite (LED_BASE + 2,LOW) ;
    digitalWrite (LED_BASE + 3,LOW) ;
    digitalWrite (LED_BASE + 4,LOW) ;
    digitalWrite (LED_BASE + 5,LOW) ;
    digitalWrite (LED_BASE + 6,LOW) ;
    for(int i=100;i<200;i++)
    {
        filt360nm[0]+=filt360nm[i];
        filt405nm[0]+=filt405nm[i];
        filt505nm[0]+=filt505nm[i];
        filt525nm[0]+=filt525nm[i];
        filt570nm[0]+=filt570nm[i];
        filt625nm[0]+=filt625nm[i];
        filt660nm[0]+=filt660nm[i];

    }

    filt360nm[0]=filt360nm[0]/100;
    filt405nm[0]=filt405nm[0]/100;
    filt505nm[0]=filt505nm[0]/100;
    filt525nm[0]=filt525nm[0]/100;
    filt570nm[0]=filt570nm[0]/100;
    filt625nm[0]=filt625nm[0]/100;
    filt660nm[0]=filt660nm[0]/100;


    ui->label_28->setNum(filt360nm[0]);
    ui->label_61->setNum(filt405nm[0]);
    ui->label_64->setNum(filt505nm[0]);
    ui->label_67->setNum(filt525nm[0]);
    ui->label_70->setNum(filt570nm[0]);
    ui->label_73->setNum(filt625nm[0]);
    ui->label_76->setNum(filt660nm[0]);

    double t360nm=0,t405nm=0,t505nm=0,t525nm=0,t570nm=0,t625nm=0,t660nm=0;
    double a360nm=0,a405nm=0,a505nm=0,a525nm=0,a570nm=0,a625nm=0,a660nm=0;

    double blank=ui->label_27->text().toDouble();
    t360nm=blank/filt360nm[0];
    a360nm=log10(t360nm);

    blank=ui->label_60->text().toDouble();
    t405nm=blank/filt405nm[0];
    a405nm=log10(t405nm);

    blank=ui->label_63->text().toDouble();
    t505nm=blank/filt505nm[0];
    a505nm=log10(t505nm);

    blank=ui->label_66->text().toDouble();
    t525nm=blank/filt525nm[0];
    a525nm=log10(t525nm);


    blank=ui->label_69->text().toDouble();
    t570nm=blank/filt570nm[0];
    a570nm=log10(t570nm);

    blank=ui->label_72->text().toDouble();
    t625nm=blank/filt625nm[0];
    a625nm=log10(t625nm);

    blank=ui->label_75->text().toDouble();
    t660nm=blank/filt660nm[0];
    a660nm=log10(t660nm);
    //qDebug()<<blank<<t360nm<<a360nm;

    ui->label_29->setText(QString::number(a360nm, 'f', 3));
    ui->label_59->setText(QString::number(a405nm, 'f', 3));
    ui->label_62->setText(QString::number(a505nm, 'f', 3));
    ui->label_65->setText(QString::number(a525nm, 'f', 3));
    ui->label_68->setText(QString::number(a570nm, 'f', 3));
    ui->label_71->setText(QString::number(a625nm, 'f', 3));
    ui->label_74->setText(QString::number(a660nm, 'f', 3));

}


void MainWindow::on_pushButton_27_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Sample points");
    opt=8;
    ui->lineEdit_9->setText(ui->lineEdit_10->text());
}

void MainWindow::on_pushButton_47_clicked()
{
    ui->stackedWidget->setCurrentIndex(4);
    ui->toolButton->setChecked(false);
    ui->toolButton_2->setChecked(false);

}

void MainWindow::on_pushButton_48_clicked()
{

    double samplingrate=0,cutoff_frequency=0;
    QSqlQuery query;
    query.prepare("select samprate, cutoff from FIA where sno=1");
    query.exec();
    while(query.next())
    {

        samplingrate=query.value(0).toDouble();
        cutoff_frequency=query.value(1).toDouble();

    }

    QThread::sleep(1);
    const int order = 2; // 4th order (=2 biquads)
    Iir::Butterworth::LowPass<order> fwave;
    //const float samplingrate = 1500; // Hz
    //const float cutoff_frequency = 4; // Hz
    fwave.setup (samplingrate, cutoff_frequency);
    int wavelength=0;
    wavelength=ui->comboBox->currentIndex();
    //qDebug()<<wavelength;
    int blank[200];
    digitalWrite (LED_BASE + 6-wavelength,HIGH) ;
    QThread::msleep(200);
    for(int i=0;i<3;i++)
    {
        for(int i=0;i<200;i++)
        {
            blank[i]=readadc(wavelength);
            QThread::msleep(1);
            filtwave[i]=fwave.filter(blank[i]);

        }


        //QThread::msleep(200);

        QThread::msleep(10);
        for(int i=100;i<200;i++)
        {
            filtwave[0]+=filtwave[i];

        }

    }
    digitalWrite (LED_BASE + 6-wavelength,LOW) ;
    filtwave[0]=filtwave[0]/100;

    ui->label_33->setNum(filtwave[0]);


}

void MainWindow::on_pushButton_49_clicked()
{
    double samplingrate=0,cutoff_frequency=0;
    QSqlQuery query;
    query.prepare("select samprate, cutoff from FIA where sno=1");
    query.exec();
    while(query.next())
    {

        samplingrate=query.value(0).toDouble();
        cutoff_frequency=query.value(1).toDouble();

    }

    QThread::sleep(1);
    const int order = 2; // 4th order (=2 biquads)
    Iir::Butterworth::LowPass<order> fwave;
    //const float samplingrate = 1500; // Hz
    //const float cutoff_frequency = 4; // Hz
    fwave.setup (samplingrate, cutoff_frequency);

    //nt read[200];
    unsigned long interval=ui->lineEdit_19->text().toULong();
    clearData();
    plot();
    int wavelength=0;
    wavelength=ui->comboBox->currentIndex();
    //qDebug()<<wavelength;
    double blank=ui->label_33->text().toInt();
    double transmission=0,absorbance=0;
    int read1[200];
    double samp=ui->lineEdit_10->text().toInt();

    QThread::msleep(200);
    for(int i=0;i<=samp+1;i++)
    {
        digitalWrite (LED_BASE + 6-wavelength,HIGH) ;
        QThread::msleep(100);
        for(int j=0;j<200;j++)
        {
            read1[j]=readadc(wavelength);
            QThread::msleep(1);
            filtwave[j]=fwave.filter(read1[j]);
            //qDebug()<<read1[j]<<filtwave[j];

        }
        digitalWrite (LED_BASE + 6-wavelength,LOW) ;
        QThread::msleep(10);

        for(int j=100;j<200;j++)
        {
            filtwave[0]+=filtwave[j];

        }
        filtwave[0]=filtwave[0]/100;
        if(i>0)
        {
            transmission=blank/filtwave[0];
            absorbance=log10(transmission);
            //qDebug()<<filtwave[0]<<transmission<<absorbance;
            ui->label_36->setNum(filtwave[0]);
            ui->label_37->setText(QString::number(absorbance, 'f', 3));
            addPoint(i-1,absorbance);
            plot();
        }
        QApplication::processEvents();
        QThread::sleep(interval);
    }



}

void MainWindow::addPoint(double x, double y)
{
    qv_x.append(x);
    qv_y.append(y);
    if(y>bc_y_val)
        bc_y_val=y;
}

void MainWindow::clearData()
{
    qv_x.clear();
    qv_y.clear();

}

void MainWindow::plot()
{
    int samp=ui->lineEdit_10->text().toInt();
    //ui->customPlot_2->addGraph();
    ui->customPlot_2->graph(0)->setData(qv_x,qv_y);
    //ui->customPlot_2->graph(0)->setVisible(false);
    //ui->customPlot_2->addGraph();
    // give the axes some labels:
    ui->customPlot_2->xAxis->setLabel("POINTS");
    ui->customPlot_2->yAxis->setLabel("OD");
    // set axes ranges, so we see all data:
    ui->customPlot_2->xAxis->setRange(0, samp);
    ui->customPlot_2->yAxis->setRange(-0.5,bc_y_val+1);
    ui->customPlot_2->replot();
    ui->customPlot_2->update();


}

void MainWindow::on_pushButton_57_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Interval(S)");
    opt=9;
    ui->lineEdit_9->setText(ui->lineEdit_19->text());
}

void MainWindow::on_pushButton_50_clicked()
{
     ui->stackedWidget->setCurrentIndex(5);
}

void MainWindow::on_pushButton_62_clicked()
{
     ui->stackedWidget->setCurrentIndex(6);
}

void MainWindow::on_pushButton_63_clicked()
{
     ui->stackedWidget->setCurrentIndex(7);
}

void MainWindow::on_pushButton_64_clicked()
{
     ui->stackedWidget->setCurrentIndex(8);
}

void MainWindow::on_pushButton_97_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Incubation");
    opt=10;
    ui->lineEdit_9->setText(ui->lineEdit_31->text());
}

void MainWindow::on_pushButton_101_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Lag");
    opt=11;
    ui->lineEdit_9->setText(ui->lineEdit_32->text());
}

void MainWindow::on_pushButton_103_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Read");
    opt=12;
    ui->lineEdit_9->setText(ui->lineEdit_33->text());
}

void MainWindow::on_pushButton_105_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Read");
    opt=13;
    ui->lineEdit_9->setText(ui->lineEdit_35->text());

}

void MainWindow::on_pushButton_106_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Read");
    opt=14;
    ui->lineEdit_9->setText(ui->lineEdit_34->text());
}

void MainWindow::on_pushButton_108_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_17->setText("Read");
    opt=15;
    ui->lineEdit_9->setText(ui->lineEdit_36->text());
}

void MainWindow::on_pushButton_99_clicked()
{
    double samplingrate=0,cutoff_frequency=0;
    QSqlQuery query;
    query.prepare("select samprate, cutoff from FIA where sno=1");
    query.exec();
    while(query.next())
    {

        samplingrate=query.value(0).toDouble();
        cutoff_frequency=query.value(1).toDouble();

    }

    QThread::sleep(1);
    const int order = 2; // 4th order (=2 biquads)
    Iir::Butterworth::LowPass<order> fwave;
    //const float samplingrate = 1500; // Hz
    //const float cutoff_frequency = 4; // Hz
    fwave.setup (samplingrate, cutoff_frequency);
    int wavelength=0;
    wavelength=ui->comboBox_4->currentIndex();
    //qDebug()<<wavelength;
    int blank[200];
    digitalWrite (LED_BASE + 6-wavelength,HIGH) ;
    QThread::msleep(200);
    for(int i=0;i<3;i++)
    {
        for(int i=0;i<200;i++)
        {
            blank[i]=readadc(wavelength);
            QThread::msleep(1);
            filtwave[i]=fwave.filter(blank[i]);

        }


        //QThread::msleep(200);

        QThread::msleep(10);
        for(int i=100;i<200;i++)
        {
            filtwave[0]+=filtwave[i];

        }

    }
    digitalWrite (LED_BASE + 6-wavelength,LOW) ;
    filtwave[0]=filtwave[0]/100;

    ui->label_173->setNum(filtwave[0]);
}

void MainWindow::on_pushButton_102_clicked()
{
    double samplingrate=0,cutoff_frequency=0;
    QSqlQuery query;
    query.prepare("select samprate, cutoff from FIA where sno=1");
    query.exec();
    while(query.next())
    {

        samplingrate=query.value(0).toDouble();
        cutoff_frequency=query.value(1).toDouble();

    }

    QThread::sleep(1);
    const int order = 2; // 4th order (=2 biquads)
    Iir::Butterworth::LowPass<order> fwave;
    //const float samplingrate = 1500; // Hz
    //const float cutoff_frequency = 4; // Hz
    fwave.setup (samplingrate, cutoff_frequency);
    int wavelength=0;
    wavelength=ui->comboBox_5->currentIndex();
    //qDebug()<<wavelength;
    int blank[200];
    digitalWrite (LED_BASE + 6-wavelength,HIGH) ;
    QThread::msleep(200);
    for(int i=0;i<3;i++)
    {
        for(int i=0;i<200;i++)
        {
            blank[i]=readadc(wavelength);
            QThread::msleep(1);
            filtwave[i]=fwave.filter(blank[i]);

        }


        //QThread::msleep(200);

        QThread::msleep(10);
        for(int i=100;i<200;i++)
        {
            filtwave[0]+=filtwave[i];

        }

    }
    digitalWrite (LED_BASE + 6-wavelength,LOW) ;
    filtwave[0]=filtwave[0]/100;

    ui->label_182->setNum(filtwave[0]);
}

void MainWindow::on_pushButton_107_clicked()
{
    double samplingrate=0,cutoff_frequency=0;
    QSqlQuery query;
    query.prepare("select samprate, cutoff from FIA where sno=1");
    query.exec();
    while(query.next())
    {

        samplingrate=query.value(0).toDouble();
        cutoff_frequency=query.value(1).toDouble();

    }

    QThread::sleep(1);
    const int order = 2; // 4th order (=2 biquads)
    Iir::Butterworth::LowPass<order> fwave;
    //const float samplingrate = 1500; // Hz
    //const float cutoff_frequency = 4; // Hz
    fwave.setup (samplingrate, cutoff_frequency);
    int wavelength=0;
    wavelength=ui->comboBox_6->currentIndex();
    //qDebug()<<wavelength;
    int blank[200];
    digitalWrite (LED_BASE + 6-wavelength,HIGH) ;
    QThread::msleep(200);
    for(int i=0;i<3;i++)
    {
        for(int i=0;i<200;i++)
        {
            blank[i]=readadc(wavelength);
            QThread::msleep(1);
            filtwave[i]=fwave.filter(blank[i]);

        }


        //QThread::msleep(200);

        QThread::msleep(10);
        for(int i=100;i<200;i++)
        {
            filtwave[0]+=filtwave[i];

        }

    }
    digitalWrite (LED_BASE + 6-wavelength,LOW) ;
    filtwave[0]=filtwave[0]/100;

    ui->label_189->setNum(filtwave[0]);
}

void MainWindow::on_pushButton_98_clicked()
{
    ui->label_174->setText("");
    ui->label_175->setText("");
    double samplingrate=0,cutoff_frequency=0;
    QSqlQuery query;
    query.prepare("select samprate, cutoff from FIA where sno=1");
    query.exec();
    while(query.next())
    {

        samplingrate=query.value(0).toDouble();
        cutoff_frequency=query.value(1).toDouble();

    }

    QThread::sleep(1);
    const int order = 2; // 4th order (=2 biquads)
    Iir::Butterworth::LowPass<order> fwave;
    //const float samplingrate = 1500; // Hz
    //const float cutoff_frequency = 4; // Hz
    fwave.setup (samplingrate, cutoff_frequency);

    //nt read[200];
    //unsigned long interval=ui->lineEdit_31->text().toULong();
    clearData();
    plot_end();
    int wavelength=0;
    wavelength=ui->comboBox_4->currentIndex();
    //qDebug()<<wavelength;
    double blank=ui->label_173->text().toInt();
    double transmission=0,absorbance=0;
    int read1[200];
    double samp=ui->lineEdit_31->text().toInt();

    QThread::msleep(200);
    for(int i=0;i<=samp+1;i++)
    {
        digitalWrite (LED_BASE + 6-wavelength,HIGH) ;
        QThread::msleep(100);
        for(int j=0;j<200;j++)
        {
            read1[j]=readadc(wavelength);
            QThread::msleep(1);
            filtwave[j]=fwave.filter(read1[j]);
            //qDebug()<<read1[j]<<filtwave[j];

        }
        digitalWrite (LED_BASE + 6-wavelength,LOW) ;
        QThread::msleep(10);

        for(int j=100;j<200;j++)
        {
            filtwave[0]+=filtwave[j];

        }
        filtwave[0]=filtwave[0]/100;
        if(i>0)
        {
            transmission=blank/filtwave[0];
            absorbance=log10(transmission);
            //qDebug()<<filtwave[0]<<transmission<<absorbance;
            //ui->label_36->setNum(filtwave[0]);
            //ui->label_37->setText(QString::number(absorbance, 'f', 3));
            addPoint(i-1,absorbance);
            plot_end();
        }
        QApplication::processEvents();
        QThread::msleep(615);
    }
    ui->label_174->setNum(filtwave[0]);
    ui->label_175->setText(QString::number(absorbance, 'f', 3));

}

void MainWindow::plot_end()
{
    int samp=ui->lineEdit_31->text().toInt();
    //ui->customPlot_2->addGraph();
    //qDebug()<<qv_x<<qv_y
    ui->customPlot_7->graph(0)->setData(qv_x,qv_y);
    //ui->customPlot_2->graph(0)->setVisible(false);
    //ui->customPlot_2->addGraph();
    // give the axes some labels:
    ui->customPlot_7->xAxis->setLabel("POINTS");
    ui->customPlot_7->yAxis->setLabel("OD");
    // set axes ranges, so we see all data:
    ui->customPlot_7->xAxis->setRange(0, samp);
    ui->customPlot_7->yAxis->setRange(-0.5,bc_y_val+1);
    ui->customPlot_7->replot();
    ui->customPlot_7->update();


}

void MainWindow::on_pushButton_100_clicked()
{
    ui->label_180->setText("");
    ui->label_177->setText("");
    ui->label_184->setText("");
    ui->label_185->setText("");
    ui->label_196->setText("");
    double samplingrate=0,cutoff_frequency=0;
    QSqlQuery query;
    query.prepare("select samprate, cutoff from FIA where sno=1");
    query.exec();
    while(query.next())
    {

        samplingrate=query.value(0).toDouble();
        cutoff_frequency=query.value(1).toDouble();

    }

    QThread::sleep(1);
    const int order = 2; // 4th order (=2 biquads)
    Iir::Butterworth::LowPass<order> fwave;
    //const float samplingrate = 1500; // Hz
    //const float cutoff_frequency = 4; // Hz
    fwave.setup (samplingrate, cutoff_frequency);

    //nt read[200];
    unsigned long lag=ui->lineEdit_32->text().toULong();
    //int lagg=ui->lineEdit_32->text().toInt();
     unsigned long red=ui->lineEdit_33->text().toULong();
    clearData();
    plot_two();
    int wavelength=0;
    wavelength=ui->comboBox_5->currentIndex();
    //qDebug()<<wavelength;
    double blank=ui->label_182->text().toInt();
    double transmission=0,absorbance=0;
    int read1[200];
    double samp=lag+red;

    QThread::msleep(200);
    for(int i=0;i<=samp+1;i++)
    {
        digitalWrite (LED_BASE + 6-wavelength,HIGH) ;
        QThread::msleep(100);
        for(int j=0;j<200;j++)
        {
            read1[j]=readadc(wavelength);
            QThread::msleep(1);
            filtwave[j]=fwave.filter(read1[j]);
            //qDebug()<<read1[j]<<filtwave[j];

        }
        digitalWrite (LED_BASE + 6-wavelength,LOW) ;
        QThread::msleep(10);

        for(int j=100;j<200;j++)
        {
            filtwave[0]+=filtwave[j];

        }
        filtwave[0]=filtwave[0]/100;
        if(i>0)
        {
            transmission=blank/filtwave[0];
            absorbance=log10(transmission);
            //qDebug()<<filtwave[0]<<transmission<<absorbance;
            //ui->label_36->setNum(filtwave[0]);
            //ui->label_37->setText(QString::number(absorbance, 'f', 3));
            addPoint(i-1,absorbance);
            plot_two();
        }
        QApplication::processEvents();
        QThread::msleep(615);
        if(i==lag+1)
        {
            ui->label_180->setNum(filtwave[0]);
            ui->label_177->setText(QString::number(absorbance, 'f', 3));
        }
        //QTime ct = QTime::currentTime();
        //qDebug()<<i<<ct;
    }
    ui->label_184->setNum(filtwave[0]);
    ui->label_185->setText(QString::number(absorbance, 'f', 3));
    double od1=ui->label_177->text().toInt();
    double od2=ui->label_185->text().toInt();
    double dod=od1-od2;
    ui->label_196->setText(QString::number(dod, 'f', 3));

}

void MainWindow::plot_two()
{
    int lag=ui->lineEdit_32->text().toInt();
    int red=ui->lineEdit_33->text().toInt();
    int samp=lag+red;
    //ui->customPlot_2->addGraph();
    //qDebug()<<qv_x<<qv_y
    ui->customPlot_8->graph(0)->setData(qv_x,qv_y);
    //ui->customPlot_2->graph(0)->setVisible(false);
    //ui->customPlot_2->addGraph();
    // give the axes some labels:
    ui->customPlot_8->xAxis->setLabel("POINTS");
    ui->customPlot_8->yAxis->setLabel("OD");
    // set axes ranges, so we see all data:
    ui->customPlot_8->xAxis->setRange(0, samp);
    ui->customPlot_8->yAxis->setRange(-0.5,bc_y_val+1);
    ui->customPlot_8->replot();
    ui->customPlot_8->update();

}

void MainWindow::on_pushButton_104_clicked()
{
    ui->label_195->setText("");
    ui->label_193->setText("");
    ui->label_194->setText("");
    ui->label_186->setText("");
    ui->label_199->setText("");
    ui->label_198->setText("");
    ui->label_200->setText("");
    ui->label_201->setText("");
    ui->label_202->setText("");
    ui->label_203->setText("");
    ui->label_204->setText("");
    double samplingrate=0,cutoff_frequency=0;
    QSqlQuery query;
    query.prepare("select samprate, cutoff from FIA where sno=1");
    query.exec();
    while(query.next())
    {

        samplingrate=query.value(0).toDouble();
        cutoff_frequency=query.value(1).toDouble();

    }

    QThread::sleep(1);
    const int order = 2; // 4th order (=2 biquads)
    Iir::Butterworth::LowPass<order> fwave;
    //const float samplingrate = 1500; // Hz
    //const float cutoff_frequency = 4; // Hz
    fwave.setup (samplingrate, cutoff_frequency);

    //nt read[200];
    unsigned long lag=ui->lineEdit_35->text().toULong();
    //int lagg=ui->lineEdit_32->text().toInt();
     unsigned long red=ui->lineEdit_34->text().toULong();
     unsigned long del=ui->lineEdit_36->text().toULong();
    clearData();
    plot_kinetic();
    int wavelength=0;
    wavelength=ui->comboBox_6->currentIndex();
    //qDebug()<<wavelength;
    double blank=ui->label_189->text().toInt();
    double transmission=0,absorbance=0;
    int read1[200];
    double samp=lag+(red*del);

    QThread::msleep(200);
    for(int i=0;i<=samp+1;i++)
    {
        digitalWrite (LED_BASE + 6-wavelength,HIGH) ;
        QThread::msleep(100);
        for(int j=0;j<200;j++)
        {
            read1[j]=readadc(wavelength);
            QThread::msleep(1);
            filtwave[j]=fwave.filter(read1[j]);
            //qDebug()<<read1[j]<<filtwave[j];

        }
        digitalWrite (LED_BASE + 6-wavelength,LOW) ;
        QThread::msleep(10);

        for(int j=100;j<200;j++)
        {
            filtwave[0]+=filtwave[j];

        }
        filtwave[0]=filtwave[0]/100;
        if(i>0)
        {
            transmission=blank/filtwave[0];
            absorbance=log10(transmission);
            //qDebug()<<filtwave[0]<<transmission<<absorbance;
            //ui->label_36->setNum(filtwave[0]);
            //ui->label_37->setText(QString::number(absorbance, 'f', 3));
            addPoint(i-1,absorbance);
            plot_kinetic();
        }
        QApplication::processEvents();
        QThread::msleep(615);
        if(i==lag+1)
        {
            ui->label_195->setNum(filtwave[0]);
            ui->label_193->setText(QString::number(absorbance, 'f', 3));
        }
        if(i==lag+(del*1))
        {
            ui->label_194->setNum(filtwave[0]);
            ui->label_186->setText(QString::number(absorbance, 'f', 3));
        }
        if(i==lag+(del*2))
        {
            ui->label_199->setNum(filtwave[0]);
            ui->label_198->setText(QString::number(absorbance, 'f', 3));
        }
        if(i==lag+(del*3))
        {
            ui->label_200->setNum(filtwave[0]);
            ui->label_201->setText(QString::number(absorbance, 'f', 3));
        }
        //QTime ct = QTime::currentTime();
        //qDebug()<<i<<ct;
    }
   if(red==2)
   {
    double od1=ui->label_193->text().toInt();
    double od2=ui->label_186->text().toInt();
    double od3=ui->label_198->text().toInt();
    double dod1=od1-od2;
    double dod2=od2-od3;
    ui->label_202->setText(QString::number(dod1, 'f', 3));
    ui->label_203->setText(QString::number(dod2, 'f', 3));
   }
   if(red==3)
   {
    double od1=ui->label_193->text().toInt();
    double od2=ui->label_186->text().toInt();
    double od3=ui->label_198->text().toInt();
    double od4=ui->label_201->text().toInt();
    double dod1=od1-od2;
    double dod2=od2-od3;
    double dod3=od3-od4;
    ui->label_202->setText(QString::number(dod1, 'f', 3));
    ui->label_203->setText(QString::number(dod2, 'f', 3));
    ui->label_204->setText(QString::number(dod3, 'f', 3));
   }
}

void MainWindow::plot_kinetic()
{
    int lag=ui->lineEdit_35->text().toInt();
    int red=ui->lineEdit_34->text().toInt();
     int del=ui->lineEdit_36->text().toInt();
    int samp=lag+(red*del);
    //ui->customPlot_2->addGraph();
    //qDebug()<<qv_x<<qv_y
    ui->customPlot_9->graph(0)->setData(qv_x,qv_y);
    //ui->customPlot_2->graph(0)->setVisible(false);
    //ui->customPlot_2->addGraph();
    // give the axes some labels:
    ui->customPlot_9->xAxis->setLabel("POINTS");
    ui->customPlot_9->yAxis->setLabel("OD");
    // set axes ranges, so we see all data:
    ui->customPlot_9->xAxis->setRange(0, samp);
    ui->customPlot_9->yAxis->setRange(-0.5,bc_y_val+1);
    ui->customPlot_9->replot();
    ui->customPlot_9->update();


}

