#ifndef SimpleClassifierParser_H
#define SimpleClassifierParser_H

#include "Configures.h"
#include "Parser.h"
#include "SimpleIndividuo.h"
#include "DifferencialEvolution.h"
#include "EvolutionStrategie.h"
#include <vector>
#include <stack>
#include <tuple>
#include <cmath>

class SimpleClassifierParser : public Parser {
  public:
    SimpleClassifierParser();
    virtual string nameParser();
    virtual double Evaluate(Subject* s);
    virtual void Optimize(Subject* s);
    virtual double Operate(int opType, int opValue, double a, double b = NULL, double c = NULL);

    DifferencialEvolution* diffEvo;
    EvolutionStrategie* evoStrat;

    virtual void setDataSet(double ** x,int tam) {
        tamDataset = tam;
        dataset = x;
    }

    virtual void printResult(Subject * s) {}

    ~SimpleClassifierParser();
    double** dataset;
    int tamDataset;
  protected:
  private:
    double AuxEvaluate(Subject* s, int model, double* dat);
};

#endif // SimpleClassifierParser_H
