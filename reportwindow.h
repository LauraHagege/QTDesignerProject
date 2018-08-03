#ifndef REPORTWINDOW_H
#define REPORTWINDOW_H

#include <QMainWindow>

using namespace std;

namespace Ui {
class ReportWindow;
}

class ReportWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ReportWindow(QWidget *parent = 0);
    ~ReportWindow();

private slots:
    void display_clinical();
    void display_simplified();
    void display_two_report();

private:
    Ui::ReportWindow *ui;
    void add_buttons();
    void render_report();
};

#endif // REPORTWINDOW_H
