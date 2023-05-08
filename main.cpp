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

float Wwidth,Wheight;

int rotateX = 0;
int rotateY = 0;

float depth = 20;

starsystems* universe;
bool modeP = true;
bool showBrute = false;
bool bruteDone = false;
int selectedBest = 0;
int antWalks = 0;
GLuint fontTex;
char* iterationString;
float oneOverHex = 1.0/16.0;

clock_t startTime;
clock_t endTime;
bool tickTimer = true;

void TLoad(char* fileName, GLuint &tex){
    int width, height;
    unsigned char* image;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D,tex);

    image = SOIL_load_image(fileName,&width,&height,0,SOIL_LOAD_RGBA); //soil loads the image here
    if(!image){
        std::cout<< "Image not found."<< std::endl;
    }

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,image);

    SOIL_free_image_data(image);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);


}

void drawLine(std::string str, float startX, float startY, float startZ, float scale, float maxLength)
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

    glDisable(GL_LIGHTING);
    glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        std::string str = std::to_string(antWalks);
        drawLine(str, -7.5, 5.5, -5, 0.8, 50);
        str = std::to_string(universe->bestList[0]->pathLength);
        drawLine(str, -7.5, 5.5-0.8, -5, 0.8, 6);
        str = std::to_string(int(1000*float(endTime - startTime)/float(CLOCKS_PER_SEC)));
        drawLine(str + "ms", -7.5, 5.5-1.6, -5, 0.8, 15);
        glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glPushMatrix();
    if(true){

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


    }
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
    }
}

void Specialkeys(int key, int x, int y)
{
    switch(key)
    {
    case GLUT_KEY_UP:

    break;

    case GLUT_KEY_DOWN:

    break;

    case GLUT_KEY_RIGHT:

    break;

    case GLUT_KEY_LEFT:

    break;

   }
  glutPostRedisplay();
}


static void idle(void)
{
    if(antWalks < 1000){
        universe->walkAnts();
        antWalks++;
    }else{
        tickTimer = false;
    }

    if(tickTimer){
        endTime = clock();
    }


    glutPostRedisplay();
}

void mouseMove(int x, int y)
{
    static float prev_x=0.0;
    static float prev_y=0.0;

    prev_x = (float)x-prev_x;
    prev_y = (float)y-prev_y;

    if(abs((int)prev_x)>20|abs((int)prev_y)>20){
        prev_x = (float)x;
        prev_y = (float)y;
        return;
    }
    rotateX = (rotateX+(int)prev_x)%360;
    rotateY = (rotateY+(int)prev_y)%360;
}



void mouse(int btn, int state, int x, int y){

    float scale = 55*(Wwidth/Wheight);


    switch(btn){
        case GLUT_LEFT_BUTTON:

        if(state==GLUT_DOWN){

        }
        break;
    }
     glutPostRedisplay();
};



static void init(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    universe = new starsystems();
    int seed = int(time(NULL));
    seed = 205; //205, 452, 10895, 20001
    universe->genRandom(50, seed);   //

    universe->asRankPercentile = 10;

    universe->mode = universe->ASRANK;
    universe->alwaysExploit = false;
    //universe->myBonus = true;

    universe->rollOrigins = true;
    universe->bonusDecay = 0.8;
    universe->bonusMultiplier = 3.0;
    universe->addRate = 0.2;

    universe->pherAppRate = 0.045;

    universe->a_param = 0.85;
    universe->b_param = 1.0;

    universe->forgetLength = 5;
    universe->forgetOffset = 93;

    TLoad("images/bombadier.png", fontTex);

    startTime = clock();
    endTime = clock();
}

int main(int argc, char *argv[])
{

    glutInit(&argc, argv);

    glutInitWindowSize(900,700);
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
