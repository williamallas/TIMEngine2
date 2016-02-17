#ifndef INPUTMANAGER_H_INCLUDED
#define INPUTMANAGER_H_INCLUDED

#include "Vector.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
    template <typename KeyCode, typename ScanCode>
    class InputManager
    {
    public:

        enum MouseButton
        {
            LEFT=0,
            RIGHT,
            MIDDLE,
            X1,
            X2,
        };

        struct State
        {
            bool pressed;
            bool firstPress, firstRelease;
        };
        const State NO_STATE = {false,false,false};
        const State STATE_PRESS = {true,true,false};
        const State STATE_RELEASE = {false,false,false};

        InputManager()
        {
            for(size_t i=0 ; i<5 ; ++i)
                _mouseState[i]=NO_STATE;

            _quit = false;
        }

        virtual void getEvent() = 0;

        const State& keyState(KeyCode k) const
        {
            auto s=_keyState.find(k);
            if(s == _keyState.end())
                return NO_STATE;
            else return s->second;
        }

        const State& physicalKeyState(ScanCode k) const
        {
            auto s=_physicalState.find(k);
            if(s == _physicalState.end())
                return NO_STATE;
            else return s->second;
        }

        const State& mouseState(MouseButton mb) const { return _mouseState[mb]; }
        const ivec2 mousePos() const { return _mousePos; }
        const ivec2 mouseRel() const { return _mouseRel; }
        const ivec2 mouseWheel() const { return _mouseWheel; }
        bool quitState() const { return _quit; }

    protected:
        std::map<KeyCode, State> _keyState;
        std::map<ScanCode, State> _physicalState;

        ivec2 _mousePos, _mouseRel;
        State _mouseState[5];
        ivec2 _mouseWheel;
        bool _quit;
    };
}
}
#include "MemoryLoggerOff.h"

#endif // INPUTMANAGER_H_INCLUDED
