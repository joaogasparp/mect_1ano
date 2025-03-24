
#include "utf-8.h"

int utf8_decode(UTF8DecoderState &state, uint16_t byte, uint32_t *codepoint)
{
    // vai ver se o byte é de continuacão ou não
    if (state.remaining_bytes == 0)
    {
        // se é 1 byte
        if (byte <= 0x7F)
        {
            *codepoint = byte;
            return 1;
        }
        // se é 2 bytes
        else if ((byte & 0xE0) == 0xC0)
        {
            state.codepoint = byte & 0x1F;
            state.remaining_bytes = 1;
            state.number_bytes = 2;
        }
        // se é 3 bytes
        else if ((byte & 0xF0) == 0xE0)
        {
            state.codepoint = byte & 0x0F;
            state.remaining_bytes = 2;
            state.number_bytes = 3;
        }
        // se é 4 bytes
        else if ((byte & 0xF8) == 0xF0)
        {
            state.codepoint = byte & 0x07;
            state.remaining_bytes = 3;
            state.number_bytes = 4;
        }
        // se não é nada
        else
        {
            return -1;
        }
        return 0;
    }
    // byte de continuação
    else
    {
        // tem de começar por 10
        if ((byte & 0xC0) != 0x80)
        {
            return -1;
        }
        // adiciona os bits ao codepoint
        state.codepoint = (state.codepoint << 6) | (byte & 0x3F);
        state.remaining_bytes--;
        if (state.remaining_bytes == 0)
        {
            *codepoint = state.codepoint;
            return state.number_bytes;
        }
        return 0;
    }
}

bool utf8_is_letter(uint32_t codepoint)
{
    // basic latin letters
    if ((codepoint >= 0x41 && codepoint <= 0x5A) ||
        (codepoint >= 0x61 && codepoint <= 0x7A))
        return true;
    // extended latin and european letters
    if ((codepoint >= 0xC0 && codepoint <= 0xFF) ||
        (codepoint >= 0x100 && codepoint <= 0x17F) ||
        (codepoint >= 0x180 && codepoint <= 0x24F) ||
        (codepoint >= 0x250 && codepoint <= 0x2AF))
        return true;
    // other scripts
    if ((codepoint >= 0x370 && codepoint <= 0x3FF) ||
        (codepoint >= 0x400 && codepoint <= 0x4FF) ||
        (codepoint >= 0x500 && codepoint <= 0x52F))
        return true;
    // cjk (chinese, japanese, korean)
    if (codepoint >= 0x4E00 && codepoint <= 0x9FFF)
        return true;
    return false;
}

bool utf8_is_space(uint32_t codepoint)
{
    // ascii whitespace
    if (codepoint == 0x0009 || codepoint == 0x000A || codepoint == 0x000B ||
        codepoint == 0x000C || codepoint == 0x000D || codepoint == 0x0020)
        return true;
    // unicode spaces
    if (codepoint == 0x00A0 || codepoint == 0x2007 || codepoint == 0x202F || codepoint == 0x2060)
        return true;
    return false;
}
