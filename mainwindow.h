#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <unordered_map>
#include <unordered_set>
#include <QShortcut>
#include <string>
#include <QStringListModel>

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
    std::unordered_map<std::string, std::string> dict_; // 词典
    std::unordered_set<std::string> cn_set_;   // 重复提醒用
    std::unordered_set<std::string> akn_set_;   //重复提醒用
    const QString dictPath_ = "dic.json";       //保存词典文件名

    QShortcut * sc_to_right;
    QShortcut * sc_to_left;
    QShortcut * sc_enter;

    QStringListModel * recent_word_model = nullptr;
    QStringList recent_list;
    const int MAX_RECENT = 15;

    void loadDictionary();
    void saveDictionary() const;
    void createShortcuts();
};
#endif // MAINWINDOW_H
