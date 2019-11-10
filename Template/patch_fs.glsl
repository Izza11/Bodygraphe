#version 450

out vec4 fragcolor;   

layout(location = 1) uniform float time = 0.0;
layout(location = 2) uniform float frame_num = 0.0;
layout(location = 3) uniform sampler2D colorTex;
layout(location = 4) uniform int shader_mode = 0;
layout(location = 5) uniform float params[10];
layout(location = 15) uniform vec4 color[4];


in vec3 normal;  
in vec3 pos;
in vec2 tex_coord;
in float frame_id;

vec4 lighting();
vec4 proc_tex();

vec4 skeleton_reverb(float p1, float p2, float p3, float p4);
vec4 stripes(float p1, float p2, float p3);
vec4 waves(float p1, float p2, float p3);
vec4 circles(float p1, float p2);
vec4 grid(float p1, float p2);
vec4 checkerboard(float p1, float p2);
float quasicrystal(vec2 p);

void main(void)
{   

	if(shader_mode == 0)
	{
		vec2 tc = tex_coord;
		tc.y = 50.01*frame_id;
		tc.y *= 0.1;
		float mod = (1.0-frame_id);
		fragcolor = 2.0*mod*texture2D(colorTex, tc)*lighting();
		fragcolor.a = mod;	
		return;
	}
	else if(shader_mode == 1)
	{
		fragcolor = color[0]*skeleton_reverb(params[1], params[2], params[3], params[4]);
		return;
	}
	else if(shader_mode == 2)
	{
		vec2 tc = tex_coord;
		tc.y = 5.01*frame_id;
		float mod = (1.0-frame_id);
		fragcolor = 2.0*mod*texture2D(colorTex, tc)*lighting()+vec4(1.5*pos.y, 0.0, 0.0, 0.0);
		fragcolor.a = (color[0].a+0.5f)*mod*stripes(params[1], params[2], params[3]).r;
	return;
	}
	else if(shader_mode == 3)
	{
		fragcolor = color[0]*circles(params[1], params[2]);	
		return;
	}
	else if(shader_mode == 4)
	{
		vec4 c = checkerboard(params[1], params[2]);
		fragcolor = mix(color[0], color[1], c.r);
		return;
	}
	else if(shader_mode == 5)
	{
		float q = quasicrystal(100.0*params[1]*vec2(tex_coord.x, (1.0-frame_id)));	
		fragcolor = mix(color[0], color[1], q);
		fragcolor.a *= 1.0-frame_id;
		return;
	}
	else if(shader_mode == 6)
	{
		fragcolor = mix(color[0], color[1], waves(params[1], params[2], params[3]));
		return;
	}
	else if(shader_mode == 7)
	{
		fragcolor = color[0]*grid(params[1], params[2]);
		return;
	}
	else if(shader_mode == 8)
	{
		
	}
	else if(shader_mode == 9)
	{

	}
	else if(shader_mode == 10)
	{
		fragcolor =  mix(color[1], color[0], (pos.y+1.0));
		fragcolor.a *= 1.0-frame_id;
		//fragcolor = vec4(tex_coord, 1.0, 1.0);
		//fragcolor = vec4(vec3(frame_id), 1.0);
		//fragcolor = vec4(normal, 1.0);
		return;
	}


	//debug
	fragcolor.rgb = abs(normalize(normal));
	//fragcolor.rgb = abs(normal);
	fragcolor.a = 1.0;
}


vec4 skeleton_reverb(float p1, float p2, float p3, float p4)
{
	float fg = 0.5*sin(p1*200.0*frame_id) +p2;
	fg = smoothstep(p3, p4, fg);
	return vec4(fg);
}

vec4 stripes(float p1, float p2, float p3)
{
	float fg = 0.5*sin(p1*25.0*tex_coord.x) + 0.5;
	fg = smoothstep(0.45+p2, 0.55+p3, fg);
	return vec4(fg);
}

float func(vec2 x)
{
	return abs(sin(x.x)-x.y);
}

vec2 grad( in vec2 x )
{
    vec2 h = vec2( 0.01, 0.0 );
    return vec2( func(x+h.xy) - func(x-h.xy),
                 func(x+h.yx) - func(x-h.yx) )/(2.0*h.x);
}

vec4 waves(float p1, float p2, float p3)
{
	vec2 x = 200.0*vec2(p1,p2)*(vec2(frame_id, fract(5.0*tex_coord.x))-vec2(0.5));
	float v = func(x);
	vec2  g = grad( x );
    float de = abs(v)/length(g);
    return vec4(smoothstep( 0.19, 0.20, de ));
}


vec4 circles(float p1, float p2)
{
	vec2 c = fract(2.0*vec2(tex_coord.x, 5.0*frame_id))-vec2(0.5);
	float fg = (smoothstep(p1-0.01, p1+0.01, length(c)) - smoothstep(p2+0.04, p2+0.05, length(c)));
	return vec4(fg);
}

vec4 grid(float p1, float p2)
{
	vec2 c = fract(2.0*vec2(tex_coord.x, 5.0*frame_id))-vec2(0.5);
	float fg = (smoothstep(p1-0.01, p1+0.01, abs(c.x)) - smoothstep(p2+0.04, p2+0.05, abs(c.y)));
	return vec4(fg);
}

vec4 checkerboard(float p1, float p2)
{
	vec2 c = fract(15.0*p1*vec2(tex_coord.x, 15.0*p2*frame_id))-vec2(0.5);
	float fg = float(step(0.0, c.x) == step(0.0, c.y));
	return vec4(fg);
}

float wave(float a, vec2 p) 
{
	return (cos(dot(p,vec2(cos(a),sin(a)))) + 1.) / 2.;
}


float wrap(float v) 
{
    float f = fract(v);
    return v-f < 2e-4 ? f : 1. - f;
}

#define PI 3.14159268
#define LAYERS 4.
float quasicrystal(vec2 p) 
{
    float sum = 0.;
    for(float a = 0.; a < PI; a += PI/LAYERS) 
        sum += wave(a, p);

    return wrap(sum);
}



vec4 lighting()
{
	const vec3 l = vec3(1.0, 0.0, 1.0);
	const vec4 amb_color = vec4(0.2, 0.2, 0.2, 1.0)*color[0];
	const vec4 diff_color = vec4(0.5, 0.5, 0.4, 1.0)*color[0];
	const vec4 spec_color = vec4(0.6, 0.6, 0.6, 1.0);

	vec3 n = normalize(normal);
	vec3 eye = vec3(0.0);
	float ndotl = max(0.0, abs(dot(n, l)));
	vec3 v = normalize(eye-pos);
	vec3 r = normalize(reflect(-l, n));

	float rdotv = max(0.0, dot(r, v));

	return amb_color + diff_color*ndotl + spec_color*pow(rdotv, 20.0);
}