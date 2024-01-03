#include "Paint.h"

using D2D1::RenderTargetProperties;
using D2D1::HwndRenderTargetProperties;
using D2D1::SizeU;
using D2D1::BezierSegment;
using D2D1::QuadraticBezierSegment;
using D2D1::LinearGradientBrushProperties;

#define NUM_RAD_STOPS 2
#define NUM_LIN_STOPS 3

HRESULT Paint::createFactory() {
    if (d2d_factory != nullptr) {
        return 1; // the factory has already been created!
    }
    HRESULT hret = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory
    );
    return hret;
}

HRESULT Paint::createRectangleFromWindow(HWND& hwnd) {
    if (!GetClientRect(hwnd, (LPRECT)&rc)) {
        return E_FAIL;
    }
    return S_OK;
}

HRESULT Paint::createRenderTarget(HWND& hwnd) {
    return d2d_factory->CreateHwndRenderTarget(
        RenderTargetProperties(),
        HwndRenderTargetProperties(
            hwnd,
            SizeU(
                static_cast<UINT32>(rc.right) -
                static_cast<UINT32>(rc.left),
                static_cast<UINT32>(rc.bottom) -
                static_cast<UINT32>(rc.top)
            )
        ),
        &d2d_render_target
    );
}

HRESULT Paint::createBrush() {
    return d2d_render_target->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Black, 1.0f),
        &myBrush
    );
}

HRESULT Paint::createLinearBrush() {
    ID2D1GradientStopCollection* lin_stops = nullptr;
    D2D1_GRADIENT_STOP lin_stops_data[NUM_LIN_STOPS];

    lin_stops_data[0] = {
    .position = 0.0f, .color = D2D1::ColorF(D2D1::ColorF::Black, 1.0f)
    };
    lin_stops_data[1] = {
        .position = 0.25f, .color = D2D1::ColorF(D2D1::ColorF::Black, 1.0f)
    };
    lin_stops_data[2] = {
        .position = 0.75f, .color = D2D1::ColorF(D2D1::ColorF::Gray, 1.0f)
    };

    HRESULT hr = d2d_render_target->CreateGradientStopCollection(
        lin_stops_data, NUM_LIN_STOPS, &lin_stops
    );
    if (FAILED(hr)) {
        return hr;
    }

    return d2d_render_target->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(
            D2D1::Point2F(0, 0),
            D2D1::Point2F(WIN_WIDTH, WIN_HEIGHT)
        ),
        lin_stops,
        &lin_brush
    );
}

void Paint::freeResources() {
    if (d2d_render_target) d2d_render_target->Release();
    if (d2d_factory) d2d_factory->Release();
    if (myBrush) myBrush->Release();
    if (write_factory) write_factory->Release();
    if (text_format) text_format->Release();
    if (lin_brush) lin_brush->Release();
    if (pIWICFactory) pIWICFactory->Release();
    if (pBgBitmap) pBgBitmap->Release();
    if (pLogoBitmap) pLogoBitmap->Release();
    if (straightSegment) straightSegment->Release();
    if (curvedSegment) curvedSegment->Release();
    if (headSegment) headSegment->Release();
    if (tailSegment) tailSegment->Release();
    if (eatingParticleSegment) eatingParticleSegment->Release();
}

void Paint::setBackground(D2D1::ColorF color) {
    d2d_render_target->Clear(color);
}


void Paint::beginDraw() {
    d2d_render_target->BeginDraw();
}

void Paint::endDraw() {
    d2d_render_target->EndDraw();
}

HRESULT Paint::createWriteFactory() {
    return DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&write_factory)
    );
}

HRESULT Paint::createTextFormat() {
    return write_factory->CreateTextFormat(
        L"Times New Roman",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        FONT_SIZE,
        L"en-us",
        &text_format
    );
}

void Paint::writeText(const WCHAR* text, D2D1::ColorF col, UINT32 len, float x, float y) {
    lin_brush->SetOpacity(0.2f);
    d2d_render_target->DrawText(
        text,
        len,
        text_format,
        D2D1::RectF(
            x + 10, y + 10,
            static_cast<FLOAT>(rc.right),
            static_cast<FLOAT>(rc.bottom)
        ),
        lin_brush
    );
    myBrush->SetColor(col);
    d2d_render_target->DrawText(
        text,
        len,
        text_format,
        D2D1::RectF(
            x, y,
            static_cast<FLOAT>(rc.right),
            static_cast<FLOAT>(rc.bottom)
        ),
        myBrush
    );
    
}


HRESULT Paint::createIWICFactory() {
    HRESULT hret = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hret)) {
        return hret;
    }

    return CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(IWICImagingFactory),
        reinterpret_cast<LPVOID*>(&pIWICFactory)
    );
}

HRESULT Paint::createBitmaps() {
    HRESULT hr;
    // Create the factory
    if (FAILED(hr = createIWICFactory())) {
        return hr;
    }
    if (FAILED(hr = createBitmap(L"bg.png", &pBgBitmap))) {
        return hr;
    }
    return createBitmap(L"logo.png", &pLogoBitmap);
}

HRESULT Paint::createBitmap(LPCWSTR file_name, ID2D1Bitmap** ptr) {
    HRESULT hr;
    IWICBitmapDecoder* pDecoder = nullptr;
    IWICBitmapFrameDecode* pSource = nullptr;
    IWICFormatConverter* pConverter = nullptr;

    // Create the decoder
    hr = pIWICFactory->CreateDecoderFromFilename(
        file_name,
        NULL,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &pDecoder
    );
    if (FAILED(hr)) {
        return hr;
    }

    // Create the initial frame.
    hr = pDecoder->GetFrame(0, &pSource);
    if (FAILED(hr)) {
        return hr;
    }

    // Create converter
    hr = pIWICFactory->CreateFormatConverter(&pConverter);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pConverter->Initialize(
        pSource,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        NULL,
        0.f,
        WICBitmapPaletteTypeMedianCut
    );
    if (FAILED(hr)) {
        return hr;
    }

    // Create Bitmap
    hr = d2d_render_target->CreateBitmapFromWicBitmap(
        pConverter,
        nullptr,
        ptr
    );
    if (FAILED(hr)) {
        return hr;
    }

    pDecoder->Release();
    pSource->Release();
    pConverter->Release();

    return hr;
}

void Paint::drawBgBitmap() {
    d2d_render_target->DrawBitmap(
        pBgBitmap,
        D2D1::RectF(0, 0, WIN_WIDTH, WIN_HEIGHT),
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
    );
}

void Paint::drawLogo() {
    d2d_render_target->DrawBitmap(
        pLogoBitmap,
        D2D1::RectF(WIN_WIDTH / 2, WIN_HEIGHT / 2, WIN_WIDTH, WIN_HEIGHT),
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
    );
}

HRESULT Paint::createStraightSegment() {
    HRESULT hr = d2d_factory->CreatePathGeometry(&straightSegment);
    if (FAILED(hr)) {
        return hr;
    }

    // Open a sink to write to the path geometry.
    ID2D1GeometrySink* geometry_sink = nullptr;
    hr = straightSegment->Open(&geometry_sink);
    if (FAILED(hr)) {
        return hr;
    }

    geometry_sink->BeginFigure({ 300.00f , 300.00f }, D2D1_FIGURE_BEGIN_FILLED);
    geometry_sink->AddLine({ 300.00f , 400.00f });
    geometry_sink->AddBezier({ { 333.00 , 450.00 }, { 367.00 , 350.00 }, { 400.00 , 400.00 } });
    geometry_sink->AddLine({ 400.00 , 300.00 });
    geometry_sink->AddBezier({ { 367.00 , 250.00 }, { 333.00 , 350.00 }, { 300.00 , 300.00 } });


    geometry_sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    if (SUCCEEDED(hr)) {
        hr = geometry_sink->Close();
    }

    // Release the resources.
    geometry_sink->Release();

    return hr;
}

HRESULT Paint::createCurvedSegment() {
    HRESULT hr = d2d_factory->CreatePathGeometry(&curvedSegment);
    if (FAILED(hr)) {
        return hr;
    }

    // Open a sink to write to the path geometry.
    ID2D1GeometrySink* geometry_sink = nullptr;
    hr = curvedSegment->Open(&geometry_sink);
    if (FAILED(hr)) {
        return hr;
    }

    geometry_sink->BeginFigure({ 400.00f , 300.00f }, D2D1_FIGURE_BEGIN_FILLED);
    geometry_sink->AddBezier({ { 350.00 , 333.00 }, { 450.00 , 367.00 }, { 400.00 , 400.00 } });
    geometry_sink->AddBezier({ { 367.00 , 350.00 }, { 333.00 , 450.00 }, { 300.00 , 400.00 } });
    geometry_sink->AddBezier({ { 300.00 , 300.00} , { 300.00 , 300.00} , { 400.00 , 300.00} });


    geometry_sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    if (SUCCEEDED(hr)) {
        hr = geometry_sink->Close();
    }

    // Release the resources.
    geometry_sink->Release();

    return hr;
}

HRESULT Paint::createHead() {
    HRESULT hr = d2d_factory->CreatePathGeometry(&headSegment);
    if (FAILED(hr)) {
        return hr;
    }

    // Open a sink to write to the path geometry.
    ID2D1GeometrySink* geometry_sink = nullptr;
    hr = headSegment->Open(&geometry_sink);
    if (FAILED(hr)) {
        return hr;
    }

    geometry_sink->BeginFigure({ 400.00f , 300.00f }, D2D1_FIGURE_BEGIN_FILLED);
    geometry_sink->AddBezier({ { 350.00 , 333.00 }, { 450.00 , 367.00 }, { 400.00 , 400.00 } });
    geometry_sink->AddBezier({ { 375.00 , 388.00 }, { 375.00 , 388.00 }, { 300.00 , 375.00 } });
    geometry_sink->AddLine({ 350.00 , 350.00 });
    geometry_sink->AddLine({ 300.00 , 325.00 });
    geometry_sink->AddBezier({ { 312.00 , 325.00} , { 312.00 , 325.00} , { 400.00 , 300.00} });


    geometry_sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    if (SUCCEEDED(hr)) {
        hr = geometry_sink->Close();
    }

    // Release the resources.
    geometry_sink->Release();

    return hr;
}

HRESULT Paint::createTail() {
    HRESULT hr = d2d_factory->CreatePathGeometry(&tailSegment);
    if (FAILED(hr)) {
        return hr;
    }

    // Open a sink to write to the path geometry.
    ID2D1GeometrySink* geometry_sink = nullptr;
    hr = tailSegment->Open(&geometry_sink);
    if (FAILED(hr)) {
        return hr;
    }

    geometry_sink->BeginFigure({ 400.00f , 300.00f }, D2D1_FIGURE_BEGIN_FILLED);
    geometry_sink->AddBezier({ { 350.00 , 333.00 }, { 450.00 , 367.00 }, { 400.00 , 400.00 } });
    geometry_sink->AddBezier({ { 325.00 , 375.00 }, { 325.00 , 375.00 }, { 300.00 , 350.00 } });
    geometry_sink->AddBezier({ { 325.00 , 325.00} , { 325.00 , 325.00} , { 400.00 , 300.00} });


    geometry_sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    if (SUCCEEDED(hr)) {
        hr = geometry_sink->Close();
    }

    // Release the resources.
    geometry_sink->Release();

    return hr;
}


HRESULT Paint::drawStraightSegment(int x, int y, int orientation, D2D1::ColorF color) {
    ID2D1TransformedGeometry* transformed_geometry;

    const D2D1_MATRIX_3X2_F transformationMatrix = getTransformation(x, y, orientation);
    HRESULT hr;
    hr = d2d_factory->CreateTransformedGeometry(straightSegment, &transformationMatrix, &transformed_geometry);
    if (FAILED(hr)) {
        return hr;
    }
    myBrush->SetColor(color);
    d2d_render_target->FillGeometry(transformed_geometry, myBrush);
    myBrush->SetColor(D2D1::ColorF(color.r / 2, color.g / 2, color.b / 2));
    d2d_render_target->DrawGeometry(transformed_geometry, myBrush);
    return S_OK;
}

HRESULT Paint::drawCurvedSegment(int x, int y, int orientation, D2D1::ColorF color) {
    ID2D1TransformedGeometry* transformed_geometry;

    const D2D1_MATRIX_3X2_F transformationMatrix = getTransformation(x, y, orientation + 2);
    HRESULT hr;
    hr = d2d_factory->CreateTransformedGeometry(curvedSegment, &transformationMatrix, &transformed_geometry);
    if (FAILED(hr)) {
        return hr;
    }

    myBrush->SetColor(color);
    d2d_render_target->FillGeometry(transformed_geometry, myBrush);
    myBrush->SetColor(D2D1::ColorF(color.r / 2, color.g / 2, color.b / 2));
    d2d_render_target->DrawGeometry(transformed_geometry, myBrush);
    return S_OK;
}

HRESULT Paint::drawTail(int x, int y, int orientation) {
    ID2D1TransformedGeometry* transformed_geometry;

    const D2D1_MATRIX_3X2_F transformationMatrix = getTransformation(x, y, orientation + 3);
    HRESULT hr;
    hr = d2d_factory->CreateTransformedGeometry(tailSegment, &transformationMatrix, &transformed_geometry);
    if (FAILED(hr)) {
        return hr;
    }

    D2D1::ColorF color = D2D1::ColorF(0.5, 0.25, 0.0);
    myBrush->SetColor(color);
    d2d_render_target->FillGeometry(transformed_geometry, myBrush);
    myBrush->SetColor(D2D1::ColorF(color.r / 2, color.g / 2, color.b / 2));
    d2d_render_target->DrawGeometry(transformed_geometry, myBrush);
    return S_OK;
}

const D2D1_MATRIX_3X2_F Paint::getTransformation(int x, int y, int orientation) {   
    return D2D1::Matrix3x2F::Scale(
            D2D1::SizeF(FIELD_HEIGHT / 100.0f, FIELD_WIDTH / 100.0f), D2D1::Point2F(350.00f, 350.00f)
    ) * D2D1::Matrix3x2F::Rotation(
            90.0f * orientation, D2D1::Point2F(350.00f, 350.00f)
    ) * D2D1::Matrix3x2F::Translation(
        FIELD_HEIGHT * y - 350.00f + FIELD_HEIGHT / 2 + MARGIN, FIELD_WIDTH * x - 350.00f + FIELD_WIDTH / 2 + MARGIN
    );
}

HRESULT Paint::drawHead(int x, int y, int orientation) {
    ID2D1TransformedGeometry* transformed_geometry;

    const D2D1_MATRIX_3X2_F transformationMatrix = getTransformation(x, y, orientation + 1);
    HRESULT hr;
    hr = d2d_factory->CreateTransformedGeometry(headSegment, &transformationMatrix, &transformed_geometry);
    if (FAILED(hr)) {
        return hr;
    }

    D2D1::ColorF color = D2D1::ColorF(0.5, 1.0, 0.5);
    myBrush->SetColor(color);
    d2d_render_target->FillGeometry(transformed_geometry, myBrush);
    myBrush->SetColor(D2D1::ColorF(color.r / 2, color.g / 2, color.b / 2));
    d2d_render_target->DrawGeometry(transformed_geometry, myBrush);
    return S_OK;
}

void Paint::drawBorders(float width) {
    D2D1_RECT_F rectangle = D2D1::RectF(
        (float) MARGIN - width,
        (float) MARGIN - width,
        (float) WIN_WIDTH - MARGIN,
        (float) WIN_HEIGHT - MARGIN);
    myBrush->SetColor(D2D1::ColorF(1, 0, 0));
    d2d_render_target->DrawRectangle(&rectangle, myBrush, width);
}

void Paint::drawCandy(int x, int y, D2D1::ColorF color) {
    myBrush->SetColor(color);
    auto center = D2D1::Point2F(
        (float) FIELD_HEIGHT * y + FIELD_HEIGHT / 2 + MARGIN,
        (float) FIELD_WIDTH * x + FIELD_WIDTH / 2 + MARGIN
    );
    auto ellipse = D2D1::Ellipse(center, FIELD_HEIGHT / 2, FIELD_WIDTH / 2);
    d2d_render_target->FillEllipse(ellipse, myBrush);
    myBrush->SetColor(D2D1::ColorF(color.r / 2, color.b / 2, color.g / 2));
    d2d_render_target->DrawEllipse(ellipse, myBrush, 1.0f);
}

HRESULT Paint::drawEatingParticle(int x, int y, float orientation, D2D1::ColorF color) {
    ID2D1TransformedGeometry* transformed_geometry;

    const D2D1_MATRIX_3X2_F transformationMatrix = getTransformation(x, y, orientation + 1);
    HRESULT hr;
    hr = d2d_factory->CreateTransformedGeometry(eatingParticleSegment, &transformationMatrix, &transformed_geometry);
    if (FAILED(hr)) {
        return hr;
    }

    myBrush->SetColor(color);
    d2d_render_target->FillGeometry(transformed_geometry, myBrush);
    myBrush->SetColor(D2D1::ColorF(color.r / 2, color.g / 2, color.b / 2));
    d2d_render_target->DrawGeometry(transformed_geometry, myBrush);
    return S_OK;
}

HRESULT Paint::createEatingParticle() {
    HRESULT hr = d2d_factory->CreatePathGeometry(&eatingParticleSegment);
    if (FAILED(hr)) {
        return hr;
    }

    // Open a sink to write to the path geometry.
    ID2D1GeometrySink* geometry_sink = nullptr;
    hr = eatingParticleSegment->Open(&geometry_sink);
    if (FAILED(hr)) {
        return hr;
    }

    geometry_sink->BeginFigure({ 370.00f , 300.00f }, D2D1_FIGURE_BEGIN_FILLED);
    geometry_sink->AddLine({ 400.0f, 330.0f });
    geometry_sink->AddBezier({ { 400.00 , 300.00 }, { 400.00 , 300.00 }, { 390.00 , 300.00 } });


    geometry_sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    if (SUCCEEDED(hr)) {
        hr = geometry_sink->Close();
    }

    // Release the resources.
    geometry_sink->Release();

    return hr;
}

HRESULT Paint::drawEatingAnimation(int x, int y, int orientation, D2D1::ColorF color) {
    HRESULT hr = drawEatingParticle(x, y, orientation + 2.5f, color);
    if (FAILED(hr)) {
        return hr;
    }
    return drawEatingParticle(x, y, orientation + 3.0f, color);
}

int Paint::createResources(HWND& hwnd) {
    // Create  necessary resources:
    HRESULT hret = createFactory();
    if (hret != S_OK) {
        return 1;
    }

    hret = createRectangleFromWindow(hwnd);
    if (hret != S_OK) {
        return 1;
    }

    if (FAILED(createRenderTarget(hwnd))) {
        return 1;
    }

    if (FAILED(createBrush())) {
        return 1;
    }

    if (FAILED(createLinearBrush())) {
        return 1;
    }

    if (FAILED(createWriteFactory())) {
        return 1;
    }

    if (FAILED(createTextFormat())) {
        return 1;
    }

    if (FAILED(createBitmaps())) {
        return 1;
    }

    if (FAILED(createStraightSegment())) {
        return 1;
    }

    if (FAILED(createCurvedSegment())) {
        return 1;
    }

    if (FAILED(createHead())) {
        return 1;
    }

    if (FAILED(createTail())) {
        return 1;
    }

    if (FAILED(createEatingParticle())) {
        return 1;
    }
    
    return 0;
}

Paint::~Paint() {
    freeResources(); //TODO add new resources' freeing
}