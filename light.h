#ifndef LIGHT_H
#define LIGHT_H
#include"mathlib.h"
typedef  unsigned int UINT;
#define MAX_LIGHTS					8
#define MAX_MATERIALS				8
#define LIGHT_ATTR_AMBIENT			0x0001
#define LIGHT_ATTR_INFINITE			0x0002
#define LIGHT_ATTR_POINT			0x0004
#define LIGHT_ATTR_SPOTLIGHT1		0x0008
#define LIGHT_ATTR_SPOTLIGHT2		0x00016

#define LIGHT_STATE_ON				1
#define LIGHT_STATE_OFF				0

typedef struct
{
	union 
	{
		int rgba;
		UINT M[4];
		struct {UINT r, g, b, a;};
	};
}RGBAV1, *RGBAV1_PTR;
typedef struct
{
	int state;
	int id;
	char name[64];
	int attr;

	RGBAV1 color;
	float ka, kd, ks, power;
	RGBAV1 ra, rd, rs;

	char texture_file[80];
}MATV1,*MATV1_PTR;

typedef struct
{
	int state;
	int id;
	int attr;

	RGBAV1 c_ambient;
	RGBAV1 c_diffuse;
	RGBAV1 c_specular;
	point_t pos;
	vector_t dir;
	float kc, kl, kq;
	float spot_inner;
	float spot_outer;
	float pf;
}LIGHTV1,*LIGHTV1_PTR;

class Light
{
public:
	int Init_Light_LIGHTV1(
		int,                       //光源id
		int,                       //光源状态
		int,                       //光源类型
		RGBAV1,                    //环境光强度
		RGBAV1,                    //散射光
		RGBAV1,                    //镜面反射
		point_t*,                  //位置
		vector_t*,                 //方向
		float,                     //衰减因子 kc,kl,kq
		float,
		float,
		float,                     //内锥角
		float,                     //外锥角
		float);                    //聚光灯指数因子
	int Light_Renderer_vertex(
		vertex_t*,
		vector_t*,
		point_t*);
	int Light_Renderer(
		vertex_t*,
		vertex_t*,
		vertex_t*,
		point_t*);
private:
	MATV1 materials[MAX_MATERIALS];
	int num_materials;

	LIGHTV1 lights[MAX_LIGHTS];
	int num_lights;
};
extern Light light;
#endif