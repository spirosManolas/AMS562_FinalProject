#include <SFML/Graphics.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>      
#include <optional>

#include "Population.hpp"

// Draw legend + counts + step info on the right side
void drawLegend(sf::RenderWindow& window,
                const sf::Font& font,
                const Population& pop,
                float gridPixelSize,
                int step)
{
    const float panelX = gridPixelSize + 20.f;
    float y = 20.f;

    // Title
    if (!font.getInfo().family.empty()) {
        sf::Text title(font);
        title.setString("Legend");
        title.setCharacterSize(20);
        title.setFillColor(sf::Color::White);
        title.setPosition({panelX, y});
        window.draw(title);
    }
    y += 40.f;

    // Local helper for colors (mirror of Population::colorForState)
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

    // Step number
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
    // ==== Simulation parameters ====
    const int   gridSize      = 30;
    const float cellSize      = 20.f;
    const float gap           = 1.f;
    const float stepSeconds   = 0.25f;
    const int   maxSteps      = 1000;

    Population pop(gridSize);

    // Seed a few initial infections
    pop.set_inf(15, 15);
    pop.set_inf(15 , 16);
    pop.set_inf(16, 15);

    // === NEW: open CSV and write header ===
    std::ofstream csv("state_counts.csv");
    if (!csv) {
        std::cerr << "Error: could not open state_counts.csv for writing.\n";
        return 1;
    }
    csv << "step,susceptible,infected,recovered,vaccinated\n";

    // log initial state (step 0)
    {
        Population::Counts c0 = pop.countStates();
        csv << 0 << ','
            << c0.susceptible << ','
            << c0.infected    << ','
            << c0.recovered   << ','
            << c0.vaccinated  << '\n';
    }
    // === end NEW CSV init ===

    // Pixel size of the grid area
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

    // Load a font
    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        std::cerr << "Warning: could not open font 'arial.ttf'. "
                  << "Legend text will not be shown.\n";
    }

    sf::Clock stepClock;
    int  step = 0;
    bool shouldSaveFrame = true; // save initial state

    while (window.isOpen()) {
        // --- Event loop (SFML 3 style) ---
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* keyPressed =
                           event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    window.close();
            }
        }

        // Advance simulation at fixed time step
        if (step < maxSteps &&
            stepClock.getElapsedTime().asSeconds() >= stepSeconds) {
            pop.Update();
            ++step;
            stepClock.restart();
            shouldSaveFrame = true;

            // === NEW: log counts after each update ===
            Population::Counts c = pop.countStates();
            csv << step << ','
                << c.susceptible << ','
                << c.infected    << ','
                << c.recovered   << ','
                << c.vaccinated  << '\n';
            // (csv will flush on close; you can also do csv.flush() occasionally)
            // === end NEW ===
        }

        // Draw current state
        pop.draw(window, cellSize, gap); // clears + draws grid
        drawLegend(window, font, pop, gridPixelSize, step);
        window.display();

        // Save timelapse frame after each step (including step 0)
        if (shouldSaveFrame) {
            sf::Texture texture({window.getSize()});
            texture.update(window);
            sf::Image screenshot = texture.copyToImage();

            std::ostringstream name;
            name << "frame_" << std::setw(4) << std::setfill('0') << step << ".png";
            if (!screenshot.saveToFile(name.str())) {
                std::cerr << "Failed to save frame: " << name.str() << "\n";
            } else {
                std::cout << "Saved " << name.str() << "\n";
            }

            shouldSaveFrame = false;
        }
    }

    // csv closes automatically here (RAII), flushing data to disk
    return 0;
}
