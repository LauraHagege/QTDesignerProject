#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog.h"
#include <QLabel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setFilepath(QString path){
    QMessageBox::information(this,tr("Directory path"), path);

}
