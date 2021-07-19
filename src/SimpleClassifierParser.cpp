#include "SimpleClassifierParser.h"

SimpleClassifierParser::SimpleClassifierParser() {
    diffEvo = new DifferencialEvolution();
    evoStrat = new EvolutionStrategie();
}

string SimpleClassifierParser::nameParser() {
    return "SimpleClassifierParser";
}

void SimpleClassifierParser::Optimize(Subject* s){
    if(conf->approach == 1 || conf->approach == 2 || conf->approach == 9 || conf->approach == 13 || conf->approach == 14 || conf->approach == 21){
        diffEvo->Optimize(s, this);
    }else{
        evoStrat->Optimize(s, this);
    }
}

double SimpleClassifierParser::Evaluate(Subject* s) {
    //return  1;
    double totalFit = 0, fit, r;
    for(int arvore = 0; arvore < conf->numTree; arvore++) {
        s->trees[arvore]->fitness = 0;
        fit = 0;
        for(int j = 0; j <tamDataset; j++) { // para todos os dados do conjunto de treinamento
            r = AuxEvaluate(s,arvore + 1,dataset[j]);
            //cout << "Evaluate: " << r << endl;
            if(std::isinf(r) || std::isnan(r)) {
                s->trees[arvore]->fitness = INFINITY;
//                s->trees[arvore]->root->print();
                cout << "Deu ruim aqui " << endl;
                cin.get();
                break;
            }
//            s->print();
//            cout << r << " = " << dataset[j][data->variables + arvore] << endl;
//            cin.get();
            if(r != dataset[j][data->variables + arvore])
                fit += 1;
        }
        s->trees[arvore]->fitness = fit/tamDataset;
        totalFit += fit/tamDataset;
    }
    return totalFit;
}

double SimpleClassifierParser::Operate(int opType, int opValue, double a, double b, double c) {
    double r;
    if(opType == conf->bynaryArithmeticOperators) {
        if(opValue == 0)
            r = a + b;
        else if(opValue == 1)
            r = a - b;
        else if(opValue == 2)
            r = a * b;
        else if(opValue == 3)
            r = a / b;
        else if(opValue == 4)
            r = pow(a, b);
    } else if(opType == conf->unaryArithmeticOperators) {
        if(opValue == 0)
            r = log(a);
        else if(opValue == 1)
            r = exp(a);
        else if(opValue == 2)
            r = sqrt(a);
        else if(opValue == 3)
            r = cos(a);
        else if(opValue == 4)
            r = sin(a);
        else if(opValue == 5)
            r = tan(a);
        else if(opValue == 6)
            r = acos(a);
        else if(opValue == 7)
            r = asin(a);
        else if(opValue == 8)
            r = atan(a);
    } else if(opType == conf->binaryLogicalOperators) {
        if(opValue == 0)
            r = a && b;
        else if(opValue == 1)
            r = a || b;
        else if(opValue == 2)
            r = bool(a) ^ bool(b);
    } else if(opType == conf->unaryLogicalOperators) {
        if(opValue == 0)
            r = !bool(a);
        else if(opValue == 1)
            r = 1;
        else if(opValue == 2)
            r = 0;
    } else if(opType == conf->comparisonOperators) {
        if(opValue == 0)
            r = a < b;
        else if(opValue == 1)
            r = a <= b;
        else if(opValue == 2)
            r = a == b;
        else if(opValue == 3)
            r = a >= b;
        else if(opValue == 4)
            r = a > b;
        else if(opValue == 5)
            r = a != b;
    } else if(opType == conf->programOperators) {
        if(opValue == 0){ // if-else
            if(c)
                r = a;
            else
                r = b;
        }
    }

//    if(std::isnan(r) || std::isinf(r)) {
////        cout << "      " << opType << " " << opValue << " " << a << " " << b << " " << r << endl;
//        return NULL;
//    }
//    cout << "      " << r << endl;
    return r;
}

double SimpleClassifierParser::AuxEvaluate(Subject* s, int model, double* dat) {
    SimpleIndividuo * s1 = dynamic_cast<SimpleIndividuo*>(s);
    int coeficientCount = 0;
    stack<double> stk;
    for(tuple<double, double> t : s->trees[model - 1]->linearModel) { // -1 because model is the n-1 tree of s and n y ou fn(x)
        int var = (int)get<0>(t);
        if( var == conf->constant ) { // constant
            stk.push(get<1>(t));
        } else if( var == conf->variable ) { // variable;
            stk.push(dat[(int)get<1>(t)]);
        } else if( var == conf->bynaryArithmeticOperators) { // bynaryArithmeticOperators
            double b = stk.top();
            stk.pop();
            double a = stk.top();
            stk.pop();
            stk.push(Operate(get<0>(t), get<1>(t), a, b));
        } else if( var ==  conf->unaryArithmeticOperators) { // unaryArithmeticOperators
            double a = stk.top();
            stk.pop();
            stk.push(Operate(get<0>(t), get<1>(t), a));
        } else if( var == conf->binaryLogicalOperators) { // binaryLogicalOperators
            double b = stk.top();
            stk.pop();
            double a = stk.top();
            stk.pop();
            stk.push(Operate(get<0>(t), get<1>(t), a, b));
        } else if( var == conf->unaryLogicalOperators) { // unaryLogicalOperators
            double a = stk.top();
            stk.pop();
            stk.push(Operate(get<0>(t), get<1>(t), a));
        } else if( var == conf->comparisonOperators) { // comparisonOperators
            double b = stk.top();
            stk.pop();
            double a = stk.top();
            stk.pop();
            stk.push(Operate(get<0>(t), get<1>(t), a, b));
        } else if( var == conf->programOperators) {
            if((int)get<1>(t) == 0){ //if-selse
                double bool_ = stk.top();
                stk.pop();
                double b = stk.top();
                stk.pop();
                double a = stk.top();
                stk.pop();
                stk.push(Operate(get<0>(t), get<1>(t), a, b, bool_));
            }
            //programOperators
        } else if( var == conf->adjustedCoeficient ) { // constant
//            cout << "   " << coeficientCount << endl;
            if(s1->constants != NULL)
                stk.push(s1->constants[model - 1][coeficientCount]);
            else
                stk.push(1);
//            cout << s1->constants[model - 1][coeficientCount] << " " << coeficientCount << endl;
            coeficientCount++;
        }

    }

    return stk.top();
}

SimpleClassifierParser::~SimpleClassifierParser() {
    //dtor
}


