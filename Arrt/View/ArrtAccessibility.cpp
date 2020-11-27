#include <QAbstractItemView>
#include <View/ArrtAccessibility.h>

namespace ArrtAccesibility
{
    /*
    The default accessible role for QComboBox is QAccessible::ComboBox, which is exposed to UI automation
    as "Combo box". Microsoft accessibility compliance expect combo boxes to implement the ExpandCollapse interface
    and the Selection interface, see
    https://docs.microsoft.com/en-us/dotnet/framework/ui-automation/ui-automation-support-for-the-combobox-control-type
	but that's not the case in the Qt implementation.
	https://bugreports.qt.io/browse/QTBUG-81874    
    */
} // namespace ArrtAccesibility
