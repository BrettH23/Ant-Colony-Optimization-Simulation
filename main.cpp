#include <string.h>



#include <GL/glut.h>


#include <GL/gl.h>

#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>

#include <SOIL.h>

#define PI 3.14159

#include <starsystems.h>

using namespace std;

bool WireFrame= false;
float i =0;
const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

    float xpos =0;
    float ypos =0;
    float Wwidth,Wheight;

    int rotateX = 0;
    int rotateY = 0;

    GLuint tex1;
    GLuint gTex;
    GLuint *skyTex;
    GLuint planeTex;

    float xMouse, yMouse;
    float t = 0;

    float depth = 300;

    GLdouble realMX, realMY, realMZ;

    bool shoot = false;
    clock_t oldTime;
    clock_t idleTime;
    clock_t bigIdle;
    clock_t pain;

    float dirX = 1.5, dirY = 0;

    float xMax, yMax, xMin, yMin;
    int frames;

    float tX, tY, tZ;

    float funnyrad = 0;
    float prevx=0, prevy=0;
    float spin;

    starsystems* universe;
    bool modeP = true;
    bool showBrute = false;
    bool bruteDone = false;
    int selectedBest = 0;
    int antWalks = 0;
    GLuint fontTex;
    char* iterationString;
/* GLUT callback Handlers */

void TLoad(char* fileName, GLuint &tex){
    int width, height;
    unsigned char* image;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D,tex);

    image = SOIL_load_image(fileName,&width,&height,0,SOIL_LOAD_RGBA); //soil loads the image here
    if(!image){
        cout<< "Image not found."<< endl;
    }

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,image);

    SOIL_free_image_data(image);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);


}
float oneOverHex = 1.0/16.0;
void drawLine(string str, float startX, float startY, float startZ, float scale, float maxLength)
{
    float scale_t = scale*0.5;
    float posX = startX;
    float posY = startY;

    for(int i = 0; i < str.length()&& i < maxLength; i++){

        int charAsInt = int(str[i]);
        float xCoord = oneOverHex*float(charAsInt%16);
        float yCoord = oneOverHex*float(charAsInt/16);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glPushMatrix();
        glBindTexture(GL_TEXTURE_2D,fontTex);


        glBegin(GL_QUADS);

            glTexCoord2f(xCoord,yCoord+oneOverHex);
            glVertex3f(posX-scale_t, posY-scale_t, startZ);

            glTexCoord2f(xCoord+oneOverHex,yCoord+oneOverHex);
            glVertex3f(posX+scale_t, posY-scale_t, startZ);

            glTexCoord2f(xCoord+oneOverHex,yCoord);
            glVertex3f(posX+scale_t, posY+scale_t, startZ);

            glTexCoord2f(xCoord,yCoord);
            glVertex3f(posX-scale_t, posY+scale_t, startZ);
        glEnd();
        glPopMatrix();
        posX += scale;
    }
}

static void resize(int width, int height)
{
    double Ratio;

    Wwidth = (float)width;
    Wheight = (float)height;

    Ratio= (double)width /(double)height;

    glViewport(0,0,(GLsizei) width,(GLsizei) height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective (45.0f,Ratio,0.1f, depth);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

 }



static void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0,0,10,0.0,0.0,0.0,0.0,1.0,100.0);;

    if(WireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		//Draw Our Mesh In Wireframe Mesh
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);		//Toggle WIRE FRAME


    glDisable(GL_LIGHTING);
    glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        string str = to_string(antWalks);
        drawLine(str, -7.5, 5.5, -5, 0.8, 50);
        str = to_string(universe->bestList[0]->pathLength);
        drawLine(str, -7.5, 5.5-0.8, -5, 0.8, 5);
        glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glPushMatrix();


        glRotated(rotateY,1,0,0);
        glRotated(rotateX,0,1,0);

        //spawnBox(100.0);


        glPointSize(6.0);
        glColor4f(1.0f,1.0f,1.0f, 1.0f);
        glBegin(GL_POINTS);

        for(int i = 0;i < universe->tNodes;i++){

            glVertex3f(universe->points[i].x,universe->points[i].y,universe->points[i].z);
        }
        glEnd();

        if(modeP){
            for(int i = 0;i < universe->tNodes; i++){
                for(int j = i+1; j < universe->tNodes; j++){
                    float alph = universe->paths[i][j].pOld;
                    //glLineWidth(universe->paths[i][j].pFresh);
                    glColor4f(1.0,1 - sqrt(alph),0.0,sqrt(alph+0.03)-0.07);
                    glBegin(GL_LINES);
                        glVertex3f(universe->points[i].x,universe->points[i].y,universe->points[i].z);
                        glVertex3f(universe->points[j].x,universe->points[j].y,universe->points[j].z);
                    glEnd();
                    //glColor4f(1.0,1.0,1.0, 1.0);
                }
            }
        }else{
            glColor4f(0.0,0.0,1.0,1.0);

            for(int i = 0;i < universe->tNodes; i++){
                int best1, best2;

                if(i == 0){
                    best1 = universe->bestList[selectedBest]->steps[universe->tNodes-1];
                }else{
                    best1 = universe->bestList[selectedBest]->steps[i-1];
                }
                best2 = universe->bestList[selectedBest]->steps[i];

                glBegin(GL_LINES);
                    glVertex3f(universe->points[best1].x,universe->points[best1].y,universe->points[best1].z);
                    glVertex3f(universe->points[best2].x,universe->points[best2].y,universe->points[best2].z);
                glEnd();

            }
        }
        if(showBrute){
            //std::cout << "hereiam" << std::endl;
            glColor4f(0.0,1.0,0.0,1.0);
            glLineWidth(3.0);
            for(int i = 0;i < universe->tNodes; i++){
                int best1, best2;

                if(i == 0){
                    best1 = universe->bruteBest.steps[universe->tNodes-1];
                }else{
                    best1 = universe->bruteBest.steps[i-1];
                }
                best2 = universe->bruteBest.steps[i];

                glBegin(GL_LINES);
                    glVertex3f(universe->points[best1].x,universe->points[best1].y,universe->points[best1].z);
                    glVertex3f(universe->points[best2].x,universe->points[best2].y,universe->points[best2].z);
                glEnd();

            }
            glLineWidth(1.0);
        }



        glColor4f(1.0,1.0,1.0, 1.0);



    glPopMatrix();
    glEnable(GL_LIGHTING);

    glutSwapBuffers();
}




static void key(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27 :
        case 'q':
            exit(0);
            break;
        case ' ':
            modeP = true;
            break;
        case '0':
            selectedBest = 9;
            modeP = false;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            selectedBest = int(key)-49;
            modeP = false;
            break;

        case 'b':
            if(!bruteDone){
                //universe->bruteForce(); //ain't no way this is getting finished before the heat death of the universe
                //bruteDone = true;

            }
            break;
        case 'c':
            if(bruteDone){
                showBrute = !showBrute;
                //
            }

            break;
        case 'w':
            WireFrame =!WireFrame;
            break;
    }
}

void Specialkeys(int key, int x, int y)
{
    switch(key)
    {
    case GLUT_KEY_UP:
        ypos+=1;

    break;

    case GLUT_KEY_DOWN:
        ypos-=1;

    break;

    case GLUT_KEY_RIGHT:
        xpos+=1;

    break;

    case GLUT_KEY_LEFT:
        xpos-=1;

    break;

   }
  glutPostRedisplay();
}


static void idle(void)
{
    // Use parametric equation with t increment for xpos and y pos
    // Don't need a loop
    if(antWalks < 1000){
        universe->walkAnts();
        antWalks++;
    }
    if(clock()-bigIdle>1000){
        //dirX = (rand() % 3) -1;
        if(xpos>50){
            dirX = -1.5;
        }else if(xpos<-50){
            dirX=1.5;
        }
        dirY = (rand() % 3) -1;

        xMax+=(1.0/(float)frames);
        xMin+=(1.0/(float)frames);

        bigIdle = clock();

    }
    /*
    if(ypos>=2.5){
        dirY=-1;
    }
    else if(ypos<=-2.5){
        dirY=1;
    }
    if(clock()-idleTime>10){
        xpos += (dirX*0.15);
        ypos += (dirY*0.03);
        idleTime = clock();
    }
*/
    if(clock()-idleTime>10){
        tX = prevx*cos(2.0*PI/180)-prevy*sin(2.0*PI/180);
        tZ = prevy*cos(2.0*PI/180)+prevx*sin(2.0*PI/180);
        tY = sin(spin*PI/180);
        prevx = tX;
        prevy = tZ;
        spin+=0.5;
        idleTime = clock();

        //funnyrad>4?dir=-1:(funnyrad<0?dir=1:NULL);
        //funnyrad+=dir*0.01;

        //funnyrad = float((50000 - (clock()%100000))/50000);
        //printf("%f", funnyrad);
    }



    glutPostRedisplay();
}

void mouseMove(int x, int y)
{
    static float prev_x=0.0;
    static float prev_y=0.0;

    prev_x = (float)x-prev_x;
    prev_y = (float)y-prev_y;

    if(abs((int)prev_x)>15|abs((int)prev_y)>15){
        prev_x = (float)x;
        prev_y = (float)y;
        return;
    }
    rotateX = (rotateX+(int)prev_x)%360;
    rotateY = (rotateY+(int)prev_y)%360;
}

void getRealMouse(int x, int y){
    GLint viewPort[4];
    GLdouble modelView[16];
    GLdouble projectionMat[16];
    GLfloat winX, winY, winZ;

    glGetDoublev(GL_PROJECTION_MATRIX, projectionMat);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
    glGetIntegerv(GL_VIEWPORT, viewPort);



    winX = (GLfloat)x;
    winY = (GLfloat)viewPort[3] - (GLfloat)y;
    glReadPixels(x,int(winY),1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&winZ);

    gluUnProject(winX, winY, winZ, modelView, projectionMat, viewPort, &realMX, &realMY, &realMZ);

    //std::cout << "real X: " << realMX << ", real Y: " << realMY << ", real Z: " << realMZ << std::endl;

}

void mouse(int btn, int state, int x, int y){

    float scale = 55*(Wwidth/Wheight);


    switch(btn){
        case GLUT_LEFT_BUTTON:

        if(state==GLUT_DOWN){
            //xMouse = (float)(x-Wwidth/2)/scale;
            //yMouse = (float)(Wheight/2-y)/scale;
            getRealMouse(x,y);


            shoot = true;
            oldTime = clock();
            t=0;
            //std::cout << "Actual X: " << x << ", Actual Y: " << y << std::endl;
            //std::cout << "Mouse x is " << xMouse << ", Mouse y is "<< yMouse << std::endl;
               // get new mouse coordinates for x,y
               // use scale to match right
        }
        break;
    }
     glutPostRedisplay();
};



static void init(void)
{
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);                 // assign a color you like

    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_DEPTH_TEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);



    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    //glutSetCursor(GLUT_CURSOR_CROSSHAIR);
    oldTime = clock();
    idleTime = clock();
    bigIdle = clock();

    universe = new starsystems();
    int seed = int(time(NULL));
    seed = 205; //205, 452, 10895, 20001
    universe->genRandom(50, seed);   //

    universe->mode = universe->ASRANK;
    universe->asRankPercentile = 15;
    universe->alwaysExploit = false;

    //universe->myBonus = true;
    universe->rollOrigins = true;
    universe->bonusDecay = 0.8;
    universe->bonusMultiplier = 3.0;
    universe->addRate = 0.2;

    universe->pherAppRate = 0.045;

    universe->a_param = 0.85;
    universe->b_param = 1.0;

    //universe->walkAnts();

    TLoad("images/bombadier.png", fontTex);

    //std::cout <<"opengl ver "<< glGetString(GL_VERSION) << std::endl;
    //std::cout << "rand: " << float(((rand() << 15) + (rand())))/float(((RAND_MAX << 15) + (RAND_MAX))) << std::endl;
}

void testLoop(){
    //for(int i = 0; i < 10; i++){

    //}

}

/* Program entry point */

int main(int argc, char *argv[])
{

    glutInit(&argc, argv);

    glutInitWindowSize(1200,900);
    glutInitWindowPosition(0,0);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Space Ant Farm");
    init();
    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMove);
    glutKeyboardFunc(key);
    glutSpecialFunc(Specialkeys);

    glutIdleFunc(idle);

    glutMainLoop();



    return EXIT_SUCCESS;
}
