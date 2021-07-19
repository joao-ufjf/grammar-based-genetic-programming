#include "RK4LSDerivadasParser.h"

RK4LSDerivadasParser::RK4LSDerivadasParser() {
    //ctor
}

void RK4LSDerivadasParser::calcDerivadas() {
    //cout << "Tam dataset " << tamDataset << endl;
    banco_derivadas = new double*[tamDataset];


    for(int i = 0 ; i < tamDataset; i++) {
        if(i < tamDataset ) {
            // cout << "Derivada do registro " << i << endl;
            banco_derivadas[i] = calcDerivadaRegistroI(i);
            //cin.get();
        } else
            banco_derivadas[i] = calcDerivadaRegistroI(i-1);
    }

//    cout << "\n\nBANCO DE DERIVADAS" << endl;
//
//     for(int i = 0 ; i < tamDataset; i++){
//        for(int j = 0; j < data->prediction+data->variables; j++){
//            cout << banco_derivadas[i][j] << " ";
//        }
//        cout << endl;
//     }
//
//     cin.get();
}

void RK4LSDerivadasParser::setDataSet(double ** x, int tam) {
    dataset = x;
    tamDataset = tam;
    calcDerivadas();
}


double RK4LSDerivadasParser::Evaluate(Subject* s) {

    RK4LSIndividuo * s1 = dynamic_cast<RK4LSIndividuo*>(s);
    s1->leastSquareSize = new int[conf->numTree];
    s1->leastSquare = new double*[conf->numTree];

    for(int i  = 0; i < conf->numTree; i++) {
        s1->leastSquare[i] = NULL;
    }

    double * r;
    s1->fitness = 0;
    for(int arvore = 0; arvore < conf->numTree; arvore++) {
        s1->trees[arvore]->fitness = 0;
        r = AuxEvaluate(s1,arvore,dataset,tamDataset);
        for( int j = 0; j < tamDataset ; j++) { // para todos os dados do conjunto de treinamento
            if(std::isinf(r[j]) || std::isnan(r[j])) {
                s1->trees[arvore]->fitness = INFINITY;
                //s->trees[arvore]->root->print();
                break;
            }
            s1->trees[arvore]->fitness += pow(r[j] - banco_derivadas[j][data->variables + arvore], 2);
        }
        s1->fitness += s1->trees[arvore]->fitness;
        delete [] r;
    }
    return s1->fitness;
}


double * RK4LSDerivadasParser::calcDerivadaRegistroI(int registro) {

    double *anterior, *proximo;
    double * derivadas = new double[data->prediction+data->variables];

    for(int i = 0; i < data->variables; i++) {
        derivadas[i] = dataset[registro][i];
    }


    if(registro > 0 && registro < tamDataset -1) {
        anterior = dataset[registro-1];
        proximo  = dataset[registro+1];

        for(int i = data->variables ; i <= data->variables + data->prediction - 1; i++) {
            //cout << (proximo[i] - anterior[i])/(proximo[0] - anterior[0]);
            derivadas[i] = (proximo[i] - anterior[i])/(proximo[0] - anterior[0]);
            //cout << derivadas[i-data->variables];
        }

    } else if ( registro == 0) {


        anterior = dataset[0];
        proximo  = dataset[1];

        for(int i = data->variables ; i <= data->variables + data->prediction - 1; i++) {
            //cout << (proximo[i] - anterior[i])/(proximo[0] - anterior[0]) ;
            derivadas[i] = (proximo[i] - anterior[i])/(proximo[0] - anterior[0]);

        }
    } else if (registro == (tamDataset-1) ) {

        anterior = dataset[tamDataset -2];
        proximo  = dataset[tamDataset -1];

        for(int i = data->variables; i < data->variables + data->prediction; i++) {
            // cout << proximo[i] - anterior[i] ;
            derivadas[i] = (proximo[i] - anterior[i])/(proximo[0] - anterior[0]);
        }
    }

    for(int i = 0; i < data->variables + data->prediction ; i++) {
        cout << derivadas[i] << " " ;
    }
//    cout << endl;
    return derivadas;

}


double * RK4LSDerivadasParser::AuxEvaluate(RK4LSIndividuo * s, int model, double ** dat,int tam) {

    //LeastSquareIndividuo * s1 = s;
    //s->trees[model]->root->print();
//    cout << endl;
//    s->trees[model]->root->print();
//   cout << "________________________" << endl;
    //cin.get();

    /// Find base functions
    int sub = 1;
    for(int i = 0; i < s->trees[model]->linearModel.size(); i++)
        if((int)get<0>(s->trees[model]->linearModel.at(i)) == conf->leastSquareOperator)
            sub++;
    s->leastSquareSize[model] = sub;

//    cout <<"LeastSquareSize : " ;
//    cout << s->leastSquareSize[model];
//    cout << " TAM : " << tam << endl;
//    cin.get();

    double** subMatrix = new double*[tam];
    for(int i = 0; i < tam; i++) {
        subMatrix[i] = new double[sub];
    }

    //cout << " subMatrix criada\n";
//    cin.get();

//    for(tuple<double, double> t : s->trees[model]->linearModel)
//        cout << "(" << get<0>(t) << "," << get<1>(t) << ")";
//    cout << endl << "tam = " << tam << " sub = " << sub << endl;

    /// Operate with a Stack
    int j = 0;
    for(int i = 0; i < tam; i++) {
        stack<double> stk;
//        cout << "  i = " << i;
        for(tuple<double, double> t : s->trees[model]->linearModel) {
            int var = (int)get<0>(t);
            if( var == conf->constant ) { // constant
                stk.push(get<1>(t));
            } else if( var == conf->variable ) { // variable
                stk.push(dat[i][(int)get<1>(t)]);
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
//                programOperators
            } else if(var == conf->leastSquareOperator) {
//                subMatrix[i][j] = stk.top();
//                cin.get();
//                stk.pop();
            }
        }

//        if(stk.size() != sub){
//            cout << "Teste " << stk.size() << "  " << sub << endl;
//            for(tuple<double, double> tt : s->trees[model]->linearModel)
//                cout << "(" << get<0>(tt) << "," << get<1>(tt) << ")";
//            cout << endl;
//            cin.get();
//        }

        int j = sub - 1;
        while(!stk.empty() && j >= 0) {
            subMatrix[i][j] = stk.top();
//            cout << subMatrix[i][j] << ", ";
//            cout << stk.top() << ", ";
            stk.pop();
//            cout << " j = " << j << " pilha = " << stk.size() << " ";
            j--;
        }
//        cout << endl;
    }

    //cout <<"Pilha operada \n";
//    cin.get();

//////    /// Solve Least Square
    QRDecomposition* qrDec = new QRDecomposition(subMatrix, tam, sub);
    double* b = new double[tam];
    for(int i = 0; i < tam; i++) {
        b[i] = dat[i][data->variables + model];
        //cout << b[i] << endl;
    }

    //cout <<"Solve LeastSquare\n";
//    cin.get();

    double * coef = qrDec->solveLeastSquares(b, tam);
    s->leastSquareSize[model] = sub;

    //cout <<"Bora calcular o erro?\n";
//    cin.get();

//    s->trees[model]->leastSquare = NULL;
    if(coef != NULL) {
        //cout << "deu" << endl;
    } else {
//        cout << "nao deu" << endl;
        coef = new double[sub];
        for(int j = 0; j < sub; j++) {
            coef[j] = 1;
        }
    }

//    cout << "model " ;
//    s->trees[model]->root->print();
    /// Calculate Error
    double* r = new double[tam];
    for(int i = 0; i < tam; i++) {
        r[i] = 0;
        for(int j = 0; j < sub; j++) {
//            if(j == 0){
//            cout << "Coeficiente : " << s->leastSquare[model][j] <<endl;
//            cout << "valor : " << subMatrix[i][j] << endl;
//            //cin.get();
//            }
            //          cout << "\n[" << i  <<  ","<< model << "," << j <<"] ";
//            cout << " LS : " << double(coef[j]) ;
            //        cout <<" subMatriz " << double(subMatrix[i][j]) << endl;
            //cin.get();
            r[i] += double(coef[j]) * double(subMatrix[i][j]);
            if(std::isnan(r[i]) || std::isinf(r[i]))
                r[i] = INFINITY;
        }
    }

//    for(int i = 0; i < tam; i++){
//        for(int j = 0; j < sub; j++)
//            cout << subMatrix[i][j] << " ";
//            cout << s->trees[model]->leastSquare[j] << " ";
//        cout << endl;
//    }
//    cin.get();

    //cout << "Todos calcs realizados\n";
    // cin.get();
    s->leastSquare[model] = coef;
    //delete [] coef;
    for(int i = 0; i < tam; i++) {
        delete [] subMatrix[i];
    }
    delete [] subMatrix;

    delete [] b;
    delete qrDec;
//    cout << "vetor r : ";
//    for(int i = 0; i < tam; i++){
//                cout << r[i] << " ";
//    }
//    cout << endl;
//    cin.get();
//    delete []s->leastSquare;
//    delete []s->leastSquareSize;

    // cout << "Finalizado retornando \n";
    //cin.get();
    return r;
}


RK4LSDerivadasParser::~RK4LSDerivadasParser() {
    //dtor
}
