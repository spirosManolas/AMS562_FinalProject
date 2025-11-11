/**
 * @file Population.hpp
 * @brief Declaration & implementation of the Population class for epidemic modeling
 */

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <iostream>
#include <vector>
#include "Person.hpp"
#include <string>
#include <random>

/**
 * @class Population
 * @brief Represents the class Population, an n*n Matrix of People among which disease spread will be modeled
 */
class Population{
private:
    std::vector<std::vector<Person>> _m; /** <The n*n matrix m which holds elements of type Person */
    int _n; /** <This represents that the matrix is n*n */
    float _ri = 0.20; /** < This represents the infection rate */
    float _rr = 1.0/20.0; /* < This represents the recovery rate*/
    float _rm = 1.0/200.0; /* <This represents the mutaiton rate*/

public:
    /**
     * @brief Parameterized constructor initializes a matrix m of size n*n which holds elements of type T. All elements are initially set to susceptible people
     * @param n size of matrix
     */
    Population(int n) : _n(n) {
        std::vector<Person> vec(n, Person()); // initializing an n-element vector of Person() objects
        std::vector<std::vector<Person>> _m(n, vec); // intializing a n*n matrix of Person() objects     
    }

    //Accesors
    Person getPerson(int i, int j) const {return _m[i][j];}
    std::string getState(int i, int j) const {return _m[i][j].getState();}
    int size() const {return _n;}

    //Mutators
    void set_sus(int i, int j) {_m[i][j].set_sus();}
    void set_inf(int i, int j) {_m[i][j].set_inf();}
    void set_rec(int i, int j) {_m[i][j].set_rec();}
    void set_vac(int i, int j) {_m[i][j].set_vac();}

    /**
     * @brief Updates the state of the population according to our Markov Chain model
     */
    void Update(){
        std::vector<std::vector<Person>> mOld = _m; //copying old state

        std::random_device rd;  // Seed for random number 
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
        std::uniform_real_distribution<> dis(0.0, 1.0); //generating U(0,1) for future probabilities


        for (int i = 0; i < _n; i++){
            for (int j = 0; j < _n; j++){
                if (mOld[i][j].getState() == "susceptible"){ //update for susceptible Persons
                    //finding number of infected neighbors
                    int sum = 0;
                    if (i-1 >= 0 && mOld[i-1][j].getState() == "infected"){
                        sum += 1;
                    }
                    if (j-1 >= 0 && mOld[i][j-1].getState() == "infected"){
                        sum += 1;
                    }
                    if (i+1 < _n && mOld[i+1][j].getState() == "infected"){
                        sum += 1;
                    }
                    if (j+1 < _n && mOld[i][j+1].getState() == "infected"){
                        sum += 1;
                    }
                    float chance = sum*_ri; //chance of infection = number of infected neighbors * infection rate
                    if (dis(gen) < chance){
                        _m[i][j].set_inf();
                    }
                }
                if (mOld[i][j].getState() == "infected") { //update for infected Persons
                    if (dis(gen) < _rr){ //with a recovery rate % chance, set the Person to recovered
                        _m[i][j].set_rec();
                    }
                }
                if (mOld[i][j].getState() == "recovered") { //update for recovered Persons
                    if (dis(gen) < _rm){ //with a mutation rate % chance, set the Person to susceptible
                        _m[i][j].set_sus();
                    }
                }
            }
        }
    }

};

#endif //POPULATION_HPP