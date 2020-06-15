#include <Utils/StyleUtils.h>

#include <QStringList>


QString StyleUtils::formatParameterList(const QStringList& parameterNames)
{
    QString s;
    for (int i = 0; i < parameterNames.size(); ++i)
    {
        const QString& p = parameterNames[i];
        s += QString("<br><b>" + p + ":<font color=\'white\'> %%1 </font></b>").arg(i + 1);
    }
    return s;
}
