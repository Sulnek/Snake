#pragma once
#ifndef PAINT_H
#define PAINT_H

#define _USE_MATH_DEFINES

#include <windows.h>
#include <d2d1_3.h>
#include <numbers>
#include <iostream>
#include <chrono>
#include <ctime>
#include <math.h>
#include <d2d1helper.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <math.h>


const int WIN_WIDTH = 1200;
const int WIN_HEIGHT = 620;
const int GRID_WIDTH = 40;
const int GRID_HEIGHT = 20;
const int MARGIN = 20;

const int FIELD_WIDTH = (WIN_WIDTH - 2 * MARGIN) / GRID_WIDTH;
const int FIELD_HEIGHT = (WIN_HEIGHT - 2 * MARGIN) / GRID_HEIGHT;

const float SPEED = (float) 0.4;
const float FONT_SIZE = 50.0f;
const float BOARDER_WIDTH = 5.0f;

class Paint {
private:
	ID2D1Factory7* d2d_factory = nullptr;
	ID2D1HwndRenderTarget* d2d_render_target = nullptr;
	RECT rc;
	ID2D1SolidColorBrush* myBrush = nullptr;
	ID2D1LinearGradientBrush* lin_brush = nullptr;
	IDWriteFactory* write_factory = nullptr;
	IDWriteTextFormat* text_format = nullptr;
	IWICImagingFactory* pIWICFactory = nullptr;
	ID2D1Bitmap* pBgBitmap = nullptr;
	ID2D1Bitmap* pLogoBitmap = nullptr;
	ID2D1PathGeometry* straightSegment = nullptr;
	ID2D1PathGeometry* curvedSegment = nullptr;
	ID2D1PathGeometry* headSegment = nullptr;
	ID2D1PathGeometry* tailSegment = nullptr;
	ID2D1PathGeometry* eatingParticleSegment = nullptr;

	HRESULT createIWICFactory();

	HRESULT createFactory();

	HRESULT createRectangleFromWindow(HWND& hwnd);

	HRESULT createRenderTarget(HWND& hwnd);

	HRESULT createBrush();

	HRESULT createLinearBrush();

	HRESULT createWriteFactory();

	HRESULT createTextFormat();

	HRESULT createBitmaps();

	HRESULT createBitmap(LPCWSTR file_name, ID2D1Bitmap** ptr);

	HRESULT createStraightSegment();

	HRESULT createCurvedSegment();

	HRESULT createHead();

	HRESULT createTail();

	void DiscardRenderDeviceResources();

	int CreateRenderDeviceResources(HWND& hwnd);

	void freeResources();

	const D2D1_MATRIX_3X2_F getTransformation(int x, int y, int orientation);

	HRESULT drawEatingParticle(int x, int y, int orientation, D2D1::ColorF color);

	HRESULT createEatingParticle();

public:

	~Paint();

	void setBackground(D2D1::ColorF color);

	void beginDraw();

	int endDraw(HWND& hwnd);

	void writeText(const WCHAR* text, D2D1::ColorF col, UINT32 len, float x, float y);

	void drawBgBitmap();

	HRESULT drawStraightSegment(int x, int y, int orientation, D2D1::ColorF color);

	HRESULT drawCurvedSegment(int x, int y, int orientation, D2D1::ColorF color);

	HRESULT drawHead(int x, int y, int orientation);

	HRESULT drawTail(int x, int y, int orientation);

	int createResources(HWND& hwnd);

	void drawBorders(float width);

	void drawCandy(int x, int y, D2D1::ColorF color);

	HRESULT drawEatingAnimation(int x, int y, int orientation, D2D1::ColorF color);

	void drawLogo();
};

#endif 