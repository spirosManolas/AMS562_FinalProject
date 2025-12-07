#include <SFML/Graphics.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <optional>
#include <filesystem>   
#include <random>
#include "Population.hpp"

void drawLegend(sf::RenderWindow& window,
                const sf::Font& font,
                const Population& pop,
                float gridPixelSize,
                int step)
{
    const float panelX = gridPixelSize + 20.f;
    float y = 20.f;

    if (!font.getInfo().family.empty()) {
        sf::Text title(font);
        title.setString("Legend");
        title.setCharacterSize(20);
        title.setFillColor(sf::Color::White);
        title.setPosition({panelX, y});
        window.draw(title);
    }
    y += 40.f;

    auto stateColor = [](const std::string& s) {
        if (s == "infected")    return sf::Color(255, 182, 193);
        if (s == "recovered")   return sf::Color(173, 216, 230);
        if (s == "susceptible") return sf::Color(255, 239, 186);
        if (s == "vaccinated")  return sf::Color(152, 251, 152);
        return sf::Color(240, 240, 240);
    };

    Population::Counts c = pop.countStates();

    struct Entry { const char* name; int count; std::string key; };
    Entry entries[] = {
        {"Susceptible", c.susceptible, "susceptible"},
        {"Infected",    c.infected,    "infected"},
        {"Recovered",   c.recovered,   "recovered"},
        {"Vaccinated",  c.vaccinated,  "vaccinated"}
    };

    for (const auto& e : entries) {
        sf::RectangleShape box({20.f, 20.f});
        box.setFillColor(stateColor(e.key));
        box.setPosition({panelX, y});
        window.draw(box);

        if (!font.getInfo().family.empty()) {
            std::ostringstream oss;
            oss << e.name << " : " << e.count;

            sf::Text txt(font);
            txt.setString(oss.str());
            txt.setCharacterSize(16);
            txt.setFillColor(sf::Color::White);
            txt.setPosition({panelX + 30.f, y - 3.f});
            window.draw(txt);
        }

        y += 35.f;
    }

    if (!font.getInfo().family.empty()) {
        y += 15.f;
        std::ostringstream oss;
        oss << "Step: " << step;

        sf::Text stepText(font);
        stepText.setString(oss.str());
        stepText.setCharacterSize(18);
        stepText.setFillColor(sf::Color::White);
        stepText.setPosition({panelX, y});
        window.draw(stepText);
    }
}

int main()
{
    namespace fs = std::filesystem;

    const int   gridSize      = 100;
    const float cellSize      = 20;
    const float gap           = 1;
    const float stepSeconds   = 0.25;
    const int   maxSteps      = 1000;

    const std::string framesDir = "frames";
    std::error_code fsErr;
    if (!fs::exists(framesDir, fsErr)) {
        if (!fs::create_directory(framesDir, fsErr)) {
            std::cerr << "Error: could not create directory '" << framesDir
                      << "': " << fsErr.message() << "\n";
            return 1;
        }
    }


Population pop(gridSize);

std::mt19937 rng(std::random_device{}());
std::uniform_real_distribution<float> dist(0.0, 1.0);

float infectionProbability = 0.75;

int start = 25;
int end   = 75;  

for (int i = start; i < end; ++i) {
    for (int j = start; j < end; ++j) {
        if (dist(rng) < infectionProbability) {
            pop.set_inf(i, j);
        }
    }
}

    std::ofstream csv("state_counts.csv");
    if (!csv) {
        std::cerr << "Error: could not open state_counts.csv for writing.\n";
        return 1;
    }
    csv << "step,susceptible,infected,recovered,vaccinated\n";

    {
        Population::Counts c0 = pop.countStates();
        csv << 0 << ','
            << c0.susceptible << ','
            << c0.infected    << ','
            << c0.recovered   << ','
            << c0.vaccinated  << '\n';
    }

    float gridPixelSize = gap + gridSize * (cellSize + gap);

    const unsigned legendWidth = 260;
    unsigned windowWidth  = static_cast<unsigned>(gridPixelSize) + legendWidth;
    unsigned windowHeight = static_cast<unsigned>(gridPixelSize);

    sf::RenderWindow window(
        sf::VideoMode({windowWidth, windowHeight}),
        "Epidemic Simulation",
        sf::Style::Titlebar | sf::Style::Close
    );
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        std::cerr << "Warning: could not open font 'arial.ttf'. "
                  << "Legend text will not be shown.\n";
    }

    sf::Clock stepClock;
    int  step = 0;
    bool shouldSaveFrame = true; 

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* keyPressed =
                           event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    window.close();
            }
        }

        
        if (step < maxSteps &&
            stepClock.getElapsedTime().asSeconds() >= stepSeconds) {
            pop.Update();
            ++step;
            stepClock.restart();
            shouldSaveFrame = true;

            Population::Counts c = pop.countStates();
            csv << step << ','
                << c.susceptible << ','
                << c.infected    << ','
                << c.recovered   << ','
                << c.vaccinated  << '\n';
        }

        
        pop.draw(window, cellSize, gap); 
        drawLegend(window, font, pop, gridPixelSize, step);
        window.display();

        if (shouldSaveFrame) {
            sf::Texture texture({window.getSize()});
            texture.update(window);
            sf::Image screenshot = texture.copyToImage();

            std::ostringstream name;
            name << framesDir << "/frame_"
                 << std::setw(4) << std::setfill('0') << step
                 << ".png";

            if (!screenshot.saveToFile(name.str())) {
                std::cerr << "Failed to save frame: " << name.str() << "\n";
            } else {
                std::cout << "Saved " << name.str() << "\n";
            }

            shouldSaveFrame = false;
        }
    }

    return 0;
}
