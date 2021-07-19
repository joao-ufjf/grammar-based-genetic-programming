#ifndef SIMPLEFUNCTIONALEQUATIONSPARSER_H
#define SIMPLEFUNCTIONALEQUATIONSPARSER_H

#include "Configures.h"
#include "Parser.h"
#include "SimpleIndividuo.h"
#include "DifferencialEvolution.h"
#include "EvolutionStrategie.h"
#include <vector>
#include <stack>
#include <tuple>
#include <cmath>

class SimpleFunctionalEquationsParser : public Parser {
  public:
    SimpleFunctionalEquationsParser();
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

    ~SimpleFunctionalEquationsParser();
  protected:
    double** dataset;
    int tamDataset;
  private:
    double AuxEvaluate(Subject* s, int model, double* dat);

    double FunctionalEquation1(Subject* s, double* d);
    double FunctionalEquation2(Subject* s, double* d);
    double FunctionalEquation3(Subject* s, double* d);
    double FunctionalEquation4(Subject* s, double* d);
    double FunctionalEquation5(Subject* s, double* d);
    double FunctionalEquation6(Subject* s, double* d);
};

#endif // SIMPLEFUNCTIONALEQUATIONSPARSER_H
