#include "GuiTabInTitlebar.h"
#include <QApplication>
#include <QStyle>
#include <QDesktopWidget>
#include <QToolBar>
#include <QHBoxLayout>
#include <QDebug>

GuiDwmApi qutty_dwm_api;

GuiTabInTitlebar::GuiTabInTitlebar(QMainWindow *mainwindow, QTabWidget *tabarea, QTabBar *tabbar)
    : mainWindow(mainwindow),
      tabArea(tabarea),
      tabBar(tabbar)
{
    mainWindow->setAttribute(Qt::WA_TranslucentBackground, true);

    // Handle window creation.
    RECT rcClient;
    HWND hwnd = mainWindow->winId();
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
    BOOL fDwmEnabled = FALSE;
    LRESULT lRet = 0;
    HRESULT hr = S_OK;
    HWND hWnd       = msg->hwnd;
    UINT message    = msg->message;
    WPARAM wParam   = msg->wParam;
    LPARAM lParam   = msg->lParam;

    if (!qutty_dwm_api.dwmIsCompositionEnabled())
        return false;

    fCallDWP = !qutty_dwm_api.dwmDefWindowProc(hWnd, message, wParam, lParam, &lRet);

    // Handle window activation.
    if (message == WM_ACTIVATE)
    {
        // Extend the frame into the client area.
        MARGINS margins;

        margins.cxLeftWidth = 0;      // 8
        margins.cxRightWidth = 0;    // 8
        margins.cyBottomHeight = 0; // 20
        margins.cyTopHeight = mainWindow->style()->pixelMetric(QStyle::PM_TitleBarHeight);

        hr = qutty_dwm_api.dwmExtendFrameIntoClientArea(hWnd, &margins);

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
