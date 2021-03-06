#include "Message.h"

namespace vidrevolt {
    namespace midi {
        Message::Message(std::vector<unsigned char> message) : message_(message) {
        }

        MessageType Message::getType() const {
            unsigned char type = message_[0];

            switch(type & 0xf0) {
                case 0x80: return MESSAGE_TYPE_NOTE_OFF;
                case 0x90: return MESSAGE_TYPE_NOTE_ON;
                case 0xB0: return MESSAGE_TYPE_CONTROL;
            }

            return MESSAGE_TYPE_UNKNOWN;
        }

        std::string Message::str() const {
            return "Message(channel=" + std::to_string(static_cast<int>(getChannel())) +
                ", note=" + std::to_string(static_cast<int>(getNote())) +
                ", value=" + std::to_string(static_cast<int>(getValue())) +
                ", type=" + std::to_string(static_cast<int>(getType())) +
                ")";
        }

        unsigned char Message::getChannel() const {
            return message_[0] & 0x0f;

        }

        // Retrieves the note number. Available for MESSAGE_TYPE_NOTE_ON/OFF.
        unsigned char Message::getNote() const {
            return message_[1];
        }

        // Retrieves the velocity of a note. Available for MESSAGE_TYPE_NOTE_ON/OFF.
        unsigned char Message::getVelocity() const {
            return message_[2];
        }

        // Retrieves the function of a control event. Available for MESSAGE_TYPE_CONTROL.
        unsigned char Message::getFunction() const {
            return message_[1];
        }

        // Retrieves the value of a control event. Available for MESSAGE_TYPE_CONTROL.
        // Note: Does not take into consideration LSB vs MSB.
        unsigned char Message::getValue() const {
            return message_[2];
        }
    }
}
