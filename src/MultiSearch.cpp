#include "MultiSearch.h"
#include <vector>

/**
* Construtores da função de busca
*   Recebem todos os parametros relativos a busca
*   Parameters
*       Parser - responsavel por avaliar os individuos
*       PopulationReplace - reponsavel pela substituição da população para a proxima geração
*       IndividuoBuilder - responsavel pela criação de individuos
*       Mutation - Operador de mutação
*       Crossover - Operador genetico de mutação
*       Selection - Operador de seleção de indivudos
**/
MultiSearch::MultiSearch()
{
    setParser(NULL);
    setPopulationReplace(NULL);
    setIndividuoBuilder(NULL);
    setMutation(NULL);
    setCrossover(NULL);
    setSelection(NULL);

   // omp_set_num_threads(conf->NUM_THREADS);

}


MultiSearch::MultiSearch(Parser* parser,PopulationReplacement * populationreplace, IndividuoBuilder * individuobuilder)
{
    setParser(parser);
    setPopulationReplace(populationreplace);
    setIndividuoBuilder(individuobuilder);

    setMutation(NULL);
    setCrossover(NULL);
    setSelection(NULL);

   // omp_set_num_threads(conf->NUM_THREADS);

}

/**
    Função privada que instancia e preenche a população inicial de individuos
**/
void MultiSearch::createsInitialPopulation(){
//    cout << "Create Initial Population..." << endl;
    pop = new Subject*[conf->popSize * 2];

   // #pragma omp parallel for num_threads(conf->NUM_THREADS)
    for(int i = 0; i < conf->popSize; i++){
        pop[i] = individuoBuilder->generateIndividuo();
    }
}
//Substitui todos os valores constantes por _c_
void MultiSearch::replaceAllConst(int initialIndex, int finalIndex){
    #pragma omp parallel for num_threads(conf->NUM_THREADS)
    for(int i = initialIndex; i < finalIndex; i++){
        if(!pop[i]->optimized){
            pop[i]->trees[0]->replaceAllConst();
        }
    }
}

/**
*
*   Realiza a busca atravez do processo evolutivo
*   @return void
**/
void MultiSearch::evolve(){
    int tolerance_count = 0;
    cout << "Criar populacao inicial" << endl;
    this->createsInitialPopulation();
    DifferencialEvolution* de = new DifferencialEvolution();
    EvolutionStrategie* es = new EvolutionStrategie();

    EvaluatePopulation(0, conf->popSize);
    CalcRank(conf->popSize);

/// The best in validation will be optimized by some optimizer
    if(conf->approach == 0){
        cout << "No optimitazion required!" << endl;
    }
    else if(conf->approach == 1){
        cout << "Optimizing first front!" << endl;
//        cout << "Ind que entra na opt " << pop[0]->fitness << " " << pop[0]->fitnessValidation << endl;
        int r = 0;
        while(pop[r]->ranking == 0)
           r++;
//        cout << "   optimizing " << r << " individuals" << endl;

        EvaluatePopulation(0, r, true);
//        cin.get();
    }


    Subject* best = NULL;

    int timeStamp = 100;
    int DECounter = 1;
    int maxEvaluations = 1000000;

    conf->optimizationEvaluations = 100;
    vector<double> fitInTime;
    fitInTime.push_back(pop[0]->fitness);

    grammar->print();

    // Para todos os casos que o otimizador executara no fim
    if(conf->approach == 1){
        conf->optimizationEvaluations = maxEvaluations * 0.1;
        maxEvaluations *= 0.9;
    }

    double best_error = INFINITY;

    for(int it = 1; conf->evaluations < maxEvaluations; it++){
//    int it = 0;
//    while(best_error > 1){
        it++;
        Operate();//Realiza cross e mut

        if(conf->evaluations > DECounter * 0.1 * maxEvaluations){//Entao devo trocar as constantes por C
            if(conf->approach == 1){ // faz DE no best
                int r = 0;
                while(pop[r]->ranking == 0)
                r++;
//                cout << "   optimizing " << r << " individuals" << endl;

                ++DECounter;
                // neste caso, a avaliação executa o DE ou EE para o range indicado
                EvaluatePopulation(0, r, true);
//                cin.get();
            }
        }

        EvaluatePopulation(conf->popSize, conf->popSize * 2);

        CalcRank(conf->popSize * 2);

        for(int i = conf->popSize; i < conf->popSize * 2; i++){
            delete pop[i];
        }

        for(int k = 0; k < conf->popSize && pop[k]->ranking == 0; k++){
            cout << it << " " << pop[k]->ranking << " " << pop[k]->fitness << " " << pop[k]->complexity << " "; // << endl;
            pop[k]->print();
//            cin.get();
//        cout << endl;
        }
//        cin.get();
//        pop[0]->print();
        fitInTime.push_back(pop[0]->fitness);

//        cin.get();
    }


    cout << "ACABOU TREINAMENTO" << endl;;
    /*** Com a populacao treinada, hora de realizar a validacao ***/
    for(int i = 0; i < conf->popSize; i++){
        pop[i]->fitness = parserValidation->Evaluate(pop[i]);
        pop[i]->complexity = 0;
        for(Tree* t : pop[i]->trees)
            pop[i]->complexity += t->linearModel.size();
    }
    CalcRank(conf->popSize);
    cout << "--------------Results--------------" << endl;
    for(int k = 0; pop[k]->ranking == 0 && k < conf->popSize; k++){
        cout << k << " " << pop[k]->ranking << " " << pop[k]->fitness << " " << pop[k]->complexity << " "; // << endl;
        pop[k]->print();
//        cout << endl;
    }
    cout << "VALIDADO:" << endl;
    cout << "Iniciando testes";
    /*** Com a populacao treinada, hora de realizar a validacao ***/
    for(int i = 0; i < conf->popSize; i++){
        pop[i]->fitness = parserTest->Evaluate(pop[i]);
        pop[i]->complexity = 0;
        for(Tree* t : pop[i]->trees)
            pop[i]->complexity += t->linearModel.size();
    }

    cout << "Testado" << endl;
    CalcRank(conf->popSize);
    int nModels = 0;
    cout << "--------------Results--------------" << endl;
    for(int k = 0; pop[k]->ranking == 0 && k < conf->popSize; k++){
        nModels++;
        cout << k << " " << pop[k]->ranking << " " << pop[k]->fitness << " " << pop[k]->complexity << " "; // << endl;
        pop[k]->print();
//        cout << endl;
    }

    /// The best in validation will be optimized by some optimizer
    if(conf->approach == 1){ // faz DE no best
        int r = 0;
        while(pop[r]->ranking == 0)
        r++;
        cout << "   optimizing " << r << " individuals" << endl;
        // neste caso, a avaliação executa o DE ou EE para o range indicado
        EvaluatePopulation(0, r, true);
        cin.get();
    }

    cout << "Modelos" << endl;

//    ofstream models("models.txt");
//    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
//    std::cout.rdbuf(models.rdbuf()); //redirect std::cout to out.txt!
    cout << nModels << endl;
    for(int k = 0; pop[k]->ranking == 0 && k < conf->popSize; k++){
        cout << "; ";
        pop[k]->print();
        cout << "; ";
    }

//    std::cout.rdbuf(coutbuf); //reset to standard output again
}

int MultiSearch::Tournament(int a, int b){
    if(pop[a]->ranking < pop[b]->ranking)
        return a;
    else if(pop[a]->ranking > pop[b]->ranking)
        return b;
    else{
        if(pop[a]->crowdingDistance > pop[b]->crowdingDistance)
            return a;
        else
            return b;
    }
}

bool MultiSearch::Dominate(Subject* a, Subject* b){
    if (a->fitness <= b->fitness && a->complexity <= b->complexity) {
        if (a->complexity < b->complexity || a->fitness < b->fitness) {
            return true;
        }
    }
    return false;
}

bool MultiSearch::SortPopulationRank(Subject* a, Subject* b){
     //cout << "FITNESS: " << a->trees[0]->fitness << " - " << b->trees[0]->fitness << endl;
     return a->ranking < b->ranking;
}

bool MultiSearch::SortPopulationCrowding(Subject* a, Subject* b){
     //cout << "FITNESS: " << a->trees[0]->fitness << " - " << b->trees[0]->fitness << endl;
     return a->crowdingDistance > b->crowdingDistance;
}

bool MultiSearch::SortPopulationComplexity(Subject* a, Subject* b){
     //cout << "FITNESS: " << a->trees[0]->fitness << " - " << b->trees[0]->fitness << endl;
     return a->complexity < b->complexity;
}

void MultiSearch::crowdingDistanceFitness(int i, int f){
    if(i == f){
        pop[i]->crowdingDistance = INFINITY;
    }
    else{
        pop[i]->crowdingDistance = pop[f]->crowdingDistance = INFINITY;
        for(int j = i + 1; j < f; j++){
            pop[j]->crowdingDistance += (pop[j + 1]->fitness - pop[j - 1]->fitness)/(pop[f]->fitness - pop[i]->fitness);
        }
    }
}

void MultiSearch::crowdingDistanceComplexity(int i, int f){
    if(i == f){
        pop[i]->crowdingDistance = INFINITY;
    }
    else{
        pop[i]->crowdingDistance = pop[f]->crowdingDistance = INFINITY;
        for(int j = i + 1; j < f; j++){
            pop[j]->crowdingDistance += (pop[j + 1]->complexity - pop[j - 1]->complexity)/(pop[f]->complexity - pop[i]->complexity);
        }
    }
}

void MultiSearch::CalcRank(int size){
    /**
        Juntar tudo
        selecionar 1s não dominados rank = 0
        aumentar rank dos outros

    **/

    sort(pop, pop + size, SortPopulationComplexity);
    sort(pop, pop + size, SortPopulationFitness);

    int ranking = 0;

    for(int i = 0; i < size; i++)
        pop[i]->ranking = 0;

    bool stop = false;
    while(!stop)
    {
        stop = true;
        for(int i = 0; i < size; i++)  // ele
        {
            if(pop[i]->ranking == ranking)   // se tiver na hora dele
            {
                for(int j = 0; j < size; j++)  // pra cada um candidato
                {
                    if(i != j && (pop[j]->ranking == ranking))  // se não for ele e se tiver no mesmo ranking
                    {
//                        if(pop[i]->fitness == pop[j]->fitness && pop[i]->complexity == pop[j]->complexity)
//                        {
//                            pop[i]->fitness = INFINITY;
//                        }
//                        else
                        if(Dominate(pop[i], pop[j]))  // se for dominado por j então ele aumenta o ranking
                        {
                            pop[j]->ranking = ranking + 1;
                            stop = false;
                        }
                    }
                }
            }
        }
        ranking++;
    }
    sort(pop, pop + size, SortPopulationRank);

    int i, f, r;
    i = f = r = 0;
    while(f <= conf->popSize){
        while(f < conf->popSize && pop[f]->ranking == r)
            f++;

        sort(pop + i, pop + f, SortPopulationFitness);
        crowdingDistanceFitness(i, f - 1);
        sort(pop + i, pop + f, SortPopulationComplexity);
        crowdingDistanceComplexity(i, f - 1);

        if(f > conf->popSize)
            sort(pop + i, pop + f - 1, SortPopulationCrowding);

        i = f;
        f++;
        r++;
    }

    int ii = 0; int jj = 1;
    while(jj < size){
//        cout << pop[ii]->fitness << "," << pop[ii]->complexity << " e " <<  pop[jj]->fitness << "," << pop[jj]->complexity << " = ";
        if(pop[ii]->fitness == pop[jj]->fitness && pop[ii]->complexity == pop[jj]->complexity){
            pop[jj]->ranking = INFINITY;
//            cout << 1 << endl;
        }
        else{
            ii = jj;
//            cout << 2 << endl;
        }
        jj++;
    }

    stable_sort(pop, pop + size, SortPopulationRank);
}

void MultiSearch::Replace(){
    stable_sort(pop, pop + conf->popSize * 2, SortPopulationFitness);
}

/**
    Parameters
        int initialIndex - indice inicial a ser avaliado
        int finalIndex - indice final a pop sera avaliada até o finalIndex -1
    Percorre o vetor pop (população) avaliando os individuos do indice initialIndex até finalIndex
    @return void
**/
void MultiSearch::EvaluatePopulation(int initialIndex, int finalIndex, bool optimize){
    #pragma omp parallel for num_threads(conf->NUM_THREADS)
    for(int i = initialIndex; i < finalIndex; i++){
        if(optimize){
//            cout << "   Entrou no Otimizador!" << endl;
            Subject* ind_ = pop[i]->clone();
            ind_->fitness = parser->Evaluate(ind_);
            parser->Optimize(ind_);
            pop[i]->print();
            ind_->print();
            if(ind_->fitness < pop[i]->fitness){
//                cout << "   Houve melhora" << endl;
//                cout << pop[i]->fitness << " " << ind_->fitness << endl;
                swap(pop[i], ind_);
            }
            delete ind_;
        }
        else{
            pop[i]->fitness = parser->Evaluate(pop[i]);
        }
        pop[i]->complexity = 0;
        for(Tree* t : pop[i]->trees)
            pop[i]->complexity += t->linearModel.size();
        conf->evaluations++;
//        cout << "   " << i << " " << pop[i]->fitness << endl;
    }
}

void MultiSearch::EvaluatePopulationValidation(int initialIndex, int finalIndex, int optimizeRange){
    #pragma omp parallel for num_threads(conf->NUM_THREADS)
    for(int i = initialIndex; i < finalIndex; i++){
        if(i < optimizeRange && !pop[i]->optimized){
            Subject* ind_ = pop[i]->clone();
//            ind_->trees[0]->replaceAllConst();
            parserValidation->Optimize(ind_);
            ind_->fitness = parser->Evaluate(ind_);
            if(ind_->fitness < pop[i]->fitness){
//                cout << ind_->fitness << " " << pop[i]->fitness << endl;
//                cin.get();
                swap(pop[i], ind_);
                delete ind_;
            }
        }
        else{
            pop[i]->fitness = parser->Evaluate(pop[i]);
        }
        conf->evaluations++;
//        cout << "   " << i << " " << pop[i]->fitness << endl;
    }

//    #pragma omp parallel for num_threads(conf->NUM_THREADS)
//    for(int i = initialIndex; i < finalIndex; i++){
//        parser->Optimize(pop[i]);
//        pop[i]->fitness = parser->Evaluate(pop[i]);
//        conf->evaluations++;
////        cout << "   " << i << " " << pop[i]->fitness << endl;
//    }
}

/**
    Aplica os operadores genéticos de mutação e cruzamento
    @return void
**/
void MultiSearch::Operate(){
    int num_individuos = seletor->tam_selection;
    Subject ** selecionados;

    //#pragma omp parallel for private(selecionados)
    for(int i = conf->popSize; i < conf->popSize * 2; i += num_individuos){

        selecionados = new Subject*[num_individuos];

        for(int j = 0; j < num_individuos; j++){
            selecionados[j] = seletor->selects(pop)->clone();
            pop[i+j] = selecionados[j];
        }

        crossoverOperator->cross(selecionados,num_individuos);

        for(int j = 0; j < num_individuos; j++){
            mutationOperator->mutate(selecionados[j]);
        }

        for(int j = 0; j < num_individuos; j++){
            selecionados[j] = NULL;
        }
        delete selecionados;
    }

}


/**
    Set Parameters
**/
void MultiSearch::setParser(Parser * parser){
  if(parser==NULL){
        SimpleParser * avaliador  = new SimpleParser();
        avaliador->setDataSet(data->training,data->totalTraining);
        this->parser = avaliador;
    }else{
        this->parser = parser;
        this->parserTraining = parser;
    }
}

void MultiSearch::setPopulationReplace(PopulationReplacement * popReplace){
    if(popReplace == NULL){
        this->replacer = new SimpleReplace();
    }else{
        this->replacer = popReplace;
    }
}

void MultiSearch::setIndividuoBuilder(IndividuoBuilder * indBuilder){
    if(indBuilder == NULL){
        this->individuoBuilder = new SimpleIndividuoBuilder();
    }else{
        this->individuoBuilder = indBuilder;
    }
}

void MultiSearch::setMutation(Mutation * opMutation){
    if(opMutation==NULL){
        mutationOperator = new SimpleMutation();
    }else{
        mutationOperator = opMutation;
    }
}

void MultiSearch::setCrossover(Crossover * opCrossover){
    if(opCrossover==NULL){
        crossoverOperator = new SimpleCrossover();
    }else{
        crossoverOperator = opCrossover;
    }
}

void MultiSearch::setSelection(Selection * seletor){
    if(seletor==NULL){
        this->seletor = new TournamentSelection();
    }else{
        this->seletor = seletor;
    }
}

/** Print Functions **/
void MultiSearch::printParameters(){
    cout << endl;
    cout << "************* PARAMETERS **************" << endl;
    cout << "*   GENERATIONS    " << conf->generations << endl;
    cout << "*   POP SIZE       " << conf->popSize << endl;
    cout << "*   ELITISM        " << conf->elitism << endl;
    cout << "*   MAXDEEP        " << conf->MAXDEEP << endl;
    cout << "*   CROSSOVER RATE " << conf->crossoverRate << endl;
    cout << "*   MUTATION RATE  " << conf->mutationRate << endl;
    cout << "*   INDIVIDUO      " << individuoBuilder->nameIndividuo() << endl;
    cout << "*   PARSER         " << parser->nameParser() << endl;
    cout << "***************************************" << endl << endl;
}

void MultiSearch::printResult(){
    cout << "--------------Results--------------" << endl;
    pop[0]->print();
//    cout <<  "SERIE TREINO" << endl;
    parser->printResult(pop[0]);
//    cout <<  "\nSERIE VALIDAÇÃO" << endl;
    parserValidation->printResult(pop[0]);
//    cout <<  "\n\nSERIE TESTE" << endl;
    parserTest->printResult(pop[0]);
}

void MultiSearch::printBestIndividuo(){

        pop[0]->print();
//        cout << " (" << pop[0]->fitness << ") " << endl;
//        for(int m = 0; m < conf->numTree; m++){
//            cout << "   f" << m << "() = (" << pop[0]->trees[m]->fitness << ") ";
//            pop[0]->trees[m]->root->print();
////            cout << "leastSquare=[";
////            for(int j = 0; j < pop[0]->trees[m]->leastSquareSize; j++)
////                cout << pop[0]->trees[m]->leastSquare[j] << ", ";
////            cout << "]";
//        }
        cout << endl;
}

 bool MultiSearch::SortPopulationFitness(Subject* a, Subject* b){
     //cout << "FITNESS: " << a->trees[0]->fitness << " - " << b->trees[0]->fitness << endl;
     return a->trees[0]->fitness < b->trees[0]->fitness;
 }

 bool MultiSearch::SortPopulationFitnessValidation(Subject* a, Subject* b){
    return a->fitnessValidation < b->fitnessValidation;
 }

MultiSearch::~MultiSearch()
{
    //dtor
}
