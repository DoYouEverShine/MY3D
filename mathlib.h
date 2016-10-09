#ifndef MATHLIB_H
#define MATHLIB_H
#include<cmath>


#define pi 3.141592
#define eps 0.001
#define FCMP(a,b) (fabs(a-b)<eps)


typedef struct {
	float m[4][4];
}matrix_t;

typedef struct { float x, y, z, w; }vector_t;
typedef vector_t point_t;
typedef struct { float r, g, b; } color_t;
typedef struct { float u, v; } texcoord_t;
typedef vector_t point_t;
typedef struct { point_t pos;  color_t base_color;color_t light_color; float rhw; } vertex_t,*vertex_t_ptr;
typedef struct
{
	int state;
	int attr;
	int color;
	int lit_color[3];
	int mati;
	vertex_t_ptr vlist;
	int vert[3];
	int text[3];
	float nlength;
}ploy_t,*ploy_t_ptr;
class Vector
{
public:
	static float vector_length(const vector_t *x)
	{
		return sqrt(x->x*x->x + x->y*x->y + x->z*x->z);
	}
	static void vector_copy(vector_t *y, vector_t *x)
	{
		y->x = x->x;
		y->y = x->y;
		y->z = x->z;
		y->w = x->w;
	}
	static void vector_normalize(vector_t *y, vector_t *x)
	{
		float s = vector_length(x);
		y->x = x->x / s;
		y->y = x->y / s;
		y->z = x->z / s;
		y->w = 1.0f;
	}
	static void vector_sub(vector_t *res, const vector_t *a, const vector_t *b)
	{
		res->x = a->x - b->x;
		res->y = a->y - b->y;
		res->z = a->z - b->z;
		res->w = 1.0f;
	}
	static float vector_dotproduct(vector_t *a, vector_t *b)
	{
		return a->x*b->x + a->y*b->y + a->z*b->z;
	}
	static void vector_crossproduct(vector_t *res, const vector_t *a, const vector_t *b)
	{
		res->x = a->y*b->z - a->z*b->y;
		res->y = a->z*b->x - a->x*b->z;
		res->z = a->x*b->y - a->y*b->x;
		res->w = 1.0f;
	}
};
class Matrix
{
public:
	static void matrix_mul(matrix_t *res, const matrix_t *a, const matrix_t *b)
	{
		matrix_t tmp;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				tmp.m[i][j] = a->m[i][0] * b->m[0][j] +
					a->m[i][1] * b->m[1][j] +
					a->m[i][2] * b->m[2][j] +
					a->m[i][3] * b->m[3][j];
			}
		}
		res[0] = tmp;
	}

	static void matrix_transpose(matrix_t *y, matrix_t *x)
	{
		matrix_t tmp;
		tmp.m[0][0] = x->m[0][0]; tmp.m[0][1] = x->m[1][0]; tmp.m[0][2] = x->m[2][0]; tmp.m[0][3] = x->m[3][0];
		tmp.m[1][0] = x->m[0][1]; tmp.m[1][1] = x->m[1][1]; tmp.m[1][2] = x->m[2][1]; tmp.m[1][3] = x->m[3][1];
		tmp.m[2][0] = x->m[0][2]; tmp.m[2][1] = x->m[1][2]; tmp.m[2][2] = x->m[2][2]; tmp.m[2][3] = x->m[3][2];
		tmp.m[3][0] = x->m[0][3]; tmp.m[3][1] = x->m[1][3]; tmp.m[3][2] = x->m[2][3]; tmp.m[3][3] = x->m[3][3];
		y[0] = tmp;
	}
	static void matrix_set_identity(matrix_t *m)
	{
		m->m[0][0] = m->m[1][1] = m->m[2][2] = m->m[3][3] = 1.0f;
		m->m[0][1] = m->m[0][2] = m->m[0][3] =
			m->m[1][0] = m->m[1][2] = m->m[1][3] =
			m->m[2][0] = m->m[2][1] = m->m[2][3] =
			m->m[3][0] = m->m[3][1] = m->m[3][2] = 0.0f;
	}
	static void matrix_set_zero(matrix_t *m)
	{
		m->m[0][0] = m->m[1][1] = m->m[2][2] = m->m[3][3] =
			m->m[0][1] = m->m[0][2] = m->m[0][3] =
			m->m[1][0] = m->m[1][2] = m->m[1][3] =
			m->m[2][0] = m->m[2][1] = m->m[2][3] =
			m->m[3][0] = m->m[3][1] = m->m[3][2] = 0.0f;
	}
	static void matrix_rotate_x(matrix_t *m, float angle)
	{
		matrix_set_zero(m);
		m->m[0][0] = 1.0f;
		m->m[1][1] = m->m[2][2] = cos(angle);
		m->m[1][2] = -sin(angle);
		m->m[2][1] = -m->m[1][2];
		m->m[3][3] = 1.0f;
	}
	static void matrix_rotate_y(matrix_t *m, float angle)
	{
		matrix_set_zero(m);
		m->m[1][1] = 1.0f;
		m->m[0][0] = m->m[2][2] = cos(angle);
		m->m[0][2] = -sin(angle);
		m->m[2][0] = -m->m[0][2];
		m->m[3][3] = 1.0f;
	}
	static void matrix_rotate_z(matrix_t *m, float angle)
	{
		matrix_set_zero(m);
		m->m[2][2] = 1.0f;
		m->m[0][0] = m->m[1][1] = cos(angle);
		m->m[0][1] = -sin(angle);
		m->m[1][0] = -m->m[0][1];
		m->m[3][3] = 1.0f;
	}
	static void matrix_apply(matrix_t *m, point_t *y, const point_t *x)
	{
		float X, Y, Z, W;
		X = x->x * m->m[0][0] + x->y * m->m[1][0] + x->z * m->m[2][0] + x->w * m->m[3][0];
		Y = x->x * m->m[0][1] + x->y * m->m[1][1] + x->z * m->m[2][1] + x->w * m->m[3][1];
		Z = x->x * m->m[0][2] + x->y * m->m[1][2] + x->z * m->m[2][2] + x->w * m->m[3][2];
		W = x->x * m->m[0][3] + x->y * m->m[1][3] + x->z * m->m[2][3] + x->w * m->m[3][3];
		y->x = X; y->y = Y; y->z = Z; y->w = W;
	}
	static void matrix_set_rotation(matrix_t *m, float x, float y, float z, float angle)
	{
		matrix_set_identity(m);                   //先绕x轴旋转至xz平面（atan(y/z)),再绕y轴旋转至z轴（asin(x/sqrt(x^2+y^2+z^2))
		matrix_t Rx, Ry, Rz, iRy, iRx;
		float theta_x, theta_y;
		if (y == 0 && z == 0)theta_x = 0;
		else
		if (y != 0 && z == 0) theta_x = pi / 2;
		else theta_x = atan(y / z);
		if (x == 0 && sqrt(z*z + y*y) == 0)theta_y = 0;
		else
		if (x != 0 && sqrt(z*z + y*y) == 0) theta_y = pi / 2;
		else theta_y = atan(x / sqrt(z*z + y*y));
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
	static void matrix_set_perspective(matrix_t *m, float fov, float aspect, float n, float f)   //采用左手系，从原点看向z正方向，将z从(n,f)投影到(0,1),由于矩阵右乘，需要转置
	{
		float t = tan(fov / 2)*n, r = t*aspect;
		matrix_set_zero(m);
		m->m[0][0] = n / r; m->m[1][1] = n / t; m->m[2][2] = f / (f - n); m->m[3][2] = -1.0f*f*n / (f - n);
		m->m[2][3] = 1.0f;
	}
	static void matrix_set_lookat(matrix_t *m, point_t *eye, vector_t *at, vector_t *up)       //view = [uvw]T * matrix[-e]
	{
		matrix_set_zero(m);
		vector_t xaxis, yaxis, zaxis;
		Vector::vector_sub(&zaxis, at, eye);                          //u
		Vector::vector_normalize(&zaxis, &zaxis);
		Vector::vector_crossproduct(&xaxis, up, &zaxis);               //v
		Vector::vector_normalize(&xaxis, &xaxis);
		Vector::vector_crossproduct(&yaxis, &zaxis, &xaxis);               //w
		Vector::vector_normalize(&yaxis, &yaxis);
		m->m[0][0] = xaxis.x; m->m[1][0] = xaxis.y; m->m[2][0] = xaxis.z; m->m[3][0] = -Vector::vector_dotproduct(&xaxis, eye);
		m->m[0][1] = yaxis.x; m->m[1][1] = yaxis.y; m->m[2][1] = yaxis.z; m->m[3][1] = -Vector::vector_dotproduct(&yaxis, eye);
		m->m[0][2] = zaxis.x; m->m[1][2] = zaxis.y; m->m[2][2] = zaxis.z; m->m[3][2] = -Vector::vector_dotproduct(&zaxis, eye);
		m->m[3][3] = 1.0f;
	}

};
#endif