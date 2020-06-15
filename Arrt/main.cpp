#include <Model/IncludesAzureRemoteRendering.h>
#include <Model/IncludesAzureStorage.h>
#include <QtWidgets/QApplication>
#include <View/ApplicationView.h>
#include <View/ArrtAccessibility.h>
#include <View/ArrtStyle.h>
#include <ViewModel/ApplicationModel.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma warning(push)
#pragma warning(disable : 4100)
#include <AzureRemoteRendering.inl>
#pragma warning(pop)
#pragma clang diagnostic pop

int WinMain(HINSTANCE, HINSTANCE, char*, int)
{
    try
    {
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);
        QApplication::setAttribute(Qt::AA_DisableHighDpiScaling, true);

        int argc = 0;
        QApplication a(argc, 0);

        QApplication::setStyle(new ArrtStyle());
        qRegisterMetaType<RR::ApiHandle<RR::Material>>();
        qRegisterMetaType<RR::ApiHandle<RR::Entity>>();
        qRegisterMetaType<RR::Float2>();
        qRegisterMetaType<RR::Color4>();
        qRegisterMetaType<RR::PbrMaterialFeatures>();

        QAccessible::installFactory(ArrtAccesibility::factory);
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
        OutputDebugStringW(error);
        return -1;
    }
}
