#include"light.h"
#include<cstdio>
Light light;
int Light::Init_Light_LIGHTV1(
	int				index,
	int				_state,
	int				_attr,
	RGBAV1			_c_ambient,
	RGBAV1			_c_diffuse,
	RGBAV1			_c_specular,
	point_t*		_pos,
	vector_t*		_dir,
	float			_kc,
	float			_kl,
	float			_kq,
	float			_spot_inner,
	float			_spot_outer,
	float			_pf)
{
	if (index < 0 || index >= MAX_LIGHTS)
		return 0;
	lights[index].state = _state;
	lights[index].id = index;
	lights[index].attr = _attr;
	lights[index].c_ambient = _c_ambient;
	lights[index].c_diffuse = _c_diffuse;
	lights[index].c_specular = _c_specular;
	lights[index].kc = _kc;
	lights[index].kl = _kl;
	lights[index].kq = _kq;

	if (_pos) Vector::vector_copy(&lights[index].pos, _pos);
	if (_dir){
		Vector::vector_copy(&lights[index].dir, _dir);
		Vector::vector_normalize(&lights[index].dir, &lights[index].dir);
	}
	lights[index].spot_inner = _spot_inner;
	lights[index].spot_outer = _spot_outer;
	lights[index].pf = _pf;
	num_lights = 8;
	return index;
}

int Light::Light_Renderer_vertex(vertex_t* v,vector_t* n,point_t* eye)
{
	float r_base = v->base_color.r, g_base = v->base_color.g, b_base = v->base_color.b,
		r_sum = 0, g_sum = 0, b_sum = 0,
		shaded_color = 0;
	float dp, dist, i, nl, atten;

	for (int curr_light = 0; curr_light < num_lights; ++curr_light)
	{
		if (!lights[curr_light].state) continue;
		int ATTR = lights[curr_light].attr;
		if (ATTR == LIGHT_ATTR_AMBIENT)
		{
			r_sum += lights[curr_light].c_ambient.r*r_base;
			g_sum += lights[curr_light].c_ambient.g*g_base;
			b_sum += lights[curr_light].c_ambient.b*b_base;
		}
		if (ATTR == LIGHT_ATTR_INFINITE)
		{
			nl = Vector::vector_length(n);
			dp = Vector::vector_dotproduct(n, &lights[curr_light].dir);
			if (dp > 0)
			{
				i = dp / (nl);
				r_sum += lights[curr_light].c_diffuse.r*r_base*i;
				g_sum += lights[curr_light].c_diffuse.g*g_base*i;
				b_sum += lights[curr_light].c_diffuse.b*b_base*i;
			}
		}
		if (ATTR == LIGHT_ATTR_POINT)
		{
			nl = Vector::vector_length(n);
			vector_t l;
			Vector::vector_sub(&l, &v->pos, &lights[curr_light].pos);
			dist = Vector::vector_length(&l);
			dp = Vector::vector_dotproduct(n, &l);
			if (dp > 0)
			{
				atten = (lights[curr_light].kc + lights[curr_light].kl * dist + lights[curr_light].kq *dist *dist);
				i = dp / (nl*dist*atten);
				r_sum += lights[curr_light].c_diffuse.r*r_base*i;
				g_sum += lights[curr_light].c_diffuse.g*g_base*i;
				b_sum += lights[curr_light].c_diffuse.b*b_base*i;
			}
		}
		if (ATTR == LIGHT_ATTR_SPOTLIGHT1)
		{
			nl = Vector::vector_length(n);
			vector_t l;
			Vector::vector_sub(&l, &v->pos, &lights[curr_light].pos);
			dist = Vector::vector_length(&l);
			dp = Vector::vector_dotproduct(n, &lights[curr_light].dir);
			if (dp > 0)
			{
				atten = (lights[curr_light].kc + lights[curr_light].kl * dist + lights[curr_light].kq *dist *dist);
				i = dp / (nl*atten);
				r_sum += lights[curr_light].c_diffuse.r*r_base*i;
				g_sum += lights[curr_light].c_diffuse.g*g_base*i;
				b_sum += lights[curr_light].c_diffuse.b*b_base*i;
			}
		}
		if (ATTR == LIGHT_ATTR_SPOTLIGHT2)
		{
			nl = Vector::vector_length(n);
			dp = Vector::vector_dotproduct(n, &lights[curr_light].dir);
			if (dp > 0)
			{
				vector_t l;
				Vector::vector_sub(&l, &lights[curr_light].pos ,&v->pos );
				dist = Vector::vector_length(&l);
				float dpsl = Vector::vector_dotproduct(&l, &lights[curr_light].dir) / dist;
				if (dpsl > 0)
				{
					atten = (lights[curr_light].kc + lights[curr_light].kl * dist + lights[curr_light].kq *dist *dist);
					float dpsl_exp = dpsl;
					for (int k = 1; k < (int)lights[curr_light].pf; ++k)
					{
						dpsl_exp *= dpsl;
					}
					i = dp * dpsl_exp / (nl*atten);
					r_sum += lights[curr_light].c_diffuse.r*r_base*i;
					g_sum += lights[curr_light].c_diffuse.g*g_base*i;
					b_sum += lights[curr_light].c_diffuse.b*b_base*i;
				}
			}
		}
	}
	r_sum = (r_sum > 255) ? 255 : r_sum;
	g_sum = (g_sum > 255) ? 255 : g_sum;
	b_sum = (b_sum > 255) ? 255 : b_sum;
	v->light_color.r = r_sum/255.0f;
	v->light_color.g = g_sum / 255.0f;
	v->light_color.b = b_sum / 255.0f;
	return 1;
}
int Light::Light_Renderer(vertex_t* a, vertex_t* b, vertex_t* c, point_t* eye)
{
	vector_t n, u, v;
	Vector::vector_sub(&u, &b->pos, &a->pos);
	Vector::vector_sub(&v, &c->pos, &b->pos);
	Vector::vector_crossproduct(&n, &v, &u);
	Light_Renderer_vertex(a,&n, eye);
	Light_Renderer_vertex(b,&n, eye);
	Light_Renderer_vertex(c,&n, eye);
	return 1;
}