#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <iomanip>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(500, 300);

    connect(ui->btn_translate, &QPushButton::clicked,
            this, &MainWindow::translateWord);

    connect(ui->btn_add, &QPushButton::clicked,
            this, &MainWindow::addWord);

    loadDictionary();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadDictionary()
{
    statusBar()->showMessage(u8"正在读取词典...", 5000);
    QFile file(dictPath_);
    if (!file.open(QIODevice::ReadOnly))
        return;                                           // 首次运行还没有文件

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    file.close();

    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        statusBar()->showMessage(u8"❌ 词典读取失败", 3000);
        return;
    }

    const QJsonObject obj = doc.object();
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {    // 遍历键值对
        dict_[it.key().toStdString()] = it.value().toString().toStdString();
    }
    statusBar()->showMessage(u8"读取完成", 3000);
}

void MainWindow::saveDictionary() const
{
    QJsonObject obj;
    for (const auto &pair : dict_)
        obj.insert(QString::fromStdString(pair.first),
                   QString::fromStdString(pair.second));

    QJsonDocument doc(obj);

    QFile file(dictPath_);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        statusBar()->showMessage(u8"❌ 词典保存失败", 3000);
        return;
    }
    file.write(doc.toJson(QJsonDocument::Indented));      // 自动缩进格式化 :contentReference[oaicite:3]{index=3}
    file.close();            // pretty print :contentReference[oaicite:5]{index=5}
}

void MainWindow::translateWord()
{
    const QString cn = ui->word_input->text().trimmed();
    if (cn.isEmpty()) return;

    const auto it = dict_.find(cn.toStdString());
    if (it != dict_.end()) {
        ui->label_output->setText(QString::fromStdString(it->second));
        statusBar()->showMessage(u8"✅ 找到翻译", 2000);
    } else {
        ui->label_output->setText(u8"未找到");
        ui->word_cn->setText(cn);                        // auto‑fill for quick add
        statusBar()->showMessage(u8"⚠️ 未找到，已填入添加框", 3000);
    }
}

void MainWindow::addWord()
{
    const QString cn  = ui->word_cn->text().trimmed();
    const QString akn = ui->word_akn->text().trimmed();
    if (cn.isEmpty() || akn.isEmpty()) {
        statusBar()->showMessage(u8"❌ 输入为空，添加失败", 3000);
        return;
    }

    dict_[cn.toStdString()] = akn.toStdString();         // O(1) update, unordered_map :contentReference[oaicite:6]{index=6}
    saveDictionary();
    statusBar()->showMessage(u8"✅ 添加成功", 2000);

    ui->word_cn->clear();
    ui->word_akn->clear();
}
