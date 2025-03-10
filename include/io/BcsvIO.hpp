#pragma once

#include "../lib/bStream/bstream.h"
#include "DOM/DOMNodeSerializable.hpp"
#include "GenUtil.hpp"
#include <memory>
#include <vector>
#include <map>

constexpr size_t JMP_HEADER_SIZE = 16;
constexpr size_t JMP_FIELD_DEF_SIZE = 12;
constexpr uint32_t JMP_HASH_PRIME = 33554393;

class SBcsvIO;

class ISerializable
{
public:
	virtual ~ISerializable() = default;

	// Save this object to the given JmpIO instance at the given index.
	virtual void Serialize(SBcsvIO* JmpIO, uint32_t entry_index) const = 0;
	// Read the data from the given JmpIO instance at the given index into this object.
	virtual void Deserialize(SBcsvIO* JmpIO, uint32_t entry_index) = 0;
};

enum class EJmpFieldType : uint8_t
{
	Integer,
	String,
	Float,
	Integer2, //what. why?
	Short,
	Byte,
	StringOffset
};

// Definition of a field making up an entry in the JMP files.
struct SBcsvFieldInfo
{
	// The hash of the name of this field.
	uint32_t Hash;
	// The bitmask that isolates this field within a bitfield.
	uint32_t Bitmask;
	// The offset of this field within the JMP entry.
	uint16_t Start;
	// The shift required to compensate for the bitmask.
	uint8_t Shift;
	// The underlying data type of this field.
	EJmpFieldType Type;
};

typedef std::tuple<uint32_t, float, std::string> SBcsvValue; 

typedef std::map<uint32_t, SBcsvValue> SBcsvEntry;

// Handles reading and writing data from the map JMP files.
class SBcsvIO
{
	// Number of individual entries in this JMP file.
	int32_t mEntryCount { 0 };
	// Number of fields defined in this JMP file.
	int32_t mFieldCount { 0 };
	// Offset of the first entry in this JMP file.
	uint32_t mEntryStartOffset { 0 };
	// Size of an entry in this JMP file.
	uint32_t mEntrySize { 0 };

	// Size of a string. Can vary between 16 and 32. BCSV uses this for _inline_ strings!
	size_t mStringSize { 32 };

	size_t mStringTableSize { 0 };

	// A vector of the fields that define the data within the JMP entries.
	std::vector<SBcsvFieldInfo> mFields;

	std::vector<SBcsvEntry> mData;

	// Hashes the given field name so that the field can be found from the list of loaded instances.
	uint32_t HashFieldName(std::string name) const;

	// Returns a pointer to the field info corresponding to the given name if it exists within this JMP file,
	// or nullptr if it does not exist.
	const SBcsvFieldInfo* FetchJmpFieldInfo(std::string name);
	// Returns a pointer to the field info corresponding to the given hash if it exists within this JMP file,
	// or nullptr if it does not exist.
	const SBcsvFieldInfo* FetchJmpFieldInfo(uint32_t hash);


	// Recalculates the size of each entry by examining the fields defining the entry data.
	uint32_t CalculateNewEntrySize();

public:
	SBcsvIO();
	~SBcsvIO();

	int32_t GetEntryCount() const { return mEntryCount; }
	uint32_t GetEntrySize() const { return mEntrySize; }
	int32_t GetFieldCount() const { return mFieldCount; }

	size_t GetStringSize() const { return mStringSize; }
	void SetStringSize(uint32_t newStringSize) { mStringSize = newStringSize; }

	size_t CalculateNewFileSize(size_t entityCount) {
		return JMP_HEADER_SIZE + (mFieldCount * JMP_FIELD_DEF_SIZE) + (entityCount * mEntrySize);
	}

	void AddField(std::string name, EJmpFieldType type);
	void RemoveField(std::string name);

/*== Input ==*/
	// Attempts to load a JMP file from the given stream. Returns
	// true if the load succeeded, false if not.
	bool Load(bStream::CMemoryStream* stream);

	// Attempts to return the value of the given field from the given JMP entry
	// as an unsigned int; returns 0 if the field is invalid.
	uint32_t GetUnsignedInt(uint32_t entry_index, std::string field_name);

	// Attempts to return the value of the given field from the given JMP entry
	// as an signed int; returns 0 if the field is invalid.
	int32_t GetSignedInt(uint32_t entry_index, std::string field_name);

	// Attempts to return the value of the given field, using the hash to look up the field.
	// Returns 0 if invalid.
	int32_t GetSignedInt(uint32_t entry_index, uint32_t field_hash);

	uint16_t GetShort(uint32_t entry_index, std::string field_name);

	uint8_t GetChar(uint32_t entry_index, std::string field_name);

	// Attempts to return the value of the given field from the given JMP entry
	// as float; returns 0.0f if the field is invalid.
	float GetFloat(uint32_t entry_index, std::string field_name);

	// Attempts to return the value of the given field from the given JMP entry
	// as a boolean; returns false if the field is invalid.
	bool GetBoolean(uint32_t entry_index, std::string field_name);

	// Attempts to return the value of the given field from the given JMP entry
	// as a string; returns "(null)" if the field is invalid.
	std::string GetString(uint32_t entry_index, std::string field_name);

/*== Output ==*/
	// Saves the current JMP data to the given stream.
	bool Save(std::vector<std::shared_ptr<SDOMNodeSerializable>> entities, bStream::CMemoryStream& stream, std::function<void(SBcsvIO*, int, std::shared_ptr<SDOMNodeSerializable> node)> Serializer = {});

	// Writes an unsigned int to the given field in the specified JMP entry,
	// packing into a bitfield if required.
	bool SetUnsignedInt(uint32_t entry_index, std::string field_name, uint32_t value);

	// Writes a signed int to the given field in the specified JMP entry, searching by name.
	bool SetSignedInt(uint32_t entry_index, std::string field_name, int32_t value);

	// Writes a signed int to the given field in the specified JMP entry, searching by hash.
	bool SetSignedInt(uint32_t entry_index, uint32_t field_hash, int32_t value);

	bool SetShort(uint32_t entry_index, std::string field_name, uint16_t value);

	bool SetChar(uint32_t entry_index, std::string field_name, uint8_t value);

	// Writes a float to the given field in the specified JMP entry.
	bool SetFloat(uint32_t entry_index, std::string field_name, float value);

	// Writes a boolean to the given field in the specified JMP entry,
	// packing into a bitfield if required.
	bool SetBoolean(uint32_t entry_index, std::string field_name, bool value);

	// Writes a string to the given field in the specified JMP entry; pads the string
	// to 32 bytes in length.
	bool SetString(uint32_t entry_index, std::string field_name, std::string value);
};
