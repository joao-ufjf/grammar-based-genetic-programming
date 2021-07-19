#ifndef TOURNAMENTSELECTION_H
#define TOURNAMENTSELECTION_H
#include "Selection.h"
#include "Subject.h"

class TournamentSelection : public Selection {
  public:
    TournamentSelection();
    virtual Subject * selects(Subject ** pop) {
        int a = rand()%conf->popSize;
        int b = rand()%conf->popSize;

        Subject * x = pop[a];
        Subject * y = pop[b];

        if(x->ranking < y->ranking) {
//            cout << "Selec " << a << endl;
//            cin.get();
            return x;
        }
        else if (x->ranking == y->ranking && x->crowdingDistance >= y->crowdingDistance){
//            cout << "Selec " << a << endl;
//            cin.get();
            return x;
        }
//        cout << "Selec " << b << endl;
//        cin.get();
        return y;
    }
    virtual ~TournamentSelection();

  protected:

  private:
};

#endif // TOURNAMENTSELECTION_H
