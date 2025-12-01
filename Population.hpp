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
#include <SFML/Graphics.hpp>


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
    float _rm = 1.0/200.0; /* <This represents the mutation rate*/
    float _rv = 1.0/1000.0; /* < This represents the vaccination rate*/
    int _t = 0; /* <This represents the number of days elapsed*/
    int _tv = 200; /* <This represents the number of days until the vaccine is available*/
/**
 * @brief Map a state string to a display color.
 * @param s State string: "susceptible", "infected", "recovered", or "vaccinated".
 * @return A  @c sf::Color for the given state; light gray if unknown.
 */
    sf::Color colorForState(const std::string& s) const {
    // match the example’s pastel palette
    if (s == "infected")   return sf::Color(255, 182, 193); //  pink
    if (s == "recovered")  return sf::Color(173, 216, 230); //  blue
    if (s == "susceptible") return sf::Color(255, 239, 186); //  yellow
    if (s == "vaccinated") return sf::Color(152, 251, 152); // green
    return sf::Color(240, 240, 240);                        //  gray 
}
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
        _t = _t + 1; //Updating the day counter
        std::vector<std::vector<Person>> mOld = _m; //copying old state

        std::random_device rd;  // Seed for random number 
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
        std::uniform_real_distribution<> dis(0.0, 1.0); //generating U(0,1) for future probabilities


        for (int i = 0; i < _n; i++){
            for (int j = 0; j < _n; j++){
                seed = dis(gen) //the seed to determine which event happens for this person
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
                    float chance_inf = sum*_ri; //chance of infection = number of infected neighbors * infection rate
                    if (seed < chance_inf){
                        _m[i][j].set_inf();
                    } else if (_t >= _tv){
                        if (chance_inf < seed && seed < chance_inf + _rv){
                            _m[i][j].set_vac();
                        }
                    }
                }
                if (mOld[i][j].getState() == "infected") { //update for infected Persons
                    if (seed < _rr){ //with a recovery rate % chance, set the Person to recovered
                        _m[i][j].set_rec();
                    }
                }
                if (mOld[i][j].getState() == "recovered") { //update for recovered Persons
                    if (seed < _rm){ //with a mutation rate % chance, set the Person to susceptible
                        _m[i][j].set_sus();
                    } else if (_t > _tv){
                        if (_rm < seed && seed < _rm + _rv){
                            [i][j].set_vac();
                    }
                }
            }
        }

   

    }
    /**
    * @brief Aggregate counts of each epidemiological state in the grid.
    */
    struct Counts {
    int susceptible = 0;
    int infected = 0;
    int recovered = 0;
    int vaccinated = 0;
    };

    Counts countStates() const {
        Counts c;
        for (int i = 0; i < _n; ++i) {
            for (int j = 0; j < _n; ++j) {
                const auto& s = _m[i][j].getState();
                if      (s == "susceptible") ++c.susceptible;
                else if (s == "infected")    ++c.infected;
                else if (s == "recovered")   ++c.recovered;
                else if (s == "vaccinated")  ++c.vaccinated;
        }
    }
    return c;
}
/**
 * @brief Render the grid to an SFML window using state-dependent colors.
 * @param window Target @c sf::RenderWindow to draw into.
 * @param cellSize Side length of each square cell in pixels.
 * @param gap Spacing between adjacent cells in pixels.
 *
 * The background is cleared to a dark gray and each cell is drawn as a filled rectangle.
 */
    void draw(sf::RenderWindow& window, float cellSize = 25.f, float gap = 1.f) const {
        const unsigned bgR = 40, bgG = 40, bgB = 40; 
        window.clear(sf::Color(bgR, bgG, bgB));

        sf::RectangleShape cell({cellSize, cellSize});
        const int n = _n;

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                float x = gap + j * (cellSize + gap);
                float y = gap + i * (cellSize + gap);
                cell.setPosition({x, y});
                cell.setFillColor(colorForState(_m[i][j].getState()));
                window.draw(cell);
        }
    }
}
    

};    


#endif // POPULATION_HPP