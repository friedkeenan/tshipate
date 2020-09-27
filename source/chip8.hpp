#pragma once

#include <cstdint>
#include <concepts>
#include <stack>
#include <string>
#include <random>
#include <mutex>
#include <thread>
#include <SFML/Graphics.hpp>

#include "defines.hpp"
#include "instruction.hpp"

namespace tsh {

    template<typename T> requires std::integral<T>
    class Register {
        public:
            T value;

            constexpr ALWAYS_INLINE Register() : value(0) { }

            constexpr ALWAYS_INLINE const T &Get() const {
                return this->value;
            }

            constexpr ALWAYS_INLINE void Set(const T &value) {
                this->value = value;
            }
    };

    template<typename T> requires std::integral<T>
    class ProgramCounter {
        public:
            T value;

            constexpr ALWAYS_INLINE ProgramCounter() : value(0) { }

            constexpr ALWAYS_INLINE const T &Get() const {
                return this->value;
            }

            constexpr ALWAYS_INLINE void Set(const T &value) {
                this->value = value;
            }

            constexpr ALWAYS_INLINE void Advance(const T &delta) {
                this->Set(this->Get() + delta);
            }
    };

    template<typename T> requires std::integral<T>
    class Timer {
        public:
            T value;
            std::mutex mutex;

            constexpr ALWAYS_INLINE Timer() : value(0) { }

            ALWAYS_INLINE const T &Get() {
                std::scoped_lock lk(this->mutex);

                return this->value;
            }

            ALWAYS_INLINE void Set(const T &value) {
                std::scoped_lock lk(this->mutex);

                this->value = value;
            }

            ALWAYS_INLINE void Decrement() {
                std::scoped_lock lk(this->mutex);

                if (this->value > 0) {
                    this->value--;
                }
            }
    };

    template<typename T> requires std::integral<T>
    class AddressSpace {
        public:
            T start, end;

            constexpr ALWAYS_INLINE AddressSpace(const T &start, const T &end) : start(start), end(end) { }

            constexpr ALWAYS_INLINE T Size() const {
                return end - start;
            }
    };

    class Chip8 {
        public:
            enum class Key : std::uint8_t {
                Zero    = 0x00,
                One     = 0x01,
                Two     = 0x02,
                Three   = 0x03,
                Four    = 0x04,
                Five    = 0x05,
                Six     = 0x06,
                Seven   = 0x07,
                Eight   = 0x08,
                Nine    = 0x09,
                A       = 0x0A,
                B       = 0x0B,
                C       = 0x0C,
                D       = 0x0D,
                E       = 0x0E,
                F       = 0x0F,
                Invalid = 0x10,
            };

            using Address = std::uint16_t;
            using Coord = std::uint8_t;
            using RandomGenerator = std::mt19937;
            using RandomDistribution = std::uniform_int_distribution<std::uint8_t>;

            static constexpr auto TotalSpace   = AddressSpace<Address>(0x0000, 0x1000);
            static constexpr auto ProgramSpace = AddressSpace<Address>(0x0200, 0x1000);

            using Handler = InstructionHandler<
                CLR,
                RTS,
                JP_Addr,
                CALL,
                SE_Byte,
                SNE_Byte,
                LD_Byte,
                ADD_Byte,
                LD_V,
                SNE_V,
                LD_I,
                RND,
                DRW,
                SKP,
                SKNP,
                LD_V_DT,
                LD_DT_V,
                ADD_I_V
            >;

            std::uint8_t memory[TotalSpace.Size()];

            ProgramCounter<Address> pc;

            Register<std::uint8_t> V[0x10];
            Register<Address>      I;

            Timer<std::uint8_t> DT; // Delay timer
            Timer<std::uint8_t> ST; // Sound timer

            std::stack<Address> stack;

            std::uint64_t display[32]; // 64x32 display

            RandomGenerator rand_gen;
            RandomDistribution rand_dis;

            /* Not really emulation-specific stuff */
            static constexpr int WindowWidth = 640;
            static constexpr int WindowHeight = WindowWidth / 2;
            sf::RenderWindow window;

            std::mutex should_stop_timers_mutex;
            bool should_stop_timers;
            std::thread timers_thread;

            std::uint8_t RandomByte();
            Key KeyPressed();
            bool Draw(Address addr, Coord height, Coord x0, Coord y0);

            void TimersThread();
            bool ShouldStopTimers();
            void SetShouldStopTimers(bool stop);
            void StopTimers();

            void Reset();
            void Initialize();
            void Exit();

            bool LoadProgram(const std::uint8_t *data, std::size_t size);
            bool LoadProgram(const std::string &path);

            bool Display();

            bool Tick();
            void Loop();
    };

}
