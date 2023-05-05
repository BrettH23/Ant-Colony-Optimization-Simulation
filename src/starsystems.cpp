#include "starsystems.h"

starsystems::starsystems()
{
    generated = false;
}

starsystems::~starsystems()
{
    if(generated){
        best* temp;
    delete[] bestList;
    bestPaths = nullptr;
    for(int i = 0; i < tNodes; i++){
        delete[] paths[i];
    }
    delete[] paths;
    delete[] points;
    delete[] ants;
    }
}

/*
void starsystems::draw()
{

    glBegin(GL_POINTS);
    for(int i = 0;i < tNodes; i++){
        glVertex3f(points[i].x,points[i].y,points[i].z);
    }
    glEnd();
}
*/
void starsystems::genRandom(int t, int seed)
{
    tNodes = t;
    numAnts = 50;

    pherAppRate = .05;
    addRate = 0.2;
    decayMinimum = 0.8;
    mode = AS;
    a_param = 0.85;
    b_param = 1.0;

    bonusDecay = 0.9;
    bonusMultiplier = 5.0;

    asRankPercentile = 10;
    iterations = 0;
    itMod = 1;
    rollingOrigin = 0;

    bounds = 3.0;
    worstBest = 2*bounds*sqrt(3)*tNodes + 1.0;    //maximum possible length + 1 (unrealistic that any path will even reach 50% of this)
    totalBest = 0;
    //std::cout << worstBest << std::endl;

    //tLines = (t-1)*(t*0.5);
    points = new point[tNodes];
    paths = new path*[tNodes];
    ants = new ant[numAnts];

    bestPaths = new best;
    bestPaths->pathLength = worstBest;
    for(int i = 0; i < 10; i++){
        bestList[i] = nullptr;
    }


    for(int i = 0; i < tNodes; i++){
        paths[i] = new path[tNodes];
    }
    int usedLines = 0;
    for(int i = 0; i < numAnts; i++){
        ants[i].steps = new int[tNodes];
    }

    srand(seed);
/*
    for(int i = 0;i < tNodes; i++){
        points[i].connections = new int[tNodes-1];
    }*/
    for(int i = 0;i < tNodes; i++){
        points[i].x = randomCoord();
        points[i].y = randomCoord();
        points[i].z = randomCoord();
    }

    for(int i = 0;i < tNodes; i++){
        for(int j = i+1; j < tNodes; j++){
            float x_d = points[i].x - points[j].x;
            float y_d = points[i].y - points[j].y;
            float z_d = points[i].z - points[j].z;

            paths[i][j].length = sqrt(x_d*x_d + y_d*y_d + z_d*z_d);
            paths[i][j].pFresh = 0.0;
            paths[i][j].pOld = 0.0;
        }
    }




    generated = true;
    std::cout << "Universe Generated" << std::endl;
}
float starsystems::randomCoord()
{
    return bounds*((2*float(rand())/float(RAND_MAX))-1);
}

void starsystems::walkAnts()
{
    if(rollOrigins)rollingOrigin = (tNodes + iterations) %tNodes;
    //std::cout << rollingOrigin << std::endl;
    for(int i = 0;i < numAnts; i++){
        ants[i].distanceTraveled = 0.0;
        int j = 0;
        while(ants[i].progress < tNodes){
            int selectedPath = weightedRandom(ants[i]);
            if(selectedPath >= ants[i].current){
                //selectedPath++;
            }

            vec2order vec = getOrder(selectedPath, ants[i].current);

            //paths[first][second].pFresh += (pRate+ 0.08/paths[first][second].length);
            //std::cout << ants[i].progress << ":" << ants[i].current << " ";
            ants[i].steps[ants[i].progress] = ants[i].current;
            //std::cout << ants[i].current << " ";
            ants[i].progress++;
            ants[i].current = selectedPath;
            ants[i].distanceTraveled+= paths[vec.first][vec.second].length;
            //std::cout << j << " hereiam" << std::endl;
            //
            j++;

        }//std::cout << std::endl;
        //for(int j = 0;j < ants[i].progress; j++){
        //  std::cout << "visited node " << ants[i].steps[j] << std::endl;
        //}

        ants[i].progress = 0;
        ants[i].current = 0;
        //std::cout << "distance: " << ants[i].distanceTraveled << std::endl;
    }
    iterations++;
    updatePaths();
    if(false&& iterations% 500 == 0){
        if(iterations > 499){
            a_param *= 0.5;
            b_param *= 0.5;
        }

    }
}

void starsystems::updatePaths()
{
    switch(mode){
    case AS:
        {

            for(int i = 0; i < numAnts; i++){
                int currentNode = ants[i].steps[tNodes-1];
                for(int j = 0;j < tNodes; j++){
                    vec2order vec = getOrder(currentNode , ants[i].steps[j]);
                    //std::cout << i << ", " << ants[i].steps[j] << std::endl;
                    paths[vec.first][vec.second].pFresh += pherAppRate; //(pRate* 3/paths[first][second].length);
                    currentNode = ants[i].steps[j];
                }
                recordGoodPath(ants[i]);

            }
        }

        break;
    case ASRANK:
        {
           int bestMax = numAnts / asRankPercentile;
            int bestAnts[bestMax];
            float fitness[bestMax];
            for(int i = 0; i < bestMax; i++){
                fitness[i] = 0;
            }

            for(int i = 0; i < numAnts; i++){
                bool done = false;
                for(int j = 0; j < bestMax; j++){
                    float antonyFitness = 1/ants[i].distanceTraveled;
                    if(fitness[j] < antonyFitness){
                        float tempFit = fitness[j];
                        float tempAnt = bestAnts[j];
                        fitness[j] = antonyFitness;
                        bestAnts[j] = i;
                        j++;
                        while(j < bestMax){
                            float tempFit2 = fitness[j];
                            float tempAnt2 = bestAnts[j];
                            fitness[j] = tempFit;
                            bestAnts[j] = tempAnt;
                            tempFit = tempFit2;
                            tempAnt = tempAnt2;
                            j++;
                        }

                    }
                }
            }
            for(int z = 0; z < bestMax; z++){
                int i = bestAnts[z];
                int currentNode = ants[i].steps[tNodes-1];
                for(int j = 0;j < tNodes; j++){
                    vec2order vec = getOrder(currentNode , ants[i].steps[j]);
                    //std::cout << i << ", " << ants[i].steps[j] << std::endl;
                    paths[vec.first][vec.second].pFresh += pherAppRate*(asRankPercentile-asRankPercentile*(z/bestMax)); //(pRate* 3/paths[first][second].length);
                    currentNode = ants[i].steps[j];
                }
                recordGoodPath(ants[i]);


            }
        }

        break;
    }

    if(alwaysExploit || ((iterations + 100)%100 >5-(float(iterations)/1000.0)*3)){


        for(int i = 0;i < tNodes; i++){
            for(int j = i+1; j < tNodes; j++){
                float thisDecay = (1-addRate);
                if(paths[i][j].pOld > 1.0){
                    //thisDecay *= (3/4) + 1/(4 * paths[i][j].pOld);
                }

                paths[i][j].pOld *= thisDecay;
                //paths[i][j].pOld *= (1-addRate);
                paths[i][j].pOld += addRate * paths[i][j].pFresh;
                paths[i][j].pFresh = 0;


                if(paths[i][j].pOld > 1){
                    //paths[i][j].pOld *= pow(0.95, paths[i][j].pOld); //*= pow(0.90, paths[i][j].pOld);
                }
            }
        }
        bool force = false;
        if(myBonus &&  iterations - itMod*10 > 0){
            itMod++;
            /*
            int cap = iterations;
            if(cap > 9){
                cap = 10;
            }*/
            for(int j = 0;j < 10; j++){
                //best* whichBest = bestList[j];
                //std::cout << "hereiam" << std::endl;
                vec2order vec;
                //if(whichBest != NULL){
                    for(int i = 1;i < tNodes;i++){
                        vec = getOrder(bestList[j]->steps[i-1], bestList[j]->steps[i]);
                        //std::cout << vec.first << ", " << vec.second << std::endl;
                        if(paths[vec.first][vec.second].pFresh > 0){
                            //paths[vec.first][vec.second].pFresh = 0.0;

                        }else{
                            paths[vec.first][vec.second].pFresh = pherAppRate * bonusMultiplier;
                        }
                    }

                    vec = getOrder(bestList[j]->steps[0], bestList[j]->steps[tNodes-1]);
                    if(paths[vec.first][vec.second].pFresh > 0){
                        //paths[vec.first][vec.second].pFresh = 0.0;
                    }else{
                        paths[vec.first][vec.second].pFresh = (pherAppRate * bonusMultiplier);
                    }
                //}


            }
        }
    }else{
        for(int i = 0;i < tNodes; i++){
            for(int j = i+1; j < tNodes; j++){
                float thisDecay = (1-addRate);
                if(paths[i][j].pOld > 1.0){
                    //thisDecay *= (3/4) + 1/(4 * paths[i][j].pOld);
                }

                paths[i][j].pOld *= thisDecay;
                //paths[i][j].pOld *= (1-addRate);



                if(paths[i][j].pOld > 1){
                    //paths[i][j].pOld *= pow(0.95, paths[i][j].pOld); //*= pow(0.90, paths[i][j].pOld);
                }
            }
        }
    }
}

int starsystems::weightedRandom(ant& currentAnt)
{
    lList* weightedList = nullptr;
    float total = 0;
    int selected;
    //srand(time(NULL));
    float n_param;
    float t_param;
    float e0 = 0.0001;
    for(int i = 0;i < tNodes;i++){
        bool unvisisted = true;
        for(int j = 0;j < currentAnt.progress&&unvisisted;j++){
            if(i == currentAnt.steps[j]){
                unvisisted = false;
                //std::cout << j << " ";
            }
        }
        if(unvisisted&&i!=currentAnt.current){
            lList* temp = new lList;
            temp->next = weightedList;
            temp->id = i;
            vec2order vec = getOrder(i , currentAnt.current);
            if(i!=rollingOrigin){
                n_param = pow(1.0/paths[vec.first][vec.second].length, b_param);
                t_param = pow(paths[vec.first][vec.second].pOld, a_param);
                total+= (n_param * t_param + e0);
            }

            weightedList = temp;
        }

    }//std::cout << std::endl;
    lList* temp;
    temp = weightedList;
    while(temp!=NULL){
        //std::cout << "id: " << temp->id << std::endl;
        temp = temp->next;
    }

    float randomValue = total*float(rand())/float(RAND_MAX);
    //std::cout << total << ", " << randomValue << ", product: " << total*randomValue << std::endl;

    if(currentAnt.progress < tNodes-1){
        temp = weightedList;

        while(temp!=nullptr){
            vec2order vec = getOrder(temp->id , currentAnt.current);
            n_param = pow(1.0/paths[vec.first][vec.second].length, b_param);
            t_param = pow(paths[vec.first][vec.second].pOld, a_param);

            if(temp->next == nullptr || n_param * t_param + e0 >= randomValue){
                selected = temp->id;
                temp = nullptr;
            }else{
                //std::cout << total << ", " << randomValue << std::endl;
                randomValue -= (n_param* t_param + e0);
                temp = temp->next;
            }
        }
        temp = weightedList;
        while(weightedList!=NULL){
            temp = weightedList->next;
            delete weightedList;
            weightedList = temp;
        }
    }else{
        selected = rollingOrigin;
    }



    //srand(time(0));
    //selected = rand();
    //std::cout << "selected: " << selected << std::endl;
    return selected;
}

vec2order starsystems::getOrder(int i, int j)
{
    vec2order vec;
    if(i < j) {
        vec.first = i;
        vec.second = j;
    }else{
        vec.first = j;
        vec.second = i;
    }
    return vec;
}

bool starsystems::checkIdentical(ant& antony)
{
    best* selected = bestPaths;

    while(selected!=nullptr){
        bool hasMatch = true;
        if(selected->pathLength == antony.distanceTraveled){
            return true;
        }
        for(int i = 0; i < tNodes; i++){
            if(selected->steps[i] != antony.steps[i]){
                hasMatch = false;
            }
        }
        if(hasMatch){
            return true;
        }
        selected = selected->next;
    }
    return false;
}

void starsystems::recordGoodPath(ant& antony)
{
    //std::cout << "hereiam" << std::endl;
    if(antony.distanceTraveled < worstBest&& !checkIdentical(antony)){

        if(bestPaths->pathLength > antony.distanceTraveled){
            best* temp = new best;
            int lSize = tNodes*int(sizeof(int));
            temp->steps = (int*)malloc(lSize);
            memcpy(temp->steps, antony.steps, lSize);
            temp->pathLength = antony.distanceTraveled;
            temp->next = bestPaths;
            bestPaths = temp;
        }else{
            best* selected = bestPaths;
            int i = 0;
            bool notAdded = true;
            while(selected != nullptr){

                if(selected->next != nullptr){
                    if(selected->next->pathLength > antony.distanceTraveled && notAdded){

                        best* temp = new best;
                        temp->next = nullptr;
                        int lSize = tNodes*int(sizeof(int));
                        temp->steps = (int*)malloc(lSize);
                        //temp->steps = (int*)realloc(antony.steps,lSize);
                        memcpy(temp->steps, antony.steps, lSize);

                        temp->pathLength = antony.distanceTraveled;

                        temp->next = selected->next;
                        selected->next = temp;
                        totalBest++;

                        notAdded = false;
                    }
                }else if(i < 10&& selected->next == nullptr){
                    best* temp = new best;
                        temp->next = nullptr;
                        int lSize = tNodes*int(sizeof(int));
                        temp->steps = (int*)malloc(lSize);
                        //temp->steps = (int*)realloc(antony.steps,lSize);
                        memcpy(temp->steps, antony.steps, lSize);

                        temp->pathLength = antony.distanceTraveled;
                        selected->next = temp;
                }

                if(i >= 9){

                    if(selected->next != nullptr){
                        free(selected->next->steps);
                        delete selected->next;
                        selected->next = nullptr;
                        worstBest = selected->pathLength;
                        totalBest = 10;
                    }

                }

                i++;

                selected = selected->next;
            }
            selected = bestPaths;
            for(int j = 0;j < i; j++){
                bestList[j] = selected;
                selected = selected->next;
            }

            //std::cout << totalBest << std::endl;
        }

    }
    /*
    best* temp = bestPaths;
    while(temp != nullptr){
        for(int i = 0; i < tNodes; i++){
            //std::cout << temp->steps[i] << " ";
        }
        //std::cout << std::endl;
        temp = temp->next;
    }*/
}



void starsystems::bruteForce()
{
    char c = 0;
    //std::string str ="" + c;
    //std::cout << c << std::endl;

    bruteBest = bruteLoop("", 0);

    std::cout << bruteBest.pathLength << std::endl;
    //std::cout << "hereiam" << std::endl;

}
int bruteCount = 0;
best starsystems::bruteLoop(std::string str, int step)
{
    best minBrute;
    minBrute.pathLength = 2*bounds*sqrt(3)*tNodes + 1.0;
    minBrute.steps = new int[tNodes];

    if(step == tNodes){
        //std::cout << int(str[1]) << std::endl;

        vec2order vec = getOrder(int(str[0]), int(str[tNodes-1]));
        minBrute.steps[0] = int(str[0]);
        minBrute.pathLength = paths[vec.first][vec.second].length;

        for(int i = 1;i < tNodes; i++){
            minBrute.steps[i] = int(str[i]);
            vec = getOrder(int(str[i-1]), int(str[i]));
            minBrute.pathLength += paths[vec.first][vec.second].length;

        }//std::cout << minBrute.pathLength << std::endl;
        bruteCount++;

        return minBrute;
    }else{

        bool validity[tNodes];
        for(int i = 0; i < tNodes; i++){
            validity[i] = true;
        }
        for(int i = 0; i < str.length(); i++){
            validity[int(str[i])] = false;

        }

        for(int i = 0; i < tNodes; i++){
            if(validity[i]){
                std::string tempstr = str + char(i);
                best tempBest = bruteLoop(tempstr, step+1);
                //std::cout << str << ", i:" << i << ", step" << step  << std::endl;
                if(tempBest.pathLength < minBrute.pathLength){
                    memcpy(minBrute.steps, tempBest.steps, tNodes * sizeof(int));
                     minBrute.pathLength = tempBest.pathLength;
                }
            }
        }if(step == 1)std::cout << minBrute.pathLength << ", " << bruteCount << std::endl;
        return minBrute;
    }

}


