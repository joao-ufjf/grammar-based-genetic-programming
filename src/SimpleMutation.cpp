#include "SimpleMutation.h"

SimpleMutation::SimpleMutation() {
    //ctor
}

void SimpleMutation::mutate(Subject * individuo) {
    individuo->complexity = 0;
    for(Tree* t : individuo->trees) {
        if(checkRate(conf->mutationRate)) {
            auxMutate(t);
            individuo->complexity += t->linearModel.size();
        }
    }
}

void SimpleMutation::auxMutate(Tree* t) {
    No* n = t->subTree();
    n->erase();
    grammar->derivate(n);
    t->update();
}

SimpleMutation::~SimpleMutation() {
    //dtor
}
