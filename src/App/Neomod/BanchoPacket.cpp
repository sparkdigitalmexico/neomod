// Copyright (c) 2023, kiwec, All rights reserved.

#include "BanchoPacket.h"
#include "BanchoProtocol.h"
#include "UniString.h"

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <string_view>
#include <string>

using std::string_view_literals::operator""sv;

void Packet::read_bytes(u8 *bytes, size_t n) {
    if(this->pos + n > this->size) {
        this->pos = this->size + 1;
    } else {
        memcpy(bytes, this->memory + this->pos, n);
        this->pos += n;
    }
}

u32 Packet::read_uleb128() {
    u32 result = 0;
    u32 shift = 0;
    u8 byte = 0;

    do {
        byte = this->read<u8>();
        result |= (byte & 0x7f) << shift;
        shift += 7;
    } while(byte & 0x80);

    return result;
}

std::string Packet::read_stdstring() {
    u8 empty_check = this->read<u8>();
    if(empty_check == 0) return {};

    u32 len = this->read_uleb128();
    u8 *str = new u8[len + 1];
    this->read_bytes(str, len);

    // convert arbitrary bytes to valid utf (sanity)
    std::string str_out{UniString::to_utf8(reinterpret_cast<const char *>(str), len)};
    delete[] str;

    return str_out;
}

MD5String Packet::read_hash_chars() {
    MD5String hash;

    u8 empty_check = this->read<u8>();
    if(empty_check == 0) return hash;

    u32 len = this->read_uleb128();
    if(len > 32) {
        len = 32;
    }

    this->read_bytes((u8 *)hash.data(), len);
    return hash;
}

MD5Hash Packet::read_hash_digest() {
    MD5Hash hash;

    u8 empty_check = this->read<u8>();
    if(empty_check == 0) return hash;

    u32 len = this->read_uleb128();
    if(len > 16) {
        len = 16;
    }

    this->read_bytes((u8 *)hash.data(), len);
    return hash;
}

void Packet::skip_string() {
    u8 empty_check = this->read<u8>();
    if(empty_check == 0) {
        return;
    }

    u32 len = this->read_uleb128();
    this->pos += len;
}

void Packet::write_bytes(u8 *bytes, size_t n) {
    assert(bytes != nullptr);

    if(this->pos + n > this->size) {
        this->memory = (unsigned char *)realloc(this->memory, this->size + n + 4096);
        assert(this->memory && "realloc failed");
        this->size += n + 4096;
        if(!this->memory) return;
    }

    memcpy(this->memory + this->pos, bytes, n);
    this->pos += n;
}

void Packet::write_uleb128(u32 num) {
    if(num == 0) {
        u8 zero = 0;
        this->write<u8>(zero);
        return;
    }

    while(num != 0) {
        u8 next = num & 0x7F;
        num >>= 7;
        if(num != 0) {
            next |= 0x80;
        }
        this->write<u8>(next);
    }
}

void Packet::write_hash_chars(const MD5String &hash_str) {
    this->write<u8>(0x0B);
    this->write<u8>(hash_str.length());
    this->write_bytes((u8 *)hash_str.data(), hash_str.length());
}

void Packet::write_hash_digest(const MD5Hash &hash_digest) {
    this->write<u8>(0x0B);
    this->write<u8>(hash_digest.length());
    this->write_bytes((u8 *)hash_digest.data(), hash_digest.length());
}

bool Packet::write_string_isnull(const char *str) {
    if(!str || str[0] == '\0') {
        const u8 zero = 0;
        this->write<u8>(zero);
        return true;
    }
    return false;
}

void Packet::write_string_nonnull(const char *str, size_t len) {
    const u8 empty_check = 0x0B;
    this->write<u8>(empty_check);

    this->write_uleb128(len);
    this->write_bytes((u8 *)str, len);
}
