#ifndef __BYTEREADER_H__
#define __BYTEREADER_H__

#include <fstream>

namespace zenith
{
	class ByteReader
	{
	protected:
		virtual void readBytes(char *ptr, unsigned size) = NULL;

	public:
		template <typename T>
		void read(T *ptr, unsigned size = sizeof(T))
		{
			readBytes(reinterpret_cast<char*>(ptr), size);
		}

		virtual std::streampos position() const = NULL;
		virtual std::streampos max() const = NULL;
		virtual void skip(unsigned amount) = NULL;
		virtual void seek(unsigned long whereTo) = NULL;
		virtual bool eof() const = NULL;
	};

	class FileByteReader : public ByteReader
	{
	private:
		std::istream *file;
		std::streampos pos;
		std::streampos maxPos;

	public:
		FileByteReader(const std::string &filepath, std::streampos begin=0)
		{
			file = new std::ifstream(filepath, std::ifstream::in |
				std::ifstream::binary |
				std::ifstream::ate);

			maxPos = file->tellg();
			file->seekg(begin);
			pos = file->tellg();
		}

		~FileByteReader()
		{
			delete file;
		}

		std::streampos position() const
		{
			return pos;
		}

		std::streampos max() const
		{
			return maxPos;
		}

		void readBytes(char *ptr, unsigned size)
		{
			file->read(ptr, size);
			pos += size;
		}

		void skip(unsigned amount)
		{
			file->seekg(pos += amount);
		}

		void seek(unsigned long whereTo)
		{
			file->seekg(pos = whereTo);
		}

		bool eof() const
		{
			return file->eof();
		}
	};
}

#endif