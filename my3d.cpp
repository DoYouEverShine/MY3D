#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <windows.h>
#include <tchar.h>
typedef unsigned int IUINT32;
#define pi 3.141592
//=====================================================================
// ��ѧ�⣺�˲���Ӧ�ò�����⣬��Ϥ D3D ����任����
//=====================================================================
typedef struct {
	float m[4][4];
}matrix_t;

typedef struct { float x, y, z, w; }vector_t;
typedef vector_t point_t;
typedef struct { float r, g, b; } color_t;
typedef struct { float u, v; } texcoord_t;
typedef vector_t point_t;
//typedef struct { point_t pos; texcoord_t tc; color_t color; float rhw; } vertex_t;
typedef struct { point_t pos; color_t color; float rhw; } vertex_t;

float vector_length(const vector_t *x)
{
	return sqrt(x->x*x->x + x->y*x->y + x->z*x->z);
}
void vector_normalize(vector_t *y, vector_t *x)
{
	float s = vector_length(x);
	y->x = x->x / s;
	y->y = x->y / s;
	y->z = x->z / s;
	y->w = 1.0f;
}
void vector_sub(vector_t *res, const vector_t *a, const vector_t *b)
{
	res->x = a->x - b->x;
	res->y = a->y - b->y;
	res->z = a->z - b->z;
	res->w = 1.0f;
}
float vector_dotproduct(vector_t *a, vector_t *b)
{
	return a->x*b->x + a->y*b->y + a->z*b->z;
}
void vector_crossproduct(vector_t *res, const vector_t *a, const vector_t *b)
{
	res->x = a->y*b->z - a->z*b->y;
	res->y = a->z*b->x - a->x*b->z;
	res->z = a->x*b->y - a->y*b->x;
	res->w = 1.0f;
}
void matrix_mul(matrix_t *res, const matrix_t *a, const matrix_t *b)
{
	matrix_t tmp;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			tmp.m[i][j] = a->m[i][0] * b->m[0][j] +
						  a->m[i][1] * b->m[1][j] +
						  a->m[i][2] * b->m[2][j] +
						  a->m[i][3] * b->m[3][j] ;
		}
	}
	res[0] = tmp;
}

void matrix_transpose(matrix_t *y, matrix_t *x)
{
	matrix_t tmp;
	tmp.m[0][0] = x->m[0][0]; tmp.m[0][1] = x->m[1][0]; tmp.m[0][2] = x->m[2][0]; tmp.m[0][3] = x->m[3][0];
	tmp.m[1][0] = x->m[0][1]; tmp.m[1][1] = x->m[1][1]; tmp.m[1][2] = x->m[2][1]; tmp.m[1][3] = x->m[3][1];
	tmp.m[2][0] = x->m[0][2]; tmp.m[2][1] = x->m[1][2]; tmp.m[2][2] = x->m[2][2]; tmp.m[2][3] = x->m[3][2];
	tmp.m[3][0] = x->m[0][3]; tmp.m[3][1] = x->m[1][3]; tmp.m[3][2] = x->m[2][3]; tmp.m[3][3] = x->m[3][3];
	y[0] = tmp;
}
void matrix_set_identity(matrix_t *m)
{
	m->m[0][0] = m->m[1][1] = m->m[2][2] = m->m[3][3] = 1.0f;
	m->m[0][1] = m->m[0][2] = m->m[0][3] =
	m->m[1][0] = m->m[1][2] = m->m[1][3] =
	m->m[2][0] = m->m[2][1] = m->m[2][3] =
	m->m[3][0] = m->m[3][1] = m->m[3][2] = 0.0f;
}
void matrix_set_zero(matrix_t *m)
{
	m->m[0][0] = m->m[1][1] = m->m[2][2] = m->m[3][3] =
	m->m[0][1] = m->m[0][2] = m->m[0][3] =
	m->m[1][0] = m->m[1][2] = m->m[1][3] =
	m->m[2][0] = m->m[2][1] = m->m[2][3] =
	m->m[3][0] = m->m[3][1] = m->m[3][2] = 0.0f;
}
void matrix_rotate_x(matrix_t *m, float angle)
{
	matrix_set_zero(m);
	m->m[0][0] = 1.0f;
	m->m[1][1] = m->m[2][2] =  cos(angle);
	m->m[1][2] = -sin(angle);
	m->m[2][1] = -m->m[1][2];
	m->m[3][3] = 1.0f;
}
void matrix_rotate_y(matrix_t *m, float angle)
{
	matrix_set_zero(m);
	m->m[1][1] = 1.0f;
	m->m[0][0] = m->m[2][2] = cos(angle);
	m->m[0][2] = -sin(angle);
	m->m[2][0] = -m->m[0][2];
	m->m[3][3] = 1.0f;
}
void matrix_rotate_z(matrix_t *m, float angle)
{
	matrix_set_zero(m);
	m->m[2][2] = 1.0f;
	m->m[0][0] = m->m[1][1] = cos(angle);
	m->m[0][1] = -sin(angle);
	m->m[1][0] = -m->m[0][1];
	m->m[3][3] = 1.0f;
}
void matrix_apply(matrix_t *m, point_t *y, const point_t *x)
{
	float X, Y, Z,W;
	X = x->x * m->m[0][0] + x->y * m->m[1][0] + x->z * m->m[2][0] + x->w * m->m[3][0];
	Y = x->x * m->m[0][1] + x->y * m->m[1][1] + x->z * m->m[2][1] + x->w * m->m[3][1];
	Z = x->x * m->m[0][2] + x->y * m->m[1][2] + x->z * m->m[2][2] + x->w * m->m[3][2];
	W = x->x * m->m[0][3] + x->y * m->m[1][3] + x->z * m->m[2][3] + x->w * m->m[3][3];
	y->x = X; y->y = Y; y->z = Z; y->w = W;
}
void matrix_set_rotation(matrix_t *m, float x, float y, float z, float angle)
{
	matrix_set_identity(m);                   //����x����ת��xzƽ�棨atan(y/z)),����y����ת��z�ᣨasin(x/sqrt(x^2+y^2+z^2))
	matrix_t Rx, Ry, Rz, iRy, iRx;
	float theta_x, theta_y;
	if (y == 0 && z == 0)theta_x = 0;
	else
		if (y != 0 && z == 0) theta_x = pi/2;
		else theta_x = atan(y / z);
	if (x == 0 && sqrt(z*z + y*y) == 0)theta_y = 0;
	else
	if (x != 0 && sqrt(z*z + y*y) == 0) theta_y = pi / 2;
	else theta_y = atan(x / sqrt(z*z+y*y));
	matrix_rotate_x(&Rx, theta_x);
	matrix_rotate_y(&Ry, theta_y);
	matrix_rotate_z(&Rz, -angle);
	matrix_rotate_y(&iRy, -theta_y);
	matrix_rotate_x(&iRx, -theta_x);
	matrix_mul(m, &Ry, &Rx);
	matrix_mul(m, &Rz, m);
	matrix_mul(m, &iRy, m);
	matrix_mul(m, &iRx, m);
	matrix_transpose(m, m);
}
void matrix_set_perspective(matrix_t *m, float fov, float aspect, float n, float f)   //��������ϵ����ԭ�㿴��z�����򣬽�z��(n,f)ͶӰ��(0,1),���ھ����ҳˣ���Ҫת��
{
	float t = tan(fov / 2)*n,r = t*aspect;
	matrix_set_zero(m);
	m->m[0][0] = n / r; m->m[1][1] = n / t; m->m[2][2] = f / (f - n); m->m[3][2] = -1.0f*f*n / (f - n);
	m->m[2][3] = 1.0f;
}
void matrix_set_lookat(matrix_t *m, point_t *eye, vector_t *at, vector_t *up)       //view = [uvw]T * matrix[-e]
{
	matrix_set_zero(m);
	vector_t xaxis, yaxis, zaxis;
	vector_sub(&zaxis, at, eye);                          //u
	vector_normalize(&zaxis, &zaxis);
	vector_crossproduct(&xaxis, up, &zaxis);               //v
	vector_normalize(&xaxis, &xaxis);
	vector_crossproduct(&yaxis, &zaxis, &xaxis);               //w
	vector_normalize(&yaxis, &yaxis);
	m->m[0][0] = xaxis.x; m->m[1][0] = xaxis.y; m->m[2][0] = xaxis.z; m->m[3][0] = -vector_dotproduct(&xaxis, eye);
	m->m[0][1] = yaxis.x; m->m[1][1] = yaxis.y; m->m[2][1] = yaxis.z; m->m[3][1] = -vector_dotproduct(&yaxis, eye);
	m->m[0][2] = zaxis.x; m->m[1][2] = zaxis.y; m->m[2][2] = zaxis.z; m->m[3][2] = -vector_dotproduct(&zaxis, eye);
	m->m[3][3] = 1.0f;
}

//=====================================================================
// ����任
//=====================================================================
typedef struct {
	matrix_t world;         // ��������任
	matrix_t view;          // ��Ӱ������任
	matrix_t projection;    // ͶӰ�任
	matrix_t transform;     // transform = world * view * projection
	float w, h;             // ��Ļ��С
}	transform_t;

void transform_update(transform_t *ts)
{
	matrix_t m;
	matrix_mul(&m, &ts->view, &ts->projection);
	matrix_mul(&ts->transform, &ts->world, &m);
}
void transform_init(transform_t *ts, int width, int  height)
{
	matrix_set_identity(&ts->world);
	matrix_set_identity(&ts->view);
	float aspect = 1.0 * width / height;
	matrix_set_perspective(&ts->projection,pi / 2,aspect,1.0f,500.0f);
	ts->w = width;
	ts->h = height;
	transform_update(ts);
}

void transform_apply(transform_t *m, vertex_t *y, const vertex_t *x)
{
	matrix_apply(&m->transform, &y->pos, &x->pos);
	y->color.r = x->color.r;
	y->color.g = x->color.g;
	y->color.b = x->color.b;
}
void transform_homogenize(transform_t *ts, vertex_t *y, const vertex_t *x)
{
	float rhw = 1.0f / x->pos.w;
	y->pos.x = (x->pos.x * rhw + 1.0f) *ts->w * 0.5f;
	y->pos.y = (1.0f - x->pos.y * rhw) *ts->h * 0.5f;
	y->pos.z = x->pos.z * rhw;
	y->pos.w = 1.0f;
	y->color.r = x->color.r;
	y->color.g = x->color.g;
	y->color.b = x->color.b;
}
//=====================================================================
// ���μ��㣺���㡢ɨ���ߡ���Ե�����Ρ���������
//=====================================================================
void interp(vertex_t *y, vertex_t *x1, vertex_t *x2, float factor)
{
	float ifactor = 1 - factor;
	y->pos.x = x2->pos.x * factor + x1->pos.x * ifactor;
	y->pos.y = x2->pos.y * factor + x1->pos.y * ifactor;
	y->pos.z = x2->pos.z * factor + x1->pos.z * ifactor;
	y->color.r = x2->color.r * factor + x1->color.r * ifactor;
	y->color.g = x2->color.g * factor + x1->color.g * ifactor;
	y->color.b = x2->color.b * factor + x1->color.b * ifactor;
	y->rhw = x2->rhw * factor + x1->rhw * ifactor;
}
//=====================================================================
// ��Ⱦ�豸
//=====================================================================

typedef struct
{
	transform_t transform;
	int width;
	int height;
	IUINT32 **framebuffer;      //framebuffer[j] ָ���j��
	float **zbuffer;
	int render_state;           // ��Ⱦ״̬
	IUINT32 foreground;
	IUINT32 background;
}device_t;




#define RENDER_STATE_WIREFRAME      1		// ��Ⱦ�߿�
#define RENDER_STATE_TEXTURE        2		// ��Ⱦ����
#define RENDER_STATE_COLOR          4		// ��Ⱦ��ɫ

int device_init(device_t* device, int width, int height, void* fb)
{ 
	int need = 8*height + height*width*4;
	char *ptr = (char*)malloc(need);
	char *framebuf, *zbuf;
	int j;
	assert(ptr);
	device->framebuffer = (IUINT32**)ptr;
	ptr += sizeof(IUINT32**)*height;
	device->zbuffer = (float**)ptr;
	framebuf = (char*)fb;
	ptr += sizeof(float**)*height;
	zbuf = (char*)ptr;
	for (j = 0; j < height; ++j)
	{
		device->framebuffer[j] = (IUINT32*)(framebuf + j*width*4);
		device->zbuffer[j] = (float*)(zbuf + j*width * 4);
	}
	transform_init(&device->transform, width, height);
	device->render_state = RENDER_STATE_COLOR;
	device->foreground = 0xc0c0c0;
	device->background = 0.0f;
	device->width = width;
	device->height = height;
	return 1;
}


void device_pixel(device_t *device, int x, int y, IUINT32 color) {
	if (((IUINT32)x) < (IUINT32)device->width && ((IUINT32)y) < (IUINT32)device->height) {
		device->framebuffer[y][x] = color;
	}
}
void device_clear(device_t *device)
{
	for (int i = 0; i < device->height;++i)
	for (int j = 0; j < device->width; ++j)
	{
		device->framebuffer[i][j] = device->background;
		device->zbuffer[i][j] = 0.0f;
	}
}
void device_draw_line(device_t *device, int x0, int y0, int x1, int y1,IUINT32 color)
{
	if (abs(y1 - y0) <= abs(x1 - x0)){
		int x = x0;
		int increm = (x0 < x1) ? 1 : -1;
		float y = y0, det_y, det = x1 - x0;
		det_y = 1.0 * (y1 - y0) / det;
		for (x = x0;; x += increm){
			if (x == x1) break;
			device_pixel(device, x, 0.5 + y, color);
			y += increm * det_y;
		}
	}else {
		int y = y0;
		int increm = (y0 < y1) ? 1 : -1;
		float x = x0, det_x, det = y1 - y0;
		det_x = 1.0 * (x1 - x0) / det;
		for (y = y0;; y += increm){
			if (y == y1) break;
			device_pixel(device, x, 0.5 + y, color);
			x += increm * det_x;
		}
	}
}
void device_draw_Scanline(device_t *device, vertex_t *s, vertex_t *e, int y)
{
	IUINT32 *framebuffer = device->framebuffer[y];
	float *zbuffer = device->zbuffer[y];
	vertex_t *tmp = NULL;
	if (s->pos.x > e->pos.x){ tmp = s; s = e; e = tmp; }
	int startX = s->pos.x + 0.5, endX = e->pos.x + 0.5;
	for (int i = startX; i <= endX; ++i)
	{
		float factor = 0;
		if (startX != endX)  factor = (float)(i - endX) / (startX - endX);
		vertex_t point;
		interp(&point, e, s, factor);
		int R = point.color.r *255.0f;
		int G = point.color.g *255.0f;
		int B = point.color.b *255.0f;
		int c = (R << 16) | (G << 8) | B;
		framebuffer[i] = c;
	}
}
void device_draw_triangle(device_t *device, vertex_t *a, vertex_t *b, vertex_t *c)           //    a
{                                                                                            // b   
	vertex_t *tmp = NULL;                                                                           //      c     
	if (a->pos.y < b->pos.y) { tmp = a; a = b; b = tmp; }
	if (a->pos.y < c->pos.y) { tmp = a; a = c; c = tmp; }
	if (b->pos.y < c->pos.y) { tmp = b; b = c; c = tmp; }
	vertex_t middle;
	float mid = (b->pos.y - c->pos.y)/(a->pos.y - c->pos.y);
	interp(&middle, c, a, mid);
	int startY = a->pos.y + 0.5, endY = b->pos.y + 0.5;
	for (int i = startY; i >= endY; --i)
	{
		float factor = 0;
		if (startY!=endY) factor = (float)(i - endY) / (startY - endY);
		vertex_t xa, xb;
		interp(&xa, b, a, factor);
		interp(&xb, &middle, a, factor);
		device_draw_Scanline(device, &xa,&xb,i);
	}
	startY = endY;
	endY = c->pos.y;
	for (int i = startY; i >= endY; --i)
	{
		float factor = 0;
		if (startY != endY)  factor = (float)(i - endY) / (startY - endY);
		vertex_t xa, xb;
		interp(&xa, c, b, factor);
		interp(&xb, c, &middle, factor);
		device_draw_Scanline(device, &xa, &xb, i);
	}

}
bool device_backface_judge(point_t *a, point_t *b, point_t *c, point_t *n)
{
	point_t p1, p2,n1;
	vector_sub(&p1, b, a);
	vector_sub(&p2, c, b);
	vector_crossproduct(&n1, &p1, &p2);
	float res = vector_dotproduct(&n1, n);
	return res < 0 ? 1 : 0;
}
//=====================================================================
// ��Ⱦʵ��
//=====================================================================

void device_draw_primitive(device_t *device, const  vertex_t *v1, const  vertex_t *v2, const vertex_t *v3)
{
	vertex_t p1, p2, p3, c1, c2, c3;
	int render_state = device->render_state;
	transform_apply(&device->transform, &c1, v1);
	transform_apply(&device->transform, &c2, v2);
	transform_apply(&device->transform, &c3, v3);                  //������ͼ���� v * world * view * projection
	point_t n = { 0, 0, -1, 1 };
	transform_homogenize(&device->transform, &p1, &c1);                 //������������ת��Ϊ��Ļ����
	transform_homogenize(&device->transform, &p2, &c2);
	transform_homogenize(&device->transform, &p3, &c3);
	if (device_backface_judge(&p1.pos, &p2.pos, &p3.pos, &n)) return;
	if (render_state == RENDER_STATE_COLOR)
	{
		device_draw_triangle(device, &p1, &p2, &p3);
	}
	if (render_state == RENDER_STATE_WIREFRAME)
	{
		device_draw_line(device, (int)p1.pos.x, (int)p1.pos.y, (int)p2.pos.x, (int)p2.pos.y, device->foreground);
		device_draw_line(device, (int)p1.pos.x, (int)p1.pos.y, (int)p3.pos.x, (int)p3.pos.y, device->foreground);
		device_draw_line(device, (int)p3.pos.x, (int)p3.pos.y, (int)p2.pos.x, (int)p2.pos.y, device->foreground);
	}
}
//=====================================================================
// Win32 ���ڼ�ͼ�λ��ƣ�Ϊ device �ṩһ�� DibSection �� FB
//=====================================================================
int screen_w, screen_h, screen_exit = 0;
int screen_mx = 0, screen_my = 0, screen_mb = 0;
int screen_keys[512];	// ��ǰ���̰���״̬
static HWND screen_handle = NULL;		// ������ HWND
static HDC screen_dc = NULL;			// ���׵� HDC
static HBITMAP screen_hb = NULL;		// DIB
static HBITMAP screen_ob = NULL;		// �ϵ� BITMAP
unsigned char *screen_fb = NULL;		// frame buffer
long screen_pitch = 0;

int screen_init(int w, int h, const TCHAR *title);	// ��Ļ��ʼ��
int screen_close(void);								// �ر���Ļ
void screen_dispatch(void);							// ������Ϣ
void screen_update(void);							// ��ʾ FrameBuffer

// win32 event handler
static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

// ��ʼ�����ڲ����ñ���
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
// ������
//=====================================================================
vertex_t mesh[8] = {
	{ { 1, -1, 1, 1 }, { 1.0f, 0.2f, 0.2f }, 1 },
	{ { -1, -1, 1, 1 },  { 0.2f, 1.0f, 0.2f }, 1 },
	{ { -1, 1, 1, 1 },  { 0.2f, 0.2f, 1.0f }, 1 },
	{ { 1, 1, 1, 1 },  { 1.0f, 0.2f, 1.0f }, 1 },
	{ { 1, -1, -1, 1 }, { 1.0f, 1.0f, 0.2f }, 1 },
	{ { -1, -1, -1, 1 }, { 0.2f, 1.0f, 1.0f }, 1 },
	{ { -1, 1, -1, 1 },  { 1.0f, 0.3f, 0.3f }, 1 },
	{ { 1, 1, -1, 1 },  { 0.2f, 1.0f, 0.3f }, 1 },
};

void draw_plane(device_t *device, int a, int b, int c, int d)
{
	vertex_t p1, p2, p3,p4;
	p1 = mesh[a]; p2 = mesh[b]; p3 = mesh[c]; p4 = mesh[d];
	device_draw_primitive(device, &p1,&p2,&p3);
	device_draw_primitive(device, &p3, &p4, &p1);
}
void draw_box(device_t *device, float angle)
{
	matrix_set_rotation(&device->transform.world, 1,1, 1, angle);
	transform_update(&device->transform);
	draw_plane(device, 0, 1, 2, 3);
	draw_plane(device, 7, 6, 5, 4);
	draw_plane(device, 0, 4, 5, 1);
	draw_plane(device, 1, 5, 6, 2);
	draw_plane(device, 2, 6, 7, 3);
	draw_plane(device, 3, 7, 4, 0);
}
void camera_at_zero(device_t *device, float x, float y, float z)
{
	point_t eye = { x, y, z, 1 }, at = { 0, 0, 0, 1 }, up = { 0, 0, 1, 1 };
	matrix_set_lookat(&device->transform.view, &eye, &at, &up);
	transform_update(&device->transform);
}
int main(void)
{
	device_t device;
	TCHAR *title = _T("Mini3d (software render tutorial) - ")
		_T("Left/Right: rotation, Up/Down: forward/backward, Space: switch state");
	float alpha = 0.0f;
	float pos = 3.5f;
	if (screen_init(800, 600, title))
		return -1;
	device_init(&device, 800, 600, screen_fb);
	//camera_at_zero(&device, 3.5f, 0, 0);
	//camera_at_zero(&device, 0, 3.5f, 0);


	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0) {
		
		screen_dispatch();
		device_clear(&device);
		camera_at_zero(&device, pos, 0, 0);
		if (screen_keys[VK_UP]) pos -= 0.01f;
		if (screen_keys[VK_DOWN]) pos += 0.01f;
		if (screen_keys[VK_LEFT]) alpha -= 0.01f;
		if (screen_keys[VK_RIGHT]) alpha += 0.01f;
		draw_box(&device, alpha);
		screen_update();
		Sleep(1);
	}
	return 0;
}