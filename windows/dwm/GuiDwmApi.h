#ifndef QTDWMAPI_H
#define QTDWMAPI_H

#include <QLibrary>
#include "windows.h"
#include "WinUser.h"
#include "windowsx.h"
#include "dwmapi.h"

/*
 * Wrapper for some of the DWM apis
 */
class GuiDwmApi
{
    typedef HRESULT (*typefnptrDwmIsCompositionEnabled)(BOOL* pfEnabled);
    typedef HRESULT (*typefnptrDwmDefWindowProc)(HWND hWnd, UINT msg,
                                WPARAM wParam, LPARAM lParam, LRESULT *plResult);
    typedef HRESULT (*typefnptrDwmExtendFrameIntoClientArea)(HWND hWnd, const MARGINS* pMarInset);

    typefnptrDwmIsCompositionEnabled fnptrDwmIsCompositionEnabled;
    typefnptrDwmDefWindowProc fnptrDwmDefWindowProc;
    typefnptrDwmExtendFrameIntoClientArea fnptrDwmExtendFrameIntoClientArea;
public:
    GuiDwmApi()
        : fnptrDwmIsCompositionEnabled(NULL),
          fnptrDwmDefWindowProc(NULL),
          fnptrDwmExtendFrameIntoClientArea(NULL)
    {
        if (QSysInfo::windowsVersion() < QSysInfo::WV_VISTA)
            return ;
        QLibrary lib("dwmapi");
        if (lib.load()) {
            fnptrDwmIsCompositionEnabled = (typefnptrDwmIsCompositionEnabled) lib.resolve("DwmIsCompositionEnabled");
            fnptrDwmDefWindowProc = (typefnptrDwmDefWindowProc) lib.resolve("DwmDefWindowProc");
            fnptrDwmExtendFrameIntoClientArea = (typefnptrDwmExtendFrameIntoClientArea) lib.resolve("DwmExtendFrameIntoClientArea");
        }
    }

    bool dwmIsCompositionEnabled()
    {
        HRESULT hr;
        BOOL enabled;
        if (fnptrDwmIsCompositionEnabled) {
            hr = fnptrDwmIsCompositionEnabled(&enabled);
            if (SUCCEEDED(hr) && enabled)
                return true;
        }
        return false;
    }

    HRESULT dwmDefWindowProc(HWND hWnd, UINT msg,
                             WPARAM wParam, LPARAM lParam, LRESULT *plResult)
    {
        if (fnptrDwmDefWindowProc)
            return fnptrDwmDefWindowProc(hWnd, msg, wParam, lParam, plResult);
    }

    HRESULT dwmExtendFrameIntoClientArea(HWND hWnd, const MARGINS* pMarInset)
    {
        if (fnptrDwmExtendFrameIntoClientArea)
            return fnptrDwmExtendFrameIntoClientArea(hWnd, pMarInset);
    }
};

extern GuiDwmApi qutty_dwm_api;

#endif // QTDWMAPI_H
