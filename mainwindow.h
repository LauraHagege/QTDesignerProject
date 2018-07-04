#ifndef MAINWINDOW_H
#define MAINWINDOW_H
//#include "/Users/laurah/Desktop/malon/dcmtk-3.6.3-install/usr/local/include/dcmtk/dcmdata/dctk.h"
//#include "/Users/laurah/Desktop/malon/dcmtk-3.6.3-install/usr/local/include/dcmtk/dcmimgle/dcmimage.h"
//#include <dcmtk/config/osconfig.h>
//#include <dcmtk/dcmimgle/dcmimage.h>
//#include "/Users/laurah/Desktop/malon/dcmtk-3.6.3/dcmimgle/libsrc/dcmimage.cc"
#include <QString>

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setFilepath(QString path);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
