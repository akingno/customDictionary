#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <unordered_map>
#include <string>

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
private slots:
    void translateWord();
    void addWord();

private:
    Ui::MainWindow *ui;
    std::unordered_map<std::string, std::string> dict_;
    const QString dictPath_ = "dic.json";

    void loadDictionary();
    void saveDictionary() const;
};
#endif // MAINWINDOW_H
