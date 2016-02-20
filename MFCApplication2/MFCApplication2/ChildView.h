
// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
private:
	
	HBITMAP Background;
	HBITMAP Midground;
	HBITMAP Foreground;

	HBITMAP BackgroundMask;
	HBITMAP MidgroundMask;
	HBITMAP ForegroundMask;

	ULONG_PTR gdiplusToken;

	Gdiplus::Bitmap* slingbottom;
	Gdiplus::Bitmap* bird;
	Gdiplus::Bitmap* slingTop;

	int birdValueX;
	int birdValueY;

	int birdMomentumX;
	int birdMomentumY;

	bool shot;
	int degreeRotation;
	int timeout;
// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
//	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

