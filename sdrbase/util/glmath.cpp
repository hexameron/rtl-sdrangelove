#include <QGLWidget>
#include <QMatrix4x4>
#include <math.h>

static QMatrix4x4 mv[20];
static int pos=0;
static std::vector<QMatrix4x4*> stack;

void initeglcompat() {
	mv[0].setToIdentity();
};

void glPushMatrix(){
	pos++;
	mv[pos] = mv[pos-1];
}

void glPopMatrix(){
	if(pos >0){
		pos--;
	}
}
void glScalef(float x, float y, float z){
	mv[pos].scale(x,y,z);
}
void glTranslatef(float x, float y, float z){
	mv[pos].translate(x,y,z);
}
QMatrix4x4 getMV() {
	return mv[pos];
}

