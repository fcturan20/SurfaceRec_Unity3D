#pragma once

class Bitset {
	bool* boolarray;
	unsigned int buffer_length = 0;
public:
	Bitset(const size_t byte_length);
	void SetBit_True(const size_t index);
	void SetBit_False(const size_t index);
	bool GetBit_Value(const size_t index) const;
	unsigned int GetByte_Length() const;
	unsigned int GetIndex_FirstFalse() const;
	unsigned int GetIndex_FirstTrue() const;
	void Clear(bool zero_or_one);
	void Expand(size_t expand_size);
};