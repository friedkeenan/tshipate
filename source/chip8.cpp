#include "common.hpp"
#include "util.hpp"
#include "chip8.hpp"
#include "instruction.hpp"

namespace tsh {

    bool Display::Render(sf::RenderWindow &window) const {
        window.clear();

        auto rect = sf::RectangleShape(sf::Vector2f(PixelWidth, PixelWidth));
        rect.setFillColor(sf::Color::White);

        for (const auto x : std::views::iota(Coord{0}, DisplayWidth)) {
            for (const auto y : std::views::iota(Coord{0}, DisplayHeight)) {
                if (this->GetPixel(x, y)) {
                    rect.setPosition(PixelWidth * x, PixelWidth * y);
                    window.draw(rect);
                }
            }
        }

        window.display();

        return true;
    }

    bool Keyboard::HandleEvent(const sf::Event &event) {
        if (event.type == sf::Event::KeyPressed) {
            const auto key = KeyToInternal.KeyForValue(event.key.code);

            if (key.has_value()) {
                this->current_key = *key;
            }
        }

        return true;
    }

    bool Chip8::PropagateEvent(const sf::Event &event) {
        /*
            Wish I could use reflection to loop over attributes
            and see which can handle an event.
        */

        /* Don't use braced-if to require a semicolon after macro. */
        #define HANDLE_EVENT(attr) if (!this->attr.HandleEvent(event)) return false

        HANDLE_EVENT(keyboard);

        #undef HANDLE_EVENT

        return true;
    }

    bool Chip8::Tick() {
        const auto op = Opcode(this->ReadRawOpcode());

        const auto advance = Instructions::Execute(*this, op);
        if (!advance.has_value()) {
            fmt::print("Unhandled opcode: {:04X}\n", op.Get());

            return false;
        }

        this->PC.Increment(*advance * sizeof(Opcode));

        std::this_thread::sleep_for(InstructionDuration);

        return true;
    }

    void Chip8::Loop() {
        auto window = this->display.OpenWindow();

        this->DT.StartThread([this](std::stop_token token) {
            while (!token.stop_requested()) {
                if (this->DT.Get() != 0) {
                    this->DT.Decrement(1);
                }

                std::this_thread::sleep_for(FrameDuration);
            }
        });

        this->ST.StartThread([this](std::stop_token token) {
            while (!token.stop_requested()) {
                if (this->ST.Get() != 0) {
                    this->speaker.PlaySound();

                    this->ST.Decrement(1);
                }

                std::this_thread::sleep_for(FrameDuration);
            }
        });

        while (window.isOpen()) {

            const auto should_break = [&]() {
                /* Will be wholly changed by pollEvent call. */
                sf::Event event;
                while (window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed) {
                        return true;
                    }

                    if (!this->PropagateEvent(event)) {
                        return true;
                    }
                }

                return false;
            }();

            if (should_break) {
                break;
            }

            if (!this->Tick()) {
                break;
            }

            if (!this->display.Render(window)) {
                break;
            }
        }

        this->DT.RequestStop();
        this->DT.Join();

        window.close();
    }

}
