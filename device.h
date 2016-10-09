#ifndef DEVICE_H
#define DEVICE_H
#include<cstdlib>
#include<cassert>
#include<cstdio>
#include"mathlib.h"
#include"transform.h"
#include"light.h"
typedef unsigned int IUINT32;
extern point_t cam;

typedef struct
{
	transform_t transform;
	int width;
	int height;
	IUINT32 **framebuffer;      //framebuffer[j] 指向第j行
	float **zbuffer;
	int render_state;           // 渲染状态
	IUINT32 foreground;
	IUINT32 background;
}device_t;




#define RENDER_STATE_WIREFRAME      1		// 渲染线框
#define RENDER_STATE_TEXTURE        2		// 渲染纹理
#define RENDER_STATE_COLOR          4		// 渲染颜色
class Device
{
public:
	static int device_init(device_t* device, int width, int height, void* fb)
	{
		int need = 8 * height + height*width * 4;
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
			device->framebuffer[j] = (IUINT32*)(framebuf + j*width * 4);
			device->zbuffer[j] = (float*)(zbuf + j*width * 4);
		}
		Transform::transform_init(&device->transform, width, height);
		device->render_state = RENDER_STATE_COLOR;
		device->foreground = 0xc0c0c0;
		device->background = 0.0f;
		device->width = width;
		device->height = height;
		return 1;
	}


	static void device_pixel(device_t *device, int x, int y, IUINT32 color) {
		if (((IUINT32)x) < (IUINT32)device->width && ((IUINT32)y) < (IUINT32)device->height && x >= 0 && y >= 0) {
			device->framebuffer[y][x] = color;
		}
	}
	static void device_clear(device_t *device)
	{
		for (int i = 0; i < device->height; ++i)
		for (int j = 0; j < device->width; ++j)
		{
			device->framebuffer[i][j] = device->background;
			device->zbuffer[i][j] = 0.0f;
		}
	}

	static void device_interp(vertex_t *y, vertex_t *x1, vertex_t *x2, float factor)
	{
		float ifactor = 1 - factor;
		y->pos.x = x2->pos.x * factor + x1->pos.x * ifactor;
		y->pos.y = x2->pos.y * factor + x1->pos.y * ifactor;
		y->pos.z = x2->pos.z * factor + x1->pos.z * ifactor;
		y->light_color.r = x2->light_color.r * factor + x1->light_color.r * ifactor;
		y->light_color.g = x2->light_color.g * factor + x1->light_color.g * ifactor;
		y->light_color.b = x2->light_color.b * factor + x1->light_color.b * ifactor;
		y->rhw = x2->rhw * factor + x1->rhw * ifactor;
	}

	static void device_draw_line(device_t *device, int x0, int y0, int x1, int y1, IUINT32 color)
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
		}
		else {
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
	static void device_draw_Scanline(device_t *device, vertex_t *s, vertex_t *e, int y)
	{
		if (((IUINT32)y) > (IUINT32)device->height || y < 0) return;
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
			device_interp(&point, e, s, factor);
			int R = point.light_color.r *255.0f;
			int G = point.light_color.g *255.0f;
			int B = point.light_color.b *255.0f;
			int c = (R << 16) | (G << 8) | B;
			device_pixel(device, i, y, c);
		}
	}
	static void device_draw_triangle(device_t *device, vertex_t *a, vertex_t *b, vertex_t *c)           //    a
	{                      
		static float max;// b   
		vertex_t *tmp = NULL;                                                                           //      c     
		if ((FCMP(a->pos.y, b->pos.y) && FCMP(a->pos.y, c->pos.y)) ||
			(FCMP(a->pos.x, b->pos.x) && FCMP(a->pos.x, c->pos.x))) return;
		if (a->pos.y < b->pos.y) { tmp = a; a = b; b = tmp; }
		if (a->pos.y < c->pos.y) { tmp = a; a = c; c = tmp; }
		if (b->pos.y < c->pos.y) { tmp = b; b = c; c = tmp; }
		vertex_t middle;
		float mid = (b->pos.y - c->pos.y) / (a->pos.y - c->pos.y), lenY = (a->pos.y - b->pos.y);
		device_interp(&middle, c, a, mid);
		int startY = a->pos.y, endY = b->pos.y+1;
		for (int i = startY; i >= endY; --i)
		{
			float factor = 0;
			if (!FCMP(lenY,0)) factor = (float)(i - b->pos.y) / lenY;
			if (factor > max) 
			{ 
				max = factor; 
				printf("%0.2f %d %0.2f\n", factor, i, b->pos.y);
			}

			vertex_t xa, xb;
			device_interp(&xa, b, a, factor);
			device_interp(&xb, &middle, a, factor);
			device_draw_Scanline(device, &xa, &xb, i);
		}
		startY = b->pos.y;
		endY = c->pos.y+1;
		lenY = (b->pos.y - c->pos.y);
		for (int i = startY; i >= endY; --i)
		{
			float factor = 0;
			if (!FCMP(lenY, 0))  factor = (float)(i - c->pos.y) / lenY;
			vertex_t xa, xb;
			device_interp(&xa, c, b, factor);
			device_interp(&xb, c, &middle, factor);
			device_draw_Scanline(device, &xa, &xb, i);
		}
	}
	static bool device_backface_judge(point_t *a, point_t *b, point_t *c, point_t *n)
	{
		point_t p1, p2, n1;
		Vector::vector_sub(&p1, b, a);
		Vector::vector_sub(&p2, c, b);
		Vector::vector_crossproduct(&n1, &p1, &p2);
		float res = Vector::vector_dotproduct(&n1, n);
		return res < 0 ? 1 : 0;
	}
	//=====================================================================
	// 渲染实现
	//=====================================================================

	static void device_draw_primitive(device_t *device, const  vertex_t *v1, const  vertex_t *v2, const vertex_t *v3)
	{
		vertex_t p1, p2, p3, c1, c2, c3;
		int render_state = device->render_state;
		Transform::transform_apply_world(&device->transform, &c1, v1);
		Transform::transform_apply_world(&device->transform, &c2, v2);
		Transform::transform_apply_world(&device->transform, &c3, v3);                  //乘以视图矩阵 v * world * view * projection
		light.Light_Renderer(&c1, &c2, &c3, &cam);
		Transform::transform_apply_view_projection(&device->transform, &c1, &c1);
		Transform::transform_apply_view_projection(&device->transform, &c2, &c2);
		Transform::transform_apply_view_projection(&device->transform, &c3, &c3);                  //乘以视图矩阵 v * world * view * projection

		Transform::transform_homogenize(&device->transform, &p1, &c1);                 //除以齐次坐标项并转换为屏幕坐标
		Transform::transform_homogenize(&device->transform, &p2, &c2);
		Transform::transform_homogenize(&device->transform, &p3, &c3);
		point_t n = { 0, 0, -1, 1 };
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
};

#endif