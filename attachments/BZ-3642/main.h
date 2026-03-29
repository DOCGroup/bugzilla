#ifndef __MAIN_H
#define __MAIN_H
#include <QtGui/QApplication>
class Handler;
class TestApp : public QApplication
{
    Q_OBJECT
public:
    TestApp(int argc, char **argv);
    virtual ~TestApp();
    int exec();
private:
    Handler *handler;
};

#endif
