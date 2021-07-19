#include "SimpleFunctionalEquationsParser.h"

SimpleFunctionalEquationsParser::SimpleFunctionalEquationsParser() {
    diffEvo = new DifferencialEvolution();
    evoStrat = new EvolutionStrategie();
}

string SimpleFunctionalEquationsParser::nameParser() {
    return "SimpleFunctionalEquationsParser";
}

void SimpleFunctionalEquationsParser::Optimize(Subject* s){
    if(conf->approach == 1 || conf->approach == 2 || conf->approach == 9 || conf->approach == 13 || conf->approach == 14 || conf->approach == 21){
        diffEvo->Optimize(s, this);
    }else{
        evoStrat->Optimize(s, this);
    }
}

bool validate(double* fx, double** x, int len){
    int count = 0;
    for(int i = 1; i < len; i++){
        double f = fx[i] - fx[i - 1];
        double d = 0;

        d = x[i][0] - x[i - 1][0];
//        cout << x[i][0] << " " << x[i - 1][0] << " " << f/d << endl;

        if(abs(f/d) < pow(10, -4))
            count++;

        if(std::isinf(d) || std::isnan(d) || std::isinf(f) || std::isnan(f)) {
            return false;
        }

//        cout << f/d << endl;
    }
//    cout << count << endl;
//        cin.get();

    return count/double(len - 1) < 0.1;
}

double SimpleFunctionalEquationsParser::FunctionalEquation1(Subject* s, double* d){
    double* fx2_d = new double[data->variables];
    for(int i = 0; i < data->variables; i++){
        fx2_d[i] = pow(d[i], 2);
    }

    double fx2 = AuxEvaluate(s, 1, fx2_d);

    double* fx_d = new double[data->variables];
    for(int i = 0; i < data->variables; i++){
        fx_d[i] = d[i];
    }
    double fx = AuxEvaluate(s, 1, fx_d);

    delete [] fx2_d;
    delete [] fx_d;

    return fx2 - fx - 1;
}

double SimpleFunctionalEquationsParser::FunctionalEquation2(Subject* s, double* d){
    double* a_d = new double[data->variables]; //x = 1
    for(int i = 0; i < data->variables; i++){
        a_d[i] = d[i] + 1;
    }

    double a = AuxEvaluate(s, 1, a_d); //f(x + 1)

    double b = AuxEvaluate(s, 1, d); //[f(x)]^2
    b = pow(b, 2);

    delete [] a_d;

//    cout << "a = " << a << " b = " << b << " a - b = " << a - b << endl;

    return a - b;
}

double SimpleFunctionalEquationsParser::FunctionalEquation3(Subject* s, double* d){
    double* a_d = new double[data->variables]; //x - 1
    double* b_d = new double[data->variables]; //x + 1
    for(int i = 0; i < data->variables; i++){
        a_d[i] = d[i] - 1;
        b_d[i] = d[i] + 1;
    }

    double a = AuxEvaluate(s, 1, a_d); //f(x - 1)

    double b = AuxEvaluate(s, 1, b_d); //f(x + 1)

    double fx = AuxEvaluate(s, 1, d); //f(x)
    double* fx_d = new double[data->variables];
    fx_d[0] = fx;
    double ffx = AuxEvaluate(s, 1, fx_d); //f(f(x))

    delete [] a_d;
    delete [] b_d;
    delete [] fx_d;

//    cout << "a = " << a << " b = " << b << " a - b = " << a - b << endl;

    return a*b - ffx;
}

double SimpleFunctionalEquationsParser::FunctionalEquation4(Subject* s, double* d){
    double x = d[0];
    double y = d[1];

    double* a_d = new double[data->variables]; //x + 1
    double* b_d = new double[data->variables]; //x - 1
    for(int i = 0; i < data->variables; i++){
        a_d[i] = d[0] + d[1];
        b_d[i] = d[0] - d[1];
    }

    double a = AuxEvaluate(s, 1, a_d); //f(x + 1)

    double b = AuxEvaluate(s, 1, b_d); //f(x - 1)

    double left = (x - y) * a - (x + y) * b;
    double right = 4 * x * y * (pow(x, 2) - pow(y, 2));

//    s->trees[0]->print();
//    cout << endl << x << " " << y << " " << left << " " << right << " " << endl;
//    cin.get();

    delete [] a_d;
    delete [] b_d;

//    cout << "a = " << a << " b = " << b << " a - b = " << a - b << endl;

    return left - right;
}

double SimpleFunctionalEquationsParser::FunctionalEquation5(Subject* s, double* d){
    double* a_d = new double[data->variables]; //(x - 1)/x
    double* b_d = new double[data->variables]; //x + 1
    for(int i = 0; i < data->variables; i++){
        a_d[i] = (d[i] - 1)/d[i];
        b_d[i] = d[i] + 1;
    }

    double a = AuxEvaluate(s, 1, a_d); //f((x - 1)/x)

    double b = b_d[0]; //x + 1

    double fx = AuxEvaluate(s, 1, d); //f(x)

    delete [] a_d;
    delete [] b_d;

//    cout << "a = " << a << " b = " << b << " a - b = " << a - b << endl;

    return fx + a - b;
}

double SimpleFunctionalEquationsParser::FunctionalEquation6(Subject* s, double* d){
    double* a_d = new double[data->variables]; //x
    double* b_d = new double[data->variables]; //x + 1
    for(int i = 0; i < data->variables; i++){
        a_d[i] = d[i];
        b_d[i] = d[i] + 1;
    }

    double a = AuxEvaluate(s, 1, a_d); //f(x)
    double b = AuxEvaluate(s, 1, b_d); //f(x + 1)

    delete [] a_d;
    delete [] b_d;

//    cout << "a = " << a << " b = " << b << " a - b = " << a - b << endl;

    return a*b - (a - b);
}

double SimpleFunctionalEquationsParser::Evaluate(Subject* s) {
    double r = 0;
    double fit = 0;
    double* fx = new double[tamDataset];
    double** x = new double*[tamDataset];

//    s->trees[0]->root->print();
//    cout << endl;

    for(int j = 0; j <tamDataset; j++) {
        r = FunctionalEquation5(s, dataset[j]);
        fx[j] = AuxEvaluate(s, 1, dataset[j]);
        x[j] = dataset[j];
//        cout << fx[j] << " " << dataset[j][0] << endl;
//        cin.get();

        if(std::isinf(r) || std::isnan(r)) {
//            cout << "zoo" << endl;
//            s->trees[0]->fitness = INFINITY;
//            s->trees[0]->root->print();
//            cout << endl;
//            cin.get();
            s->trees[0]->fitness = INFINITY;
            return INFINITY;
        }

        fit += pow(r, 2); // data->variables pois só tem 1 árvore
    }

    if(!validate(fx, x, tamDataset)){
        s->trees[0]->fitness = INFINITY;
    }
    else{
        fit /= tamDataset;
        s->trees[0]->fitness = fit;

    //    cout << "fit " << fit << " " << pow(-1, 1.0/2.0) << " " << sqrt(-1) << endl;
    //        cin.get();

    //    cout << "Final: " << fit << endl;
    }

//    cout << s->trees[0]->fitness << endl;
//        cin.get();

    delete [] fx;
    delete [] x;

    return s->trees[0]->fitness;
}

double SimpleFunctionalEquationsParser::Operate(int opType, int opValue, double a, double b, double c) {
    double r;
    if(opType == conf->bynaryArithmeticOperators) {
        if(opValue == 0)
            r = a + b;
        else if(opValue == 1)
            r = a - b;
        else if(opValue == 2)
            r = a * b;
        else if(opValue == 3){
            if(b == 0.0)
                r = NAN;
            else
                r = a / b;
        }
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
    }

//    if(std::isnan(r) || std::isinf(r)) {
//        cout << "      " << opType << " " << opValue << " " << a << " " << b << " " << r << endl;
//        return NULL;
//    }
//    cout << "      " << r << endl;
    return r;
}

double SimpleFunctionalEquationsParser::AuxEvaluate(Subject* s, int model, double* dat) {
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

SimpleFunctionalEquationsParser::~SimpleFunctionalEquationsParser() {
    //dtor
}



