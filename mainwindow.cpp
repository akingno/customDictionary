#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(550, 300);

    createShortcuts();

    connect(ui->btn_translate, &QPushButton::clicked,
            this, &MainWindow::translateWord);

    connect(ui->btn_add, &QPushButton::clicked,
            this, &MainWindow::addWord);

    recent_word_model = new QStringListModel(this);
    ui->added_word_list->setModel(recent_word_model);
    ui->added_word_list->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(recent_word_model, &QAbstractItemModel::rowsInserted,
        this, [this](const QModelIndex &parent, int first, int last) {
    Q_UNUSED(parent);
    Q_UNUSED(last);
    ui->added_word_list->scrollTo(recent_word_model->index(first),
                                  QAbstractItemView::PositionAtTop);});

    loadDictionary();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createShortcuts()            // NEW
{
    sc_to_right = new QShortcut(this);
    sc_to_left  = new QShortcut(this);
    sc_enter = new QShortcut(this);

    sc_to_right->setKey(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
    sc_to_left ->setKey(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
    sc_enter   ->setKey(QKeySequence(Qt::CTRL | Qt::Key_Backslash));


    sc_to_right->setAutoRepeat(false);
    sc_to_left ->setAutoRepeat(false);
    sc_enter ->setAutoRepeat(false);
    // Ctrl + → : 中文 → 外文

    connect(sc_to_right, &QShortcut::activated, this,
            [this]{ ui->word_akn->setFocus(); });

    // Ctrl + ← : 外文 → 中文

    connect(sc_to_left, &QShortcut::activated, this,
            [this]{ ui->word_cn->setFocus(); });

    connect(sc_enter, &QShortcut::activated, this,
            &MainWindow::addWord);
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
        const std::string cn  = it.key().toStdString();
        const std::string akn = it.value().toString().toStdString();
        dict_[cn] = akn;
        cn_set_.insert(cn);
        akn_set_.insert(akn);
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
        // 当未找到时, 添加如添加框方便添加
        ui->word_cn->setText(cn);
        statusBar()->showMessage(u8"⚠️ 未找到，已填入添加框", 3000);
    }
}

void MainWindow::addWord()
{
    const QString  cnQ  = ui->word_cn->text().trimmed();
    const QString  aknQ = ui->word_akn->text().trimmed();

    std::string cn  = cnQ .toStdString();
    std::string akn = aknQ.toStdString();

    if (cn.empty() || akn.empty()) {
        statusBar()->showMessage(u8"❌ 输入为空，添加失败", 3000);
        return;
    }
    //如果单词有重复
    if (akn_set_.count(akn) > 0) {
        // 使用对话框询问用户是否继续添加
        auto reply = QMessageBox::question(
            this,
            tr("此单词已存在"),
            tr("此单词 \"%1\" 已在字典中，是否仍要添加？")
                .arg(aknQ),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            // 用户选择不继续添加
            statusBar()->showMessage(u8"⛔ 已取消添加", 3000);
            return;
        }
    }

    dict_[cn] = akn;
    cn_set_.insert(cn);
    akn_set_.insert(akn);
    saveDictionary();
    statusBar()->showMessage(u8"✅ 添加成功", 2000);

    // 添加进最近添加栏
    QString combined = cnQ + " – " + aknQ;
    recent_list.prepend(combined);

    if (recent_list.size() > MAX_RECENT)
        recent_list.removeLast();

    recent_word_model->setStringList(recent_list);

    // 清除输入
    ui->word_cn->clear();
    ui->word_akn->clear();
}
