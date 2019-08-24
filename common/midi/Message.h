#ifndef FRAG_MIDI_MESSAGE_H_
#define FRAG_MIDI_MESSAGE_H_

// STL
#include <vector>
#include <string>

namespace vidrevolt {
    namespace midi {
        enum MessageType {
            MESSAGE_TYPE_UNKNOWN,
            MESSAGE_TYPE_NOTE_ON,
            MESSAGE_TYPE_NOTE_OFF,
            MESSAGE_TYPE_CONTROL
        };

        class Message {
            public:
                explicit Message(std::vector<unsigned char> message);
                MessageType getType() const;
                unsigned char getVelocity() const;
                unsigned char getFunction() const;
                unsigned char getNote() const;
                unsigned char getValue() const;
                unsigned char getChannel() const;
                std::string str() const;

            private:
                std::vector<unsigned char> message_;
        };
    }
}
#endif
