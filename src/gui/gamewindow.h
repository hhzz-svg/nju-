#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include <QVector>

class Game;
class QLabel;
class QGraphicsView;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QProgressBar;

class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget* parent = nullptr);
    ~GameWindow();

private slots:
    void onResetButtonClicked();
    void onStartCombatClicked();
    void onRefreshShopClicked();
    void onShopButtonClicked();
    void onBenchButtonClicked();
    void onBuyExpClicked();
    void onSaveClicked();
    void onLoadClicked();
    void onGameOver(bool playerWon);

private:
    void setupUI();
    void refreshUI();
    QString defaultSavePath() const;

    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QGraphicsView* m_view;

    QPushButton* m_resetButton;
    QPushButton* m_refreshShopButton;
    QPushButton* m_startCombatButton;
    QPushButton* m_buyExpButton;
    QPushButton* m_saveButton;
    QPushButton* m_loadButton;

    QLabel* m_statusLabel;
    QLabel* m_synergyLabel;
    QLabel* m_equipmentLabel;
    QLabel* m_playerStatsLabel;
    // 顶栏三块小卡片：金币/等级/人口
    QLabel* m_goldLabel  = nullptr;
    QLabel* m_levelLabel = nullptr;
    QLabel* m_popLabel   = nullptr;
    QProgressBar* m_hpBar;

    QVector<QPushButton*> m_shopButtons;
    QVector<QPushButton*> m_benchButtons;

    Game* m_game;
};

#endif
