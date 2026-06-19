#include "gamewindow.h"
#include "core/game.h"
#include "core/shop.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QVariant>

namespace {
QString displayNameForUnit(const QString& name)
{
    if (name == QStringLiteral("Knight"))  return QStringLiteral("骑士");
    if (name == QStringLiteral("Archer"))  return QStringLiteral("弓手");
    if (name == QStringLiteral("Mage"))    return QStringLiteral("法师");
    if (name == QStringLiteral("Priest"))  return QStringLiteral("牧师");
    if (name == QStringLiteral("Stalker")) return QStringLiteral("潜行者");
    if (name == QStringLiteral("Paladin")) return QStringLiteral("圣骑士");
    if (name == QStringLiteral("Goblin"))  return QStringLiteral("哥布林");
    if (name == QStringLiteral("Brute"))   return QStringLiteral("蛮兵");
    return name;
}


QString shopIconPathForUnit(const QString& name)
{
    if (name == QStringLiteral("Knight"))
        return QStringLiteral("assets/craftpix-reaper-man-chibi-2d-game-sprites/Reaper_Man_1/PNG/PNG Sequences/Idle/0_Reaper_Man_Idle_000.png");
    if (name == QStringLiteral("Archer"))
        return QStringLiteral("assets/craftpix-satyr-tiny-style-2d-sprites/PNG/Satyr_01/PNG Sequences/Idle/Satyr_01_Idle_000.png");
    if (name == QStringLiteral("Mage"))
        return QStringLiteral("assets/craftpix-satyr-tiny-style-2d-sprites/PNG/Satyr_02/PNG Sequences/Idle/Satyr_02_Idle_000.png");
    if (name == QStringLiteral("Goblin"))
        return QStringLiteral("assets/craftpix-satyr-tiny-style-2d-sprites/PNG/Satyr_03/PNG Sequences/Idle/Satyr_03_Idle_000.png");
    if (name == QStringLiteral("Brute"))
        return QStringLiteral("assets/craftpix-reaper-man-chibi-2d-game-sprites/Reaper_Man_3/PNG/PNG Sequences/Idle/0_Reaper_Man_Idle_000.png");
    // 其余玩家单位先用一张默认图
    return QStringLiteral("assets/craftpix-reaper-man-chibi-2d-game-sprites/Reaper_Man_2/PNG/PNG Sequences/Idle/0_Reaper_Man_Idle_000.png");
}

QIcon loadShopIcon(const QString& unitName)
{
    const QString relative = shopIconPathForUnit(unitName);
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString roots[] = {
        QFileInfo(appDir + "/..").canonicalFilePath(),
        QFileInfo(appDir + "/../..").canonicalFilePath()
    };

    QPixmap pix;
    for (const QString& root : roots) {
        if (root.isEmpty()) continue;
        pix.load(root + "/" + relative);
        if (!pix.isNull()) break;
    }
    if (pix.isNull()) return QIcon();
    return QIcon(pix.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
}

GameWindow::GameWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_centralWidget(new QWidget(this))
    , m_mainLayout(new QVBoxLayout())
    , m_view(new QGraphicsView(this))
    , m_resetButton(new QPushButton(QStringLiteral("新开局"), this))
    , m_refreshShopButton(new QPushButton(QStringLiteral("刷新商店 (2金)"), this))
    , m_startCombatButton(new QPushButton(QStringLiteral("开始战斗"), this))
    , m_buyExpButton(new QPushButton(QStringLiteral("购买经验 (4金)"), this))
    , m_saveButton(new QPushButton(QStringLiteral("存档"), this))
    , m_loadButton(new QPushButton(QStringLiteral("读档"), this))
    , m_statusLabel(new QLabel(this))
    , m_synergyLabel(new QLabel(this))
    , m_equipmentLabel(new QLabel(this))
    , m_playerStatsLabel(new QLabel(this))
    , m_hpBar(new QProgressBar(this))
    , m_game(new Game(this))
{
    setupUI();
    m_game->initialize();
}

GameWindow::~GameWindow() = default;

void GameWindow::onResetButtonClicked()
{
    // 软重置：清掉所有单位，重新初始化
    m_game->clearAllUnits();
    m_game->setRound(1);
    m_game->initialize();
    refreshUI();
}

void GameWindow::onStartCombatClicked()
{
    if (m_game) {
        m_game->startCombat();
        refreshUI();
    }
}

void GameWindow::onRefreshShopClicked()
{
    if (m_game) {
        m_game->refreshShop();
        refreshUI();
    }
}

void GameWindow::onShopButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button || !m_game) return;
    const int index = button->property("shopIndex").toInt();
    m_game->buyShopUnit(index);
    refreshUI();
}

void GameWindow::onBenchButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button || !m_game) return;
    const int index = button->property("benchIndex").toInt();
    m_game->deployBenchUnit(index);
    refreshUI();
}

void GameWindow::onBuyExpClicked()
{
    if (m_game) {
        m_game->buyExperience();
        refreshUI();
    }
}

QString GameWindow::defaultSavePath() const
{
    return QDir(QCoreApplication::applicationDirPath()).filePath("synera_save.json");
}

void GameWindow::onSaveClicked()
{
    const QString path = QFileDialog::getSaveFileName(this,
        QStringLiteral("保存存档"), defaultSavePath(),
        QStringLiteral("Synera 存档 (*.json)"));
    if (path.isEmpty()) return;

    if (m_game->saveToFile(path)) {
        QMessageBox::information(this, QStringLiteral("存档成功"),
            QStringLiteral("已写入：\n%1").arg(path));
    } else {
        QMessageBox::warning(this, QStringLiteral("存档失败"),
            QStringLiteral("写入文件时出错。"));
    }
}

void GameWindow::onLoadClicked()
{
    const QString path = QFileDialog::getOpenFileName(this,
        QStringLiteral("读取存档"), defaultSavePath(),
        QStringLiteral("Synera 存档 (*.json)"));
    if (path.isEmpty()) return;

    if (m_game->loadFromFile(path)) {
        refreshUI();
        QMessageBox::information(this, QStringLiteral("读档成功"),
            QStringLiteral("已恢复到存档状态。"));
    } else {
        QMessageBox::warning(this, QStringLiteral("读档失败"),
            QStringLiteral("文件损坏或格式不正确。"));
    }
}

void GameWindow::onGameOver(bool playerWon)
{
    Q_UNUSED(playerWon);
    QMessageBox::information(this, QStringLiteral("游戏结束"),
        QStringLiteral("玩家血量归零，本局失败。\n点击「新开局」重新开始。"));
}

void GameWindow::setupUI()
{
    setWindowTitle(QStringLiteral("Synera 自走棋 — 忽哲 251880102"));
    resize(1280, 820);

    setCentralWidget(m_centralWidget);
    m_centralWidget->setLayout(m_mainLayout);
    m_mainLayout->setContentsMargins(14, 14, 14, 14);
    m_mainLayout->setSpacing(10);

    // 暖夜主题：深紫蓝底 + 琥珀高亮
    setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #15172a;
            color: #f1ecda;
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
        }
        QPushButton {
            background-color: #2b2d4a;
            color: #f1ecda;
            border: 1px solid #4a4e7a;
            border-radius: 5px;
            padding: 6px 10px;
            font-size: 12px;
            min-height: 22px;
        }
        QPushButton:hover { background-color: #3a3e64; border-color: #e9b44c; }
        QPushButton:disabled { color: #6f7280; background-color: #1d1f33; border-color: #2a2c46; }

        QPushButton#actionButton {
            background-color: #c0832e;
            border: 1px solid #e9b44c;
            color: #fff5dd;
            font-weight: 600;
        }
        QPushButton#actionButton:hover { background-color: #e9b44c; color: #2b1f0a; }
        QPushButton#dangerButton {
            background-color: #6a2d3a;
            border: 1px solid #913f55;
        }
        QPushButton#dangerButton:hover { background-color: #913f55; }

        QPushButton#shopCard {
            text-align: left;
            padding-left: 8px;
            font-size: 12px;
        }

        QLabel { font-size: 12px; }
        QLabel#statusLabel { color: #e9b44c; font-weight: 600; font-size: 13px; }
        QLabel#sectionTitle {
            color: #b3c6ff;
            font-size: 14px;
            font-weight: 700;
            padding: 4px 0;
            border-bottom: 1px solid #353a60;
        }
        QLabel#detailLabel { color: #d8dde7; }

        QLabel#statCard {
            background-color: #232547;
            color: #f1ecda;
            border: 1px solid #353a60;
            border-radius: 6px;
            padding: 4px 12px;
            font-size: 12px;
            font-weight: 600;
        }

        QWidget#topBar, QWidget#sidePanel, QWidget#benchPanel {
            background-color: #1d1f3a;
            border: 1px solid #353a60;
            border-radius: 8px;
        }
        QGraphicsView {
            background-color: #0f1020;
            border: 1px solid #353a60;
            border-radius: 8px;
        }
        QProgressBar {
            border: 1px solid #353a60;
            border-radius: 4px;
            background: #0f1020;
            text-align: center;
            color: #f1ecda;
            height: 16px;
        }
        QProgressBar::chunk {
            background-color: #c84a4a;
            border-radius: 3px;
        }
    )");

    // -------- 顶部状态栏：状态文字 + 三个小卡片 + 按钮组 --------
    QWidget* topBar = new QWidget(this);
    topBar->setObjectName("topBar");
    QHBoxLayout* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(12, 8, 12, 8);
    topLayout->setSpacing(8);

    m_statusLabel->setObjectName("statusLabel");
    topLayout->addWidget(m_statusLabel);

    // 三个小卡片
    m_goldLabel  = new QLabel(this);
    m_levelLabel = new QLabel(this);
    m_popLabel   = new QLabel(this);
    for (QLabel* card : { m_goldLabel, m_levelLabel, m_popLabel }) {
        card->setObjectName("statCard");
        card->setAlignment(Qt::AlignCenter);
        topLayout->addWidget(card);
    }

    topLayout->addStretch(1);

    m_startCombatButton->setObjectName("actionButton");
    m_resetButton->setObjectName("dangerButton");

    topLayout->addWidget(m_buyExpButton);
    topLayout->addWidget(m_refreshShopButton);
    topLayout->addWidget(m_startCombatButton);
    topLayout->addWidget(m_saveButton);
    topLayout->addWidget(m_loadButton);
    topLayout->addWidget(m_resetButton);
    m_mainLayout->addWidget(topBar);

    // -------- 玩家血量行 --------
    QWidget* hpRow = new QWidget(this);
    QHBoxLayout* hpLayout = new QHBoxLayout(hpRow);
    hpLayout->setContentsMargins(0, 0, 0, 0);
    hpLayout->setSpacing(8);
    m_playerStatsLabel->setObjectName("statusLabel");
    m_playerStatsLabel->setMinimumWidth(140);
    m_hpBar->setRange(0, 30);
    m_hpBar->setValue(30);
    m_hpBar->setFormat("%v / %m HP");
    hpLayout->addWidget(m_playerStatsLabel);
    hpLayout->addWidget(m_hpBar, 1);
    m_mainLayout->addWidget(hpRow);

    // -------- 主区：棋盘 + 右侧面板 --------
    QWidget* content = new QWidget(this);
    QHBoxLayout* contentLayout = new QHBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(10);

    m_view->setRenderHint(QPainter::Antialiasing, true);
    m_view->setDragMode(QGraphicsView::NoDrag);
    m_view->setMouseTracking(true);
    m_view->viewport()->setMouseTracking(true);
    m_view->setScene(m_game->scene());
    contentLayout->addWidget(m_view, 1);

    // 右侧面板（商店 + 备战席 + 信息）
    QWidget* sidePanel = new QWidget(this);
    sidePanel->setObjectName("sidePanel");
    sidePanel->setFixedWidth(320);
    QVBoxLayout* sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(12, 12, 12, 12);
    sideLayout->setSpacing(6);

    QLabel* shopTitle = new QLabel(QStringLiteral("招募商店"), this);
    shopTitle->setObjectName("sectionTitle");
    sideLayout->addWidget(shopTitle);
    for (int i = 0; i < 5; ++i) {
        QPushButton* button = new QPushButton(this);
        button->setObjectName("shopCard");
        button->setMinimumHeight(48);
        button->setIconSize(QSize(40, 40));
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setProperty("shopIndex", i);
        connect(button, &QPushButton::clicked,
                this, &GameWindow::onShopButtonClicked);
        m_shopButtons.append(button);
        sideLayout->addWidget(button);
    }

    QLabel* benchTitle = new QLabel(QStringLiteral("备战席 (点击上场)"), this);
    benchTitle->setObjectName("sectionTitle");
    sideLayout->addWidget(benchTitle);
    for (int i = 0; i < 8; ++i) {
        QPushButton* button = new QPushButton(QStringLiteral("空"), this);
        button->setMinimumHeight(28);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setProperty("benchIndex", i);
        connect(button, &QPushButton::clicked,
                this, &GameWindow::onBenchButtonClicked);
        m_benchButtons.append(button);
        sideLayout->addWidget(button);
    }

    QLabel* synergyTitle = new QLabel(QStringLiteral("羁绊"), this);
    synergyTitle->setObjectName("sectionTitle");
    sideLayout->addWidget(synergyTitle);
    m_synergyLabel->setWordWrap(true);
    m_synergyLabel->setObjectName("detailLabel");
    sideLayout->addWidget(m_synergyLabel);

    QLabel* equipmentTitle = new QLabel(QStringLiteral("装备说明"), this);
    equipmentTitle->setObjectName("sectionTitle");
    sideLayout->addWidget(equipmentTitle);
    m_equipmentLabel->setWordWrap(true);
    m_equipmentLabel->setObjectName("detailLabel");
    sideLayout->addWidget(m_equipmentLabel);

    sideLayout->addStretch();
    contentLayout->addWidget(sidePanel);

    m_mainLayout->addWidget(content, 1);

    // Connections
    connect(m_resetButton, &QPushButton::clicked,
            this, &GameWindow::onResetButtonClicked);
    connect(m_startCombatButton, &QPushButton::clicked,
            this, &GameWindow::onStartCombatClicked);
    connect(m_refreshShopButton, &QPushButton::clicked,
            this, &GameWindow::onRefreshShopClicked);
    connect(m_buyExpButton, &QPushButton::clicked,
            this, &GameWindow::onBuyExpClicked);
    connect(m_saveButton, &QPushButton::clicked,
            this, &GameWindow::onSaveClicked);
    connect(m_loadButton, &QPushButton::clicked,
            this, &GameWindow::onLoadClicked);
    connect(m_game, &Game::stateChanged,
            this, &GameWindow::refreshUI);
    connect(m_game, &Game::gameOver,
            this, &GameWindow::onGameOver);

    refreshUI();
}

void GameWindow::refreshUI()
{
    if (!m_game) return;

    m_statusLabel->setText(m_game->statusText());
    m_synergyLabel->setText(m_game->synergyText());
    m_equipmentLabel->setText(m_game->equipmentText());
    m_playerStatsLabel->setText(m_game->playerStatsText());

    Player* p = m_game->player();
    m_hpBar->setRange(0, p->maxHp());
    m_hpBar->setValue(p->hp());

    // 三个小卡片单独更新（statusLabel 那里也有但显得拥挤，所以分开展示一份）
    if (m_goldLabel)  m_goldLabel->setText(QStringLiteral("金币 %1").arg(p->gold()));
    if (m_levelLabel) m_levelLabel->setText(QStringLiteral("等级 %1  (%2/%3)")
                                              .arg(p->level()).arg(p->exp()).arg(p->expToNext()));
    if (m_popLabel) {
        // 这里用 board cap 当上限，count 由 statusText 已经算过；为了简单直接用 boardCap 上限
        m_popLabel->setText(QStringLiteral("人口 上限 %1").arg(p->boardCap()));
    }

    const bool inPrep = m_game->phase() == GamePhase::Prep;
    m_startCombatButton->setEnabled(inPrep && p->isAlive());
    m_refreshShopButton->setEnabled(inPrep && p->isAlive());
    m_buyExpButton->setEnabled(inPrep && p->isAlive());

    const QVector<UnitTemplate>& offers = m_game->shop()->offers();
    for (int i = 0; i < m_shopButtons.size(); ++i) {
        if (i >= offers.size()) {
            m_shopButtons[i]->setText(QStringLiteral("-"));
            m_shopButtons[i]->setIcon(QIcon());
            m_shopButtons[i]->setEnabled(false);
            continue;
        }

        const UnitTemplate& unit = offers.at(i);
        m_shopButtons[i]->setIcon(loadShopIcon(unit.name));
        m_shopButtons[i]->setText(QStringLiteral("  %1   ★%2\n  HP %3   •  %4 金")
            .arg(displayNameForUnit(unit.name))
            .arg(1)
            .arg(unit.stats.maxHp)
            .arg(unit.cost));
        m_shopButtons[i]->setEnabled(inPrep && p->isAlive());
    }

    const QVector<Unit*>& bench = p->bench();
    for (int i = 0; i < m_benchButtons.size(); ++i) {
        if (i >= bench.size() || !bench.at(i)) {
            m_benchButtons[i]->setText(QStringLiteral("[空]"));
            m_benchButtons[i]->setEnabled(false);
            continue;
        }
        Unit* u = bench.at(i);
        m_benchButtons[i]->setText(QStringLiteral("%1 ★%2  HP %3")
            .arg(displayNameForUnit(u->name())).arg(u->star()).arg(u->maxHp()));
        m_benchButtons[i]->setEnabled(inPrep && p->isAlive());
    }
}
