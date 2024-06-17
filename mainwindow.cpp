#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>
#include <QDateTime>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    LabFileName = new QLabel("当前打开文件:",this);
    LabFileName->setMinimumWidth(200);
    ui->statusbar->addWidget(LabFileName);
    LabRowInfo = new QLabel("当前行:",this);
    LabRowInfo->setMinimumWidth(200);
    ui->statusbar->addWidget(LabRowInfo);
    LabSumLine = new QLabel("总行数:",this);
    LabRowInfo->setMinimumWidth(200);
    ui->statusbar->addWidget(LabSumLine);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actOpen_triggered()
{
    ui->plainTextEdit->clear();
    QString curPath = QCoreApplication::applicationDirPath();
    QString fileName = QFileDialog::getOpenFileName(this, "Open the file",curPath,"数据文件(*.txt);;(*.*)");
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Warning", "Cannot open file: " + file.errorString());
        LabFileName->setText(QString("当前打开文件:"));
        LabRowInfo->setText(QString("当前行数:"));
        LabSumLine->setText(QString("总行数:"));
        return;
    }
    else{
        currentFileName = fileName;
        QTextStream in(&file);
        fileContent = in.readAll();
        ui->plainTextEdit->appendPlainText(fileContent);
        file.close();
        // 打开文件时，光标设置在文件头部位置
        QTextCursor cursor = ui->plainTextEdit->textCursor();
        cursor.movePosition(QTextCursor::Start);
        ui->plainTextEdit->setTextCursor(cursor);

        int totalLines = ui->plainTextEdit->document()->blockCount();
        LabFileName->setText(QString("当前打开文件： %1 ").arg(fileName));
        LabSumLine->setText(QString("总行数: %1 ").arg(totalLines));

        ui->actSave->setEnabled(true);
        ui->actAnalysis->setEnabled(true);
        ui->actZoomIn->setEnabled(true);
        ui->actZoomOut->setEnabled(true);
    }
}


void MainWindow::on_actAnalysis_triggered()
{
    bool ok;
    QString teamName = QInputDialog::getText(this, "输入主队名称", "主队名称:", QLineEdit::Normal, "YuShan2024", &ok);
    if (ok && !teamName.isEmpty()) {
        analysisResult = analyzeTeamWinRate(teamName, fileContent);
        ui->plainTextEdit->setPlainText(analysisResult);
    }
}

void MainWindow::on_actSave_triggered()
{
    if(currentFileName.isEmpty()){
        QMessageBox::warning(this,"保存错误","当前没有打开的文件！");
        return;
    }
    QString defaultFileName = QFileInfo(currentFileName).fileName();
    QString filePath = QFileDialog::getSaveFileName(this, "保存分析结果", defaultFileName, "文本文件 (*.txt);;所有文件 (*.*)");

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            // 添加当前时间、总测试场数到分析结果的顶部
            QDateTime currentTime = QDateTime::currentDateTime();
            QString timestamp = currentTime.toString("yyyy-MM-dd hh:mm:ss");
            out << "\n"<< "Save At " << timestamp << "\n\n";
            out << "Total Games : " << TotalGames << "\n\n";
            out << analysisResult;
            file.close();
        } else {
            QMessageBox::warning(this, "文件保存失败", "无法保存文件！");
        }
    }
}


QString MainWindow::analyzeTeamWinRate(const QString &teamName, const QString &content) {
    QStringList lines = content.split("\n");
    QMap<QString, QMap<QString, QVariant>> stats; // 用于存储不同对手的统计数据

    int totalLines = lines.size();
    Q_UNUSED(totalLines);
    int processedLines = 0;

    for (const QString &line : lines) {
        if (line.trimmed().isEmpty()) continue;

        QStringList parts = line.split("\t");
        if (parts.size() < 3) {
            qDebug() << "Skipping line due to insufficient parts:" << line;
            continue;
        }
        QString teams = parts[0];
        QString score = parts[1];
        QString log = parts[2];

        QStringList teamNames = teams.split(":");
        QStringList scores = score.split(":");

        if (teamNames.size() != 2 || scores.size() != 2){
            qDebug() << "Skipping line due to incorrect team names or scores format:" << line;
            continue;
        }

        QString homeTeam = teamNames[0].trimmed();
        QString awayTeam = teamNames[1].trimmed();
        int homeScore = scores[0].toInt();
        int awayScore = scores[1].toInt();

        // QString opponent = homeTeam == teamName ? awayTeam : homeTeam == teamName ? homeTeam : QString();
        QString opponent = (homeTeam == teamName) ? awayTeam : homeTeam;
        if (opponent.isEmpty()){
             qDebug() << "Skipping line due to team name mismatch:" << line;
            continue;
        }

        QMap<QString, QVariant> &teamStats = stats[opponent];
        if (teamStats.isEmpty()) {
            teamStats["win"] = 0;
            teamStats["draw"] = 0;
            teamStats["lose"] = 0;
            teamStats["goalsFor"] = 0;
            teamStats["goalsAgainst"] = 0;
            teamStats["winLogs"] = QStringList();
            teamStats["drawLogs"] = QStringList();
            teamStats["loseLogs"] = QStringList();
        }

        teamStats["goalsFor"] = teamStats["goalsFor"].toInt() + (homeTeam == teamName ? homeScore : awayScore);
        teamStats["goalsAgainst"] = teamStats["goalsAgainst"].toInt() + (homeTeam == teamName ? awayScore : homeScore);

        QStringList winLogs = teamStats["winLogs"].toStringList();
        QStringList drawLogs = teamStats["drawLogs"].toStringList();
        QStringList loseLogs = teamStats["loseLogs"].toStringList();

        if (homeTeam == teamName) {
            if (homeScore > awayScore) {
                teamStats["win"] = teamStats["win"].toInt() + 1;
                winLogs.append(log);
            } else if (homeScore == awayScore) {
                teamStats["draw"] = teamStats["draw"].toInt() + 1;
                drawLogs.append(log);
            } else {
                teamStats["lose"] = teamStats["lose"].toInt() + 1;
                loseLogs.append(log);
            }
        } else {
            if (awayScore > homeScore) {
                teamStats["win"] = teamStats["win"].toInt() + 1;
                winLogs.append(log);
            } else if (awayScore == homeScore) {
                teamStats["draw"] = teamStats["draw"].toInt() + 1;
                drawLogs.append(log);
            } else {
                teamStats["lose"] = teamStats["lose"].toInt() + 1;
                loseLogs.append(log);
            }
        }

        teamStats["winLogs"] = winLogs;
        teamStats["drawLogs"] = drawLogs;
        teamStats["loseLogs"] = loseLogs;
        processedLines++;
    }

    TotalGames = processedLines;
    // qDebug() << "Total lines:" << totalLines;
    // qDebug() << "Processed lines:" << processedLines;

    QString result;
    QTextStream out(&result);
    QMapIterator<QString, QMap<QString, QVariant>> i(stats);
    while (i.hasNext()) {
        i.next();
        QString opponent = i.key();
        QMap<QString, QVariant> teamStats = i.value();
        int win = teamStats["win"].toInt();
        int draw = teamStats["draw"].toInt();
        int lose = teamStats["lose"].toInt();
        int goalsFor = teamStats["goalsFor"].toInt();
        int goalsAgainst = teamStats["goalsAgainst"].toInt();
        int totalGames = win + draw + lose;
        double winRate = totalGames > 0 ? (win * 3.0 + draw) / (totalGames * 3) * 100 : 0;
        double avgGoalsFor = totalGames > 0 ? static_cast<double>(goalsFor) / totalGames : 0;
        double avgGoalsAgainst = totalGames > 0 ? static_cast<double>(goalsAgainst) / totalGames : 0;

        out << teamName << " -VS- " << opponent << "\n";
        out << "WIN\tPEACE\tLOSE\n";
        out << win << "\t" << draw << "\t" << lose << "\n";
        out << "-Total-:\t" << totalGames << "\n";
        out << "-GainNum-:\t" << goalsFor << "\n";
        out << "-LoseNum-:\t" << goalsAgainst << "\n";
        out << "Win_Prob:\t" << QString::number(winRate, 'f', 2) << "%\n";
        out << "Avg-Gain:\t" << QString::number(avgGoalsFor, 'f', 2) << "\n";
        out << "Avg-Lose:\t" << QString::number(avgGoalsAgainst, 'f', 2) << "\n";
        out << "----------\n--WIN--\n";
        for (const QString &log : teamStats["winLogs"].toStringList()) {
            out << log << "\n";
        }
        out << "--PEACE--\n";
        for (const QString &log : teamStats["drawLogs"].toStringList()) {
            out << "\t" <<log << "\n";
        }
        out << "--LOSE--\n";
        for (const QString &log : teamStats["loseLogs"].toStringList()) {

            out << "\t\t" << log << "\n";
            }
        out <<"******************************************************************************************" << "\n";
    }
    return result;

}



void MainWindow::on_actAbout_triggered()
{
    QMessageBox::information(this,tr("Result Analysis"),tr("<p>Result Analysis Tool<br/>""Developed by Zhang<br/>""Copyright © 2024 </p>"));
}


void MainWindow::on_plainTextEdit_cursorPositionChanged()
{
    // 当光标位置发生变化时，行数发生变化
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    int lineNumber = cursor.blockNumber() + 1;
    LabRowInfo->setText(QString("当前行数: %1").arg(lineNumber));
}


void MainWindow::on_actZoomIn_triggered()
{
    QFont currentFont = ui->plainTextEdit->font();
    int newFontSize = currentFont.pointSize()+1;
    currentFont.setPointSize(newFontSize);
    ui->plainTextEdit->setFont(currentFont);
}


void MainWindow::on_actZoomOut_triggered()
{
    QFont currentFont = ui->plainTextEdit->font();
    int newFontSize = currentFont.pointSize()-1;
    currentFont.setPointSize(newFontSize);
    ui->plainTextEdit->setFont(currentFont);
}

