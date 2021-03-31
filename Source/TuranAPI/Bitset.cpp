#include "Bitset.h"
#include <iostream>

Bitset::Bitset(size_t byte_length) : buffer_length(byte_length) {
	boolarray = new bool[byte_length];
}
void Bitset::SetBit_True(const size_t index) {
	if (index / 8 > buffer_length - 1) {
		std::cout << "There is no such bit, maximum bit index: " << (buffer_length * 8) - 1 << std::endl;
		return;
	}
	char& byte = ((char*)boolarray)[index / 8];
	unsigned int bitindex = (index % 8);
	byte = (byte | char(1 << bitindex));
}
void Bitset::SetBit_False(const size_t index) {
	char& byte = ((char*)boolarray)[index / 8];
	char bitindex = (index % 8);
	switch (bitindex) {
	case 0:
		byte = (byte & 254);
		break;
	case 1:
		byte = (byte & 253);
		break;
	case 2:
		byte = (byte & 251);
		break;
	case 3:
		byte = (byte & 247);
		break;
	case 4:
		byte = (byte & 239);
		break;
	case 5:
		byte = (byte & 223);
		break;
	case 6:
		byte = (byte & 191);
		break;
	case 7:
		byte = (byte & 127);
		break;
	}
}
bool Bitset::GetBit_Value(const size_t index) const {
	char byte = ((char*)boolarray)[index / 8];
	char bitindex = (index % 8);
	return byte & (1 << bitindex);
}
unsigned int Bitset::GetByte_Length() const {
	return buffer_length;
}
unsigned int Bitset::GetIndex_FirstTrue() const {
	unsigned int byteindex = 0;
	for (unsigned int byte_value = 0; byte_value == 0; byteindex++) {
		byte_value = ((char*)boolarray)[byteindex];
	}
	byteindex--;
	bool found = false;
	unsigned int bitindex = 0;
	for (bitindex = 0; !found; bitindex++) {
		found = GetBit_Value((byteindex * 8) + bitindex);
	}
	bitindex--;
	return (byteindex * 8) + bitindex;
}
unsigned int Bitset::GetIndex_FirstFalse() const {
	unsigned int byteindex = 0;
	for (unsigned int byte_value = 255; byte_value == 255; byteindex++) {
		byte_value = ((char*)boolarray)[byteindex];
	}
	byteindex--;
	bool found = false;
	unsigned int bitindex = 0;
	for (bitindex = 0; !found; bitindex++) {
		found = !GetBit_Value((byteindex * 8) + bitindex);
	}
	bitindex--;
	return (byteindex * 8) + bitindex;
}
void Bitset::Clear(bool zero_or_one) {
	memset(boolarray, zero_or_one, buffer_length);
}
void Bitset::Expand(size_t expand_size) {
	bool* new_block = new bool[expand_size + buffer_length];
	if (new_block) {
		//This is a little bit redundant because all memory initialized with 0 at start
		//And when a memory block is deleted, all bytes are 0
		//But that doesn't hurt too much I think
		memset(new_block, 0, expand_size + buffer_length);

		memcpy(new_block, boolarray, buffer_length);
		buffer_length += expand_size;
		delete boolarray;
		boolarray = new_block;
	}
	else {
		std::cout << "Bitset expand has failed because Allocator has failed!\n";
		int x;
		std::cin >> x;
	}
}
