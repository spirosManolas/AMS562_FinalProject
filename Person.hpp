/**
 * @file Person.hpp
 * @brief Declaration & implementation of the Template Person class for epidemic modeling.
 */

#ifndef Person_HPP
#define Person_HPP

#include <iostream>
#include <string>

/**
 * @class Person
 * @brief The Person class, which represents a person in our epidemic disease model
 */
class Person{
private:
    std::string _state; /** <This represents the state of the perosn, which can be "susceptible", "infected", "recovered", or "vaccinated" */

public:
    /**
     * @brief Default empty constructor initializes a person with _state="susceptible"
     */
    Person() : _state("susceptible") {}

    //Accesors
    std::string getState() const {return _state;}

    //Mutators
    void set_sus() {_state = "susceptible";}
    void set_inf() {_state = "infected";}
    void set_rec() {_state = "recovered";}
    void set_vac() {_state = "vaccinated";}

};



#endif // PERSON_HPP