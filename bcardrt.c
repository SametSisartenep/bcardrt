#include <u.h>
#include <libc.h>

typedef struct { double x, y, z; } vec;

vec v(double x, double y, double z) { return (vec){x,y,z}; }
vec vadd(vec a, vec b) { return v(a.x+b.x,a.y+b.y,a.z+b.z); }
vec vmul(vec u, double s) { return v(u.x*s,u.y*s,u.z*s); }
double vdot(vec a, vec b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
vec vcross(vec a, vec b) { return v(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x); }
vec vnorm(vec u) { return vmul(u,1/sqrt(vdot(u,u))); }

/* aek */
//int G[] = {247570, 280596, 280600, 249748, 18578, 18577, 231184, 16, 16};
/* rgl */
int G[] = {263948, 264338, 262288, 266128, 395408, 346256, 296720, 16, 16};

double R(void) { return (double)rand() / 0x7ffffffd; }

int
TraceRay(vec src, vec dst, double *t, vec *normal)
{
	int m, k, j;
	double p;

	*t = 1e9;
	m = 0;
	p = -src.z / dst.z;
	if(p > 0.01){
		*t = p;
		*normal = v(0,0,1);
		m = 1;
	}

	for (k = 19; k--;)
		for (j = 9; j--;)
			if ((G[j] & 1<<k) != 0) {
				vec p;
				double b, c, q;

				p = vadd(src, v(-k,0,-j - 4));
				b = vdot(p, dst);
				c = vdot(p, p) - 1;
				q = b*b - c;
	
				if(q > 0){
					double s;

					s = -b - sqrt(q);
					if(s > 0.01 && s < *t){
						*t = s;
						*normal = vnorm(vadd(p, vmul(dst, *t)));
						m = 2;
					}
				}
			}
	return m;
}

vec
Sample(vec src, vec dst)
{
	vec normal, intersection, light_dir, half_vec;
	double t, lamb_f, color;
	int match;

	match = TraceRay(src, dst, &t, &normal);
	if(!match)
		return vmul(v(0.7,0.6,1), pow(1 - dst.z, 4));

	intersection = vadd(src, vmul(dst, t));
	light_dir = vnorm(vadd(v(9 + R(),9 + R(),16), vmul(intersection, -1)));
	half_vec = vadd(dst, vmul(normal, vdot(normal, dst) * -2));

	lamb_f = vdot(light_dir, normal);

	if(lamb_f < 0 || TraceRay(intersection, light_dir, &t, &normal))
		lamb_f = 0;

	color = pow(vdot(light_dir, half_vec) * (lamb_f > 0), 99);

	if((match & 1) != 0){
		intersection = vmul(intersection, 0.2);
		return ((int)(ceil(intersection.x) + ceil(intersection.y)) & 1) != 0? v(3,1,1): vmul(v(3,3,3), (lamb_f * 0.2 + 0.1));
	}

	return vadd(v(color,color,color), vmul(Sample(intersection, half_vec), 0.5));
}

void
main()
{
	vec cam_forward, cam_up, cam_right, c, color, delta;
	int x, y, r;

	print("P6 512 512 255 ");

	cam_forward = vnorm(v(-6,-16,0));
	cam_up = vmul(vnorm(vcross(v(0,0,1), cam_forward)), 0.002);
	cam_right = vmul(vnorm(vcross(cam_forward, cam_up)), 0.002);
	c = vadd(vmul(vadd(cam_up, cam_right), -256), cam_forward);

	for(y = 512; y--;)
		for(x = 512; x--;){
			color = v(13,13,13);
			for(r = 64; r--;){
				delta = vadd(vmul(vmul(cam_up, R() - 0.5), 99), vmul(vmul(cam_right, R() - 0.5), 99));
				color = vadd(vmul(Sample(vadd(v(17,16,8), delta), vnorm(vmul(vadd(vmul(delta, -1), vadd(vadd(vmul(cam_up, R()+x), vmul(cam_right, R()+y)), c)), 16))), 3.5), color);
			}
			print("%c%c%c", (int)color.x, (int)color.y, (int)color.z);
		}
	exits(nil);
}
