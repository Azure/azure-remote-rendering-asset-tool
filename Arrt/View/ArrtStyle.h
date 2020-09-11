#pragma once
#include <QColor>
#include <QProxyStyle>

class QLabel;

// customized style class, for Arrt

class ArrtStyle : public QProxyStyle
{
public:
    ArrtStyle();

    static const int s_controlHeight;

    static QColor s_focusedControlBorderColor;
    static int s_focusedControlBorderWidth;

    struct ButtonColor
    {
        QColor m_text;
        QColor m_background;
        QColor m_border;
    };

    static ButtonColor s_buttonEnabledColors;
    static ButtonColor s_buttonDisabledColors;
    static ButtonColor s_buttonHoverColors;
    static ButtonColor s_buttonPressedColors;
    static ButtonColor s_buttonPressedDisabledColors;
    static ButtonColor s_buttonPressedHoverColors;

    static QColor s_listSeparatorColor;
    static QColor s_underTextColor;

    static QColor s_graphForegroundColor;
    static QColor s_graphBackgroundColor;
    static QColor s_graphTextColor;
    static QColor s_graphLinesColor;
    static QColor s_graphTooltipBackgroundColor;

    static const QColor s_debugColor;
    static const QColor s_warningColor;
    static const QColor s_errorColor;
    static const QColor s_infoColor;
    static const QColor s_successColor;
    static const QColor s_runningColor;
    static const QColor s_failureColor;
    static const QColor s_progressBackgroundColor;
    static const QColor s_progressColor;
    static const QColor s_connectedColor;
    static const QColor s_connectingColor;
    static const QColor s_disconnectedColor;

    static const QFont s_widgetFont;
    static const QFont s_directoryPopupFont;
    static const QFont s_blobNameFont;
    static const QFont s_blobPathFont;
    static const QFont s_blobStatusFont;
    static const QFont s_conversionListFont;
    static const QFont s_conversionTimeFont;
    static const QFont s_configurationLabelFont;
    static const QFont s_logModelListFont;
    static const QFont s_sessionStatusFont;
    static const QFont s_sessionTimeFont;
    static const QFont s_formHeaderFont;
    static const QFont s_notificationFont;
    static const QFont s_mainButtonFont;
    static const QFont s_toolbarFont;
    static const QFont s_graphFont;
    static const QFont s_splitterHandleFont;

    static QIcon s_expandedIcon;
    static QIcon s_notexpandedIcon;
    static QIcon s_newfolderIcon;
    static QIcon s_showblobsIcon;
    static QIcon s_directorymodeIcon;
    static QIcon s_uploadIcon;
    static QIcon s_parentdirIcon;
    static QIcon s_directoryIcon;
    static QIcon s_modelIcon;
    static QIcon s_newIcon;
    static QIcon s_removeIcon;
    static QIcon s_collapseIcon;
    static QIcon s_expandIcon;
    static QIcon s_startIcon;
    static QIcon s_logIcon;
    static QIcon s_simpleIcon;
    static QIcon s_detailsIcon;
    static QIcon s_debugIcon;
    static QIcon s_warningIcon;
    static QIcon s_criticalIcon;
    static QIcon s_infoIcon;
    static QIcon s_extendtimeIcon;
    static QIcon s_inspectorIcon;
    static QIcon s_stopIcon;
    static QIcon s_autoextendtimeIcon;
    static QIcon s_session_not_activeIcon;
    static QIcon s_session_stoppedIcon;
    static QIcon s_session_expiredIcon;
    static QIcon s_session_errorIcon;
    static QIcon s_session_startingIcon;
    static QIcon s_session_readyIcon;
    static QIcon s_session_connectingIcon;
    static QIcon s_session_connectedIcon;
    static QIcon s_session_stop_requestedIcon;
    static QIcon s_conversionIcon;
    static QIcon s_renderingIcon;
    static QIcon s_settingsIcon;
    static QIcon s_conversion_runningIcon;
    static QIcon s_conversion_succeededIcon;
    static QIcon s_conversion_canceledIcon;
    static QIcon s_conversion_failedIcon;
    static QIcon s_refreshIcon;
    static QIcon s_backIcon;
    static QIcon s_moreActionsIcon;
    static QIcon s_arrowUpIcon;
    static QIcon s_arrowDownIcon;
    static QIcon s_arrowLeftIcon;
    static QIcon s_arrowRightIcon;
    static QIcon s_sceneIcon;
    static QIcon s_materialsIcon;
    static QIcon s_statsIcon;


    virtual void polish(QPalette& pal) override;
    virtual void polish(QApplication* app) override;

    virtual void drawControl(ControlElement element, const QStyleOption* opt, QPainter* p, const QWidget* widget = {}) const override;

    virtual QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const override;

    virtual void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;

    virtual int styleHint(StyleHint hint, const QStyleOption* option = {}, const QWidget* widget = {}, QStyleHintReturn* returnData = {}) const override;

    static int controlHeight();

    // helper function to create a styled label on top of every panel
    static QLabel* createHeaderLabel(const QString& title, const QString& text);

    // helper to format the tooltip with a title and details
    static QString formatToolTip(const QString& title, const QString& details);

    static void drawFocusedBorder(QPainter* p, QRect rect);
};
