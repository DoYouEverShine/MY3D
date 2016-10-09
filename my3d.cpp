#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <windows.h>
#include <tchar.h>
#include"mathlib.h"
#include"transform.h"
#include"device.h"
#include"light.h"
#include"ploygon.h"
//=====================================================================
// 坐标变换
//=====================================================================

//=====================================================================
// 几何计算：顶点、扫描线、边缘、矩形、步长计算
//=====================================================================

//=====================================================================
// 渲染设备
//=====================================================================


//=====================================================================
// Win32 窗口及图形绘制：为 device 提供一个 DibSection 的 FB
//=====================================================================
int screen_w, screen_h, screen_exit = 0;
int screen_mx = 0, screen_my = 0, screen_mb = 0;
int screen_keys[512];	// 当前键盘按下状态
static HWND screen_handle = NULL;		// 主窗口 HWND
static HDC screen_dc = NULL;			// 配套的 HDC
static HBITMAP screen_hb = NULL;		// DIB
static HBITMAP screen_ob = NULL;		// 老的 BITMAP
unsigned char *screen_fb = NULL;		// frame buffer
long screen_pitch = 0;

int screen_init(int w, int h, const TCHAR *title);	// 屏幕初始化
int screen_close(void);								// 关闭屏幕
void screen_dispatch(void);							// 处理消息
void screen_update(void);							// 显示 FrameBuffer

// win32 event handler
static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

// 初始化窗口并设置标题
int screen_init(int w, int h, const TCHAR *title) {
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)screen_events, 0, 0, 0,
		NULL, NULL, NULL, NULL, _T("SCREEN3.1415926") };
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,
		w * h * 4, 0, 0, 0, 0 } };
	RECT rect = { 0, 0, w, h };
	int wx, wy, sx, sy;
	LPVOID ptr;
	HDC hDC;

	screen_close();

	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;

	screen_handle = CreateWindow(_T("SCREEN3.1415926"), title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (screen_handle == NULL) return -2;

	screen_exit = 0;
	hDC = GetDC(screen_handle);
	screen_dc = CreateCompatibleDC(hDC);
	ReleaseDC(screen_handle, hDC);

	screen_hb = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (screen_hb == NULL) return -3;

	screen_ob = (HBITMAP)SelectObject(screen_dc, screen_hb);
	screen_fb = (unsigned char*)ptr;
	screen_w = w;
	screen_h = h;
	screen_pitch = w * 4;

	AdjustWindowRect(&rect, GetWindowLong(screen_handle, GWL_STYLE), 0);
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(screen_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(screen_handle);

	ShowWindow(screen_handle, SW_NORMAL);
	screen_dispatch();

	memset(screen_keys, 0, sizeof(int)* 512);
	memset(screen_fb, 0, w * h * 4);

	return 0;
}

int screen_close(void) {
	if (screen_dc) {
		if (screen_ob) {
			SelectObject(screen_dc, screen_ob);
			screen_ob = NULL;
		}
		DeleteDC(screen_dc);
		screen_dc = NULL;
	}
	if (screen_hb) {
		DeleteObject(screen_hb);
		screen_hb = NULL;
	}
	if (screen_handle) {
		CloseWindow(screen_handle);
		screen_handle = NULL;
	}
	return 0;
}

static LRESULT screen_events(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE: screen_exit = 1; break;
	case WM_KEYDOWN: screen_keys[wParam & 511] = 1; break;
	case WM_KEYUP: screen_keys[wParam & 511] = 0; break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

void screen_dispatch(void) {
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

void screen_update(void) {
	HDC hDC = GetDC(screen_handle);
	BitBlt(hDC, 0, 0, screen_w, screen_h, screen_dc, 0, 0, SRCCOPY);
	ReleaseDC(screen_handle, hDC);
	screen_dispatch();
}

//=====================================================================
// 主程序
//=====================================================================
//vertex_t mesh[8] = {
//	{ { 1, -1, 1, 1 }, { 1.0f, 0.2f, 0.2f }, 1 },
//	{ { -1, -1, 1, 1 },  { 0.2f, 1.0f, 0.2f }, 1 },
//	{ { -1, 1, 1, 1 },  { 0.2f, 0.2f, 1.0f }, 1 },
//	{ { 1, 1, 1, 1 },  { 1.0f, 0.2f, 1.0f }, 1 },
//	{ { 1, -1, -1, 1 }, { 1.0f, 1.0f, 0.2f }, 1 },
//	{ { -1, -1, -1, 1 }, { 0.2f, 1.0f, 1.0f }, 1 },
//	{ { -1, 1, -1, 1 },  { 1.0f, 0.3f, 0.3f }, 1 },
//	{ { 1, 1, -1, 1 },  { 0.2f, 1.0f, 0.3f }, 1 },
//};
ploy_t ploygon[12];
int ploy_num = 12;
vertex_t mesh[8] = {
	{ { 1, -1, 1, 1 }, { 0.3f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 1 },
	{ { -1, -1, 1, 1 }, { 0.3f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 1 },
	{ { -1, 1, 1, 1 }, { 0.3f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 1 },
	{ { 1, 1, 1, 1 }, { 0.3f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 1 },
	{ { 1, -1, -1, 1 }, { 0.3f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 1 },
	{ { -1, -1, -1, 1 }, { 0.3f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 1 },
	{ { -1, 1, -1, 1 }, { 0.3f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 1 },
	{ { 1, 1, -1, 1 }, { 0.3f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 1 },
};
//void draw_plane(device_t *device, int a, int b, int c, int d)
//{
//	vertex_t p1, p2, p3,p4;
//	p1 = mesh[a]; p2 = mesh[b]; p3 = mesh[c]; p4 = mesh[d];
//	Device::device_draw_primitive(device, &p1,&p2,&p3);
//	Device::device_draw_primitive(device, &p3, &p4, &p1);
//}
void draw_plane(device_t *device, ploy_t *ploygon)
{
	for (int i = 0; i < ploy_num; ++i)
	{
		auto tmp = (ploygon + i);
		Device::device_draw_primitive(device, tmp->vlist + tmp->vert[0], tmp->vlist + tmp->vert[1], tmp->vlist + tmp->vert[2]);
	}
}
void draw_box(device_t *device, float angle)
{
	Matrix::matrix_set_rotation(&device->transform.world, 1,-1, -1, angle);
	Transform::transform_update(&device->transform);
	draw_plane(device, ploygon);
}
void camera_at_zero(device_t *device, float x, float y, float z)
{
	point_t eye = { x, y, z, 1 }, at = { 0, 0, 0, 1 }, up = { 0, 0, 1, 1 };
	cam = eye;
	Matrix::matrix_set_lookat(&device->transform.view, &eye, &at, &up);
	Transform::transform_update(&device->transform);
}
void init_light()
{
	RGBAV1 white; white.r = 255; white.g = 255; white.b = 255; white.a = 0; 
	RGBAV1 yellow; yellow.r = 255; yellow.g = 255; yellow.b = 0; yellow.a = 0;
	RGBAV1 NONE; NONE.rgba = 0;
	RGBAV1 white_half; white_half.r = 128; white_half.g = 128; white_half.b = 128; white_half.a = 0;
	point_t spot_pos = { 2, 0, 0, 0 };
	vector_t spot_dir = { 1, 1, 1, 0 };
	vector_t sun_dir = { 0, 1, 0, 0 };
	vector_t sun_pos = { 0, 5, 5, 0 };
	int ambient_light = light.Init_Light_LIGHTV1(
		0,
		LIGHT_STATE_ON,
		LIGHT_ATTR_AMBIENT,
		white, NONE, NONE,
		NULL, NULL,
		0, 0, 0,
		0, 0, 0),
		sun_light = light.Init_Light_LIGHTV1(
		1,
		LIGHT_STATE_OFF,
		LIGHT_ATTR_INFINITE,
		NONE, yellow, NONE,
		NULL, &sun_dir,
		0, 0, 0,
		0, 0, 0),
		sun_spot_light = light.Init_Light_LIGHTV1(
		2,
		LIGHT_STATE_ON,
		LIGHT_ATTR_SPOTLIGHT1,
		NONE, white, NONE,
		&spot_pos, &spot_dir,
		0, 0.3, 0.1,
		30, 60, 1);
}
void init_object()
{
	ploygon[0].vlist = mesh; ploygon[0].vert[0] = 0; ploygon[0].vert[1] = 1; ploygon[0].vert[2] = 2; 
	ploygon[1].vlist = mesh; ploygon[1].vert[0] = 2; ploygon[1].vert[1] = 3; ploygon[1].vert[2] = 0;
	ploygon[2].vlist = mesh; ploygon[2].vert[0] = 7; ploygon[2].vert[1] = 6; ploygon[2].vert[2] = 5;
	ploygon[3].vlist = mesh; ploygon[3].vert[0] = 5; ploygon[3].vert[1] = 4; ploygon[3].vert[2] = 7;
	ploygon[4].vlist = mesh; ploygon[4].vert[0] = 0; ploygon[4].vert[1] = 4; ploygon[4].vert[2] = 5;
	ploygon[5].vlist = mesh; ploygon[5].vert[0] = 5; ploygon[5].vert[1] = 1; ploygon[5].vert[2] = 0;
	ploygon[6].vlist = mesh; ploygon[6].vert[0] = 1; ploygon[6].vert[1] = 5; ploygon[6].vert[2] = 6;
	ploygon[7].vlist = mesh; ploygon[7].vert[0] = 6; ploygon[7].vert[1] = 2; ploygon[7].vert[2] = 1;
	ploygon[8].vlist = mesh; ploygon[8].vert[0] = 2; ploygon[8].vert[1] = 6; ploygon[8].vert[2] = 7;
	ploygon[9].vlist = mesh; ploygon[9].vert[0] = 7; ploygon[9].vert[1] = 3; ploygon[9].vert[2] = 2;
	ploygon[10].vlist = mesh; ploygon[10].vert[0] = 3; ploygon[10].vert[1] = 7; ploygon[10].vert[2] = 4;
	ploygon[11].vlist = mesh; ploygon[11].vert[0] = 4; ploygon[11].vert[1] = 0; ploygon[11].vert[2] = 3;

}
int main(void)
{
	device_t device;
	TCHAR *title = _T("my3d (software renderer tutorial) - ")
		_T("Left/Right: rotation, Up/Down: forward/backward, Space: switch state");
	int states[] = { RENDER_STATE_TEXTURE, RENDER_STATE_COLOR, RENDER_STATE_WIREFRAME };
	int kbhit = 0;
	float alpha = 0.0f;
	float pos = 3.5f;
	int indicator = 0;
	if (screen_init(800, 600, title))
		return -1;
	Device::device_init(&device, 800, 600, screen_fb);
	init_light();
	init_object();
	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0) {
		
		screen_dispatch();
		Device::device_clear(&device);
		camera_at_zero(&device, pos, 0.0f, 0.0f);
		if (screen_keys[VK_UP]) pos -= 0.01f;
		if (screen_keys[VK_DOWN]) pos += 0.01f;
		if (screen_keys[VK_LEFT]) alpha -= 0.01f;
		if (screen_keys[VK_RIGHT]) alpha += 0.01f;
		if (screen_keys[VK_SPACE]) {
			if (kbhit == 0) {
				kbhit = 1;
				if (++indicator >= 3) indicator = 0;
				device.render_state = states[indicator];
			}
		}
		else {
			kbhit = 0;
		}

		draw_box(&device, alpha);
		screen_update();
		Sleep(1);
	}
	return 0;
}