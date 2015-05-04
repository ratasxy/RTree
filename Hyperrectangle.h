#ifndef HYPERRECTANGLE_H
#define HYPERRECTANGLE_H

#include "vector"

using namespace std;

class Hyperrectangle
{   
private:

public:
    points vertices;
    points getVertices()
    {
        return this->vertices;
    }

    Hyperrectangle()
    {

    }
    Hyperrectangle(points vertices)
    {
        this->vertices = vertices;
    }

    double area()
    {
        double tmp = 1;
        for(pair<double,double> dimension : this->vertices)
            tmp *= dimension.second - dimension.first;

        return tmp;
    }
    
    double contain(Hyperrectangle o)
    {
        points v = o.getVertices();

        double tmp = 1; int i=0;
        for(auto dimension : this->vertices){
            tmp *= (min(dimension.second, v[i].second) - max(dimension.first, v[i].first));
            i++;
        }

        return tmp;
    }

    bool overlap(Hyperrectangle o)
    {
        points v =o.getVertices();

        int i=0;
        for (pair<double,double>dimension : this->vertices)
        {
            if(v[i].first>dimension.second || dimension.first>v[i].second) return false;
            i++;
        }
        return true;
    }

};

#endif // HYPERRECTANGLE_H
