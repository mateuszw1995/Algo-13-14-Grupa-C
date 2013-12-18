#pragma once
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#define _USE_MATH_DEFINES
#include "img/image.h"
#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <complex>
#define kw(x) (x)*(x)
#define c(i, j) complex <double>(i, j)
using namespace std;

#define ID_BUTTON_OPEN_OBRAZ 101
#define ID_BUTTON_OPEN_WZOR 102
#define ID_BUTTON_START 103
#define ID_CHECKBOX_OBRAZ 201
#define ID_TRACKBAR_TOLERANCJA 301

#define OBRAZ_X 370
#define OBRAZ_Y 5

HINSTANCE hInst;
HWND hButtonOpenObraz, hButtonOpenWzor, hButtonStart;
HWND hCheckboxObraz;
HWND hStaticInfo, hStaticObraz, hStaticWzor, hStaticTolerancja;
HWND hButtonTrackbarTolerancja;
HFONT hNormalFont;

char sSciezkaObraz[MAX_PATH] = "";
char sSciezkaWzor[MAX_PATH] = "";
char sNazwaPlikuObraz[MAX_PATH] = "";
char sNazwaPlikuWzor[MAX_PATH] = "";

BOOL ObrazWybrany = FALSE, WzorWybrany = FALSE, Zakonczono = FALSE, WybranoDopasowanie = TRUE;
INT tolerancja = 20;
image obraz,wzorzec;
double tekst[1000][1000];
double wzor[1000][1000];

int tekst_w, tekst_h, wzor_w, wzor_h;
vector<vector<double> >odchylenie;
vector<vector<double> > wynik_FFT;
int zrobione=false;
double maksimum, minimum;
complex<double> M_I(0,1);
complex<double> tab_omeg[2][2100005];

complex<double> make_complex(double real, double imag)
{
    complex<double> comp(real,imag);
    return comp;
}

void gen_omegi(int N)
{
	for (int j = 0; j < N/2; j++) 
	{
		complex<double> ex (0, -2*M_PI * (j)/N);
		tab_omeg[0][j] = exp(ex);
		tab_omeg[1][j] = exp(-ex);
	}
}

vector<complex<double> > FFT(vector<complex<double> > z, int invert, int unsigned step)
{
    vector<complex<double> > wynik;
    wynik.resize(z.size());
    if(z.size()==1)
    {
        wynik[0]=make_complex(z[0].real(),z[0].imag());
        return wynik;
    }
    vector<complex<double> > parzyste,nieparzyste, FFTparzyste, FFTnieparzyste;
	parzyste.reserve(z.size());
	nieparzyste.reserve(z.size());
    for(int i=0;i<z.size();i+=2)
    {
        parzyste.push_back(z[i]);
        nieparzyste.push_back(z[i+1]);
    }
    FFTparzyste=FFT(parzyste,invert,step*2);
    FFTnieparzyste=FFT(nieparzyste,invert,step*2);
    for(int i=0;i<z.size()/2;i++)
    {
        //complex<double> omega=exp(make_complex(invert*2*M_PI*i/z.size(),0)*M_I);
        complex<double> omega;
        if (invert == 1)
			omega = tab_omeg[0][i*step];
		else
			omega = tab_omeg[1][i*step];
        wynik[i]=FFTparzyste[i]+omega*FFTnieparzyste[i];
        wynik[i+z.size()/2]=FFTparzyste[i]-omega*FFTnieparzyste[i];
    }
    return wynik;
}

double suma_wzorca()
{
	double suma=0;
	for(int y = 0; y < wzor_h; ++y)
		for(int x = 0; x < wzor_w; ++x)
			suma+=kw(wzor[x][y]);
	return suma;
}

vector<vector<double> >sumy_tekstu()
{
	vector<vector<double> > sumy_prefiksowe;
	for(int y = 0; y < tekst_h; ++y)
	{
		vector<double> temp;
		temp.reserve(tekst_w);
		temp.push_back(kw(tekst[0][y]));
		for(int x = 1; x < tekst_w; ++x)
			temp.push_back(kw(tekst[x][y])+temp[x-1]);
		sumy_prefiksowe.push_back(temp);
	}
	vector<vector<double> > sumy_prefiksowe_pion;
	for(int x = 0; x < tekst_w; ++x)
	{
		vector<double> temp;
		temp.reserve(tekst_h);
		temp.push_back(kw(sumy_prefiksowe[x][0]));
		for(int y = 1; y < tekst_h; ++y)
			temp.push_back(kw(sumy_prefiksowe[x][y])+temp[y-1]);
		sumy_prefiksowe_pion.push_back(temp);
	}
	vector<vector<double> >wynik;
	vector<double>temp;
	for(int y = 0; y < tekst_h; ++y)
	{
		for(int x = 0; x < tekst_w; ++x)
			temp.push_back(sumy_prefiksowe_pion[min(x+wzor_w-1,tekst_w-1)][min(y+wzor_h-1,tekst_h-1)]-sumy_prefiksowe_pion[x][min(y+wzor_h-1,tekst_h-1)]-sumy_prefiksowe_pion[min(x+wzor_h-1,tekst_w-1)][y]+sumy_prefiksowe_pion[x][y]);
		wynik.push_back(temp);
	}
	return wynik;
}

double licz_odchylenie_NAPALOWE(int x, int y,int maxx, int maxy)
{
	double suma=0;
	for(int i=0;i<wzor_w;i++)
		for(int j=0;j<wzor_h;j++)
			if(x+i<maxx || y+j<maxy)
				suma+=kw((tekst[x+i][y+j]-wzor[i][j]));
	return suma;
}

long long round(double x)
{
	return (long long)(x+0.5);
}

int wyswietl(int x, int y)
{
	return (odchylenie[x][y] - minimum) * 255 / (maksimum - minimum);
}

void rozpoznawaj(HWND hwnd)
{
	/*
	for(int y = 0; y < tekst_h; ++y)
		for (int x = 0; x < tekst_w; ++x)
			tekst[x][y] = (*obraz(x, y, 1)) / 255.0;
	//odwrócenie wzorca i uzupe³nienie zerami
	for(int y = 0; y < wzor_h; ++y)
	{
		for(int x = 0; x < wzor_w; ++x)
			wzor[x][y] = (*wzorzec(wzor_w-1-x, wzor_h-1-y, 1))/255.0;
		for(int x = wzor_w; x < tekst_w; ++x)
			wzor[x][y] = 0;
	}
	for(int y = wzor_h; y < tekst_h; ++y)
		for(int x = 0; x < tekst_w; ++x)
			wzor[x][y] = 0;
	//przygotowanie tablic do FFT
	vector<vector<complex<double> > >zesp_tekst;
	vector<vector<complex<double> > >zesp_wzor;
	for(int x=0; x < tekst_w; ++x)
	{
		vector<complex<double> >temp;
		for(int y=0; y < tekst_h; ++y)
			temp.push_back(make_complex(tekst[x][y],0));
		zesp_tekst.push_back(temp);
	}
	for(int x=0; x < tekst_w; ++x)
	{
		vector<complex<double> >temp;
		for(int y=0; y < tekst_h; ++y)
			temp.push_back(make_complex(wzor[x][y],0));
		zesp_wzor.push_back(temp);
	}
	gen_omegi(tekst_w); 
	//pionowe FFT
	vector<vector<complex<double> > >FFT_tekst_pion;
	vector<vector<complex<double> > >FFT_wzor_pion;
	FFT_tekst_pion.reserve(tekst_w);
	FFT_wzor_pion.reserve(tekst_w);
	MessageBox(0,0,0,0);
	for(int x=0; x < tekst_w; ++x)
		FFT_tekst_pion.push_back(FFT(zesp_tekst[x],1,1));
	MessageBox(0,0,0,0);
	for(int x=0; x < tekst_w; ++x)
		FFT_wzor_pion.push_back(FFT(zesp_wzor[x],1,1));
	MessageBox(0,0,0,0);
	//poziome FFT
	vector<vector<complex<double> > >FFT_tekst;
	vector<vector<complex<double> > >FFT_wzor;
	FFT_tekst.reserve(tekst_w);
	FFT_wzor.reserve(tekst_w);
	MessageBox(0,0,0,0);
	for(int y=0; y < tekst_h; ++y)
	{
		vector<complex<double> >poziom;
		for(int x=0; x < tekst_w; ++x)
			poziom.push_back(FFT_tekst_pion[x][y]);
		FFT_tekst.push_back(FFT(poziom,1,1));
	}
	MessageBox(0,0,0,0);
	for(int y=0; y < tekst_h; ++y)
	{
		vector<complex<double> >poziom;
		for(int x=0; x < tekst_w; ++x)
			poziom.push_back(FFT_wzor_pion[x][y]);
		FFT_wzor.push_back(FFT(poziom,1,1));
	}
		
	MessageBox(0,0,0,0);
	//mno¿enie
	vector<vector<complex<double> > >pomnozone;
	for(int x=0; x < tekst_w; ++x)
	{
		vector<complex<double> >temp;
		for(int y=0; y < tekst_h; ++y)
			temp.push_back(FFT_tekst[y][x]*FFT_wzor[y][x]);
		pomnozone.push_back(temp);
	}
		
	MessageBox(0,0,0,0);
	//FFT rev pionowe
	vector<vector<complex<double> > >rev_pion;
	for(int x=0; x < tekst_w; ++x)
		rev_pion.push_back(FFT(pomnozone[x],-1,1));
		
	MessageBox(0,0,0,0);
	//FFT rev poziome
	vector<vector<complex<double> > >rev_poziom;
	for(int y=0; y < tekst_h; ++y)
	{
		vector<complex<double> >poziom;
		for(int x=0; x < tekst_w; ++x)
			poziom.push_back(rev_pion[x][y]);
		rev_poziom.push_back(FFT(poziom,-1,1));
	}
		
	MessageBox(0,0,0,0);
	//zamiana na tablicê double
	for(int x=0; x < tekst_w; ++x)
	{
		vector<double>temp;
		for(int y=0; y < tekst_h; ++y)
			temp.push_back(rev_poziom[y][x].real()/kw(tekst_w));
		wynik_FFT.push_back(temp);
	}
		
	MessageBox(0,0,0,0);
	vector<vector<double> >sumy_tekstu_wynik=sumy_tekstu();
	double suma_wzorca_wynik=suma_wzorca();
	for(int x = 0; x < tekst_w; ++x)
	{
		vector<double>temp;
		for(int y = 0; y < tekst_h; ++y)
			temp.push_back(sumy_tekstu_wynik[x][y]-2*wynik_FFT[x][y]+suma_wzorca_wynik);
		odchylenie.push_back(temp);
	}*/
	for(int y = 0; y < tekst_h; ++y)
		for (int x = 0; x < tekst_w; ++x)
			tekst[x][y] = (*obraz(x, y, 1)) / 255.0;
	for(int y = 0; y < wzor_h; ++y)
		for(int x = 0; x < wzor_w; ++x)
			wzor[x][y] = (*wzorzec(x, y, 1))/255.0;
	odchylenie.clear();
	for(int x = 0; x < tekst_w; ++x)
	{
		vector<double>temp;
		for(int y = 0; y < tekst_h; ++y)
			temp.push_back(licz_odchylenie_NAPALOWE(x,y,tekst_w,tekst_h));
		odchylenie.push_back(temp);
	}
	maksimum=-2147483647;
	minimum=2147483647;
	for(int y = 0; y < tekst_h - wzor_h; ++y)
		for(int x = 0; x < tekst_w - wzor_w; ++x)
		{
			maksimum=max(odchylenie[x][y],maksimum);
			minimum=min(odchylenie[x][y],minimum);
		}
	Zakonczono = TRUE;
	InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
	case WM_CREATE:
	{
		hNormalFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);

		hButtonOpenObraz = CreateWindowEx(0, "BUTTON", "Wybierz obraz", WS_CHILD | WS_VISIBLE, 5, 5, 100, 28, hwnd, (HMENU)ID_BUTTON_OPEN_OBRAZ, hInst, NULL);
		SendMessage(hButtonOpenObraz, WM_SETFONT, (WPARAM)hNormalFont, 0);
		hStaticObraz = CreateWindowEx( 0, "STATIC", NULL, WS_CHILD | WS_VISIBLE |SS_LEFT, 110, 9, 200, 20, hwnd, NULL, hInst, NULL);
		SendMessage(hStaticObraz, WM_SETFONT, (WPARAM)hNormalFont, 0);
		SetWindowText(hStaticObraz, "nie wybrano pliku");

		hButtonOpenWzor = CreateWindowEx(0, "BUTTON", "Wybierz wzór", WS_CHILD | WS_VISIBLE, 5, 38, 100, 28, hwnd, (HMENU)ID_BUTTON_OPEN_WZOR, hInst, NULL);
		SendMessage(hButtonOpenWzor, WM_SETFONT, (WPARAM)hNormalFont, 0);
		hStaticWzor = CreateWindowEx( 0, "STATIC", NULL, WS_CHILD | WS_VISIBLE |SS_LEFT, 110, 42, 200, 20, hwnd, NULL, hInst, NULL);
		SendMessage(hStaticWzor, WM_SETFONT, (WPARAM)hNormalFont, 0);
		SetWindowText(hStaticWzor, "nie wybrano pliku");

		hButtonStart = CreateWindowEx(0, "BUTTON", "Start", WS_CHILD | WS_VISIBLE, 5, 71, 100, 28, hwnd, (HMENU)ID_BUTTON_START, hInst, NULL);
		SendMessage(hButtonStart, WM_SETFONT, (WPARAM)hNormalFont, 0);
		hStaticInfo = CreateWindowEx( 0, "STATIC", NULL, WS_CHILD | WS_VISIBLE |SS_LEFT, 110, 75, 80, 20, hwnd, NULL, hInst, NULL);
		SendMessage(hStaticInfo, WM_SETFONT, (WPARAM)hNormalFont, 0);

		hButtonTrackbarTolerancja = CreateWindowEx(0, TRACKBAR_CLASS, "Trackbar Control", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS, 5, 104, 255, 28, hwnd, (HMENU)ID_TRACKBAR_TOLERANCJA, hInst, NULL);
		SendMessage(GetDlgItem(hwnd,ID_TRACKBAR_TOLERANCJA), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 255));
		SendMessage(GetDlgItem(hwnd,ID_TRACKBAR_TOLERANCJA), TBM_SETPAGESIZE, 0,(LPARAM)1);
		SendMessage(GetDlgItem(hwnd,ID_TRACKBAR_TOLERANCJA), TBM_SETSEL, (WPARAM)FALSE, (LPARAM)MAKELONG(0, 255)); 
		SendMessage(GetDlgItem(hwnd,ID_TRACKBAR_TOLERANCJA), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)tolerancja);
		hStaticTolerancja = CreateWindowEx( 0, "STATIC", NULL, WS_CHILD | WS_VISIBLE |SS_LEFT, 265, 108, 100, 20, hwnd, NULL, hInst, NULL);
		SendMessage(hStaticTolerancja, WM_SETFONT, (WPARAM)hNormalFont, 0);
		stringstream ss;
		ss<<"tolerancja = "<<tolerancja;
		SetWindowText(hStaticTolerancja,ss.str().c_str());
		hCheckboxObraz = CreateWindowEx(0, "BUTTON", "dopasowanie", WS_CHILD | WS_VISIBLE | BS_CHECKBOX, 5, 137, 120, 28, hwnd, (HMENU)ID_CHECKBOX_OBRAZ, hInst, NULL);
		SendMessage(hCheckboxObraz, WM_SETFONT, (WPARAM)hNormalFont, 0);
		CheckDlgButton(hwnd, ID_CHECKBOX_OBRAZ, BST_CHECKED);
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc=BeginPaint(hwnd, &ps);
		if(Zakonczono)
		{
			if(WybranoDopasowanie)
				for(int y = 0; y < odchylenie.size() - wzor_h; ++y)
					for(int x = 0; x < odchylenie[y].size() - wzor_w; ++x)
						SetPixel(hdc, y+OBRAZ_X, x+OBRAZ_Y, RGB(wyswietl(y, x), wyswietl(y, x), wyswietl(y, x)));
			else
				for(int y = 0; y < tekst_h; ++y)
					for(int x = 0; x < tekst_w; ++x)
						SetPixel(hdc, y+OBRAZ_X, x+OBRAZ_Y, RGB(tekst[x][y]*255, tekst[x][y]*255, tekst[x][y]*255));
			HPEN Olowek, Piornik;
			Olowek = CreatePen(PS_SOLID, 1, 0x00FF00);
			Piornik = (HPEN)SelectObject(hdc, Olowek);
			for(int y = 0; y < odchylenie.size() - wzor_w; ++y)
				for(int x = 0; x < odchylenie[y].size() - wzor_h; ++x)
					if(wyswietl(y, x) < tolerancja 
						&& x+1 < odchylenie[y].size() - wzor_h && wyswietl(y, x+1) > wyswietl(y, x) 
						&& x-1 >= 0 && wyswietl(y, x-1) > wyswietl(y, x) 
						&& y+1 < odchylenie.size() - wzor_w && wyswietl(y+1, x) > wyswietl(y, x) 
						&& y-1 >= 0 && wyswietl(y-1, x) > wyswietl(y, x))
					{
						MoveToEx(hdc, y-wzor_w/2+OBRAZ_X, x-wzor_h/2+OBRAZ_Y, NULL);
						LineTo(hdc, y-wzor_w/2+OBRAZ_X, x+wzor_h/2+1+OBRAZ_Y);
						MoveToEx(hdc, y+wzor_w/2+OBRAZ_X, x-wzor_h/2+OBRAZ_Y, NULL);
						LineTo(hdc, y+wzor_w/2+OBRAZ_X, x+wzor_h/2+1+OBRAZ_Y);
						MoveToEx(hdc, y-wzor_w/2+OBRAZ_X, x-wzor_h/2+OBRAZ_Y, NULL);
						LineTo(hdc, y+wzor_w/2+1+OBRAZ_X, x-wzor_h/2+OBRAZ_Y);
						MoveToEx(hdc, y-wzor_w/2+OBRAZ_X, x+wzor_h/2+OBRAZ_Y, NULL);
						LineTo(hdc, y+wzor_w/2+1+OBRAZ_X, x+wzor_h/2+OBRAZ_Y);
					}
			SelectObject(hdc, Piornik);
			DeleteObject(Olowek);
		}
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_COMMAND:
		switch(wParam)
		{
		case ID_BUTTON_OPEN_OBRAZ:
		{
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = "Obrazy (*.bmp)\0*.bmp";
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFile = sSciezkaObraz;
			ofn.lpstrFileTitle = sNazwaPlikuObraz;
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrTitle = "Otwórz";
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.lpstrDefExt = "bmp";
			if(GetOpenFileName(&ofn))
			{
				ObrazWybrany = TRUE;
				SetWindowText(hStaticObraz, sNazwaPlikuObraz);
			}
			break;
		}
		case ID_BUTTON_OPEN_WZOR:
		{
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = "Obrazy (*.bmp)\0*.bmp";
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFile = sSciezkaWzor;
			ofn.lpstrFileTitle = sNazwaPlikuWzor;
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrTitle = "Otwórz";
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.lpstrDefExt = "bmp";
			if(GetOpenFileName(&ofn))
			{
				WzorWybrany = TRUE;
				SetWindowText(hStaticWzor, sNazwaPlikuWzor);
			}
			break;
		}
		case ID_BUTTON_START:
		{
			if(ObrazWybrany && WzorWybrany)
			{
				SetWindowText(hStaticInfo, "czekaj...");
				image::init();
				obraz.from_file(sSciezkaObraz, 1);
				wzorzec.from_file(sSciezkaWzor, 1);
				tekst_w = obraz.get_size().w;
				tekst_h = obraz.get_size().h;
				wzor_w = wzorzec.get_size().w;
				wzor_h = wzorzec.get_size().h;
				rozpoznawaj(hwnd);
				SetWindowText(hStaticInfo, "zakończono");
			}
			else
				if(ObrazWybrany == FALSE && WzorWybrany == FALSE)
					MessageBox(hwnd,"Nie wybrałeś obrazu i wzoru",0,MB_ICONWARNING);
				else
					if(ObrazWybrany == FALSE)
						MessageBox(hwnd,"Nie wybrałeś obrazu",0,MB_ICONWARNING);
					else
						MessageBox(hwnd,"Nie wybrałeś wzoru",0,MB_ICONWARNING);
			break;
		}
		case ID_CHECKBOX_OBRAZ:
		{
			if(IsDlgButtonChecked(hwnd, ID_CHECKBOX_OBRAZ) == BST_CHECKED)
			{
				CheckDlgButton(hwnd, ID_CHECKBOX_OBRAZ, BST_UNCHECKED);
				WybranoDopasowanie = FALSE;
				SetWindowText(hCheckboxObraz, "obraz");
				InvalidateRect(hwnd, NULL, TRUE);
			}
			else
			{
				CheckDlgButton(hwnd, ID_CHECKBOX_OBRAZ, BST_CHECKED);
				WybranoDopasowanie = TRUE;
				SetWindowText(hCheckboxObraz, "dopasowanie");
				InvalidateRect(hwnd, NULL, TRUE);
			}
			break;
		}
		}
		break;
	case WM_HSCROLL:
	{
		if((HANDLE)lParam == GetDlgItem(hwnd,ID_TRACKBAR_TOLERANCJA))
		{
			switch(LOWORD(wParam))
			{
			case TB_ENDTRACK:
				tolerancja=SendMessage(GetDlgItem(hwnd,ID_TRACKBAR_TOLERANCJA), TBM_GETPOS, 0, 0);
				break;
			case TB_BOTTOM:
				tolerancja=255;
				break;
			case TB_LINEDOWN:
				if(tolerancja<10)
					tolerancja++;
				break;
			case TB_LINEUP:
				if(tolerancja>1)
					tolerancja--;
				break;
			case TB_PAGEDOWN:
				if(tolerancja<10)
					tolerancja++;
				break;
			case TB_PAGEUP:
				if(tolerancja>1)
					tolerancja--;
				break;
			case TB_THUMBPOSITION:
				tolerancja=SendMessage(GetDlgItem(hwnd,ID_TRACKBAR_TOLERANCJA), TBM_GETPOS, 0, 0);
				break;
			case TB_THUMBTRACK:
				tolerancja=SendMessage(GetDlgItem(hwnd,ID_TRACKBAR_TOLERANCJA), TBM_GETPOS, 0, 0);
				break;
			case TB_TOP:
				tolerancja=1;
				break;
			}
			stringstream ss;
			ss<<"tolerancja = "<<tolerancja;
			SetWindowText(hStaticTolerancja,ss.str().c_str());
			if(Zakonczono)
				InvalidateRect(hwnd, NULL, FALSE);
		}
		break;
	}
    case WM_CLOSE:
		image::deinit();
        DestroyWindow(hwnd);
        break;
        
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
        
        default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	InitCommonControls();
	hInst=hInstance;
	LPSTR MainClassName = "ClassWndMain";
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = MainClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    if(!RegisterClassEx(&wc))return 0;
    HWND hwnd;
    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, MainClassName, "Rozpoznawanie obrazu", WS_OVERLAPPEDWINDOW, 200, 150, 654, 654, NULL, NULL, hInstance, NULL);
    if(hwnd == NULL)return 0;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
