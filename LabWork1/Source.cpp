#define _USE_MATH_DEFINES
#include <windows.h>
#include <atlimage.h>
#include <math.h>
#include <gdiplus.h>

#define BACKGROUND_COLOR GetSysColor(COLOR_WINDOW)

OPENFILENAME FileOpenDialog;
char szFile[MAX_PATH];
int posX = 1;
int posY = 1;
bool ispicture = false;

const int speed = 10;
const int hight = 100;
const int width = 100;

//поворот спрайта
void TransformSprite(HDC hdc,double angle)
{
	XFORM xf;
	xf.eM11 = 1;
	xf.eM12 = 0;
	xf.eM21 = 0;
	xf.eM22 = 1;
	xf.eDx = (float)-(posX + width / 2);
	xf.eDy = (float)-(posY + hight / 2);
	ModifyWorldTransform(hdc, &xf, MWT_RIGHTMULTIPLY);

	xf.eM11 = (float)cos(angle);
	xf.eM12 = (float)sin(angle);
	xf.eM21 = (float)-sin(angle);;
	xf.eM22 = (float)cos(angle);
	xf.eDx = 0;
	xf.eDy = 0;
	ModifyWorldTransform(hdc, &xf, MWT_RIGHTMULTIPLY);

	xf.eM11 = 1;
	xf.eM12 = 0;
	xf.eM21 = 0;
	xf.eM22 = 1;
	xf.eDx = (float)(posX + width / 2);
	xf.eDy = (float)(posY + hight / 2);
	ModifyWorldTransform(hdc, &xf, MWT_RIGHTMULTIPLY);
}

SIZE GetBitmapSize(HBITMAP hBitmap)
{
	BITMAP bitmap;
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);
	SIZE result;
	result.cx = bitmap.bmWidth;
	result.cy = bitmap.bmHeight;
	return result;

}
//обработчик сообщений
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static HBRUSH solidBrush = CreateSolidBrush(RGB(0, 255, 0));
	static HBITMAP hBitmap;
	PAINTSTRUCT ps;

	static double angle = 0;
	int wheelDelta;
	int fwKeys;
	int MB_RESULT;

	int prevGraphicsMode;
	long bitWidth = 0, bitHight = 0;

	switch (message)
	{	
	//обработка сообщений колесика мыши
	case WM_MOUSEWHEEL:
		wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		fwKeys = GET_KEYSTATE_WPARAM(wParam);
		if (wheelDelta > 0)
		{
			if (fwKeys == MK_SHIFT)
				posX += speed;
			else
				posY -= speed;
		}
		if (wheelDelta < 0)
		{
			if (fwKeys == MK_SHIFT)
				posX -= speed;
			else
				posY += speed;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	//обработка сообщений клавиш
	case WM_KEYDOWN: // Обработка нажатия клавиши
		if (wParam == 68 || wParam == 39) //вправо
			posX += speed;
		if (wParam == 65 || wParam == 37) //влево
			posX -= speed;
		if (wParam == 83 || wParam == 40) //вниз
			posY += speed;
		if (wParam == 87 || wParam == 38) //вверх
			posY -= speed;
		if (wParam == 27) //если нажали ESC то выходим 
		{
			MB_RESULT = MessageBox(hWnd, "Вы действительно хотите выйти ?", "Выход", MB_YESNO);
			if (MB_RESULT == 6)
				SendMessage(hWnd, WM_DESTROY, wParam, lParam);
		}
		//загрузка картинки
		if (wParam == 112)
		{
			if (GetOpenFileName(&FileOpenDialog) == TRUE)
			{	
				ispicture = true;
				int length = MultiByteToWideChar(CP_ACP, 0, szFile, -1, NULL, 0);
				WCHAR *wideCharFileName = new WCHAR[length];
				MultiByteToWideChar(CP_ACP, 0, szFile, -1, wideCharFileName, length);

				Gdiplus::GdiplusStartupInput gdiplusStartupInput;
				ULONG_PTR gdiplusToken;
				GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

				Gdiplus::Bitmap *img = Gdiplus::Bitmap::FromFile(wideCharFileName);
				img->GetHBITMAP(BACKGROUND_COLOR, &hBitmap);
				Gdiplus::GdiplusShutdown(gdiplusToken);				
				/*
				hdc = GetDC(hWnd); //получаем контекст устройства клиентской области окна
				memBit = CreateCompatibleDC(hdc); //создаем контекст устройства в памяти 

				WCHAR
				hBitmap = (HBITMAP)LoadImage(NULL, FileOpenDialog.lpstrFile, IMAGE_BITMAP, width, hight, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
				GetObject(hBitmap, sizeof(bm), &bm);
				SelectObject(memBit, hBitmap);
				ReleaseDC(hWnd, hdc);*/
			}
		}
		//удаление картинки
		if (wParam == 113)
			ispicture = false;
		//поворот спрайта
		if (wParam == 49 || wParam == 50)
		{
			if (wParam == 49)
				angle += M_PI / 20;
			if (wParam == 50)
				angle -= M_PI / 20;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	//обработка сообщений перерисовки
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		prevGraphicsMode = SetGraphicsMode(hdc, GM_ADVANCED);
		TransformSprite(hdc, angle);

		if (!ispicture)
		{		
			RECT rect;
			rect.top = posY;
			rect.left = posX;
			rect.right = posX + width;
			rect.bottom = posY + hight;
			FillRect(hdc, &rect, solidBrush);
		}
		else
		{
			HDC sDC = CreateCompatibleDC(hdc);
			HGDIOBJ prevobj = SelectObject(sDC, hBitmap);
			//BitBlt(hdc, posX, posY, posX + width, posY + hight, sDC, 0, 0, SRCCOPY);
			SIZE spriteSize = GetBitmapSize(hBitmap);
			bitWidth = spriteSize.cx;
			bitHight = spriteSize.cy;
			StretchBlt(hdc, posX, posY, width, hight, sDC, 0, 0, bitWidth, bitHight, SRCCOPY);
			SelectObject(sDC, prevobj);
			DeleteDC(sDC);
		}

		ModifyWorldTransform(hdc, NULL, MWT_IDENTITY);
		SetGraphicsMode(hdc, prevGraphicsMode);
		ReleaseDC(hWnd, hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		DeleteObject(solidBrush);
		PostQuitMessage(0);	
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex; 
	HWND hWnd; 
	MSG msg;
	//инициализация структуры класса окна
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_CROSS);//IDC_ARROW
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "LabWork1Class";
	wcex.hIconSm = wcex.hIcon;
	RegisterClassEx(&wcex);//регистрация окна

	//создание окна
	hWnd = CreateWindow("LabWork1Class", "OSISP.LabWork 1",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	//диалоговое окно
	ZeroMemory(&FileOpenDialog, sizeof(FileOpenDialog));
	FileOpenDialog.lStructSize = sizeof(FileOpenDialog);
	FileOpenDialog.hwndOwner = hWnd;
	FileOpenDialog.lpstrFile = szFile;
	FileOpenDialog.lpstrFile[0] = '\0';
	FileOpenDialog.nMaxFile = sizeof(szFile);
	FileOpenDialog.lpstrFilter = _T("Images\0*.bmp;*.gif;*.jpeg;*.png;*.tiff;*.exif;*.wmf;*.emf\0\0");//Bitmap files(*.bmp)\0*.bmp\0
	FileOpenDialog.nFilterIndex = 1;
	FileOpenDialog.lpstrFileTitle = NULL;
	FileOpenDialog.nMaxFileTitle = 255;
	FileOpenDialog.lpstrInitialDir = _T("C:\\Users\\Денис\\Documents\\Visual Studio 2015\\Projects\\WinAPI\\LabWork1\\LabWork1\\");
	FileOpenDialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	//отображение окна
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//прием сообщений
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}



