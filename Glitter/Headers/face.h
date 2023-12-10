#ifndef FACE_H
#define FACE_H
#pragma once

#include <glm/glm.hpp>
#include <cassert>

#define PI 3.14159

struct face{
	unsigned int a, b, c, n, t;
	face(unsigned int aa, unsigned int bb, unsigned int cc, unsigned int nn) :
		a(aa), b(bb), c(cc), n(nn){
		t = -1;
	}

	face(unsigned int aa, unsigned int bb, unsigned int cc, unsigned int nn, unsigned int tt) :
		a(aa), b(bb), c(cc), n(nn), t(tt){
	}

	face(){
		a = 0;
		b = 0;
		c = 0;
		n = 0;
		t = 0;
	}

	const unsigned int& operator[] (const unsigned int& index){
		assert(index < 5);
		switch (index){
		case 0:
			return a;
			break;
		case 1:
			return b;
			break;
		case 2:
			return c;
			break;
		case 3:
			return n;
			break;
		case 4:
			return t;
			break;
		}
	}
};

struct fvec3Comp{
	bool operator()(const glm::fvec3& v1, const glm::fvec3& v2){
		if (v1.x < v2.x){
			return true;
		}
		else if (v1.x > v2.x){
			return false;
		}
		else if (v1.x == v2.x){
			if (v1.y < v2.y)
				return true;
			else if (v1.y > v2.y)
				return false;
			else if (v1.y == v2.y){
				if (v1.z < v2.z)
					return true;
				else if (v1.z > v2.z)
					return false;
				else{
					return v1.z < v2.z;
				}
			}
		}
	}
};

#endif