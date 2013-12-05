#define _USE_MATH_DEFINES
#include "img/image.h"
#include <windows.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <complex>
#define kw(x) (x)*(x)
#define c(i, j) complex <double>(i, j)
using namespace std;

image obraz,wzorzec;
double tekst[1000][1000];
double wzor[1000][1000];

int tekst_w, tekst_h, wzor_w, wzor_h;
vector<vector<double> >odchylenie;
vector<vector<double> > wynik_FFT;
int zrobione=false;
double maksimum=-4523423.0, minimum = 447289478293.0;
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
		{
			if(x+i>=maxx || y+j>=maxy)
				suma+=0;
			else
				suma+=(tekst[x+i][y+j]-wzor[i][j])*(tekst[x+i][y+j]-wzor[i][j]);
		}
	return suma;
}

long long round(double x)
{
	return (long long)(x+0.5);
}

vector<vector<long long> > dft(image a, image b)
{
	pair <int, int> tekst_r, wzor_r; //tekst_r - wielkosc tekstu, wzor_r - wielkosc wzorca 
	complex <double> e = complex<double>(M_E, 0.0), M_i = complex<double>(0.0, 1.0); //stale
	complex <double> tekst[1000][1000], wzor[1000][1000]; //tablice wejœciowe do dft i jednoczesnie wyjsciowe z ca³ego dft
	complex <double> w_tekst[1000][1000], w_wzor[1000][1000]; //tablice wysciowe z dft po wierszach	
	if(a.get_size().w) //konwersja img a na tablicê
	{
		tekst_r = make_pair(a.get_size().h, a.get_size().w);
		for(int y = 0; y < a.get_size().h; ++y)
			for(int x = 0; x < a.get_size().w; ++x)
				tekst[x][y] = c((double)*a(x, y, 1), 0.0); //tworzenie complexu 
	}
	for(int x = 0; x < tekst_r.second; x++)
		for(int y = 0; y < tekst_r.first; y++)
		{
			wzor[x][y] =w_tekst[x][y] = w_wzor[x][y] = c(0.0, 0.0); //czyszczenie tablic zapewnia ze w iloczynie nie bedzie niepotrzebnych skladnikow
		}
	if(b.get_size().w)
	{
		wzor_r = make_pair(b.get_size().h, b.get_size().w);
		for(int y = 0; y < b.get_size().h; ++y)
			for(int x = 0; x < b.get_size().w; ++x)
				wzor[x][y] = c((double)*b(x, y, 1), 0.0);
		//odwroc wzorzec
		for(int y = 0; y < wzor_r.first; y++)
			for(int x = 0; x < wzor_r.second/2; x++)
				swap(wzor[x][y], wzor[wzor_r.second - x][y]);
		for(int x = 0; x < wzor_r.second; x++)
			for(int y = 0; y < wzor_r.first/2; y++)
				swap(wzor[x][y], wzor[x][wzor_r.first - y]);
	}
	
	for(int y = 0; y < tekst_r.first; y++) //dft po wierszach tekstu i wzorca
		for(int i = 0; i < tekst_r.second; i++)
			for(int j = 0; j < tekst_r.second; j++)
			{
				w_tekst[i][y] += (tekst[j][y]*exp(c(-2*M_PI*j*i/tekst_r.second,0)*M_i));
				w_wzor[i][y] += (wzor[j][y]*exp(c(-2*M_PI*j*i/tekst_r.second,0)*M_i));
			}
	
	for(int x = 0; x < tekst_r.second; x++) //dft po kolumnach tekstu i wzorca 
		for(int i = 0; i < tekst_r.first; i++)
			for(int j = 0; j < tekst_r.first; j++)
			{
				tekst[x][i] += (w_tekst[x][j]*exp(c(-2*M_PI*j*i/tekst_r.first,0)*M_i));
				wzor[x][i] += (w_wzor[x][j]*exp(c(-2*M_PI*j*i/tekst_r.first,0)*M_i));
			}
	
	//mnozenie
	for(int x = 0; x < tekst_r.second; x++)
		for(int y = 0; y < tekst_r.first; y++)
			tekst[x][y] *= wzor[x][y];	//za prosto to wyglada 
	//transformata odwrotna na iloczynie
	
	for(int y = 0; y < tekst_r.first; y++) 
		for(int i = 0; i < tekst_r.second; i++)
			for(int j = 0; j < tekst_r.second; j++)
				w_tekst[i][y] += (tekst[j][y]*exp(c(2*M_PI*j*i/tekst_r.second,0)*M_i));				
	for(int x = 0; x < tekst_r.second; x++) 
		for(int i = 0; i < tekst_r.first; i++)
			for(int j = 0; j < tekst_r.first; j++)
				tekst[x][i] += (w_tekst[x][j]*exp(c(2*M_PI*j*i/tekst_r.first,0)*M_i));
	
	//przepisz na wektor
	
	vector< vector<long long> > vek;
	vek.resize(tekst_r.second);
	for(int x = 0; x < tekst_r.second; x++)
		for(int y = 0; y < tekst_r.first; y++)
			vek[x].push_back((long long)round(tekst[x][y].real()));
	 return vek;
}

int wyswietl(int x, int y)
{
	return (odchylenie[x][y] - minimum)  * 255 / (maksimum - minimum);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
	case WM_CREATE:
	{
		image::init();
		obraz.from_file("tekst.bmp", 1);
		wzorzec.from_file("wzor.bmp", 1);
		tekst_w = obraz.get_size().w;
		tekst_h = obraz.get_size().h;
		wzor_w = wzorzec.get_size().w;
		wzor_h = wzorzec.get_size().h;
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
		MessageBox(hwnd,0,0,0);
		for(int x=0; x < tekst_w; ++x)
			FFT_tekst_pion.push_back(FFT(zesp_tekst[x],1,1));
		MessageBox(hwnd,0,0,0);
		for(int x=0; x < tekst_w; ++x)
			FFT_wzor_pion.push_back(FFT(zesp_wzor[x],1,1));
		MessageBox(hwnd,0,0,0);
		//poziome FFT
		vector<vector<complex<double> > >FFT_tekst;
		vector<vector<complex<double> > >FFT_wzor;
		FFT_tekst.reserve(tekst_w);
		FFT_wzor.reserve(tekst_w);
		MessageBox(hwnd,0,0,0);
		for(int y=0; y < tekst_h; ++y)
		{
			vector<complex<double> >poziom;
			for(int x=0; x < tekst_w; ++x)
				poziom.push_back(FFT_tekst_pion[x][y]);
			FFT_tekst.push_back(FFT(poziom,1,1));
		}
		MessageBox(hwnd,0,0,0);
		for(int y=0; y < tekst_h; ++y)
		{
			vector<complex<double> >poziom;
			for(int x=0; x < tekst_w; ++x)
				poziom.push_back(FFT_wzor_pion[x][y]);
			FFT_wzor.push_back(FFT(poziom,1,1));
		}
		
		MessageBox(hwnd,0,0,0);
		//mno¿enie
		vector<vector<complex<double> > >pomnozone;
		for(int x=0; x < tekst_w; ++x)
		{
			vector<complex<double> >temp;
			for(int y=0; y < tekst_h; ++y)
				temp.push_back(FFT_tekst[y][x]*FFT_wzor[y][x]);
			pomnozone.push_back(temp);
		}
		
		MessageBox(hwnd,0,0,0);
		//FFT rev pionowe
		vector<vector<complex<double> > >rev_pion;
		for(int x=0; x < tekst_w; ++x)
			rev_pion.push_back(FFT(pomnozone[x],-1,1));
		
		MessageBox(hwnd,0,0,0);
		//FFT rev poziome
		vector<vector<complex<double> > >rev_poziom;
		for(int y=0; y < tekst_h; ++y)
		{
			vector<complex<double> >poziom;
			for(int x=0; x < tekst_w; ++x)
				poziom.push_back(rev_pion[x][y]);
			rev_poziom.push_back(FFT(poziom,-1,1));
		}
		
		MessageBox(hwnd,0,0,0);
		//zamiana na tablicê double
		for(int x=0; x < tekst_w; ++x)
		{
			vector<double>temp;
			for(int y=0; y < tekst_h; ++y)
				temp.push_back(rev_poziom[y][x].real()/kw(tekst_w));
			wynik_FFT.push_back(temp);
		}
		
		MessageBox(hwnd,0,0,0);
		vector<vector<double> >sumy_tekstu_wynik=sumy_tekstu();
		double suma_wzorca_wynik=suma_wzorca();
		for(int x = 0; x < tekst_w; ++x)
		{
			vector<double>temp;
			for(int y = 0; y < tekst_h; ++y)
				temp.push_back(sumy_tekstu_wynik[x][y]-2*wynik_FFT[x][y]+suma_wzorca_wynik);
			odchylenie.push_back(temp);
		}
		for(int y = 0; y < tekst_h; ++y)
			for(int x = 0; x < tekst_w; ++x)
				maksimum=max(odchylenie[x][y],maksimum);
		for(int y = 0; y < tekst_h; ++y)
			for(int x = 0; x < tekst_w; ++x)
				minimum=min(odchylenie[x][y],minimum);
		zrobione=true;
		break;
	}
	case WM_PAINT:
	{
		HDC hdc = GetDC(hwnd);
		/*if(zrobione)
		{
			for(int y = 0; y < tekst_h; ++y)
				for (int x = 0; x < tekst_w; ++x)
					SetPixel(hdc, x, y, RGB(wyswietl(x,y), wyswietl(x,y), wyswietl(x,y)));
		}*/
		if(zrobione)
		{
			for(int y = 0; y < odchylenie.size() - wzor_h; ++y)
				for (int x = 0; x < odchylenie[y].size() - wzor_w; ++x)
					SetPixel(hdc, x, y, RGB(wyswietl(y, x), wyswietl(y, x), wyswietl(y, x)));
			for(int y = 0; y < odchylenie.size() - wzor_h; ++y)
				for (int x = 0; x < odchylenie[y].size() - wzor_w; ++x)
					if (wyswietl(y, x) > 100)
					{
						HBRUSH PedzelZiel, Pudelko;
						HPEN OlowekCzerw, Piornik;
						PedzelZiel = CreateSolidBrush(0x00FF00);
						OlowekCzerw = CreatePen(PS_DOT, 1, 0x00FF00);
						Pudelko = (HBRUSH)SelectObject(hdc, PedzelZiel);
						Piornik = (HPEN)SelectObject(hdc, OlowekCzerw);
						Ellipse(hdc, x-10, y+10, x+10, y-10);
						SelectObject(hdc, Pudelko);
						SelectObject(hdc, Piornik);
						DeleteObject(OlowekCzerw);
						DeleteObject(PedzelZiel);
					}
		}
		ReleaseDC(hwnd, hdc);
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
    wc.hbrBackground =(HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = MainClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    if(!RegisterClassEx(&wc))return 0;
    HWND hwnd;
    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, MainClassName, "Rozpoznawanie obrazu", WS_OVERLAPPEDWINDOW, 50, 50, 1024, 1024, NULL, NULL, hInstance, NULL);
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
