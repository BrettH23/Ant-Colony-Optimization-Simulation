#ifndef STARSYSTEMS_H
#define STARSYSTEMS_H

#include <iostream>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

typedef struct{
    float x, y, z;
    //int* connections;
}point;

typedef struct{
    int first, second;
}vec2order;

typedef struct{
    float length;
    float pOld, pFresh;
}path;

typedef struct ant{
    int progress = 0;
    int current = 0;
    int* steps;
    float distanceTraveled = 0;
    ~ant(){
        delete steps;
        steps = nullptr;
    }
}ant;

typedef struct lList{
    int id;
    lList* next = nullptr;
};

typedef struct best{
    float pathLength;
    int* steps;
    best* next = nullptr;
    ~best(){
        delete steps;
        steps = nullptr;
    }
};

class starsystems
{
    public:
        starsystems();
        virtual ~starsystems();

        enum algos{AS, ASRANK};
        algos mode;

        int numAnts;
        int tNodes;
        float bounds;
        int iterations;
        int itMod;
        bool generated;

        float a_param, b_param;

        point* points;
        path** paths;
        ant* ants;
        best* bestPaths;
        best* bestList[10];
        bool checkIdentical(ant&);
        float asRankPercentile;

        float worstBest;
        int totalBest;
        int rollingOrigin;

        float addRate;
        float pherAppRate;
        float decayMinimum;

        bool myBonus = false;
        bool rollOrigins = false;
        bool alwaysExploit = true;
        float bonusDecay;
        float bonusMultiplier;

        vec2order getOrder(int, int);

        void genRandom(int, int);
        void walkAnts();
        int weightedRandom(ant&);
        void updatePaths();
        void recordGoodPath(ant&);

        void bruteForce();
        best bruteLoop(std::string, int);
        best bruteBest;

    protected:

    private:
        float randomCoord();
};

#endif // STARSYSTEMS_H
