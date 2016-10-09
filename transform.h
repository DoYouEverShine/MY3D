#ifndef TRANSFORM_H
#define TRANSFORM_H
#include"mathlib.h"
typedef struct {
	matrix_t world;         // 世界坐标变换
	matrix_t view;          // 摄影机坐标变换
	matrix_t projection;    // 投影变换
	matrix_t transform;     // transform = world * view * projection
	float w, h;             // 屏幕大小
}	transform_t;
class Transform
{
public:
	static void transform_update(transform_t *ts)
	{
		matrix_t m;
		Matrix::matrix_mul(&m, &ts->view, &ts->projection);
		Matrix::matrix_mul(&ts->transform, &ts->world, &m);
	}
	static void transform_init(transform_t *ts, int width, int  height)
	{
		Matrix::matrix_set_identity(&ts->world);
		Matrix::matrix_set_identity(&ts->view);
		float aspect = 1.0 * width / height;
		Matrix::matrix_set_perspective(&ts->projection, pi / 2, aspect, 1.0f, 500.0f);
		ts->w = width;
		ts->h = height;
		transform_update(ts);
	}

	static void transform_apply(transform_t *m, vertex_t *y, const vertex_t *x)
	{
		Matrix::matrix_apply(&m->transform, &y->pos, &x->pos);
		y->base_color.r = x->base_color.r;
		y->base_color.g = x->base_color.g;
		y->base_color.b = x->base_color.b;
		y->light_color.r = x->light_color.r;
		y->light_color.g = x->light_color.g;
		y->light_color.b = x->light_color.b;
	}
	static void transform_apply_world(transform_t *m, vertex_t *y, const vertex_t *x)
	{
		Matrix::matrix_apply(&m->world, &y->pos, &x->pos);
		y->base_color.r = x->base_color.r;
		y->base_color.g = x->base_color.g;
		y->base_color.b = x->base_color.b;
		y->light_color.r = x->light_color.r;
		y->light_color.g = x->light_color.g;
		y->light_color.b = x->light_color.b;
	}
	static void transform_apply_view_projection(transform_t *m, vertex_t *y, const vertex_t *x)
	{
		Matrix::matrix_apply(&m->view, &y->pos, &x->pos);
		Matrix::matrix_apply(&m->projection, &y->pos, &y->pos);
		y->base_color.r = x->base_color.r;
		y->base_color.g = x->base_color.g;
		y->base_color.b = x->base_color.b;
		y->light_color.r = x->light_color.r;
		y->light_color.g = x->light_color.g;
		y->light_color.b = x->light_color.b;
	}
	static void transform_homogenize(transform_t *ts, vertex_t *y, const vertex_t *x)
	{
		float rhw = 1.0f / x->pos.w;
		y->pos.x = (x->pos.x * rhw + 1.0f) *ts->w * 0.5f;
		y->pos.y = (1.0f - x->pos.y * rhw) *ts->h * 0.5f;
		y->pos.z = x->pos.z * rhw;
		y->pos.w = 1.0f;
		y->base_color.r = x->base_color.r;
		y->base_color.g = x->base_color.g;
		y->base_color.b = x->base_color.b;
		y->light_color.r = x->light_color.r;
		y->light_color.g = x->light_color.g;
		y->light_color.b = x->light_color.b;
	}
};
#endif