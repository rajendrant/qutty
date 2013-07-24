#include "GuiTabInTitlebar.h"
#include <QApplication>
#include <QStyle>
#include <QDesktopWidget>
#include <QToolBar>
#include <QHBoxLayout>
#include <QWidget>
#include <QDebug>

/*
 * Tabs In Titlebar is only supported in Windows Vista+ with DWM(Aero) enabled
 *  * If DWM/Aero is disabled in Win Vista/7 Tabs will not be in titlebar
 *  * DWM is always enabled (cannot be disabled) in Windows 8 onwards
 *  * So older windows versions are not supported, since that adds more code to maintain
 *
 * Much of the logic is based on below URL
 * http://msdn.microsoft.com/en-us/library/windows/desktop/bb688195%28v=vs.85%29.aspx
 *
 * Following issues are seen:
 * * top, left portion of the mainwindow gets clipped
 * * Also if 'autohide the taskbar' is enabled in Windows, taskbar doesn't popup when
 *   mouse is moved to bottom of screen with application in maximized mode
 * These issues are fixed based on
 * http://stackoverflow.com/questions/137005/auto-hide-taskbar-not-appearing-when-my-application-is-maximized
 */

#define WINDOW_FRAME_WIDTH  (mainWindow->style()->pixelMetric(QStyle::PM_MdiSubWindowFrameWidth))
#define TITLE_BAR_HEIGHT    (mainWindow->style()->pixelMetric(QStyle::PM_TitleBarHeight))

GuiTabInTitlebar::GuiTabInTitlebar(QMainWindow *mainwindow, QTabWidget *tabarea, QTabBar *tabbar)
    : mainWindow(mainwindow),
      tabArea(tabarea),
      tabBar(tabbar),
      isCompositionEnabled(false)
{
    if (!dwmApi.dwmIsCompositionEnabled())
        return;

    isCompositionEnabled = true;

    mainWindow->setAttribute(Qt::WA_TranslucentBackground, true);

    // Handle window creation.
    RECT rcClient;
    HWND hwnd = (HWND) mainWindow->winId();
    GetWindowRect(hwnd, &rcClient);

    // Inform application of the frame change.
    SetWindowPos(hwnd,
                 NULL,
                 rcClient.left, rcClient.top,
                 rcClient.right - rcClient.left,
                 rcClient.bottom - rcClient.top,
                 SWP_FRAMECHANGED);
}

bool GuiTabInTitlebar::handleWinEvent(MSG *msg, long *result)
{
    bool fCallDWP = true;
    LRESULT lRet = 0;
    HRESULT hr = S_OK;
    HWND hWnd       = msg->hwnd;
    UINT message    = msg->message;
    WPARAM wParam   = msg->wParam;
    LPARAM lParam   = msg->lParam;

    if (!isCompositionEnabled)
        return false;

    if (message == WM_NCHITTEST)
    {
        return hitTestNCA(msg, result);
    }

    fCallDWP = !dwmApi.dwmDefWindowProc(hWnd, message, wParam, lParam, &lRet);

    // Handle window activation.
    if (message == WM_ACTIVATE)
    {
        // Extend the frame into the client area.
        MARGINS margins;

        margins.cxLeftWidth     = WINDOW_FRAME_WIDTH;
        margins.cxRightWidth    = WINDOW_FRAME_WIDTH;
        margins.cyBottomHeight  = WINDOW_FRAME_WIDTH;
        margins.cyTopHeight     = TITLE_BAR_HEIGHT;

        hr = dwmApi.dwmExtendFrameIntoClientArea(hWnd, &margins);

        if (!SUCCEEDED(hr))
        {
            // Handle error.
        }

        fCallDWP = true;
        lRet = 0;
    }

    // Handle the non-client size message.
    if ((message == WM_NCCALCSIZE) && (wParam == TRUE))
    {
        // No need to pass the message on to the DefWindowProc.
        lRet = 0;
        fCallDWP = false;
    }

    if (message == WM_GETMINMAXINFO) {
        MINMAXINFO *mmi = (MINMAXINFO*)lParam;
        QRect rect = QApplication::desktop()->availableGeometry();
        mmi->ptMaxSize.x = rect.width();
        mmi->ptMaxSize.y = rect.height()-1;
        mmi->ptMaxPosition.x = 0;
        mmi->ptMaxPosition.y = 0;
        lRet = 0;
        fCallDWP = false;
    }

    if (!fCallDWP)
        *result = lRet;

    return !fCallDWP;
}

void GuiTabInTitlebar::setTabAreaCornerWidget(QWidget *w)
{
    if (!isCompositionEnabled) {
        tabArea->setCornerWidget(w, Qt::TopRightCorner);
        return;
    }

    tabAreaCornerWidget = w;

    // add spacer to stop Qt from using the area of
    // the min, max, close buttons in titlebar
    int caption_btn_size = 100;
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    BOOL ok=SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
    if (ok) {
        caption_btn_size = 3 * (ncm.iCaptionWidth + 2*ncm.iBorderWidth) + 2 * ncm.iBorderWidth;
    }

    QWidget *cornerw = new QWidget;
    QHBoxLayout *hboxlayout = new QHBoxLayout;
    hboxlayout->setContentsMargins(0,0,0,0);
    cornerw->setLayout(hboxlayout);
    hboxlayout->addWidget(w);
    hboxlayout->addSpacing(caption_btn_size);
    tabArea->setCornerWidget(cornerw, Qt::TopRightCorner);
}

bool GuiTabInTitlebar::hitTestNCA(MSG *msg, long *result)
{
    // Get the window rectangle.
    RECT rcWindow;
    GetWindowRect(msg->hwnd, &rcWindow);

    // Get the frame rectangle, adjusted for the style without a caption.
    RECT rcFrame = { 0 };
    AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);

    int x = GET_X_LPARAM(msg->lParam), y = GET_Y_LPARAM(msg->lParam);
    QPoint p(x - rcWindow.left, y - rcWindow.top);
    if ( (tabBar->rect().contains(p) && tabBar->tabAt(p) != -1) ||
         (tabAreaCornerWidget->rect().contains(tabAreaCornerWidget->mapFromGlobal(QPoint(x, y))))
       ) {
        *result = HTCLIENT;
        return true;
    }
    qDebug() << tabAreaCornerWidget->rect() << tabAreaCornerWidget->mapFromGlobal(QPoint(x, y))
             << x<<y;
    LRESULT lRet = 0;
    if (dwmApi.dwmDefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam, &lRet)) {
        *result = lRet;
        return true;    // handled for Titlebar area
    }

    USHORT uRow = 1;
    USHORT uCol = 1;

    // Determine if the point is at the top or bottom of the window.
    if (y >= rcWindow.top && y < rcWindow.top + TITLE_BAR_HEIGHT)
        uRow = 0;
    else if (y < rcWindow.bottom && y >= rcWindow.bottom - WINDOW_FRAME_WIDTH)
        uRow = 2;

    // Determine if the point is at the left or right of the window.
    if (x >= rcWindow.left && x < rcWindow.left + WINDOW_FRAME_WIDTH)
        uCol = 0;
    else if (x < rcWindow.right && x >= rcWindow.right - WINDOW_FRAME_WIDTH)
        uCol = 2;

    LRESULT hitTests[3][3] =
    {
        { HTTOPLEFT,    y < (rcWindow.top - rcFrame.top) ? HTTOP : HTCAPTION,    HTTOPRIGHT },
        { HTLEFT,       HTNOWHERE,      HTRIGHT },
        { HTBOTTOMLEFT, HTBOTTOM,       HTBOTTOMRIGHT },
    };

    *result = hitTests[uRow][uCol];
    return *result != HTNOWHERE;
}
