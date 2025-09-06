//#include "C:/Wichtig/System/Static/Library/Math.h"
//#include "C:/Wichtig/System/Static/Library/Random.h"
//#include "C:/Wichtig/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/Math.h"
#include "/home/codeleaded/System/Static/Library/Random.h"
#include "/home/codeleaded/System/Static/Library/WindowEngine1.0.h"

#define BulletSpeed     0.35f
#define PlayerMaxSpeed  0.30f

#define SCALE           1200.0f

typedef struct Figure {
    Vec2 p;
    Vec2 v;
    F32 r;
} Figure;

typedef struct Ship {
    Vec2 p;
    Vec2 v;
    F32 r;
    F32 a;
    Vector bullets;
} Ship;

typedef struct Astroid {
    Vec2 p;
    Vec2 v;
    F32 r;
    F32 a;
    Vector points;
} Astroid;

Ship MyShip;
Vector Astroids;
unsigned long long LastSpawn = 0ULL;
double RespawnTime = 1.0;

void Astroid_Ring(Astroid* a){
    Vector_Clear(&a->points);
    for(float ag = 0.0f;ag<2*3.1415f;ag+=0.5f){
        Vec2 p = { cosf(ag) + Random_f64_New() * 0.3f,sinf(ag) + Random_f64_New() * 0.3f };
        p = Vec2_Mulf(p,a->r);
        Vector_Push(&a->points,&p);
    }
}

Astroid Astroid_New(){
    int XorY = Random_i32_Max(2);
    float r = (float)Random_f64_New()*0.05f+0.02f;
    float x = Random_f64_New();
    float y = Random_f64_New();
    if(XorY){
        x = (int)(x + 0.5f);
        x = x>0.5f?x+r:x-r;
    }
    else{
        y = (int)(y + 0.5f);
        y = y>0.5f?y+r:y-r;
    }
    Astroid a = { { x,y },{ (float)Random_f64_New() * 0.01f - 0.005f,(float)Random_f64_New() * 0.01f - 0.005f },r,0.0f,Vector_New(sizeof(Vec2)) };
    Astroid_Ring(&a);
    return a;
}

void Astroid_Create(){
    Astroid a = Astroid_New();
    Vector_Push(&Astroids,&a);
}

char Astroid_Split(Astroid* a,int Index){
    a->r *= 0.5f;
    if(a->r<0.01f){
        Vector_Remove(&Astroids,Index);
        return 0;
    }else{
        Astroid_Ring(a);
    }
    a->v = Vec2_Perp(a->v);
    Astroid na = { a->p,Vec2_Neg(a->v),a->r,a->a,Vector_New(sizeof(Vec2)) };
    Astroid_Ring(&na);
    Vector_Push(&Astroids,&na);
    return 1;
}

void Bullet_Create(Ship* s){
    Vector_Push(&s->bullets,(Figure[]){ {{s->p.x + cosf(s->a - 3.1415f/2) * (s->r+0.01f),s->p.y + sinf(s->a - 3.1415f/2) * (s->r+0.01f)},{cosf(s->a - 3.1415f/2) * BulletSpeed,sinf(s->a - 3.1415f/2) * BulletSpeed},0.0025f} });
}

void Bullet_Render(Figure* b){
    RenderCircleWire(Vec2_Mulf(b->p,SCALE),b->r * SCALE,RED,1.0f);
}

void Astroid_Render(Astroid* a){
    M2x2 rot = M2x2_RotateZ(a->a);
    Vec2 p1 = *(Vec2*)Vector_Get(&a->points,0);
    p1 = M2x2_VecMul(p1,rot);
    p1 = Vec2_Add(p1,a->p);
    Vec2 or = p1;
    
    for(int i = 0;i<a->points.size;i++){
        Vec2 p = *(Vec2*)Vector_Get(&a->points,i);
        p = M2x2_VecMul(p,rot);
        p = Vec2_Add(p,a->p);

        RenderLine(Vec2_Mulf(p1,SCALE),Vec2_Mulf(p,SCALE),YELLOW,1.0f);

        p1 = p;
    }
    
    RenderLine(Vec2_Mulf(p1,SCALE),Vec2_Mulf(or,SCALE),YELLOW,1.0f);
}

void Ship_Render(Ship* s){
    Vec2 p1 = {         0.0f,-s->r * 0.66f };
    Vec2 p2 = { -s->r * 0.4f, s->r * 0.33f };
    Vec2 p3 = {  s->r * 0.4f, s->r * 0.33f };

    M2x2 rot = M2x2_RotateZ(s->a);
    p1 = M2x2_VecMul(p1,rot);
    p2 = M2x2_VecMul(p2,rot);
    p3 = M2x2_VecMul(p3,rot);

    p1 = Vec2_Add(p1,s->p);
    p2 = Vec2_Add(p2,s->p);
    p3 = Vec2_Add(p3,s->p);

    RenderLine(Vec2_Mulf(p1,SCALE),Vec2_Mulf(p2,SCALE),WHITE,1.0f);
    RenderLine(Vec2_Mulf(p2,SCALE),Vec2_Mulf(p3,SCALE),WHITE,1.0f);
    RenderLine(Vec2_Mulf(p3,SCALE),Vec2_Mulf(p1,SCALE),WHITE,1.0f);
}

void Figure_Update(void* f,double ElapsedTime){
    Figure* fig = (Figure*)f;
    fig->p = Vec2_Add(Vec2_Mulf(fig->v,ElapsedTime),fig->p);

    if(fig->p.x+fig->r < 0.0f)       fig->p.x = 1.0f+fig->r;
    if(fig->p.x-fig->r > 1.0f)       fig->p.x = -fig->r;
    if(fig->p.y+fig->r < 0.0f)       fig->p.y = 1.0f+fig->r;
    if(fig->p.y-fig->r > 1.0f)       fig->p.y = -fig->r;
}

void Die(){
    window.Running = 0;
}

void Setup(AlxWindow* w){
    MyShip = (Ship){ {0.2f,0.2f},{0.0f,0.0f},0.05f,0.0f,Vector_New(sizeof(Figure)) };

    Astroids = Vector_New(sizeof(Astroid));
    Astroid_Create();
}
void Update(AlxWindow* w){
    if(Stroke(ALX_KEY_W).DOWN){
        M2x2 rot = M2x2_RotateZ(MyShip.a);
        Vec2 Dir = { 0.0f,-1.0f };
        Dir = M2x2_VecMul(Dir,rot);
        Dir = Vec2_Mulf(Dir,0.3f * w->ElapsedTime);
        MyShip.v = Vec2_Add(Dir,MyShip.v);
    }
    if(Stroke(ALX_KEY_S).DOWN){
        M2x2 rot = M2x2_RotateZ(MyShip.a);
        Vec2 Dir = { 0.0f,1.0f };
        Dir = M2x2_VecMul(Dir,rot);
        Dir = Vec2_Mulf(Dir,0.3f * w->ElapsedTime);
        MyShip.v = Vec2_Add(Dir,MyShip.v);
    }
    if(Stroke(ALX_KEY_A).DOWN){
        MyShip.a -= 1.5f * 3.14f * w->ElapsedTime;
    }
    if(Stroke(ALX_KEY_D).DOWN){
        MyShip.a += 1.5f * 3.14f * w->ElapsedTime;
    }
    if(Stroke(ALX_MOUSE_L).PRESSED){
        Bullet_Create(&MyShip);
    }

    double ElapsedTime = (double)(Time_Nano()-LastSpawn) / 1E9;
    if(ElapsedTime>RespawnTime){
        Astroid_Create();
        LastSpawn = Time_Nano();
    }

    Clear(BLACK);

    for(int i = 0;i<Astroids.size;i++){
        Astroid* a = (Astroid*)Vector_Get(&Astroids,i);
        Figure_Update(a,0.01f);
        Astroid_Render(a);

        if(Vec2_Mag(Vec2_Sub(MyShip.p,a->p)) < (MyShip.r-0.02f)+a->r){
            Die();
        }
        for(int j = 0;j<MyShip.bullets.size;j++){
            Figure* b = (Figure*)Vector_Get(&MyShip.bullets,j);
            if(Vec2_Mag(Vec2_Sub(b->p,a->p)) < (b->r)+a->r){
                Astroid_Split(a,i);
                Vector_Remove(&MyShip.bullets,j);
                break;
            }
        }
    }

    if(Vec2_Mag(MyShip.v)>PlayerMaxSpeed) MyShip.v = Vec2_Mulf(Vec2_Norm(MyShip.v),PlayerMaxSpeed);
    Figure_Update(&MyShip,w->ElapsedTime);
    for(int i = 0;i<MyShip.bullets.size;i++){
        Figure* b = (Figure*)Vector_Get(&MyShip.bullets,i);
        Figure_Update(b,w->ElapsedTime);
        if(Vec2_Mag(Vec2_Sub(b->p,MyShip.p)) < (MyShip.r-0.02f)+b->r){
            Die();
            break;
        }
        Bullet_Render(b);
    }
    Ship_Render(&MyShip);
}
void Delete(AlxWindow* w){
    for(int i = 0;i<Astroids.size;i++){
        Astroid* a = (Astroid*)Vector_Get(&Astroids,i);
        Vector_Free(&a->points);
    }
    Vector_Free(&Astroids);

    Vector_Free(&MyShip.bullets);
}

int main(int argc,const char *argv[]){
    if(Create("Astroids",1200,1200,1,1,Setup,Update,Delete)){
        Start();
    }
    return 0;
}