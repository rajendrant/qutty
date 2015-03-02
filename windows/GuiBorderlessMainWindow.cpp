#include "GuiBorderlessMainWindow.h"

#include <dwmapi.h>
#include <windowsx.h>
#include <stdexcept>

#include "GuiTabWidget.h"
#include "GuiTerminalWindow.h"
#include <QPushButton>
#include <QFile>
#include <QDebug>

HWND winId = 0;

GuiMainWindow *GuiBorderlessMainWindow::mainPanel;
QApplication *GuiBorderlessMainWindow::a;

GuiBorderlessMainWindow::GuiBorderlessMainWindow( QApplication *app, HBRUSH windowBackground, const int x, const int y, const int width, const int height ) : hWnd(0),
  hInstance( GetModuleHandle( NULL ) ),
  borderless( false ),
  borderlessResizeable( true ),
  aeroShadow( false ),
  closed( false ),
  visible( false )
{
  WNDCLASSEX wcx = { 0 };
  wcx.cbSize = sizeof( WNDCLASSEX );
  wcx.style = CS_HREDRAW | CS_VREDRAW;
  wcx.hInstance = hInstance;
  wcx.lpfnWndProc = WndProc;
  wcx.cbClsExtra	= 0;
  wcx.cbWndExtra	= 0;
  wcx.lpszClassName = L"WindowClass";
  wcx.hbrBackground = windowBackground;
  wcx.hCursor = LoadCursor( hInstance, IDC_ARROW );
  RegisterClassEx( &wcx );
  if ( FAILED( RegisterClassEx( &wcx ) ) ) throw std::runtime_error( "Couldn't register window class" );

  hWnd = CreateWindow( L"WindowClass", L"BorderlessWindow", static_cast<DWORD>( Style::windowed ), x, y, width, height, 0, 0, hInstance, nullptr );

  if ( !hWnd ) throw std::runtime_error( "couldn't create window because of reasons" );

  SetWindowLongPtr( hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( this ) );


  mainPanel = new GuiMainWindow( hWnd );
  winId = ( HWND )mainPanel->winId();
  
  show();
  toggleBorderless();

  a = app;
}

GuiBorderlessMainWindow::~GuiBorderlessMainWindow()
{
  hide();
  DestroyWindow( hWnd );
}

HDC hdc;
PAINTSTRUCT ps;

LRESULT CALLBACK GuiBorderlessMainWindow::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  GuiBorderlessMainWindow *window = reinterpret_cast<GuiBorderlessMainWindow*>( GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
  //qDebug() << "msg " << std::hex << message << " " << window;
  if ( !window ) return DefWindowProc( hWnd, message, wParam, lParam );

  switch ( message ) {
    case WM_KEYDOWN: {
      if ( wParam != VK_TAB ) return DefWindowProc( hWnd, message, wParam, lParam );

      SetFocus( winId );
      break;
    }

    // ALT + SPACE or F10 system menu
    case WM_SYSCOMMAND: {
      if ( wParam == SC_KEYMENU ) {
          RECT winrect;
          GetWindowRect( hWnd, &winrect );
          TrackPopupMenu( GetSystemMenu( hWnd, false ), TPM_TOPALIGN | TPM_LEFTALIGN, winrect.left + 5, winrect.top + 5, 0, hWnd, NULL);
          break;
      } else {
        return DefWindowProc( hWnd, message, wParam, lParam );
      }
    }

    case WM_SETFOCUS: {
      QString str( "Got focus" );
      QWidget *widget = QWidget::find( ( WId )HWND( wParam ) );
      if ( widget )
        str += QString( " from %1 (%2)" ).arg( widget->objectName() ).arg(widget->metaObject()->className() );
      str += "\n";
      OutputDebugStringA( str.toLocal8Bit().data() );
      mainPanel->setFocus();
      QWidget *w = mainPanel->getCurrentTerminal();
      if(w) {
          w->setFocus();
      }
      break;
    }

    case WM_NCCALCSIZE: {
      //this kills the window frame and title bar we added with
      //WS_THICKFRAME and WS_CAPTION
      if (window->borderless) {
          return 0;
      }
      break;
    }

    case WM_KILLFOCUS: {
      QString str( "Lost focus" );
      QWidget *widget = QWidget::find( (WId)HWND( wParam ) );
      if ( widget )
        str += QString( " to %1 (%2)" ).arg( widget->objectName() ).arg(widget->metaObject()->className() );
      str += "\n";

      OutputDebugStringA( str.toLocal8Bit().data() );
      break;
    }

    case WM_CLOSE: {
      qDebug() << "WM_CLOSE";
      if (!mainPanel->closeRequest())
          return 0; // user doesn't want to close
      break;
    }

    case WM_DESTROY: {
      qDebug() << "WM_DESTROY";
      PostQuitMessage(0);
      break;
    }

    case WM_NCHITTEST: {
      if ( window->borderless )
      {
        if ( window->borderlessResizeable )
        {
          const LONG borderWidth = 8; //in pixels
          RECT winrect;
          GetWindowRect( hWnd, &winrect );
          long x = GET_X_LPARAM( lParam );
          long y = GET_Y_LPARAM( lParam );

          //bottom left corner
          if ( x >= winrect.left && x < winrect.left + borderWidth &&
            y < winrect.bottom && y >= winrect.bottom - borderWidth )
          {
            return HTBOTTOMLEFT;
          }
          //bottom right corner
          if ( x < winrect.right && x >= winrect.right - borderWidth &&
            y < winrect.bottom && y >= winrect.bottom - borderWidth )
          {
            return HTBOTTOMRIGHT;
          }
          //top left corner
          if ( x >= winrect.left && x < winrect.left + borderWidth &&
            y >= winrect.top && y < winrect.top + borderWidth )
          {
            return HTTOPLEFT;
          }
          //top right corner
          if ( x < winrect.right && x >= winrect.right - borderWidth &&
            y >= winrect.top && y < winrect.top + borderWidth )
          {
            return HTTOPRIGHT;
          }
          //left border
          if ( x >= winrect.left && x < winrect.left + borderWidth )
          {
            return HTLEFT;
          }
          //right border
          if ( x < winrect.right && x >= winrect.right - borderWidth )
          {
            return HTRIGHT;
          }
          //bottom border
          if ( y < winrect.bottom && y >= winrect.bottom - borderWidth )
          {
            return HTBOTTOM;
          }
          //top border
          if ( y >= winrect.top && y < winrect.top + borderWidth )
          {
            return HTTOP;
          }
        }

        return HTCAPTION;
      }
      break;
    }

    case WM_SIZE: {
      RECT winrect;
      GetClientRect( hWnd, &winrect );

      WINDOWPLACEMENT wp;
      wp.length = sizeof( WINDOWPLACEMENT );
      GetWindowPlacement( hWnd, &wp );
      if ( wp.showCmd == SW_MAXIMIZE ) {
        mainPanel->setGeometry( 9, 0, winrect.right-18, winrect.bottom-18 );
      } else {
        mainPanel->setGeometry( 4, 30, winrect.right - 8, winrect.bottom - 8 );
      }
      break;
    }

    case WM_GETMINMAXINFO: {
      MINMAXINFO* minMaxInfo = ( MINMAXINFO* )lParam;
      if ( window->minimumSize.required ) {
        minMaxInfo->ptMinTrackSize.x = window->getMinimumWidth();
        minMaxInfo->ptMinTrackSize.y = window->getMinimumHeight();
      }

      if ( window->maximumSize.required ) {
        minMaxInfo->ptMaxTrackSize.x = window->getMaximumWidth();
        minMaxInfo->ptMaxTrackSize.y = window->getMaximumHeight();
      }
      minMaxInfo->ptMaxPosition.y = 1;
      return 0;
    }
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}

void GuiBorderlessMainWindow::toggleBorderless() {
  if ( visible )
  {
    Style newStyle = ( borderless ) ? Style::windowed : Style::aero_borderless;
    SetWindowLongPtr( hWnd, GWL_STYLE, static_cast<LONG>( newStyle ) );

    borderless = !borderless;
    if ( newStyle == Style::aero_borderless ) {
      toggleShadow();
    }
    //redraw frame
    SetWindowPos( hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE );
    show();
  }
}

void GuiBorderlessMainWindow::toggleShadow() {
  if ( borderless ) {
    aeroShadow = !aeroShadow;
    const MARGINS shadow_on = { -1, -1, -1, -1 };
    const MARGINS shadow_off = { 0, 0, 0, 0 };
    DwmExtendFrameIntoClientArea( hWnd, ( aeroShadow ) ? ( &shadow_on ) : ( &shadow_off ) );
  }
}

void GuiBorderlessMainWindow::toggleResizeable() {
  borderlessResizeable = borderlessResizeable ? false : true;
}

bool GuiBorderlessMainWindow::isResizeable() {
  return borderlessResizeable ? true : false;
}

void GuiBorderlessMainWindow::show() {
  ShowWindow( hWnd, SW_SHOW );
  visible = true;
}

void GuiBorderlessMainWindow::hide() {
  ShowWindow( hWnd, SW_HIDE );
  visible = false;
}

bool GuiBorderlessMainWindow::isVisible() {
  return visible ? true : false;
}

// Minimum size
void GuiBorderlessMainWindow::setMinimumSize( const int width, const int height ) {
  this->minimumSize.required = true;
  this->minimumSize.width = width;
  this->minimumSize.height = height;
}

bool GuiBorderlessMainWindow::isSetMinimumSize() {
    return this->minimumSize.required;
}

void GuiBorderlessMainWindow::removeMinimumSize() {
  this->minimumSize.required = false;
  this->minimumSize.width = 0;
  this->minimumSize.height = 0;
}

int GuiBorderlessMainWindow::getMinimumWidth() {
  return minimumSize.width;
}

int GuiBorderlessMainWindow::getMinimumHeight() {
  return minimumSize.height;
}

// Maximum size
void GuiBorderlessMainWindow::setMaximumSize( const int width, const int height ) {
  this->maximumSize.required = true;
  this->maximumSize.width = width;
  this->maximumSize.height = height;
}

bool GuiBorderlessMainWindow::isSetMaximumSize() {
    return this->maximumSize.required;
}

void GuiBorderlessMainWindow::removeMaximumSize() {
  this->maximumSize.required = false;
  this->maximumSize.width = 0;
  this->maximumSize.height = 0;
}

int GuiBorderlessMainWindow::getMaximumWidth() {
  return maximumSize.width;
}

int GuiBorderlessMainWindow::getMaximumHeight() {
  return maximumSize.height;
}

