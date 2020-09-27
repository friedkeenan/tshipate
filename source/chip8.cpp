#include <cstring>
#include <cstdint>
#include <cstdio>
#include <string>
#include <memory>
#include <random>
#include <mutex>
#include <thread>
#include <chrono>
#include <SFML/Graphics.hpp>

#include "chip8.hpp"
#include "instruction.hpp"
#include "util.hpp"

namespace tsh {

    using namespace std::chrono_literals;

    std::uint8_t Chip8::RandomByte() {
        return this->rand_dis(this->rand_gen);
    }

    Chip8::Key Chip8::KeyPressed() {
        return Key::Invalid;
    }

    bool Chip8::Draw(Address addr, Coord height, Coord x0, Coord y0) {
        bool ret = false;

        for (Coord y = y0; y < y0 + height; y++) {
            const auto sprite_row = static_cast<std::uint64_t>(this->memory[addr + y - y0]) << 56;
            const auto draw_row = (sprite_row >> x0) | (sprite_row << (64 - x0));

            if ((this->display[y] | draw_row) != (this->display[y] ^ draw_row)) {
                ret = true;
            }

            this->display[y] ^= draw_row;
        }

        return ret;
    }

    bool Chip8::ShouldStopTimers() {
        std::scoped_lock lk(this->should_stop_timers_mutex);

        return this->should_stop_timers;
    }

    void Chip8::SetShouldStopTimers(bool stop) {
        std::scoped_lock lk(this->should_stop_timers_mutex);

        this->should_stop_timers = stop;
    }

    void Chip8::StopTimers() {
        this->SetShouldStopTimers(true);

        this->timers_thread.join();
    }

    void Chip8::Reset() {
        std::memset(this->memory + ProgramSpace.start, 0, ProgramSpace.Size());

        this->pc.Set(ProgramSpace.start);

        for (auto &reg : this->V) {
            reg.Set(0);
        }

        this->I.Set(0);

        this->DT.Set(0);
        this->ST.Set(0);
        this->SetShouldStopTimers(false);

        while (!this->stack.empty()) {
            this->stack.pop();
        }

        std::memset(this->display, 0, sizeof(this->display));

        std::random_device rd;
        this->rand_gen.seed(rd());
    }

    void Chip8::Initialize() {
        // TODO: Put font data in memory

        this->Reset();

        this->timers_thread = std::thread([this]() {
            while (!this->ShouldStopTimers()) {
                this->DT.Decrement();
                this->ST.Decrement();

                std::this_thread::sleep_for(16.67ms); // 60 Hz
            }
        });

        this->window.create(sf::VideoMode(WindowWidth, WindowHeight, 1), "tshipate");
    }

    void Chip8::Exit() {
        this->window.close();
        this->StopTimers();
    }

    bool Chip8::LoadProgram(const std::uint8_t *data, std::size_t size) {
        if (size > ProgramSpace.Size()) {
            return false;
        }

        std::memcpy(this->memory + ProgramSpace.start, data, size);

        return true;
    }

    bool Chip8::LoadProgram(const std::string &path) {
        auto fp = std::fopen(path.c_str(), "rb");
        if (fp == nullptr) {
            return false;
        }

        ON_SCOPE_EXIT { std::fclose(fp); };

        if (std::fseek(fp, 0, SEEK_END) != 0) {
            return false;
        }

        std::size_t size = std::ftell(fp);
        if (size > ProgramSpace.Size()) {
            return false;
        }

        if (std::fseek(fp, 0, SEEK_SET) != 0) {
            return false;
        }

        auto data = std::make_unique<std::uint8_t[]>(size);

        if (std::fread(data.get(), size, 1, fp) != 1) {
            return false;
        }

        return this->LoadProgram(data.get(), size);
    }

    bool Chip8::Display() {
        if (this->window.isOpen()) {
            sf::Event event;

            while (this->window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    return false;
                }
            }

            this->window.clear();

            constexpr float PixelWidth = WindowWidth / 64.0f;
            auto rect = sf::RectangleShape(sf::Vector2f(PixelWidth, PixelWidth));
            rect.setFillColor(sf::Color::White);

            for (Coord x = 0; x < 64; x++) {
                for (Coord y = 0; y < 32; y++) {
                    if ((this->display[y] & (std::uint64_t(1) << (63 - x))) != 0) {
                        rect.setPosition(PixelWidth * x, PixelWidth * y);
                        this->window.draw(rect);
                    }
                }
            }

            this->window.display();

            return true;
        }

        return false;
    }

    bool Chip8::Tick() {
        auto op = Opcode<Handler::OpcodeType>((this->memory[this->pc.Get()] << 8) | this->memory[this->pc.Get() + 1]);

        auto advance = Handler::Execute(*this, op);
        if (advance == -1) {
            std::printf("Unhandled opcode: %04x\n", op.Get());
            return false;
        }

        this->pc.Advance(advance * sizeof(Handler::OpcodeType));

        return true;
    }

    void Chip8::Loop() {
        while (true) {
            if (!this->Tick()) {
                break;
            }

            if (!this->Display()) {
                break;
            }
        }
    }

}
