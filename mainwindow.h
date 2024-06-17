#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QLabel;
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QString fileContent;
    QString analysisResult;
    QLabel *LabRowInfo;
    QLabel *LabSumLine;
    QLabel *LabFileName;
    QString currentFileName;
    int TotalGames;

    QString analyzeTeamWinRate(const QString &teamName, const QString &content);
private slots:
    void on_actOpen_triggered();

    void on_actAnalysis_triggered();

    void on_actSave_triggered();

    void on_actAbout_triggered();

    void on_plainTextEdit_cursorPositionChanged();

    void on_actZoomIn_triggered();

    void on_actZoomOut_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
