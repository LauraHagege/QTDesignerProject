#include "reportwindow.h"
#include "ui_reportwindow.h"
#include <QFile>
#include <QString>
#include <QTextStream>
#include <iostream>

using namespace std;

ReportWindow::ReportWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ReportWindow)
{
    ui->setupUi(this);
    add_buttons();
    render_report();
}

ReportWindow::~ReportWindow()
{
    delete ui;
}

void ReportWindow::add_buttons(){

    QAction *DisplayClinical = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/w1.png"),"Show clinical report only", this);
    ui->toolBar->addAction(DisplayClinical);
    connect(DisplayClinical,SIGNAL(triggered(bool)),this,SLOT(display_clinical()) );

    QAction *DisplaySimplified = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/lightw1.png"),"Show simplified report only", this);
    ui->toolBar->addAction(DisplaySimplified);
    connect(DisplaySimplified,SIGNAL(triggered(bool)),this,SLOT(display_simplified()) );

    QAction *Display2Report = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/w2.png"),"Show only one window", this);
    ui->toolBar->addAction(Display2Report);
    connect(Display2Report,SIGNAL(triggered(bool)),this,SLOT(display_two_report()) );

}

void ReportWindow::display_clinical(){
    ui->clinicalReport->setVisible(true);
    ui->simplifiedReport->setVisible(false);
}

void ReportWindow::display_simplified(){
    ui->clinicalReport->setVisible(false);
    ui->simplifiedReport->setVisible(true);
}

void ReportWindow::display_two_report(){
    ui->clinicalReport->setVisible(true);
    ui->simplifiedReport->setVisible(true);
}

void ReportWindow::render_report(){
    cout << "render function " << endl;
    QFile file("C://Users//simms//Desktop//Laura//CTAbdomen//README.txt");


    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){

        QTextStream in(&file);
        ui->clinicalReport->setText(in.readAll());
//        while(!in.atEnd()){
//            QString line= in.readLine();
//            QByteArray ba = line.toUtf8();
//           // const char *strline = ba.data();
//            //cout << "test" << strline << endl;
//            ui->clinicalReport->append(QString::fromUtf8(ba));
//        }
    }

    file.close();
}
