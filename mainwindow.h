#ifndef MAINWINDOW_H
#define MAINWINDOW_H
//#include "/Users/laurah/Desktop/malon/dcmtk-3.6.3-install/usr/local/include/dcmtk/dcmdata/dctk.h"
//#include "/Users/laurah/Desktop/malon/dcmtk-3.6.3-install/usr/local/include/dcmtk/dcmimgle/dcmimage.h"
//#include <dcmtk/config/osconfig.h>
//#include <dcmtk/dcmimgle/dcmimage.h>
//#include "/Users/laurah/Desktop/malon/dcmtk-3.6.3/dcmimgle/libsrc/dcmimage.cc"
#include <QString>

#include <QMainWindow>


#include "qimage.h"

//#include <dcmtk/config/cfunix.h>
//#include <dcmtk/config/osconfig.h>
//#include <dcmtk/ofstd/ofstring.h>
//#include <dcmtk/ofstd/ofconsol.h>
//#include <dcmtk/dcmdata/dcdeftag.h>
//#include <dcmtk/dcmdata/dcfilefo.h>
//#include <dcmtk/dcmimgle/dcmimage.h>
//#include "dcmtk/dcmimage/diregist.h"
#include <QImage>
#include <QGraphicsScene>
#include <QLabel>
#include <QStackedWidget>
#include <QScrollBar>
#include <vector>
#include <map>
#include <string>

using namespace std;

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

private slots:

    void on_pushButton_clicked();
    void wheelEvent(QWheelEvent *event);

private:
    int index;
    int series;
    map<string, vector<QImage*>> Series;
    Ui::MainWindow *ui;
    vector<QImage*> Images;
    QImage *currentImage;
    QImage *secondImage;
    QGraphicsScene *myScene;
    QGraphicsScene *myScene2;
    QGraphicsScene *myScene3;

    QStackedWidget *stacked;
    QScrollBar *scrollbar;
};






#endif // MAINWINDOW_H
