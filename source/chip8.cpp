#include "common.hpp"
#include "util.hpp"
#include "chip8.hpp"
#include "instruction.hpp"

namespace tsh {

    bool Display::Render(sf::RenderWindow &window) const {
        window.clear();

        auto rect = sf::RectangleShape(sf::Vector2f(PixelWidth, PixelWidth));
        rect.setFillColor(sf::Color::White);

        for (const auto x : std::views::iota(Coord{0}, Coord{DisplayWidth + 1})) {
            for (const auto y : std::views::iota(Coord{0}, Coord{DisplayHeight + 1})) {
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

    bool Chip8::LoadProgram(const std::span<const std::byte> data) {
        if (data.size() > ProgramSpace.Size()) {
            return false;
        }

        std::ranges::copy(data, this->memory.begin() + ProgramSpace.start);

        return true;
    }

    bool Chip8::LoadProgram(const std::string &path) {
        const auto fp = std::fopen(path.c_str(), "rb");
        if (fp == nullptr) {
            return false;
        }

        ON_SCOPE_EXIT { std::fclose(fp); };

        if (std::fseek(fp, 0, SEEK_END) != 0) {
            return false;
        }

        const auto offset = std::ftell(fp);
        if (offset < 0) {
            return false;
        }

        const auto size = static_cast<std::size_t>(offset);

        if (size > ProgramSpace.Size()) {
            return false;
        }

        if (std::fseek(fp, 0, SEEK_SET) != 0) {
            return false;
        }

        const auto data = std::make_unique<std::byte[]>(size);
        if (std::fread(data.get(), size, 1, fp) != 1) {
            return false;
        }

        return this->LoadProgram(std::span(data.get(), size));
    }

    bool Chip8::Tick() {
        const auto op = Opcode(this->ReadRawOpcode());

        const auto advance = Handler::Execute(*this, op);
        if (!advance.has_value()) {
            std::printf("Unhandled opcode: %04X\n", op.Get());

            return false;
        }

        this->PC.Increment(*advance * sizeof(Opcode));

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
