// TechnikiProgramowaniaProjekt4.cpp : Definiuje punkt wejścia dla aplikacji.
//

#include "framework.h"
#include "TechnikiProgramowaniaProjekt4.h"
#include <objidl.h>
#include <gdiplus.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <commctrl.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "comctl32.lib")

#define MAX_LOADSTRING 100

// Zmienne globalne:
HINSTANCE hInst;                                // bieżące wystąpienie
WCHAR szTitle[MAX_LOADSTRING];                  // Tekst paska tytułu
WCHAR szWindowClass[MAX_LOADSTRING];            // nazwa klasy okna głównego
HWND hSliderPredkosc = NULL;
HWND hSliderInterwal = NULL;
float g_predkosc = 1.5f;
DWORD g_interwal = 3000;
Rect punktSortowania(1615, 420, 50, 50);

// Przekaż dalej deklaracje funkcji dołączone w tym module kodu:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

enum KierunekRuchu { W_LEWO, SKOS_GORA_LEWO, W_LEWO_2, DOL_1, DOL_2 };
enum TypProduktu { BRAK_SORTOWANIA = -1, PROSTOKAT = 0, KOLO = 1, TROJKAT = 2};

// Zmienne globalne 2
TypProduktu poprawnyTypProduktu = BRAK_SORTOWANIA;
int aktywnyTryb = 0;
HWND hButtonTyp1 = NULL;
HWND hButtonTyp2 = NULL;

// STRUKTURY + KLASY ITD.
class Produkt {
public:
    int x, szerokosc, wysokosc, typ;
    float y, predkosc, nachylenie;
    bool ruchx;
    KierunekRuchu kierunek;
    TypProduktu typProduktu;
    Produkt() : x(1920), y(415), szerokosc(40), wysokosc(40), ruchx(true), predkosc(g_predkosc), kierunek(W_LEWO), typProduktu(PROSTOKAT), nachylenie(0.1125f) {}

    void URuch(bool active) {
        ruchx = active;
    }

    void Aktualizuj() {
        if (!ruchx) {
            return;
        }

        switch (kierunek){
        case W_LEWO:
            x -= predkosc;
            if (x <= 1620) { 
                kierunek = DOL_1;
            }
            break;
        case DOL_1:
            x -= predkosc * 0.03;
            y += predkosc * 1.3;
            if (y >= 510) {
                kierunek = SKOS_GORA_LEWO;
            }
            break;
        case SKOS_GORA_LEWO:
            x -= predkosc * 0.0001f;
            y -= predkosc * 0.08f;
            if (x <= 1235) { 
                kierunek = DOL_2;
            }
            break;
        case DOL_2:
            x -= predkosc * 0.03;
            y += predkosc * 1.3;
            if (y >= 565) {
                kierunek = W_LEWO_2;
            }
            break;
        case W_LEWO_2:
            x -= predkosc;
        default:
            break;
        }
    }
};

std::vector<Produkt> produkty;
DWORD lastAdded = 0;

VOID OnPaint(HDC hdc)
{
    Graphics graphics(hdc);

    // Taśma 1
    SolidBrush beltBrush(Color(255, 160, 160, 160)); // jasnoszary
    Rect beltRect(1620, 450, 300, 20); // x, y, szerokość, wysokość
    graphics.FillRectangle(&beltBrush, beltRect);

    // Taśma 2
    Point points[] = {
        Point(1650,550), // D P
        Point(1250, 505), // D L
        Point(1250, 525), // D L
        Point(1650, 570) // G P
    };
    graphics.FillPolygon(&beltBrush, points, 4);

    // Taśma 3
    Rect beltRect3(850, 600, 400, 18);
    graphics.FillRectangle(&beltBrush, beltRect3);

    // Punkt kontrolny 1
    SolidBrush controlBrush(Color(255, 0, 0, 255)); // niebieski
    graphics.FillRectangle(&controlBrush, punktSortowania);

    // Produkty
    SolidBrush productBrush(Color(255, 200, 50, 50)); // kolor - czerwony

    for (const auto& produkt : produkty) {
        if (produkt.typProduktu == PROSTOKAT) {
            Rect productRect(produkt.x, produkt.y, produkt.szerokosc, produkt.wysokosc); // x, y, szerokość, wysokość
            graphics.FillRectangle(&productBrush, productRect);
        }
        else if (produkt.typProduktu == KOLO) {
            Rect productRect(produkt.x, produkt.y, produkt.szerokosc, produkt.wysokosc);
            graphics.FillEllipse(&productBrush, productRect);
        }
        else if (produkt.typProduktu == TROJKAT) {
            Point trojkat[3] = {
                Point(produkt.x + produkt.szerokosc / 2, produkt.y),
                Point(produkt.x, produkt.y + produkt.wysokosc),
                Point(produkt.x + produkt.szerokosc, produkt.y + produkt.wysokosc)
            };
            graphics.FillPolygon(&productBrush, trojkat, 3);
        }
    }
    
    // Suwaki
    wchar_t buf[100];
    swprintf(buf, 100, L"Prędkość: %.1f", g_predkosc);
    TextOut(hdc, 270, 50, buf, lstrlen(buf));

    swprintf(buf, 100, L"Interwał: %.1f s", g_interwal / 1000.0f);
    TextOut(hdc, 270, 100, buf, lstrlen(buf));


}



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{   
    HWND                hWnd;
    MSG                 msg;
    WNDCLASS            wndClass;
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;

    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: W tym miejscu umieść kod.

    // Inicjuj ciągi globalne
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TECHNIKIPROGRAMOWANIAPROJEKT4, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Wykonaj inicjowanie aplikacji:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TECHNIKIPROGRAMOWANIAPROJEKT4));

    // Główna pętla komunikatów:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    GdiplusShutdown(gdiplusToken);

    return (int) msg.wParam;
}



//
//  FUNKCJA: MyRegisterClass()
//
//  PRZEZNACZENIE: Rejestruje klasę okna.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TECHNIKIPROGRAMOWANIAPROJEKT4));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TECHNIKIPROGRAMOWANIAPROJEKT4);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNKCJA: InitInstance(HINSTANCE, int)
//
//   PRZEZNACZENIE: Zapisuje dojście wystąpienia i tworzy okno główne
//
//   KOMENTARZE:
//
//        W tej funkcji dojście wystąpienia jest zapisywane w zmiennej globalnej i
//        jest tworzone i wyświetlane okno główne programu.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Przechowuj dojście wystąpienia w naszej zmiennej globalnej

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNKCJA: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PRZEZNACZENIE: Przetwarza komunikaty dla okna głównego.
//
//  WM_COMMAND  - przetwarzaj menu aplikacji
//  WM_PAINT    - Maluj okno główne
//  WM_DESTROY  - opublikuj komunikat o wyjściu i wróć
//
//



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE: 
        {
            SetTimer(hWnd, 1, 10, NULL);

            lastAdded = GetTickCount();
            srand((unsigned int)time(NULL));

            Produkt p;
            int losowanie = static_cast<TypProduktu>(rand() % 3);
            p.typProduktu = static_cast<TypProduktu>(losowanie);
            produkty.push_back(p);

            InitCommonControls();

            hSliderPredkosc = CreateWindowEx(
                0, TRACKBAR_CLASS, L"Predkosc",
                WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
                50, 50, 200, 30,
                hWnd, (HMENU)1, hInst, NULL
            );
            SendMessage(hSliderPredkosc, TBM_SETRANGE, TRUE, MAKELPARAM(1, 50));
            SendMessage(hSliderPredkosc, TBM_SETPOS, TRUE, (LPARAM)15);

            hSliderInterwal = CreateWindowEx(
                0, TRACKBAR_CLASS, L"Interwał",
                WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
                50, 100, 200, 30,
                hWnd, (HMENU)2, hInst, NULL
            );
            SendMessage(hSliderInterwal, TBM_SETRANGE, TRUE, MAKELPARAM(1, 50));
            SendMessage(hSliderInterwal, TBM_SETPOS, TRUE, (LPARAM)6);

            // Przyciski od trybów:
            CreateWindow(
                L"BUTTON", L"Tryb 0: BRAK SORTOWANIA",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                50, 150, 370, 30,
                hWnd, (HMENU)7, hInst, NULL);
            hButtonTyp1 = CreateWindow(
                L"BUTTON", L"Tryb 1: KOŁO = poprawny - PROSTOKAT = niepoprawny",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                50, 190, 370, 30,
                hWnd, (HMENU)3, hInst, NULL);

            hButtonTyp2 = CreateWindow(
                L"BUTTON", L"Tryb 2: PROSTOKĄT = poprawny - KOLO = niepoprawny",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                50, 230, 370, 30,
                hWnd, (HMENU)4, hInst, NULL);

            CreateWindow(
                L"BUTTON", L"Tryb 3: KWADRAT poprawny - TRÓJKĄT niepoprawny",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                50, 270, 370, 30,
                hWnd, (HMENU)5, hInst, NULL);

            CreateWindow(
                L"BUTTON", L"Tryb 4: TRÓJKĄT poprawny - KWADRAT niepoprawny",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                50, 310, 370, 30,
                hWnd, (HMENU)6, hInst, NULL);
        }
        return 0;
    case WM_TIMER:
        {
            DWORD now = GetTickCount();
            DWORD UCzas = now - lastAdded;
            if (UCzas >= g_interwal) {
                int lNowych = UCzas / g_interwal;
                for (int i = 0; i < lNowych; ++i) {
                    Produkt p;
                    TypProduktu niepoprawnyTyp = BRAK_SORTOWANIA;
                    switch (aktywnyTryb) {
                    case 1:
                        poprawnyTypProduktu = KOLO;
                        niepoprawnyTyp = PROSTOKAT;
                        break;
                    case 2:
                        poprawnyTypProduktu = PROSTOKAT;
                        niepoprawnyTyp = KOLO;
                        break;
                    case 3: 
                        poprawnyTypProduktu = PROSTOKAT;
                        niepoprawnyTyp = TROJKAT;
                        break;
                    case 4:
                        poprawnyTypProduktu = TROJKAT;
                        niepoprawnyTyp = PROSTOKAT;
                    default: break;
                    }

                    if (poprawnyTypProduktu != BRAK_SORTOWANIA) {
                        if (rand() % 2 == 0)
                            p.typProduktu = poprawnyTypProduktu;
                        else
                            p.typProduktu = niepoprawnyTyp;
                    }
                    else {
                        int losowanie = rand() % 3;
                        p.typProduktu = static_cast<TypProduktu>(losowanie);
                    }

                    produkty.push_back(p);
                }
                lastAdded += lNowych * g_interwal;
            }

            for (auto it = produkty.begin(); it != produkty.end();) {
                it->Aktualizuj();

                if (it->x <= punktSortowania.GetRight() &&
                    it->x + it->szerokosc >= punktSortowania.GetLeft() &&
                    it->y + it->wysokosc >= punktSortowania.GetTop() &&
                    it->y <= punktSortowania.GetBottom())
                {
                    if (poprawnyTypProduktu != BRAK_SORTOWANIA && it->typProduktu != poprawnyTypProduktu) {
                        it = produkty.erase(it);
                        continue;
                    }
                }

                if (it->x <= 835) {
                    it = produkty.erase(it);
                }
                else {
                    ++it;
                }
            }
            InvalidateRect(hWnd, NULL, FALSE);
        }
        return 0;
    case WM_HSCROLL:
    {
        if ((HWND)lParam == hSliderPredkosc)
        {
            int pozycja = SendMessage(hSliderPredkosc, TBM_GETPOS, 0, 0);
            g_predkosc = pozycja / 10.0f;

            for (auto& p : produkty)
                p.predkosc = g_predkosc;
        }
        else if ((HWND)lParam == hSliderInterwal)
        {
            int pozycja = SendMessage(hSliderInterwal, TBM_GETPOS, 0, 0);
            g_interwal = pozycja * 100; // Skala interwałów
        }
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Analizuj zaznaczenia menu:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case 3:
                poprawnyTypProduktu = KOLO;
                aktywnyTryb = 1;
                break;
            case 4:
                poprawnyTypProduktu = PROSTOKAT;
                aktywnyTryb = 2;
                break;
            case 5:
                poprawnyTypProduktu = PROSTOKAT; 
                aktywnyTryb = 3;
                break;
            case 6:
                poprawnyTypProduktu = TROJKAT;
                aktywnyTryb = 4;
                break;
            case 7:
                poprawnyTypProduktu = BRAK_SORTOWANIA;
                aktywnyTryb = 0;
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {

        // PODWÓJNE BUFOROWANIE
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Tworzenie Bufora
        HDC hdcMem = CreateCompatibleDC(hdc);
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;

        HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);


        // Czyszczenie BUFOR
        FillRect(hdcMem, &clientRect, (HBRUSH)(COLOR_WINDOW + 1));

        // Rys bufor
        OnPaint(hdcMem);

        // BUFOR Copy
        BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

        // clean
        SelectObject(hdcMem, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);

        EndPaint(hWnd, &ps);
        }   
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Procedura obsługi komunikatów dla okna informacji o programie.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
