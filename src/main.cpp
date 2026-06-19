#include <QApplication>
#include "gui/gamewindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("忽哲的自走棋");
    app.setApplicationVersion("1.0");

    GameWindow window;
    window.setWindowTitle("忽哲-251880102 自走棋");
    window.resize(900, 700);
    window.show();

    return app.exec();
}
