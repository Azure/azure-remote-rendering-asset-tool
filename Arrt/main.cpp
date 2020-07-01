#include <QtWidgets/QApplication>
#include <View/ApplicationView.h>
#include <View/ArrtAccessibility.h>
#include <View/ArrtStyle.h>
#include <ViewModel/ApplicationModel.h>
#include <iostream>

int WinMain(HINSTANCE, HINSTANCE, char*, int)
{
    try
    {
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);
        QApplication::setAttribute(Qt::AA_DisableHighDpiScaling, true);

        int argc = 0;
        QApplication a(argc, 0);

        QApplication::setStyle(new ArrtStyle()); // NOLINT

        QAccessible::installFactory(ArrtAccesibility::factory); // NOLINT

        ApplicationModel model;

        ApplicationView w(&model);
        if (!w.canStart())
        {
            return -1;
        }
        w.show();
        return a.exec();
    }
    catch (const wchar_t* error)
    {
        std::cout << error;
        return -1;
    }
}
