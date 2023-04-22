#include	<iostream>
#include	<string.h>
#include	"nut/Stream.hpp"

bool is_coord_member(ActNut::Object *obj)
{
    const uint8_t OP_APPENDARRAY = 0x22;
    const uint8_t APPEND_ARRAY_TYPE_INTEGER = 2;

    Nut::SQInstruction* instruction = dynamic_cast<Nut::SQInstruction*>(obj);
    if (!instruction) {
        return false;
    }

    auto op   = dynamic_cast<ActNut::Number<uint8_t>*>((*instruction)["0"]);
    auto dest = dynamic_cast<ActNut::Number<uint8_t>*>((*instruction)["1"]);
    auto type = dynamic_cast<ActNut::Number<uint8_t>*>((*instruction)["3"]);
    if (!op || !dest || !type) {
        return false;
    }

    return *op == OP_APPENDARRAY && *dest == 3 && *type == APPEND_ARRAY_TYPE_INTEGER;
}

bool is_array_of_coords(ActNut::vector& instructions, size_t pos)
{
    if (pos + 3 > instructions.size()) {
        return false;
    }
    return is_coord_member(instructions[pos]) &&
           is_coord_member(instructions[pos + 1]) &&
           is_coord_member(instructions[pos + 2]);
}

int main(int ac, char** av)
{
    if (ac != 2) {
        std::cout << "Usage: " << av[0] << " in.nut" << std::endl;
        return 0;
    }

    std::unique_ptr<ActNut::Object> file;
    file.reset(Nut::readStream(av[1]));
    if (!file) {
        std::cerr << "File parsing failed." << std::endl;
        return 1;
    }

    auto instructions = dynamic_cast<ActNut::vector*>((*file)["Instructions"]);
    if (!instructions) {
        std::cerr << "Couldn't find instructions" << std::endl;
        return 1;
    }

    std::cout << "{" << std::endl;
    for (size_t i = 0; i < instructions->size(); ) {
        if (is_array_of_coords(*instructions, i)) {
            auto instruction_x = (*instructions)[i + 1];
            // auto instruction_y = (*instructions)[i + 2];
            int32_t x = *dynamic_cast<ActNut::Number<int32_t>*>((*instruction_x)["2"]);
            // int32_t y = *dynamic_cast<ActNut::Number<int32_t>*>((*instruction_y)["2"]);
            
            // Dump x and let the translators choose which new value they want
            std::cout << "    \"/Instructions/" << i + 1 << "/2\": \"" << x << "\"," << std::endl;
            // Make the character be displayed off-screen, so that translators can ignore this new feature and keep using "Button 1"
            // std::cout << "    \"/Instructions/" << i + 1 << "/2\": \"" << 1000000 << "\"," << std::endl;
            // Increase y by 1 (for tutorial_xxx) or 3 (for tutorial_xxx2) because it looks nicer)
            // std::cout << "    \"/Instructions/" << i + 2 << "/2\": \"" << y + 1 << "\"," << std::endl;
            i += 3;
        }
        else {
            i++;
        }
    }
    std::cout << "}" << std::endl;

    return 0;
}
