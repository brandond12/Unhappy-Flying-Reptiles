
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "MFCApplication2.h"
#include "ChildView.h"
#include <gdiplus.h>
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Gdiplus;

// CChildView


CChildView::CChildView()
{
	//start GDI+
	GdiplusStartupInput GdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &GdiplusStartupInput, NULL);
	srand(NULL);
	timeout = 0;
}

CChildView::~CChildView()
{
	GdiplusShutdown(gdiplusToken);
}



BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
//	ON_WM_LBUTTONDBLCLK()
ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void printBitmapToHDC(HBITMAP Image, HBITMAP Mask, HDC * hdc, int screenWidth, int screenHight);
HBITMAP CreateMask(HBITMAP bitmapToMask, COLORREF ColorToMask, int redTolerance, int greenTolerance, int blueTolerance);

void CChildView::OnPaint() 
{
	CPaintDC dc(this);
	int width;
	int height;

	RECT rect;
	this->GetWindowRect(&rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	//setup backbuffer HDC
	HDC backBuffer = CreateCompatibleDC(dc);
	const int nbackBuffer = SaveDC(backBuffer);
	HBITMAP hBackBuffer = CreateCompatibleBitmap(dc, width, height);
	SelectObject(backBuffer, hBackBuffer);

	Graphics graphics(backBuffer);
	SolidBrush black(Gdiplus::Color(255, 255, 255));
	graphics.FillRectangle(&black, 0, 0, width, height);

	//print background
	printBitmapToHDC(Background, BackgroundMask, &backBuffer, width, height);

	//print midground
	printBitmapToHDC(Midground, MidgroundMask, &backBuffer, width, height);

	//print forground
	printBitmapToHDC(Foreground, ForegroundMask, &backBuffer, width, height);

	Gdiplus::Graphics g(backBuffer);

	//brint slingshot infront of bird
	g.DrawImage(slingbottom, 60, height - 200);

	if (!shot)
	{
		//check is bird out of range
		if ((birdValueX + 70) > width)
		{
			//reset bird position
			birdValueX = 0;
		}
		else
		{
			//move bird
			//birdValueX += birdMomentumX;
		}

		//randomly change bird vertical speed
		//create random number of 1, 0 or -1
		int randNum = rand() % 3;
		randNum -= 1;
		birdMomentumY += randNum;

		if (birdMomentumY > 3)
		{
			birdMomentumY = 3;
		}
		else if (birdMomentumY < -3)
		{
			birdMomentumY = -3;
		}
		//check the bird is moving within bounds
		if (birdValueY <= 0)
		{
			birdValueY = 0;
			birdMomentumY = 1;
		}
		else if (birdValueY > height - height / 2)
		{
			birdValueY = height - height / 2;
			birdMomentumY = -1;
		}
		//move bird
		//birdValueY += birdMomentumY;
		//print bird
		g.DrawImage(bird, birdValueX, birdValueY);
	}
	else
	{
		birdValueX += birdMomentumX;
		birdValueY += birdMomentumY;
		degreeRotation += 5;
		if (birdValueY > height - 100)
		{
			birdMomentumX = 0;
			birdMomentumY = 0;
			timeout = 5;//70;
		}

		if (degreeRotation > 360)
		{
			degreeRotation = 0;
		}

		//find center of bird from origin
		int xTranslate = birdValueX +(64 / 2);
		int yTranslate = birdValueY +(64 / 2);

		//move origin to center of bird
		g.TranslateTransform(xTranslate, yTranslate, MatrixOrderAppend);
		//rotate canvase on origin
		g.RotateTransform(degreeRotation);

		//draw bird 
		g.DrawImage(bird, -32, -32);

		//put canvase back to what it was
		g.RotateTransform(-degreeRotation);
		g.TranslateTransform(-xTranslate, -yTranslate, MatrixOrderAppend);
	}
	
	//print slingshot behind bird
	g.DrawImage(slingTop, 60, height - 200);
	g.RotateTransform(30);
	//context switch back buffer
	BitBlt(dc, 0, 0, width, height, backBuffer, 0, 0, SRCCOPY);

	DeleteObject(hBackBuffer);
	DeleteDC(backBuffer);
	DeleteDC(dc);
}

void printBitmapToHDC(HBITMAP Image, HBITMAP Mask, HDC * hdc, int screenWidth, int screenHight)
{
	CDC hdcImage;
	CDC hdcMask;

	hdcImage.CreateCompatibleDC(NULL);
	hdcMask.CreateCompatibleDC(NULL);

	//create bitmaps used to apply mask
	HBITMAP maskBitmap = (HBITMAP)SelectObject(hdcMask, Mask);
	HBITMAP oldImageBitmap = (HBITMAP)SelectObject(hdcImage, Image);

	//bitmap for getting size
	BITMAP bm;
	GetObject(Image, sizeof(BITMAP), &bm);

	//apply mask to image 
	StretchBlt(*hdc, 0, 0, screenWidth, screenHight, hdcMask, 0, 0, bm.bmWidth, bm.bmHeight, SRCAND);
	StretchBlt(*hdc, 0, 0, screenWidth, screenHight, hdcImage, 0, 0, bm.bmWidth, bm.bmHeight, SRCPAINT);

	//clean up
	DeleteDC(hdcMask);
	DeleteDC(hdcImage);
}

bool detectHit(Bitmap* object, int x, int y)
{
	bool hit = false;
	Gdiplus::Color* clickedColor = new Gdiplus::Color();
	object->GetPixel(x, y, clickedColor);
	int a = clickedColor->GetAlpha();
	int r = clickedColor->GetRed();
	int g = clickedColor->GetGreen();
	int b = clickedColor->GetBlue();
	if (a == 0 || (r != 0 && g != 0 && b != 0))
	{
		hit = true;
	}
	return hit;
}

HBITMAP CreateMask(HBITMAP bitmapToMask, COLORREF ColorToMask, int redTolerance, int greenTolerance, int blueTolerance)
{
	HBITMAP hMask;
	BITMAP bmSize;
	HBITMAP oldBitmap;
	HDC hdc1;
	HDC hdc2;
	BITMAPINFOHEADER bitmapInfo = { 0 };
	int red = 0;
	int green = 0;
	int blue = 0;

	GetObject(bitmapToMask, sizeof(BITMAP), &bmSize);

	BYTE* ImageData = new BYTE[3 * bmSize.bmWidth * bmSize.bmHeight];

	hMask = CreateBitmap(bmSize.bmWidth, bmSize.bmHeight, 1, 1, NULL);

	hdc1 = CreateCompatibleDC(NULL);
	hdc2 = CreateCompatibleDC(NULL);

	oldBitmap = (HBITMAP)SelectObject(hdc1, bitmapToMask);
	SelectObject(hdc2, hMask);

	bitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.biPlanes = 1;
	bitmapInfo.biBitCount = 24;
	bitmapInfo.biWidth = bmSize.bmWidth;
	bitmapInfo.biHeight = -bmSize.bmHeight;
	bitmapInfo.biCompression = BI_RGB;
	bitmapInfo.biSizeImage = bmSize.bmWidth * bmSize.bmHeight;

	GetDIBits(hdc1, bitmapToMask, 0, bmSize.bmHeight, ImageData, (BITMAPINFO*)&bitmapInfo, DIB_RGB_COLORS);

	//loop through all bits
	SetBkColor(hdc1, ColorToMask);
	
	for (int y = 0; y <= bmSize.bmHeight; y++)
	{
		for (int x = 0; x <= bmSize.bmWidth; x++)
		{
			red = ImageData[(3 * ((y * bmSize.bmWidth) + x)) + 2];

			green = ImageData[(3 * ((y * bmSize.bmWidth) + x)) + 1];

			blue = ImageData[3 * ((y * bmSize.bmWidth) + x)];

			if (green >= greenTolerance && red <= redTolerance && blue <= blueTolerance)
			{
				SetPixel(hdc2, x, y, RGB(255, 255, 255));
				SetPixel(hdc1, x, y, RGB(0, 255, 0));
			}
			else
			{
				SetPixel(hdc2, x, y, RGB(0, 0, 0));
			}
		}
	}
	BitBlt(hdc1, 0, 0, bmSize.bmWidth, bmSize.bmHeight, hdc2, 0, 0, SRCINVERT);	

	DeleteDC(hdc1);
	DeleteDC(hdc2);

	return hMask;
}


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	int width;
	int height;

	RECT rect;
	this->GetWindowRect(&rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	SetTimer(1, 20, NULL);

	Background = (HBITMAP)LoadImage(NULL, "media/background.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	Midground = (HBITMAP)LoadImage(NULL, "media/Midground.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	Foreground = (HBITMAP)LoadImage(NULL, "media/Foreground.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);

	BackgroundMask = CreateMask(Background, RGB(0, 255, 0), 0, 255, 0);
	MidgroundMask = CreateMask(Midground, RGB(0, 255, 0), 100, 170, 100);
	ForegroundMask = CreateMask(Foreground, RGB(0, 255, 0), 130, 100, 50);

	slingbottom = Gdiplus::Bitmap::FromFile(L"media/slingshot1.png");
	bird = Gdiplus::Bitmap::FromFile(L"media/reptile.png");
	slingTop = Gdiplus::Bitmap::FromFile(L"media/slingshot2.png");

	//initilize bird position
	birdValueX = 0;
	birdValueY = 50;

	birdMomentumX = 2;
	birdMomentumY = 1;

	shot = false;
	degreeRotation = 0;
	return 0;
}


BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	return false;
}


void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (timeout == 0)
	{
		Invalidate();
	}
	else if(timeout > 1)
	{
		timeout--;
	}
	else
	{
		//reset game
		birdValueX = 0;
		birdValueY = 50;

		birdMomentumX = 2;
		birdMomentumY = 1;

		shot = false;
		timeout = 0;
		degreeRotation = 0;
	}

	CWnd::OnTimer(nIDEvent);
}


void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	
	if (point.x >= birdValueX && point.x <= birdValueX + 64 && point.y >= birdValueY && point.y <= birdValueY + 64)
	{
		if (detectHit(bird, point.x - birdValueX, point.y - birdValueY))
		{
			shot = true;
			birdMomentumX = 10;
			birdMomentumY = 10;
		}
	}
	

	CWnd::OnLButtonDown(nFlags, point);
}